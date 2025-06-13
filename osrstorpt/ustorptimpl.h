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
//
//	MODULE:
//
//		$File: osrstorpt\ustrptimpl.h $
//
//	ABSTRACT:
//
//      This file contains implemenation information used by the
//		driver.
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
#ifndef __OSRVMIMPL_H__
#define __OSRVMIMPL_H__

extern "C" {
#pragma warning(disable : 4115 4201 4214 4127 4702)
#include <ntifs.h>
#include <ntddscsi.h>
#include <ntddstor.h>
#include <ntdddisk.h>
#include <ntddcdrm.h>
#include <scsi.h>
#include <ntstrsafe.h>
#pragma warning(default : 4115 4201 4214 4127 4702)
};

#include <osrstorptuserintf.h>
#include <osrstorptuserioctl.h>
#include <osrstorptdebug.h>
#include <stdio.h>


#define OSR_INQUIRY_VENDOR_ID           "OSRDisk"
#define OSR_INQUIRY_PRODUCT_ID          "OSR_R_DISK"
#define OSR_INQUIRY_PRODUCT_REVISION    "V2.0"
#define OSR_INQUIRY_VENDOR_SPECIFIC     "OSR_DISK"

#define OSR_INQUIRY_VENDOR_ID_CDROM           "OSRCdrm"
#define OSR_INQUIRY_PRODUCT_ID_CDROM          "OSR_R_Cdrm"
#define OSR_INQUIRY_VENDOR_SPECIFIC_CDROM     "OSR_Cdrm"


#define REVERSE_2BYTES(Destination, Source) {                \
    PTWO_BYTE d = (PTWO_BYTE)(Destination);               \
    PTWO_BYTE s = (PTWO_BYTE)(Source);                    \
    d->Byte1 = s->Byte0;                                    \
    d->Byte0 = s->Byte1;                                    \
}

typedef struct _CONNECTION_LIST_ENTRY {

    LIST_ENTRY                  ListEntry;
    struct _USER_INSTANCE_INFO* PIInfo;
    ULONG                       BusIndex;
    ULONG                       TargetIndex;
    ULONG                       LunIndex;
    BOOLEAN                     Connected;
    BOOLEAN                     ContainingMediaRemoved;  
	ULONG						IdentifierIndex;
	BOOLEAN						Closing;  /* Indicates connection is closing */
    PVOID                       DiskBaseAddress;
    ULONG                       DiskSize;
    CONNECT_IN                  ConnectionInfo; 

} CONNECTION_LIST_ENTRY, *PCONNECTION_LIST_ENTRY;

#define LOGICAL_BLOCK_SIZE 512

//
// Maximum Transfer Size capabilities
//

#define OSRSCSI_MAXIMUM_TRANSFER_SIZE  1024*1024

#define TOC_DATA_TRACK              (0x04)


//
// This is the global data structure used by the lower layer of the OSR Stoport Miniport for
// a virtual adapter driver.   As we mentioned in our NT Insider article the driver is broken
// up into 2 layers.  The top layer handles the Storport interactions and the botton layer
// handles the implementation of the adapter and devices we want to export.
//
typedef struct _USER_GLOBAL_INFO 
{

    ULONG                   MagicNumber;
    PVOID                   OsrSPHandle;       // Handle passed to OsrSp Routines.

    LONG                    ConnectionCount;   // Number of connections in ConnectionList;
    LIST_ENTRY              ConnectionList;
    KSPIN_LOCK              ConnectionListLock;
    KMUTEX                  ConnectionMutex;

} USER_GLOBAL_INFORMATION, *PUSER_GLOBAL_INFORMATION;

#define USER_GLOBAL_INFORMATION_MAGIC_NUMBER 0x93847546

#define USER_GLOBAL_VALID(gbl) OSRASSERT((gbl)->MagicNumber == USER_GLOBAL_INFORMATION_MAGIC_NUMBER)

//
// Structure used for unique ID creation.
//

typedef struct _UDISKID {

    // Globally unique id of the device in the system

    GUID                UniqueID;
    ULONGLONG           FileId;     // unique Id on Server.


} UDISKID, *PUDISKID;


//
// This structure is used for local information.   One of these is created for
// each device instance that the user creates.   Equate these to PDOs.
//
typedef enum _OSRSTORAGETYPE {
	OsrCdrom,
	OsrDisk,
} OSRSTORAGETYPE, *POSRSTORAGETYPE;

//
// This structure represents an instance of a device as defined by the lower
// layer of the miniport driver.   One of these is created when told by a caller
// to create a connection.   The lower layer creates one of these to define the
// device.   OsrSpLocalHandle is the opaque handle that we return to the upper
// layer of the miniport.   When the upper layer of the miniport needs something
// from a particular device, it passes in the OsrSPLocalHandle and from that we
// know what is being references.
//
typedef struct _USER_INSTANCE_INFO
{
    ULONG                       MagicNumber;
    PVOID                       OsrSPLocalHandle;  // Handle passed to OsrSp Routines.
    ULONG                       BusIndex;
    ULONG                       TargetIndex;
    ULONG                       LunIndex;
    PUSER_GLOBAL_INFORMATION    PGInfo;
    PINQUIRYDATA                PInquiryData;
    PCONNECTION_LIST_ENTRY      ConnectionInformation;
    CHAR                        AsciiSignature[256];
    UDISKID                     UniqueID;
	OSRSTORAGETYPE				StorageType;
    BOOLEAN                     OsrDisk;

} USER_INSTANCE_INFORMATION, *PUSER_INSTANCE_INFORMATION;

#define USER_INSTANCE_INFORMATION_MAGIC_NUMBER 0x89da4eff

#define USER_INSTANCE_VALID(inst) OSRASSERT((inst)->MagicNumber == USER_INSTANCE_INFORMATION_MAGIC_NUMBER)

typedef struct _GLOBALS {
   
    //
    // Uid and Gid Information Stored in Registry.
    //

    ULONG           Uid;
    ULONG           Gid;

    //
    // NFS Tx/Rx sizes
    //

    ULONG           MaxRxSize;
    ULONG           MaxTxSize;

    //
    // NFS Tx/Rx Retry Count
    //

    ULONG           NfsRetryCount;

    //
    // Connection Entries to restore on Reboot.
    //

    BOOLEAN         AddConnectionsToRegistry;

    //
    // Thread Base Priority
    //

    ULONG           ThreadBasePriority;
    ULONG           ReadThreadBasePriority;

    //
    // Number of worker threads per Disk.
    //

    ULONG           ThreadCount;
    ULONG           ReadThreadCount;

    //
    // RestoreTimerInterval
    //

    ULONG           RestoreTimerInterval;

} GLOBALS;


extern GLOBALS Globals;



#define OsrMin(a,b) a < b ? a : b
#define OsrMax(a,b) a > b ? a : b

//
// device type table to build id's from
//

typedef struct _INQUIRY_DEVICE_TYPE {

    PCSTR DeviceTypeString;

    PCSTR GenericTypeString;

    PWSTR DeviceMapString;

    BOOLEAN IsStorage;

} INQUIRY_DEVICE_TYPE, *PINQUIRY_DEVICE_TYPE;


//
// Forward definitions.
//
VOID InitializeScsiIds();
BOOLEAN FindConnectionMatch(PUSER_GLOBAL_INFORMATION PGInfo, PCONNECT_IN PConnectInfo,
                            PCONNECTION_LIST_ENTRY* PPFoundEntry);
void UpdateConnectionListInRegistry(PUSER_GLOBAL_INFORMATION PGInfo);
void DeleteConnectionListInRegistry(PUSER_GLOBAL_INFORMATION PGInfo);
NTSTATUS DeleteConnection(PUSER_GLOBAL_INFORMATION PGInfo,PCONNECT_IN PDisconnectInfo);
NTSTATUS DeleteConnectionEntry(PUSER_GLOBAL_INFORMATION PGInfo,PCONNECTION_LIST_ENTRY PCle,PCONNECT_IN PConnectInfo);
void UpdateConnectionListInRegistry(PUSER_GLOBAL_INFORMATION PGInfo);
NTSTATUS EnumerateActiveConnections(PUSER_GLOBAL_INFORMATION PGInfo,PIRP Irp);
PINQUIRY_DEVICE_TYPE GetDeviceTypeInfo(IN UCHAR DeviceType);
NTSTATUS CreateConnection(PUSER_GLOBAL_INFORMATION PGInfo, PCONNECT_IN PConnectInfo);
NTSTATUS OsrUserReadData(IN PVOID UserLocalInfoHandle,PSCSI_REQUEST_BLOCK PSrb,PMDL MdlAddress,ULARGE_INTEGER StartingLbn,
                         ULONG ReadLength,PULONG PBytesRead);
NTSTATUS OsrUserWriteData(IN PVOID UserLocalInfoHandle,PSCSI_REQUEST_BLOCK PSrb,PMDL MdlAddress,ULARGE_INTEGER StartingLbn,
                         ULONG WriteLength,PULONG PBytesWritten);



VOID OsrUserGetDiskCapacity(IN PVOID UserLocalInfoHandle,ULONG* PNumberOfBlocks,ULONG* PBlockSize);
BOOLEAN  OsrUserIsDeviceReadOnly(IN PVOID UserLocalInfoHandle);


//
// Utility Routines
//

NTSTATUS CreateMultiSZ(PUNICODE_STRING MultiString,PCSTR StringArray[]);

BOOLEAN RegistryReadSubValue(PUNICODE_STRING RegistryPath, PUNICODE_STRING SubPath, 
                                PWSTR Key, ULONG Type, PVOID Value);
BOOLEAN RegistryReadValue(PUNICODE_STRING RegistryPath, PWSTR Key, ULONG Type, PVOID Value);
BOOLEAN RegistryReadBinarySubValue(PUNICODE_STRING RegistryPath, PUNICODE_STRING SubPath, PWSTR Key, 
                                      PUCHAR* PBuffer, PULONG Size);
BOOLEAN RegistryWriteSubValue(PUNICODE_STRING RegistryPath, PUNICODE_STRING SubPath, PWSTR Key, 
                                      ULONG Type, PUCHAR PBuffer, ULONG Size);

#endif __OSRVMIMPL_H__
