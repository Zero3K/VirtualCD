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
//		$File: //depot/tools/osrvmMEMsample/OsrVmSampleInc/osrspintf.h $
//
//	ABSTRACT:
//
//      This file contains OSRSP managment APIs.
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
#ifndef __OSRSPINTF_H__
#define __OSRSPINFT_H__

/*
 * This will create a new connection
 */
HANDLE ConnectToScsiPort();

DWORD OSRSPConnect(const WCHAR* InstanceName,
                   USHORT       SizeMB);

/*
 * This will disconnect a connection
 */

DWORD OSRSPDisconnect(WCHAR* InstanceName);

/*
 * This will Give a list of active connections
 */

typedef struct _ACTIVELIST_ENTRY {

    WCHAR*          InstanceName;
    USHORT          BusNumber;
    USHORT          TargetId;
    USHORT          Lun;
    USHORT          DiskSizeMB;
    USHORT          Connected;

} ACTIVELIST_ENTRY, *PACTIVELIST_ENTRY;

DWORD OSRSPGetActiveList(PACTIVELIST_ENTRY  *PPActiveList,PULONG PCount);

#endif