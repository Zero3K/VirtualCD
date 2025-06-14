///////////////////////////////////////////////////////////////////////////////
//
//	(C) Copyright 2009-2010 OSR Open Systems Resources, Inc.
//	All Rights Reserved
//
//  This work is distributed under the OSR Non-Commercial Software License which is provided
//  at "http://www.osronline.com/page.cfm?name=NonCommLicense" in the hope that it will be
//  enlightening, but WITHOUT ANY WARRANTY; without even the implied warranty of MECHANTABILITY
//
//	OSR Open Systems Resources, Inc.
//	105 Route 101A Suite 19
//	Amherst, NH 03031  (603) 595-6500 FAX: (603) 595-6503
//
//    MODULE:
//
//        $File: osrstorpt\osrstorpt.cpp $
//
//    ABSTRACT:
//
//      This contains the Osr Virtual Miniport Driver main routines.
//
//    AUTHOR:
//
//        OSR Open Systems Resources, Inc.
// 
//    REVISION:   
//
//        $Revision: #5 $
//
///////////////////////////////////////////////////////////////////////////////
#include "osrstorpt.h"

extern "C" { NTSTATUS DriverEntry(PDRIVER_OBJECT, PUNICODE_STRING); }

extern PDRIVER_OBJECT OsrDriverObject = NULL;
WCHAR  OsrRegistryPathBuffer[256];
extern UNICODE_STRING OsrRegistryPath = {0,0,OsrRegistryPathBuffer};
extern ULONG OsrTraceLevel = TRACE_LEVEL_INFORMATION;
extern ULONG OsrDbgFlags = OSRVMINIPT_DEBUG_SERVICE | OSRVMINIPT_DEBUG_IOCTL_INFO;

KSTART_ROUTINE DeleteDevicesThreadStart;

VIRTUAL_HW_INITIALIZATION_DATA OsrHwInitData = {0};

VOID OSRRegistryBreakOnEntry(PUNICODE_STRING RegistryPath);

#if (NTDDI_VERSION >= NTDDI_WIN2K)
extern "C" {
NTKERNELAPI
PDEVICE_OBJECT
IoGetDeviceAttachmentBaseRef(
    __in PDEVICE_OBJECT DeviceObject
    );
};
#endif



///////////////////////////////////////////////////////////////////////////////
//
//	DeleteDevicesThreadStart
//	
//		This thread deletes any devices that have been removed and reported
//		missing.
//
//	INPUTS:
//
//		Context - pointer to our non-paged storage area
//
//	OUTPUTS:
//
//		None.
//
//	RETURNS:
//
//		None.
//
//	IRQL REQUIREMENTS:
//
//		IRQL == PASSIVE_LEVEL
//
//	NOTES:
//
///////////////////////////////////////////////////////////////////////////////
void DeleteDevicesThreadStart(PVOID Context)
{
	POSR_DEVICE_EXTENSION	pDevExt = (POSR_DEVICE_EXTENSION) Context;
    NTSTATUS				status;
    KIRQL		lockHandle;
    LARGE_INTEGER			interval;
    ULONG                   eventCount = 2;
    PVOID                   eventList[2];
	POSR_VM_DEVICE			pDevice = NULL;

	OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,(__FUNCTION__": Entered\n"));

    interval.QuadPart = (ULONGLONG) -(5 * 1000000); // wait 6 seconds.

    //
    // Notify the creator that we have started.
    //
    KeSetEvent(&pDevExt->DeleteDevicesThreadStartEvent,IO_NO_INCREMENT,FALSE);

	eventList[0] = &pDevExt->DeleteDevicesThreadKillEvent;
	eventList[1] = &pDevExt->DeleteDevicesThreadWorkEvent;

    //
    // Continue running until such time as the package is being shut down.
    //

    while(TRUE) {
		//
		// Wait infinitely for one of the input objects to be signaled.
		//
		status = KeWaitForMultipleObjects(eventCount,eventList,WaitAny,Executive,KernelMode,
										FALSE,NULL,NULL);
      
		//
		// See which event was signaled.
		//                                    
		if(status == STATUS_WAIT_0) {
			//
			// We have been signaled to die, so get out of here.
			//
			OsrTracePrint(TRACE_LEVEL_INFORMATION,OSRVMINIPT_DEBUG_ADAPTER,
				(__FUNCTION__": Requested to Die\n"));
			break;
		}

		OsrAcquireSpinLock(&pDevExt->DeviceListLock,&lockHandle);

		for(PLIST_ENTRY pEntry = pDevExt->DeviceList.Flink;
			pEntry != &pDevExt->DeviceList; pEntry = pEntry->Flink) {

			pDevice = (POSR_VM_DEVICE) CONTAINING_RECORD(pEntry,OSR_VM_DEVICE,ListEntry);

			OSR_VM_DEVICE_VALID(pDevice);

			if(pDevice->ReportedMissing) {
				OsrTracePrint(TRACE_LEVEL_INFORMATION,OSRVMINIPT_DEBUG_PNP_INFO,
					(__FUNCTION__": Deleting Device Information %p for %d:%d:%d\n",
					pDevice,pDevice->PathId,pDevice->TargetId,pDevice->Lun));
				RemoveEntryList(&pDevice->ListEntry);
				OsrReleaseSpinLock(&pDevExt->DeviceListLock,lockHandle);
				OsrUserDeleteLocalInformation(pDevice->PUserLocalInformation);
				OsrAcquireSpinLock(&pDevExt->DeviceListLock,&lockHandle);
				ExFreePool(pDevice);
				pEntry = &pDevExt->DeviceList;
			}
		}

		OsrReleaseSpinLock(&pDevExt->DeviceListLock,lockHandle);

    }


    //
    // We're out of the loop and going to die, notify someone that we're
    // out of here.
    //
    KeSetEvent(&pDevExt->DeleteDevicesThreadDeadEvent,IO_NO_INCREMENT,FALSE);

	OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,(__FUNCTION__": Exit\n"));

	//
    // Terminate this thread.
    //
    (void) PsTerminateSystemThread(STATUS_SUCCESS);

    //
    // Can we ever even GET here?
    //
    return;
}


///////////////////////////////////////////////////////////////////////////////
//
//	OsrHwFindAdapter
//	
//		The Storport virtual miniport uses configuration information supplied
//		to this routine to further initialize itself.
//
//	INPUTS:
//
//		PDevExt - pointer to our non-paged storage area
//		PHwContext - A pointer to a context value. Not useful in a Storport 
//					virtual miniport driver
//		PBusInformation - A pointer to the miniport’s functional device object (FDO). 
//		PLowerDO - A pointer to the device object controlled by the miniport’s FDO
//		PArgumentString - A pointer to a null-terminated ASCII string. This string, 
//				if supplied, contains device information from the registry such 
//				as a base parameter.
//		PConfigInfo - A pointer to a PORT_CONFIGURATION_INFORMATION (Storport) 
//				structure. The port driver initializes this structure with any known 
//				configuration information, such as values that the miniport driver’s 
//				DriverEntry set in the VIRTUAL_HW_INITIALIZATION_DATA. 
//				VirtualHwStorFindAdapter must use any supplied information to 
//				determine if the virtual HBA it describes is one that the miniport 
//				driver supports. If so, VirtualHwStorFindAdapter initializes and 
//				configures that HBA and fills in any missing configuration information. 
//				If possible, a miniport driver should have default configuration values 
//				for each type of HBA that it supports, in the event that the operating 
//				system-dependent port driver cannot supply additional configuration 
//				information that was not provided by the miniport driver's 
//				DriverEntry routine. 
//		PBAgain - Not used
//
//	OUTPUTS:
//
//		None.
//
//	RETURNS:
//
//		SP_RETURN_FOUND 
//			Indicates that a supported HBA was found and that the HBA-relevant 
//			configuration information was determined successfully and set in the 
//			PORT_CONFIGURATION_INFORMATION structure. 
//		SP_RETURN_ERROR 
//			Indicates that an HBA was found, but an error occurred when it obtained
//			the configuration information. If possible, such an error should be 
//			logged with ScsiPortLogError. 
//		SP_RETURN_BAD_CONFIG 
//			Indicates that the supplied configuration information was invalid for the adapter. 
//		SP_RETURN_NOT_FOUND 
//			Indicates that no supported HBA was found for the supplied configuration information. 
//
//	IRQL REQUIREMENTS:
//
//		IRQL == PASSIVE_LEVEL
//
//	NOTES:
//
//		The VirtualDevice field in the configuration information stucture must be set to TRUE.
//		Other fields can be set as needed
//
///////////////////////////////////////////////////////////////////////////////
ULONG OsrHwFindAdapter(IN PVOID PDevExt,
					   IN PVOID PHwContext,        // Miniport's PDO
					   IN PVOID PBusInformation,   // Miniport's FDO.
					   IN PVOID PLowerDO,          // Device Object beneath FDO.
					   IN PCHAR PArgumentString,
					   IN OUT PPORT_CONFIGURATION_INFORMATION PConfigInfo,
					   IN PBOOLEAN PBAgain)
{
	POSR_DEVICE_EXTENSION	pDevExt = (POSR_DEVICE_EXTENSION) PDevExt;
	NTSTATUS				status;

	OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,(__FUNCTION__": Entered\n"));

	//
	// Set up our base configuration information.
	//
    PConfigInfo->AdapterInterfaceType           = PNPBus;
    PConfigInfo->VirtualDevice                  = TRUE;
    PConfigInfo->ScatterGather                  = TRUE;
    PConfigInfo->ResetTargetSupported           = TRUE;
    PConfigInfo->Master                         = TRUE;
    PConfigInfo->CachesData                     = FALSE;
    PConfigInfo->MaximumNumberOfTargets         = 8;
    PConfigInfo->NumberOfBuses                  = 1;
    PConfigInfo->Dma32BitAddresses              = FALSE;
	PConfigInfo->Dma64BitAddresses				= SCSI_DMA64_MINIPORT_SUPPORTED;
	PConfigInfo->MaximumNumberOfLogicalUnits	= SCSI_MAXIMUM_LOGICAL_UNITS;
    PConfigInfo->SynchronizationModel           = StorSynchronizeFullDuplex;
    PConfigInfo->MapBuffers                     = STOR_MAP_ALL_BUFFERS;

	//
	// Initialize our basic fields of the device extension.
	//
	pDevExt->MagicNumber = OSR_DEVICE_EXTENSION_MAGIC;
    InitializeListHead(&pDevExt->DeviceList);       
    OsrInitializeSpinLock(&pDevExt->DeviceListLock);     
	KeInitializeEvent(&pDevExt->DeleteDevicesThreadStartEvent,SynchronizationEvent,FALSE);
	KeInitializeEvent(&pDevExt->DeleteDevicesThreadKillEvent,SynchronizationEvent,FALSE);
	KeInitializeEvent(&pDevExt->DeleteDevicesThreadDeadEvent,SynchronizationEvent,FALSE);
	KeInitializeEvent(&pDevExt->DeleteDevicesThreadWorkEvent,SynchronizationEvent,FALSE);

    status = PsCreateSystemThread(&pDevExt->DeleteDevicesThreadHandle, 
                     (ACCESS_MASK)0L, 0, 0, 0, DeleteDevicesThreadStart, 
                     pDevExt);

    if(NT_SUCCESS(status)) {

        KeWaitForSingleObject(&pDevExt->DeleteDevicesThreadStartEvent,
                              Executive,KernelMode,FALSE,NULL);

    } else {
		OsrTracePrint(TRACE_LEVEL_ERROR,OSRVMINIPT_DEBUG_ADAPTER,
			(__FUNCTION__": Error creating System Thread %x.\n",status));
		pDevExt->KillThread = TRUE;
		return SP_RETURN_NOT_FOUND;
    }

	//
	// Register our own device interface so that our user mode application
	// can connect to us.
	//
#ifndef IOREGISTER_VERIFIER_WORKAROUND
	status = IoRegisterDeviceInterface((PDEVICE_OBJECT) PHwContext,
					&GUID_OSR_VIRTUALMINIPORT,NULL,&pDevExt->DeviceInterface);
#else // IOREGISTER_VERIFIER_WORKAROUND
	ULONG	verifierFlags;
    if(NT_SUCCESS(MmIsVerifierEnabled(&verifierFlags))) {
		//
		// If Driver Verifier is enabled on this driver then we cannot register a device
		// interface on the PLowerDO because it is verifier's device object below us, 
		// therefore we call IoGetDeviceAttachmentBaseRef to get the lowest device object
		// in the change which is the DO we wanted in the first place.....
		//
		PDEVICE_OBJECT lowestPdo = IoGetDeviceAttachmentBaseRef((PDEVICE_OBJECT) PLowerDO);
		if(lowestPdo) {
			status = IoRegisterDeviceInterface((PDEVICE_OBJECT) lowestPdo,
							&GUID_OSR_VIRTUALMINIPORT,NULL,&pDevExt->DeviceInterface);
			ObDereferenceObject(lowestPdo);
		}
	} else {
		status = IoRegisterDeviceInterface((PDEVICE_OBJECT) PLowerDO,
						&GUID_OSR_VIRTUALMINIPORT,NULL,&pDevExt->DeviceInterface);
	}
#endif //IOREGISTER_VERIFIER_WORKAROUND

	if(!NT_SUCCESS(status)) {
		OsrTracePrint(TRACE_LEVEL_ERROR,OSRVMINIPT_DEBUG_ADAPTER,
			(__FUNCTION__": Error calling IoRegisterDeviceInterface %x.\n",status));
		goto adapterNotFound;
	}

	//
	// Our structures are not initialized.   Call the VM user specific code to
	// do its job.
	//
	__try {
		status = OsrUserInitialize(PDevExt,(PDEVICE_OBJECT) PLowerDO,
					&pDevExt->PUserGlobalInformation,
					&pDevExt->NodeNumber);

		if(NT_SUCCESS(status)) {
			//
			// Okay the user level is initialized.   Ask the user code to get the
			// scsi capabilities for the device.
			//
			OsrUserGetScsiCapabilities(pDevExt->PUserGlobalInformation,
				&pDevExt->Capabilities);
		} else {
			RtlFreeUnicodeString(&pDevExt->DeviceInterface);
			goto adapterNotFound;
		}

		PConfigInfo->MaximumTransferLength = pDevExt->Capabilities.MaximumTransferLength;
		PConfigInfo->AlignmentMask = pDevExt->Capabilities.AlignmentMask;
		PConfigInfo->AdapterScansDown = pDevExt->Capabilities.AdapterScansDown;
		

	} __except(EXCEPTION_EXECUTE_HANDLER) {
		RtlFreeUnicodeString(&pDevExt->DeviceInterface);
		status = GetExceptionCode();
		goto adapterNotFound;
	}

    OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,("OsrHwFindAdapter Exit\n"));
    return SP_RETURN_FOUND;

adapterNotFound:
    pDevExt->KillThread = TRUE;
    KeSetEvent(&pDevExt->DeleteDevicesThreadKillEvent,IO_NO_INCREMENT,FALSE);
    return SP_RETURN_NOT_FOUND;
}


///////////////////////////////////////////////////////////////////////////////
//
//	OsrHwInitialize
//	
//		The HwStorInitialize routine initializes the miniport driver after a 
//		system reboot or power failure occurs. It is called by StorPort after 
//		HwStorFindAdapter successfully returns. HwStorInitialize initializes 
//		the HBA and finds all devices that are of interest to the miniport 
//		driver.
//
//	INPUTS:
//
//		PDevExt - pointer to our non-paged storage area
//
//	OUTPUTS:
//
//		None.
//
//	RETURNS:
//
//		TRUE. 
//
//	IRQL REQUIREMENTS:
//
//		IRQL == PASSIVE_LEVEL
//
//	NOTES:
//
//		In a miniport for a real adapter, HwStorInitialize is called at DIRQL.  
//		This routine, however is called at PASSIVE_LEVEL because there is no 
//		Interrupt object
//
///////////////////////////////////////////////////////////////////////////////
BOOLEAN OsrHwInitialize(IN PVOID PDevExt)
{
	POSR_DEVICE_EXTENSION	pDevExt = (POSR_DEVICE_EXTENSION) PDevExt;
	NTSTATUS				status;
	BOOLEAN					ok = TRUE;

    OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,("OsrHwInitialize Entered\n"));

    __try {

        status = OsrUserAdapterStarted(pDevExt->PUserGlobalInformation);

    } __except(EXCEPTION_EXECUTE_HANDLER) {

        status = GetExceptionCode();

    }

	if(!NT_SUCCESS(status)) {
		ok = FALSE;
	}

	//
	// Enable our own device interface so that user mode applications can
	// communicate with us.
	//
	if(ok) {
		status = IoSetDeviceInterfaceState(&pDevExt->DeviceInterface,TRUE);
		if(!NT_SUCCESS(status)) {
			OsrTracePrint(TRACE_LEVEL_ERROR,OSRVMINIPT_DEBUG_ADAPTER,
				(__FUNCTION__": Error calling IoSetDeviceInterfaceState %x.\n",status));
		}
	}

    OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,("OsrHwInitialize Exit\n"));

    return ok;
}


///////////////////////////////////////////////////////////////////////////////
//
//	OsrHwResetBus
//	
//		The HwStorResetBus routine is called by the port driver to clear 
//		error conditions
//
//	INPUTS:
//
//		PDevExt - pointer to our non-paged storage area
//		BusId - identifies the SCSI bus to be reset
//
//	OUTPUTS:
//
//		None.
//
//	RETURNS:
//
//		TRUE. 
//
//	IRQL REQUIREMENTS:
//
//		IRQL == DISPATCH_LEVEL
//
//	NOTES:
//
//		In a miniport for a real adapter, HwStorInitialize is called at DIRQL.  
//		This routine, however is called at PASSIVE_LEVEL because there is no 
//		Interrupt object
//
///////////////////////////////////////////////////////////////////////////////
BOOLEAN OsrHwResetBus(IN PVOID PDevExt,IN ULONG BusId)
{
    OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,
				("OsrHwResetBus Entered\n"));

    StorPortCompleteRequest(PDevExt,                  // Complete all outstanding requests.
                            (UCHAR)BusId,
                            SP_UNTAGGED,
                            SP_UNTAGGED,
                            (UCHAR) SRB_STATUS_BUS_RESET);

    OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,
				("OsrHwResetBus Exit\n"));

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//
//	OsrHwAdapterControl
//	
//		A miniport driver's HwStorAdapterControl routine is called to perform 
//		synchronous operations to control the state or behavior of an adapter, 
//		such as stopping or restarting the HBA for power management
//
//	INPUTS:
//
//		PDevExt - pointer to our non-paged storage area
//		ControlType - adapter-control operation
//		Parameters - parameters related to control type
//
//	OUTPUTS:
//
//		None.
//
//	RETURNS:
//
//		TRUE. 
//
//	IRQL REQUIREMENTS:
//
//		IRQL == DISPATCH_LEVEL
//
//	NOTES:
//
//		In a miniport for a real adapter, OsrHwAdapterControl is called at DIRQL.  
//		This routine, however is called at PASSIVE_LEVEL because there is no 
//		Interrupt object
//
///////////////////////////////////////////////////////////////////////////////
SCSI_ADAPTER_CONTROL_STATUS OsrHwAdapterControl(IN PVOID PDevExt,
												IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
												IN PVOID Parameters)
{
	POSR_DEVICE_EXTENSION pDevExt = (POSR_DEVICE_EXTENSION) PDevExt;
    PSCSI_SUPPORTED_CONTROL_TYPE_LIST pCtlTypList;
    ULONG                             i;

    OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,
				("OsrHwAdapterControl Entered\n"));

    pDevExt->AdapterState = ControlType;

    switch (ControlType) {
		//
        // determine which control types (routines) are supported
		//
        case ScsiQuerySupportedControlTypes:
			OsrTracePrint(TRACE_LEVEL_INFORMATION,OSRVMINIPT_DEBUG_ADAPTER,
						("OsrHwAdapterControl ScsiQuerySupportedControlTypes\n"));
			//
            //	Get pointer to control type list
			//
            pCtlTypList = (PSCSI_SUPPORTED_CONTROL_TYPE_LIST)Parameters;

			//
            // Cycle through list to set TRUE for each type supported
            // making sure not to go past the MaxControlType
			//
            for (i = 0; i < pCtlTypList->MaxControlType; i++)
                if ( i == ScsiQuerySupportedControlTypes ||
                     i == ScsiStopAdapter   || i == ScsiRestartAdapter ||
					 i == ScsiSetBootConfig || i == ScsiSetRunningConfig ) {
                    pCtlTypList->SupportedTypeList[i] = TRUE;
                }
            break;

        case ScsiStopAdapter:
			OsrTracePrint(TRACE_LEVEL_INFORMATION,OSRVMINIPT_DEBUG_ADAPTER,
						("OsrHwAdapterControl ScsiStopAdapter\n"));
            break;

        case ScsiRestartAdapter:
			OsrTracePrint(TRACE_LEVEL_INFORMATION,OSRVMINIPT_DEBUG_ADAPTER,
						("OsrHwAdapterControl ScsiRestartAdapter\n"));
            break;

        case ScsiSetBootConfig:
			OsrTracePrint(TRACE_LEVEL_INFORMATION,OSRVMINIPT_DEBUG_ADAPTER,
						("OsrHwAdapterControl ScsiSetBootConfig\n"));
            break;
            
        case ScsiSetRunningConfig:
			OsrTracePrint(TRACE_LEVEL_INFORMATION,OSRVMINIPT_DEBUG_ADAPTER,
						("OsrHwAdapterControl ScsiSetRunningConfig\n"));
            break;

        default:
            break;

    } // switch

    OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,
				("OsrHwAdapterControl Exit\n"));

    return ScsiAdapterControlSuccess;
}


///////////////////////////////////////////////////////////////////////////////
//
//	OsrHwStartIo
//	
//		A miniport driver's HwStorAdapterControl routine is called to perform 
//		synchronous operations to control the state or behavior of an adapter, 
//		such as stopping or restarting the HBA for power management
//
//	INPUTS:
//
//		PDevExt - pointer to our non-paged storage area
//		ControlType - adapter-control operation
//		Parameters - parameters related to control type
//
//	OUTPUTS:
//
//		None.
//
//	RETURNS:
//
//		TRUE. 
//
//	IRQL REQUIREMENTS:
//
//		IRQL == DISPATCH_LEVEL
//
//	NOTES:
//
//		In a miniport for a real adapter, HwStorInitialize is called at DIRQL.  
//		This routine, however is called at PASSIVE_LEVEL because there is no 
//		Interrupt object
//
///////////////////////////////////////////////////////////////////////////////
BOOLEAN OsrHwStartIo(IN PVOID PDevExt,
					 IN PSCSI_REQUEST_BLOCK  PSrb)
{
	POSR_DEVICE_EXTENSION pDevExt = (POSR_DEVICE_EXTENSION) PDevExt;
    UCHAR    srbStatus = SRB_STATUS_INVALID_REQUEST;
    BOOLEAN  bFlag;
    NTSTATUS status;
    BOOLEAN  bSrbCompleted = TRUE;

    OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,
				("OsrHwStartIo Entered\n"));


    OsrTracePrint(TRACE_LEVEL_INFORMATION,OSRVMINIPT_DEBUG_SRB,
				("OsrHwStartIo at IRQL level %d \n", KeGetCurrentIrql()));

    switch (PSrb->Function) {

        case SRB_FUNCTION_EXECUTE_SCSI:
			OsrTracePrint(TRACE_LEVEL_INFORMATION,OSRVMINIPT_DEBUG_SRB,
						("OsrHwStartIo SRB_FUNCTION_EXECUTE_SCSI\n"));
			srbStatus = OsrVmExecuteScsi(pDevExt, PSrb, &bSrbCompleted);
			break;

        case SRB_FUNCTION_IO_CONTROL:
			OsrTracePrint(TRACE_LEVEL_INFORMATION,OSRVMINIPT_DEBUG_SRB,
						("OsrHwStartIo SRB_FUNCTION_IO_CONTROL\n"));
			srbStatus = OsrVmIoControl(pDevExt, PSrb);
            break;

        case SRB_FUNCTION_WMI:
			srbStatus = SRB_STATUS_INVALID_REQUEST;
            break;
            
        case SRB_FUNCTION_RESET_LOGICAL_UNIT:
			OsrTracePrint(TRACE_LEVEL_INFORMATION,OSRVMINIPT_DEBUG_SRB,
						("OsrHwStartIo SRB_FUNCTION_RESET_LOGICAL_UNIT\n"));
            StorPortCompleteRequest(PDevExt,
                                    PSrb->PathId,
                                    PSrb->TargetId,
                                    PSrb->Lun,
                                    SRB_STATUS_BUSY);
            srbStatus = SRB_STATUS_SUCCESS;
            break;
            
        case SRB_FUNCTION_RESET_DEVICE:
			OsrTracePrint(TRACE_LEVEL_INFORMATION,OSRVMINIPT_DEBUG_SRB,
						("OsrHwStartIo SRB_FUNCTION_RESET_DEVICE\n"));
            StorPortCompleteRequest(PDevExt,
                                    PSrb->PathId,
                                    PSrb->TargetId,
                                    SP_UNTAGGED,
                                    SRB_STATUS_TIMEOUT);
            srbStatus = SRB_STATUS_SUCCESS;
            break;
            
        case SRB_FUNCTION_PNP:                       
			OsrTracePrint(TRACE_LEVEL_INFORMATION,OSRVMINIPT_DEBUG_SRB,
						("OsrHwStartIo SRB_FUNCTION_PNP\n"));
            status = OsrVmHwHandlePnP(pDevExt,(PSCSI_PNP_REQUEST_BLOCK) PSrb);
            srbStatus = PSrb->SrbStatus;            
            break;
            
        default:
			OsrTracePrint(TRACE_LEVEL_ERROR,OSRVMINIPT_DEBUG_SRB,
						("OsrHwStartIo Unknown Srb Function = 0x%x\n", PSrb->Function));
            srbStatus = SRB_STATUS_INVALID_REQUEST;
            break;

    } // switch (pSrb->Function)

	//
	// If the SRB completed, notify StorPort
	//
    if (bSrbCompleted) {                         

		OsrTracePrint(TRACE_LEVEL_INFORMATION,OSRVMINIPT_DEBUG_SRB,
			("OsrHwStartIo Notifying StorPort of completion of %p status: 0x%x(%s)\n",
			PSrb,srbStatus,OsrSpPrintSRBStatus(srbStatus)));

		PSrb->SrbStatus = srbStatus;

		//
		// Note:  A miniport with real hardware would not always be calling RequestComplete
		//		  from HwStorStartIo.  Rather, the miniport would typically be doing real 
		//		  I/O and would call RequestComplete only at the end of that
		//        real I/O, in its HwStorInterrupt or in a DPC routine.
		//
		StorPortNotification(RequestComplete, PDevExt, PSrb);
    }
     
    OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,
				("OsrHwStartIo Exit\n"));

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//
//	OsrHwProcessServiceRequest
//	
//		The HwStorProcessServiceRequest callback routine receives an IRP that 
//		is produced by an IOCTL when a caller, such as a user-mode application 
//		or kernel-mode driver, requires a “reverse callback” operation. The I/O 
//		is completed by the miniport driver when it needs to tell the caller of 
//		something or needs the caller to do something	such as stopping or 
//		restarting the HBA for power management
//
//	INPUTS:
//
//		PDevExt - pointer to our non-paged storage area
//		PIrp - Irp Request
//
//	OUTPUTS:
//
//		None.
//
//	RETURNS:
//
//		TRUE. 
//
//	IRQL REQUIREMENTS:
//
//		IRQL == PASSIVE_LEVEL
//
//	NOTES:
//
///////////////////////////////////////////////////////////////////////////////
VOID OsrHwProcessServiceRequest(IN PVOID PDevExt,
								IN PVOID PIrp)
{
	PIRP					pIrp = (PIRP) PIrp;
	PIO_STACK_LOCATION		irpSp = IoGetCurrentIrpStackLocation(pIrp);
	NTSTATUS				status = STATUS_INVALID_DEVICE_REQUEST;
	POSR_DEVICE_EXTENSION	pDevExt = (POSR_DEVICE_EXTENSION) PDevExt;

    OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,
				("OsrHwProcessServiceRequest Enter\n"));

	if(irpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL) {
		__try {
			status = OsrUserProcessIoCtl(pDevExt->PUserGlobalInformation,pIrp);
		} __except(EXCEPTION_EXECUTE_HANDLER) {
			status = GetExceptionCode();
		}
	}

	if(status != STATUS_PENDING) {
		pIrp->IoStatus.Status = status;
		IoCompleteRequest(pIrp,IO_NO_INCREMENT);
	}

    OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,
				("OsrHwProcessServiceRequest Exit\n"));
}


///////////////////////////////////////////////////////////////////////////////
//
//	OsrHwCompleteServiceRequest
//	
//		The HwStorCompleteServiceIrp routine is called when the virtual 
//		adapter is being removed. When this happens, the Storport virtual 
//		miniport can complete any reverse-callback IRPs received in 
//		HwStorProcessServiceRequest.
//
//	INPUTS:
//
//		PDevExt - pointer to our non-paged storage area
//
//	OUTPUTS:
//
//		None.
//
//	RETURNS:
//
//		TRUE. 
//
//	IRQL REQUIREMENTS:
//
//		IRQL == PASSIVE_LEVEL
//
//	NOTES:
//
///////////////////////////////////////////////////////////////////////////////
VOID OsrHwCompleteServiceRequest(IN PVOID PDevExt)
{
    OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,
				("OsrHwCompleteServiceRequest Enter\n"));


    OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,
				("OsrHwCompleteServiceRequest Exit\n"));
}


///////////////////////////////////////////////////////////////////////////////
//
//	OsrHwFreeAdapterResources
//	
//		The HwStorFreeAdapterResources callback routine allows the Storport 
//		virtual miniport driver to free resources when the virtual adapter is 
//		being removed. This is the last callback routine for the adapter.
//
//	INPUTS:
//
//		PDevExt - pointer to our non-paged storage area
//
//	OUTPUTS:
//
//		None.
//
//	RETURNS:
//
//		TRUE. 
//
//	IRQL REQUIREMENTS:
//
//		IRQL == PASSIVE_LEVEL
//
//	NOTES:
//
///////////////////////////////////////////////////////////////////////////////
VOID OsrHwFreeAdapterResources(IN PVOID PDevExt)
{
	POSR_DEVICE_EXTENSION	pDevExt = (POSR_DEVICE_EXTENSION) PDevExt;

    OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,
				("OsrHwFreeAdapterResources Enter\n"));

	if(pDevExt->PUserGlobalInformation) {
        __try {
            OsrUserDeleteGlobalInformation(pDevExt->PUserGlobalInformation);
			OsrTracePrint(TRACE_LEVEL_INFORMATION,OSRVMINIPT_DEBUG_PNP_INFO,
				("OsrHwFreeAdapterResources: OsrUserDeleteGlobalInformation"
				" completed\n"));
        } __except(EXCEPTION_EXECUTE_HANDLER) {
			NTSTATUS status = GetExceptionCode();
			OsrTracePrint(TRACE_LEVEL_ERROR,OSRVMINIPT_DEBUG_PNP_INFO,
				("OsrHwFreeAdapterResources: OsrUserDeleteGlobalInformation"
				" exception 0x%x\n",status));
        }
        pDevExt->PUserGlobalInformation = NULL;
	}

    pDevExt->KillThread = TRUE;
    KeSetEvent(&pDevExt->DeleteDevicesThreadKillEvent,IO_NO_INCREMENT,FALSE);
    KeWaitForSingleObject(&pDevExt->DeleteDevicesThreadDeadEvent,Executive,KernelMode,FALSE,NULL);

    OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,
				("OsrHwFreeAdapterResources Exit\n"));
}



///////////////////////////////////////////////////////////////////////////////
//
//	DriverEntry
//	
//		This is the main entry point of the driver itself; driver 
//		initialization occurs here
//
//	INPUTS:
//
//		DriverObject - the object representing this driver
//		RegistryPath - the path to this driver's registry key
//
//	OUTPUTS:
//
//		None.
//
//	RETURNS:
//		STATUS_SUCCESS - driver started successfully
//		STATUS_UNSUCCESSFUL - driver did not start
//
//	IRQL REQUIREMENTS:
//
//		IRQL == PASSIVE_LEVEL
//
//	NOTES:
//
//   None.
//
///////////////////////////////////////////////////////////////////////////////
#pragma prefast(suppress:28101,"Yes, this is DriverEntry, glad you noticed")
NTSTATUS __stdcall DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	NTSTATUS	status = STATUS_SUCCESS;


    OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,("DriverEntry Entered\n"));

    //
    // See if we breakpoint on entry.
    //
    OSRRegistryBreakOnEntry(RegistryPath);

    //
    //	Save away the valuable global data
    //
    OsrDriverObject = DriverObject;

    if (RegistryPath->MaximumLength > 0) {

		if(RegistryPath->MaximumLength > sizeof(OsrRegistryPathBuffer)) {
			return STATUS_INSUFFICIENT_RESOURCES;
		}

        OsrRegistryPath.MaximumLength = RegistryPath->MaximumLength;

        RtlCopyUnicodeString(&OsrRegistryPath, RegistryPath);

    }


    //
    // Set up information for StorPortInitialize.
    //
    RtlZeroMemory(&OsrHwInitData, sizeof(VIRTUAL_HW_INITIALIZATION_DATA));
    OsrHwInitData.HwInitializationDataSize = sizeof(VIRTUAL_HW_INITIALIZATION_DATA);

	//
    // Set up our entry points (callbacks).
	//
    OsrHwInitData.HwInitialize             = OsrHwInitialize;       // Required.
    OsrHwInitData.HwStartIo                = OsrHwStartIo;          // Required.
    OsrHwInitData.HwFindAdapter            = OsrHwFindAdapter;      // Required.
    OsrHwInitData.HwResetBus               = OsrHwResetBus;         // Required.
    OsrHwInitData.HwAdapterControl         = OsrHwAdapterControl;   // Required.
    OsrHwInitData.HwFreeAdapterResources   = OsrHwFreeAdapterResources;
    OsrHwInitData.HwProcessServiceRequest  = OsrHwProcessServiceRequest;

    OsrHwInitData.AdapterInterfaceType     = Internal;

	OsrHwInitData.MultipleRequestPerLu	   = TRUE;
	OsrHwInitData.PortVersionFlags		   = 0;

    OsrHwInitData.DeviceExtensionSize      = sizeof(OSR_DEVICE_EXTENSION);
    OsrHwInitData.SpecificLuExtensionSize  = sizeof(OSR_LU_EXTENSION);
    OsrHwInitData.SrbExtensionSize         = sizeof(HW_SRB_EXTENSION) + OsrUserGetSrbExtensionSize();

    status =  StorPortInitialize(DriverObject,
                                 RegistryPath,
                                 (PHW_INITIALIZATION_DATA)&OsrHwInitData, 
                                 NULL);

    if (STATUS_SUCCESS!=status) {                     // Port driver said not OK?                                        
        OsrTracePrint(TRACE_LEVEL_ERROR,OSRVMINIPT_DEBUG_FUNCTRACE,
			("DriverEntry failure in call to StorPortInitialize. status:0x%x\n",status));
		ASSERT(FALSE);
		return status;
    }                                     

    OsrTracePrint(TRACE_LEVEL_VERBOSE,OSRVMINIPT_DEBUG_FUNCTRACE,("DriverEntry Exit\n"));

    //
    // Return results to the I/O Manager
    //
    return status;

}


///////////////////////////////////////////////////////////////////////////////
//
//	OSRRegistryReadValue
//	
//		read a DWORD from the registry, using an absolute
//      path.  Never a default with this call
//
//	INPUTS:
//
//		RegistryPath - path
//		Key - key
//		Type - registry value type
//
//	OUTPUTS:
//
//		Value - address of location to receive value.
//
//	RETURNS:
//		TRUE if read, false otherwise
//
//	IRQL REQUIREMENTS:
//
//		IRQL == PASSIVE_LEVEL
//
//	NOTES:
//
//   None.
//
///////////////////////////////////////////////////////////////////////////////
BOOLEAN OSRRegistryReadValue(PUNICODE_STRING RegistryPath, PWSTR Key, ULONG Type, PVOID Value)
{
    NTSTATUS code;
    RTL_QUERY_REGISTRY_TABLE paramTable[2];
    PWSTR path;

    //
    // Allocate memory for the expanded string, include enough space for trailing NULL!
    //
    path = (PWSTR) ExAllocatePoolWithTag(PagedPool,RegistryPath->Length+sizeof(WCHAR),'emaN');
    if (!path) {
        OsrTracePrint(TRACE_LEVEL_ERROR,OSRVMINIPT_DEBUG_DRIVER_ENTRY,
			("Insufficient memory to read registry\n"));
        return(FALSE);
    }

    RtlZeroMemory(paramTable,sizeof(paramTable));
    RtlZeroMemory(path,RegistryPath->Length+sizeof(WCHAR));
    RtlMoveMemory(path,RegistryPath->Buffer,RegistryPath->Length);

    paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name = Key;
    paramTable[0].EntryContext = Value;
    paramTable[0].DefaultType = Type;

    code = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE|RTL_REGISTRY_OPTIONAL, 
					path, &paramTable[0], 0, 0);

    ExFreePool(path);

    return(NT_SUCCESS(code) ? TRUE : FALSE);
}


///////////////////////////////////////////////////////////////////////////////
//
//	OSRRegistryReadValue
//	
//		read the registry for the BreakOnEntry keyword for THIS driver to determine
//                            if it should break.  Of course, the intention is that 
//							  OSRRegistryBreakOnEntry
//                            is called at the top of DriverEntry.
//
//	INPUTS:
//
//		RegistryPath - path
//
//	OUTPUTS:
//
//		None.
//
//	RETURNS:
//		None.
//
//	IRQL REQUIREMENTS:
//
//		IRQL == PASSIVE_LEVEL
//
//	NOTES:
//
//   None.
//
///////////////////////////////////////////////////////////////////////////////
VOID OSRRegistryBreakOnEntry(PUNICODE_STRING RegistryPath)
{
    ULONG shouldBreak = 0;

    if(OSRRegistryReadValue(RegistryPath, L"BreakOnEntry", REG_DWORD, 
		&shouldBreak) && shouldBreak) {
        __debugbreak();
    }
    
    return;
}



///////////////////////////////////////////////////////////////////////////////
//
//	OsrInitializeSpinLock
//	
//		Initialize a spinlock.
//
//	INPUTS:
//
//		PSpinLock - address of lock
//
//	OUTPUTS:
//
//		None.
//
//	RETURNS:
//		None.
//
//	IRQL REQUIREMENTS:
//
//		IRQL == PASSIVE_LEVEL
//
//	NOTES:
//
//   This was done here to get around a duplicate symbol error while linking
//	the code.   We don't like using /FORCE to get around the issue.
//
///////////////////////////////////////////////////////////////////////////////
VOID OsrInitializeSpinLock(PKSPIN_LOCK PSpinLock)
{
	KeInitializeSpinLock(PSpinLock);
}


///////////////////////////////////////////////////////////////////////////////
//
//	OsrAcquireSpinLock
//	
//		Acquire a spinlock.
//
//	INPUTS:
//
//		PSpinLock - address of lock
//		PIrql - address of KIRQL
//		
//
//	OUTPUTS:
//
//		None.
//
//	RETURNS:
//		None.
//
//	IRQL REQUIREMENTS:
//
//		IRQL <= DISPATCH_LEVEL
//
//	NOTES:
//
//   This was done here to get around a duplicate symbol error while linking
//	the code.   We don't like using /FORCE to get around the issue.
//
///////////////////////////////////////////////////////////////////////////////
#pragma prefast(suppress:28167,"Yes, we acquire the lock here.")
VOID OsrAcquireSpinLock(PKSPIN_LOCK PSpinLock,KIRQL* PIrql) 
{
	KeAcquireSpinLock(PSpinLock,PIrql);
}


///////////////////////////////////////////////////////////////////////////////
//
//	OsrReleaseSpinLock
//	
//		Release a spinlock.
//
//	INPUTS:
//
//		PSpinLock - address of lock
//		Irql - Irql
//		
//
//	OUTPUTS:
//
//		None.
//
//	RETURNS:
//		None.
//
//	IRQL REQUIREMENTS:
//
//		IRQL == DISPATCH_LEVEL
//
//	NOTES:
//
//   This was done here to get around a duplicate symbol error while linking
//	the code.   We don't like using /FORCE to get around the issue.
//
///////////////////////////////////////////////////////////////////////////////
#pragma prefast(suppress:28167,"Yes, we release the lock here.")
VOID OsrReleaseSpinLock(PKSPIN_LOCK PSpinLock,KIRQL Irql) 
{
	KeReleaseSpinLock(PSpinLock,Irql);
}
