///////////////////////////////////////////////////////////////////////////////
//
//	(C) Copyright 2009-2010 OSR Open Systems Resources, Inc.
//	All Rights Reserved
//
//  This work is distributed under the OSR Non-Commercial Software License which is provided
//  at "http://www.osronline.com/page.cfm?name=NonCommLicense" in the hope that it will be
//  enlightening, but WITHOUT ANY WARRANTY; without even the implied warranty of MECHANTABILITY
//
//
//	OSR Open Systems Resources, Inc.
//	105 Route 101A Suite 19
//	Amherst, NH 03031  (603) 595-6500 FAX: (603) 595-6503
//
//	MODULE:
//
//		$File: osrstorptmgmt\osrspintf.cpp $
//
//	ABSTRACT:
//
//      OSR SCSI Port Interface Code.
//
//	AUTHOR:
//
//		Open Systems Resources, Inc.
// 
//	REVISION:   
//
//		$Revision: #2 $
//
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <winioctl.h>
#include <osrstorptcfg.h>
#include <osrstorptuserioctl.h>
#include <osrspintf.h>
extern "C" {
#include <ntddscsi.h>
#include <setupapi.h>
};

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IOCTL_MINIPORT_PROCESS_SERVICE_IRP CTL_CODE(IOCTL_SCSI_BASE,  0x040e, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)


HANDLE
ConnectToScsiPort(
    VOID
    ) {

    HDEVINFO                         devInfo;
    SP_DEVICE_INTERFACE_DATA         devInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA devInterfaceDetailData = NULL;
    ULONG                            devIndex;
    ULONG                            requiredSize;
    ULONG                            code;
    HANDLE                           osrSPDriverHandle;
	DWORD							 bytesReturned;
	COMMAND_IN						 command;
	BOOL							 devStatus;

    //
    // Open a handle to the device using the 
    //  device interface that the driver registers
    //
    //

    //
    // Get the device information set for all of the
    //  devices of our class (the GUID we defined
    //  in nothingioctl.h and registered in the driver
    //  with DfwDeviceCreateDeviceInterface) that are present in the 
    //  system
    //
    devInfo = SetupDiGetClassDevs(&GUID_OSR_VIRTUALMINIPORT /*GUID_DEVINTERFACE_STORAGEPORT*/,
                                  NULL,
                                  NULL,
                                  DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (devInfo == INVALID_HANDLE_VALUE) {

        printf("SetupDiGetClassDevs failed with error 0x%x\n", GetLastError());

        return INVALID_HANDLE_VALUE;

    }

    //
    // Now get information about each device installed...
    //
    
    //
    // This needs to be set before calling
    //  SetupDiEnumDeviceInterfaces
    //
    devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    //
    // Start with the first device...
    //
    devIndex = 0;

    while (SetupDiEnumDeviceInterfaces(devInfo,
                                       NULL,
                                       &GUID_OSR_VIRTUALMINIPORT/*GUID_DEVINTERFACE_STORAGEPORT*/,
                                       devIndex++, 
                                       &devInterfaceData)) {

        //
        // If you actually had a reason to keep
        //  track of all the devices in the system
        //  you obviously wouldn't want to just
        //  throw these away. Since we're just 
        //  running through to print them out
        //  and picking whatever the last one
        //  is we'll alloc and free these
        //  as we go...
        //
        if (devInterfaceDetailData != NULL) {

            free(devInterfaceDetailData);

            devInterfaceDetailData = NULL;

        }

        //
        // The entire point of this exercise is
        //  to get a string that we can hand to 
        //  CreateFile to get a handle to the device,
        //  so we need to call SetupDiGetDeviceInterfaceDetail
        //  (which will give us the string we need)
        //

        //
        // First call it with a NULL output buffer to
        //  get the number of bytes needed...
        //
        if (!SetupDiGetDeviceInterfaceDetail(devInfo,
                                             &devInterfaceData,
                                             NULL,
                                             0,
                                             &requiredSize,
                                             NULL)) {

            code = GetLastError();

            //
            // We're expecting ERROR_INSUFFICIENT_BUFFER.
            //  If we get anything else there's something
            //  wrong...
            //
            if (code != ERROR_INSUFFICIENT_BUFFER) {

                printf("SetupDiGetDeviceInterfaceDetail failed with error 0x%x\n", code);

                //
                // Clean up the mess...
                //
                SetupDiDestroyDeviceInfoList(devInfo);
                
                return INVALID_HANDLE_VALUE;

            }

        }

        //
        // Allocated a PSP_DEVICE_INTERFACE_DETAIL_DATA...
        //
        devInterfaceDetailData = 
                (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc(requiredSize);

        if (!devInterfaceDetailData) {

            printf("Unable to allocate resources...Exiting\n");

            //
            // Clean up the mess...
            //
            SetupDiDestroyDeviceInfoList(devInfo);

            return INVALID_HANDLE_VALUE;

        }

        //
        // This needs to be set before calling
        //  SetupDiGetDeviceInterfaceDetail. You
        //  would *think* that you should be setting
        //  cbSize to requiredSize, but that's not the 
        //  case. 
        //
        devInterfaceDetailData->cbSize = 
                    sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);


        if (!SetupDiGetDeviceInterfaceDetail(devInfo,
                                             &devInterfaceData,
                                             devInterfaceDetailData,
                                             requiredSize,
                                             &requiredSize,
                                             NULL)) {

            printf("SetupDiGetDeviceInterfaceDetail failed with error 0x%x\n", GetLastError());

            //
            // Clean up the mess...
            //
            SetupDiDestroyDeviceInfoList(devInfo);

            free(devInterfaceDetailData);

            return INVALID_HANDLE_VALUE;

        }

        //
        // Got one!
        //
        printf("Device found! %s\n", devInterfaceDetailData->DevicePath);

		printf("Opening device interface %s\n", 
						devInterfaceDetailData->DevicePath);
	    
		
		osrSPDriverHandle = CreateFile(devInterfaceDetailData->DevicePath,  // Name of the NT "device" to open 
							GENERIC_READ|GENERIC_WRITE,  // Access rights requested
							0,                           // Share access - NONE
							0,                           // Security attributes - not used!
							OPEN_EXISTING,               // Device must exist to open it.
							FILE_FLAG_OVERLAPPED,        // Open for overlapped I/O
							0);                          // extended attributes - not used!

		command.IoControlCode = IOCTL_OSRVMPORT_SCSIPORT;

        devStatus = DeviceIoControl(osrSPDriverHandle,IOCTL_MINIPORT_PROCESS_SERVICE_IRP,
			&command,sizeof(command),&command,sizeof(command),&bytesReturned,NULL); 

		if(!devStatus) {
			CString errorMsg;
            DWORD error = GetLastError();
			errorMsg.Format(L"\\\\.\\SCSI%s: error:%d.\n",
				devInterfaceDetailData->DevicePath,error);
			OutputDebugString((LPCTSTR) errorMsg);
            CloseHandle(osrSPDriverHandle);
            osrSPDriverHandle = INVALID_HANDLE_VALUE;
            continue;
		} else {
			//
			// Clean up the mess...
			//
			SetupDiDestroyDeviceInfoList(devInfo);
	        
			free(devInterfaceDetailData);
			return osrSPDriverHandle;
		}
    }

    code = GetLastError();

    //
    // We shouldn't get here until SetupDiGetDeviceInterfaceDetail
    //  runs out of devices to enumerate
    //
    if (code != ERROR_NO_MORE_ITEMS) {

        printf("SetupDiGetDeviceInterfaceDetail failed with error 0x%x\n", code);
        
        //
        // Clean up the mess...
        //
        SetupDiDestroyDeviceInfoList(devInfo);
        
        free(devInterfaceDetailData);
        
        return INVALID_HANDLE_VALUE;

    }

    SetupDiDestroyDeviceInfoList (devInfo);
    
    if(devInterfaceDetailData == NULL) {
        
        printf("Unable to find any Nothing devices!\n");

        return INVALID_HANDLE_VALUE;

    }

    return INVALID_HANDLE_VALUE;

}

DWORD OSRSPConnect(const WCHAR* pathName,USHORT DiskSizeMB)
{
    CONNECT_IN  connectIn;
    HANDLE      osrSPDriverHandle;
    DWORD       status = ERROR_SUCCESS;
    DWORD       bytesReturned;
	BOOL		devStatus;

    memset(&connectIn,0,sizeof(CONNECT_IN));

    if(wcslen(pathName) > MAX_NAME_LENGTH) {

        return ERROR_INVALID_PARAMETER;

    }
    
    osrSPDriverHandle = ConnectToScsiPort();

    if(osrSPDriverHandle == INVALID_HANDLE_VALUE) {

        status = GetLastError();
        OutputDebugString(L"Unable to connect to OSR Scsi Port Connection Manager.\n");

        return status;

    }

    memcpy(&connectIn.InstanceName[0],pathName,min(wcslen(pathName),MAX_NAME_LENGTH) * sizeof(WCHAR));
    connectIn.DiskSizeMB = DiskSizeMB;
	connectIn.Command.IoControlCode = IOCTL_OSRVMPORT_CONNECT;

	devStatus = DeviceIoControl(osrSPDriverHandle,IOCTL_MINIPORT_PROCESS_SERVICE_IRP,&connectIn,sizeof(CONNECT_IN),
        NULL,0,&bytesReturned,NULL);

    if(!devStatus) {

        status = GetLastError();
        OutputDebugString(L"Unable to connect to OSR Scsi Port Connection Manager.\n");
		CloseHandle(osrSPDriverHandle);

        return status;
    
    }

    CloseHandle(osrSPDriverHandle);

    return ERROR_SUCCESS;
}


DWORD OSRSPDisconnect(WCHAR* InstanceName)
{
    CONNECT_IN  disconnectIn;
    HANDLE      osrSPDriverHandle;
    DWORD       status = ERROR_SUCCESS;
    DWORD       bytesReturned;
	BOOL		devStatus;

    memset(&disconnectIn,0,sizeof(CONNECT_IN));

    if(wcslen(InstanceName) > MAX_NAME_LENGTH) {

        return ERROR_INVALID_PARAMETER;

    }
    
    osrSPDriverHandle = ConnectToScsiPort();

    if(osrSPDriverHandle == INVALID_HANDLE_VALUE) {

        status = GetLastError();
        OutputDebugString(L"Unable to connect to OSR Scsi Port Connection Manager.\n");

        return status;

    }

    memcpy(&disconnectIn.InstanceName[0],InstanceName,min(wcslen(InstanceName),MAX_NAME_LENGTH) * sizeof(WCHAR));
	disconnectIn.Command.IoControlCode = IOCTL_OSRVMPORT_DISCONNECT;

	devStatus = DeviceIoControl(osrSPDriverHandle,IOCTL_MINIPORT_PROCESS_SERVICE_IRP,
					&disconnectIn,sizeof(CONNECT_IN),
					NULL,0,&bytesReturned,NULL);

    if(!devStatus) {

        status = GetLastError();
        OutputDebugString(L"Unable to disconnect OSR Scsi Port Connection Manager.\n");

		CloseHandle(osrSPDriverHandle);

        return status;
    
    }

    CloseHandle(osrSPDriverHandle);

    return ERROR_SUCCESS;
}

DWORD OSRSPGetActiveList(PACTIVELIST_ENTRY  *PPActiveList,PULONG PCount)
{
    HANDLE              osrSPDriverHandle;
    DWORD               status = ERROR_SUCCESS;
    DWORD               bytesReturned;
    PUCHAR              pBuffer = NULL;
	COMMAND_IN			command;
	BOOL				devStatus;

    if(!PPActiveList || !PCount) {

        return ERROR_INVALID_PARAMETER;

    }

    *PPActiveList = NULL;
    *PCount = 0;

    osrSPDriverHandle = ConnectToScsiPort();

    if(osrSPDriverHandle == INVALID_HANDLE_VALUE) {

        status = GetLastError();
        OutputDebugString(L"Unable to connect to OSR Scsi Port Connection Manager.\n");

        return status;

    }

    pBuffer = new UCHAR[10000];

    if(!pBuffer) {

        CloseHandle(osrSPDriverHandle);
        return ERROR_NOT_ENOUGH_MEMORY;

    }

    memset(pBuffer,0,10000);

	command.IoControlCode = IOCTL_OSRVMPORT_GETACTIVELIST;

	devStatus = DeviceIoControl(osrSPDriverHandle,IOCTL_MINIPORT_PROCESS_SERVICE_IRP,
					&command,sizeof(command),
					pBuffer,10000,&bytesReturned,NULL);

    if(!devStatus) {

        status = GetLastError();
        OutputDebugString(L"Unable to disconnect OSR Scsi Port Connection Manager.\n");

        CloseHandle(osrSPDriverHandle);
        return status;
    
    }

    PGETACTIVELIST_OUT  pActiveConnectList = (PGETACTIVELIST_OUT) pBuffer;

    if(bytesReturned && pActiveConnectList->ActiveListCount) {

        PUCHAR              pOutBuffer = new UCHAR[bytesReturned];
        PACTIVELIST_ENTRY   pActiveListtmp = (PACTIVELIST_ENTRY) pOutBuffer;
        ULONG               offset = pActiveConnectList->ActiveListCount * sizeof(ACTIVELIST_ENTRY);

        if(pActiveListtmp) {

            PUCHAR  pStrings = &pOutBuffer[offset];  // offset to where strings are put in buffer

            *PPActiveList = pActiveListtmp;
        
            *PCount = pActiveConnectList->ActiveListCount;

            memset(pActiveListtmp,0,sizeof(bytesReturned));

            //
            // Loop through the received list copying the information from the driver to
            // the buffer allocated.  The strings are put at the end of the buffer being
            // returned to the user.
            //

            for(ULONG index = 0; index < pActiveConnectList->ActiveListCount; index ++) {
             
                pActiveListtmp[index].BusNumber = pActiveConnectList->ActiveEntry[index].BusNumber;
                pActiveListtmp[index].TargetId = pActiveConnectList->ActiveEntry[index].TargetId;
                pActiveListtmp[index].Lun = pActiveConnectList->ActiveEntry[index].Lun;
                pActiveListtmp[index].Connected = pActiveConnectList->ActiveEntry[index].Connected;
                pActiveListtmp[index].DiskSizeMB = pActiveConnectList->ActiveEntry[index].DiskSizeMB;

                pActiveListtmp[index].InstanceName  = (PWCHAR) pStrings;
				memcpy(pStrings,(PUCHAR) &pActiveConnectList->ActiveEntry[index].ConnectionInformation.InstanceName[0],
					   (wcslen(&pActiveConnectList->ActiveEntry[index].ConnectionInformation.InstanceName[0])+1) * sizeof(WCHAR));
                pStrings += (wcslen(&pActiveConnectList->ActiveEntry[index].ConnectionInformation.InstanceName[0])+1) * sizeof(WCHAR);

            }

            status = ERROR_SUCCESS;

        } else {

            status = ERROR_NOT_ENOUGH_MEMORY;

        }

    }

    delete []pBuffer;

    CloseHandle(osrSPDriverHandle);

    return status;
}

