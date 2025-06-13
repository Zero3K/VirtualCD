///////////////////////////////////////////////////////////////////////////////
//
//    (C) Copyright 2009 OSR Open Systems Resources, Inc.
//    All Rights Reserved
//
//  This work is distributed under the OSR Non-Commercial Software License which is provided
//  at "http://www.osronline.com/page.cfm?name=NonCommLicense" in the hope that it will be
//  enlightening, but WITHOUT ANY WARRANTY; without even the implied warranty of MECHANTABILITY
//
//    OSR Open Systems Resources, Inc.
//    105 Route 101A Suite 19
//    Amherst, NH 03031  (603) 595-6500 FAX: (603) 595-6503
//    email bugs to: bugs@osr.com
//
//
//    MODULE:
//
//        $File: //depot/tools/osrvmMEMsample/OsrVmSample/OsrVminipt.h $
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
//        $Revision: #3 $
//
///////////////////////////////////////////////////////////////////////////////

extern "C" {
#include <wdm.h>
#include <storport.h>  
#include <devioctl.h>
#include <ntddscsi.h>
#include <scsiwmi.h>
};

#include <osrvmuserintf.h>
#include <osrvmdebug.h>
#include <osrVmcfg.h>

extern PDRIVER_OBJECT OsrDriverObject;
extern UNICODE_STRING OsrRegistryPath;

#define MAX_TARGETS                 8

//
// Our hardware extension.   THis represents the virtual adapter that we 
// are exporting to Storport.
//
typedef struct _OSR_DEVICE_EXTENSION {
	ULONG							MagicNumber;
    SCSI_ADAPTER_CONTROL_TYPE		AdapterState;
	SCSI_WMILIB_CONTEXT				WmiLibCtx;
	IO_SCSI_CAPABILITIES			Capabilities;
	PVOID							PUserGlobalInformation;
	UNICODE_STRING					DeviceInterface;
	ULONG							NodeNumber;
	LIST_ENTRY						DeviceList;
	KSPIN_LOCK						DeviceListLock;
	KEVENT							DeleteDevicesThreadStartEvent;
	KEVENT							DeleteDevicesThreadKillEvent;
	KEVENT							DeleteDevicesThreadDeadEvent;
	KEVENT							DeleteDevicesThreadWorkEvent;
	HANDLE							DeleteDevicesThreadHandle;
	BOOLEAN							KillThread;
} OSR_DEVICE_EXTENSION, *POSR_DEVICE_EXTENSION;

#define OSR_DEVICE_EXTENSION_MAGIC 0xaaaabbbb

#define OSR_DEVEXT_VALID(x) OSRASSERT(x->MagicNumber == OSR_DEVICE_EXTENSION_MAGIC)

//
// Our Logical Unit Extension that represents a device that we export on a 
// specific bus,target,lun.   If that device is available, then the Missing
// flag will be false and OsrVmDevice will be present.
//
typedef struct _OSR_LU_EXTENSION {
    UCHAR					DeviceType;
	ULONG					PathId;
	ULONG					TargetId;
	ULONG					Lun;
	BOOLEAN					Missing;
	PVOID					PDevExt;
	struct _OSR_VM_DEVICE*	OsrVmDevice;	
} OSR_LU_EXTENSION, *POSR_LU_EXTENSION;


//
// This represents a device that has been detected on a specific bus,target,
// and lun.      PUserLocalInformation represents the handle given to us
// by the lower layer of our code that implements the adapter and scsi devices.
//
typedef struct _OSR_VM_DEVICE {
	ULONG				MagicNumber;
	LIST_ENTRY			ListEntry;
	PVOID				PUserLocalInformation;
	ULONG				PathId;
	ULONG				TargetId;
	ULONG				Lun;
	PINQUIRYDATA		PInquiryData;
	BOOLEAN				BReadOnlyDevice;
	BOOLEAN				Missing;
	PVOID				PDevExt;
	LONG				OutstandingIoCount;
	BOOLEAN				ReportedMissing;
} OSR_VM_DEVICE, *POSR_VM_DEVICE;

#define OSR_VM_DEVICE_MAGIC 0xccccaaaa

#define OSR_VM_DEVICE_VALID(x) OSRASSERT(x->MagicNumber == OSR_VM_DEVICE_MAGIC)

//
// Forward Definitions for OsrVminiPt.cpp
//
extern "C" { NTSTATUS DriverEntry(PDRIVER_OBJECT, PUNICODE_STRING); }


//
// Forward Definitions for OsrVmScsi.cpp
//
UCHAR OsrVmExecuteScsi(IN POSR_DEVICE_EXTENSION PDevExt,
					   IN PSCSI_REQUEST_BLOCK PSrb,
					   IN PBOOLEAN PComplete);

//
// Forward Definitions for OsrVmIoCtrl.cpp
//
UCHAR OsrVmIoControl(IN POSR_DEVICE_EXTENSION PDevExt,
				     IN PSCSI_REQUEST_BLOCK  PSrb);

//
// Forward Definitions for OsrVmPnp.cpp
//
NTSTATUS OsrVmHwHandlePnP(IN POSR_DEVICE_EXTENSION PDevExt,
						  IN PSCSI_PNP_REQUEST_BLOCK PSrb);

//
// Forward Definitions for OsrVmWmi.cpp
//
BOOLEAN OsrVmHandleWmiSrb(IN POSR_DEVICE_EXTENSION PDevExt,
						  IN OUT PSCSI_WMI_REQUEST_BLOCK PSrb);
VOID OsrVmWmiLibInitialize(POSR_DEVICE_EXTENSION PDevExt);


//
// Forward Definitions for OsrVmDevice.cpp
//
POSR_VM_DEVICE FindOsrVmDevice(IN POSR_LU_EXTENSION LuExt,
							   IN POSR_DEVICE_EXTENSION PDevExt,
							   IN UCHAR PathId,
							   IN UCHAR TargetId,
							   IN UCHAR Lun,
							   IN BOOLEAN ReturnMissing);
