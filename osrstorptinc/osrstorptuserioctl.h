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
//	MODULE:
//
//		$File: //depot/tools/osrvmMEMsample/OsrVmSampleInc/OSRVMUserIoctl.h $
//
//	ABSTRACT:
//
//      This h file contains the IOCTL definitions used to communicate
//      with the OSR Virtual Miniport Driver.
//
//	AUTHOR:
//
//		Open Systems Resources, Inc.
// 
//	REVISION:   
//
//		$Revision: #2 $
//
//
//  NOTE:
//
//      All WCHAR strings used by these IOCTL's are assumed to be Zero 
//      Terminated.
//
//
///////////////////////////////////////////////////////////////////////////////
#ifndef __OSRVMUSERIOCTL_H__
#define __OSRVMUSERIOCTL_H__

#define MAX_NAME_LENGTH 256

#include <OsrVmCfg.h>

#define IOCTL_OSRVMPORT_SCSIPORT CTL_CODE(FILE_DEVICE_OSRVMPORT,USER_VM_IOCTL_START,METHOD_BUFFERED,FILE_ALL_ACCESS)

#define IOCTL_OSRVMPORT_CONNECT  CTL_CODE(FILE_DEVICE_OSRVMPORT,USER_VM_IOCTL_START+2,METHOD_BUFFERED,FILE_WRITE_ACCESS)

typedef struct _CONNECT_IN {
	COMMAND_IN		Command;
    WCHAR           InstanceName[MAX_NAME_LENGTH];
    USHORT          DiskSizeMB;
} CONNECT_IN, *PCONNECT_IN;

#define IOCTL_OSRVMPORT_DISCONNECT CTL_CODE(FILE_DEVICE_OSRVMPORT,USER_VM_IOCTL_START+3,METHOD_BUFFERED,FILE_WRITE_ACCESS)

//
// USE THE CONNECT_IN structure for DISCONNECT.
//

#define IOCTL_OSRVMPORT_GETACTIVELIST CTL_CODE(FILE_DEVICE_OSRVMPORT,USER_VM_IOCTL_START+4,METHOD_BUFFERED,FILE_WRITE_ACCESS)

typedef struct _ACTIVELIST_ENTRY_OUT {

    CONNECT_IN      ConnectionInformation;
    USHORT          Connected;
    USHORT          BusNumber;
    USHORT          TargetId;
    USHORT          Lun;
    USHORT          DiskSizeMB;

} ACTIVELIST_ENTRY_OUT, *PACTIVELIST_ENTRY_OUT;

typedef struct _GETACTIVELIST_OUT {

    ULONG                   ActiveListCount;
    ACTIVELIST_ENTRY_OUT    ActiveEntry[1];

} GETACTIVELIST_OUT, *PGETACTIVELIST_OUT;

#define USER_VM_CREATE	0xA0000001
#define USER_VM_READ	0xA0000002
#define USER_VM_WRITE	0xA0000003
#define USER_VM_FATTRIB 0xA0000004
#define USER_VM_CLOSE	0xA0000005

#endif //__OSRVMUSERIOCTL_H__