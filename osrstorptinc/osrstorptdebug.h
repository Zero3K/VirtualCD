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
//		$File: osrstorptinc\osrstorptdebug.h $
//
//	ABSTRACT:
//
//      This file contains Debug information
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
#ifndef TRACE_LEVEL_INFORMATION
#define TRACE_LEVEL_NONE        0   // Tracing is not on
#define TRACE_LEVEL_FATAL       1   // Abnormal exit or termination
#define TRACE_LEVEL_ERROR       2   // Severe errors that need logging
#define TRACE_LEVEL_WARNING     3   // Warnings such as allocation failure
#define TRACE_LEVEL_INFORMATION 4   // Includes non-error cases(e.g.,Entry-Exit)
#define TRACE_LEVEL_VERBOSE     5   // Detailed traces from intermediate steps
#define TRACE_LEVEL_RESERVED6   6
#define TRACE_LEVEL_RESERVED7   7
#define TRACE_LEVEL_RESERVED8   8
#define TRACE_LEVEL_RESERVED9   9
#endif // TRACE_LEVEL_INFORMATION


#define OSRVMINIPT_DEBUG_ERROR                   0x00000001
#define OSRVMINIPT_DEBUG_FUNCTRACE               0x00000002
#define OSRVMINIPT_DEBUG_PNP_INFO                0x00000004
#define OSRVMINIPT_DEBUG_IOCTL_INFO              0x00000008
#define OSRVMINIPT_DEBUG_POWER_INFO              0x00000010
#define OSRVMINIPT_DEBUG_WMI_INFO                0x00000020
#define OSRVMINIPT_DEBUG_SRB                     0x00000040
#define OSRVMINIPT_DEBUG_USER                    0x00000080
#define OSRVMINIPT_DEBUG_USER_READ               0x00000100
#define OSRVMINIPT_DEBUG_USER_WRITE              0x00000200
#define OSRVMINIPT_DEBUG_CLUSTER                 0x00000400
#define OSRVMINIPT_DEBUG_SRB_STATUS              0x00000800
#define OSRVMINIPT_DEBUG_ADAPTER                 0x00001000
#define OSRVMINIPT_DEBUG_SHUTDOWN_FLUSH          0x00002000
#define OSRVMINIPT_DEBUG_SUMMARY                 0x00004000
#define OSRVMINIPT_DEBUG_DRIVER_ENTRY            0x00008000
#define OSRVMINIPT_DEBUG_USER_CONNECTION         0x00010000
#define OSRVMINIPT_DEBUG_SRB_USER                0x00020000
#define OSRVMINIPT_DEBUG_SERVICE                 0x00040000
#define OSRVMINIPT_DEBUG_ALL                     0xFFFFFFFF

extern ULONG OsrTraceLevel;
extern ULONG OsrDbgFlags;

//
// Define these as a way to get around the linker warnings seen
// for duplicates of KeInitializeSpinLock.   We don't like having to
// do this, but we don't like warnings either.
//
VOID OsrInitializeSpinLock(PKSPIN_LOCK a);
VOID OsrAcquireSpinLock(PKSPIN_LOCK a,KIRQL* b); 
VOID OsrReleaseSpinLock(PKSPIN_LOCK a,KIRQL b) ;



#if DBG

#define OsrTracePrint(Level,Flags,X) \
{ \
    if(Level <= OsrTraceLevel && Flags & OsrDbgFlags) { \
        DbgPrint X; \
    } \
}

#else // DBG

#define OsrTracePrint(Level,Flags,X)

#endif // DBG

#if DBG

#define OSRBreakPoint() \
do { \
      __try { \
              DbgPrint("BreakPoint %s %d\n",__FILE__,__LINE__); \
              DbgBreakPoint(); \
      } __except(_exception_code() == STATUS_BREAKPOINT ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_CONTINUE_SEARCH) { (0); }  \
} while (0)

#define OSRASSERT(exp) \
do { \
    _try{ \
        if (!(exp)) {\
            DbgPrint("ASSERTION FAILED: %s (file %s, line %d) %s\n", #exp, __FILE__, __LINE__, "" ); \
            DbgBreakPoint(); \
        } \
    } _except(EXCEPTION_EXECUTE_HANDLER) {         \
        KeBugCheckEx(0x00010001,0,0,0,0); \
    }                       \
} while (0)

#else //DBG>0

#define OSRBreakPoint()

#define OSRASSERT(x) \
    { \
        if(!(x)) {\
            KeBugCheckEx(0x00010001,0,0,0,0); \
        } \
    }

#endif //DBG>0
