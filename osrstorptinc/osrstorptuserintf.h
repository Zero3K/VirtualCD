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
//        $File: //depot/tools/osrvmMEMsample/OsrVmSampleInc/OsrVmUserIntf.h $
//
//    ABSTRACT:
//
//      This contains the Osr Virtual Miniport Driver inteface functions
//		that let the OSR Virtual miniport framework communicate with the
//		user layer that implements the scsi devices to export...
//
//    AUTHOR:
//
//        OSR Open Systems Resources, Inc.
// 
//    REVISION:   
//
//        $Revision: #2 $
//
///////////////////////////////////////////////////////////////////////////////
#ifndef __OSRVMUSERINTF_H__
#define __OSRVMUSERINTF_H__

extern "C" {
#include <scsiwmi.h>
};


extern UNICODE_STRING OsrRegistryPath;

extern "C" {

NTSTATUS OsrUserProcessIoCtl(IN PVOID PUserGlobalHandle,IN  PIRP Irp);
VOID     OsrUserGetScsiCapabilities(IN PVOID PUserGlobalHandle,
									PIO_SCSI_CAPABILITIES PCapabilities);
NTSTATUS OsrUserInitialize(PVOID OsrSpHandle,PDEVICE_OBJECT Pdo,
						   PVOID* UserGlobalInfoHandle,PULONG PNodeNumber);
NTSTATUS OsrUserAdapterStarted(IN PVOID PUserGlobalHandle);
VOID     OsrUserDeleteGlobalInformation(PVOID PUserGlobalInfo);
VOID     OsrUserShutdownNotification(PVOID PUserGlobalInfo);
NTSTATUS OsrUserRescanBus(IN PVOID PUserGlobalHandle);
NTSTATUS OsrUserResetBus(IN PVOID PUserGlobalHandle);
NTSTATUS OsrUserHandleSrb(IN PVOID UserLocalInfoHandle,PSCSI_REQUEST_BLOCK PSrb);
ULONG	 OsrUserGetSrbExtensionSize(VOID);
VOID     OsrUserDeleteLocalInformation(IN PVOID UserLocalInfoHandle);
VOID     OsrUserLocalShutdownNotification(PVOID UserLocalInfo);




PVOID   OsrSPCreateScsiDevice(IN PVOID POSRGHandle,IN ULONG BusIndex,
                              IN ULONG TargetIndex,IN ULONG LunIndex,
                              IN PVOID UserLocalHandle,
                              IN BOOLEAN BReadOnlyDevice,
                              PINQUIRYDATA PInquiryData,
                              ULONG ExtraStackLocations);

void    OsrSPAnnounceArrival(IN PVOID POSRGHandle);
void    OsrSPAnnounceDeparture(IN PVOID POSRGHandle);
BOOLEAN OsrSPSetDeviceRemovable(IN PVOID POSRLHandle,BOOLEAN BForce);
BOOLEAN OsrSPCanUserStart(IN PVOID POSRGHandle);
PDRIVER_OBJECT OsrSPGetDriverObject(IN PVOID POSRGHandle);
PDEVICE_OBJECT OsrSpGetDeviceObject(IN PVOID POSRLHandle);
PVOID	OsrSpGetSrbDataAddress(IN PVOID POSRLHandle,PSCSI_REQUEST_BLOCK PSrb);
PMDL	OsrSpGetSrbMdl(IN PVOID POSRLHandle,PSCSI_REQUEST_BLOCK PSrb);
void	OsrSPDecOutstandingIoCount(IN PVOID POSRLHandle);
void    OsrSpCompleteSrb(IN PVOID POSRLHandle,PSCSI_REQUEST_BLOCK PSrb);


PUCHAR	OsrSpPrintSCSICDBOperation(UCHAR Operation);
void	OsrSpPrintCdb10(PCDB PCdb);
VOID	OsrSpPrintCdb12(PCDB PCdb);
VOID	OsrSpPrintModeSense(UCHAR Type,PCDB PCdb);
PUCHAR	OsrSpPrintSRBStatus(USHORT Status);
PUCHAR	OsrSpPrintSCSStatus(USHORT Status);
VOID	OsrSpPrintScsiInquiryData(UCHAR Bus,UCHAR Target,UCHAR Lun,PINQUIRYDATA PInquiryData);

};

typedef struct _HW_SRB_EXTENSION_VM {
	SCSIWMI_REQUEST_CONTEXT WmiRequestContext;
} HW_SRB_EXTENSION_VM, *PHW_SRB_EXTENSION_VM;

typedef struct _HW_SRB_EXTENSION {
	//
	// Used to queue the SRB to a worker thread for execution.
	//
	HW_SRB_EXTENSION_VM		VMExtension;

	//
	// Start of User VM data;
	//
    UCHAR					UserData[1];
} HW_SRB_EXTENSION, *PHW_SRB_EXTENSION;

#endif  __OSRVMUSERINTF_H__