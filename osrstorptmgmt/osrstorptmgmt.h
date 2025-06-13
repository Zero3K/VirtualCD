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
//		$File: osrstorptmgmt\osrstorptmgmt.h $
//
//	ABSTRACT:
//
//      This file contains Management information
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
// OSRSPMgmt.h : main header file for the OSRSPMGMT application
//

#if !defined(AFX_OSRSPMGMT_H__76A6A3AE_EC19_479E_B114_910A0D526DE7__INCLUDED_)
#define AFX_OSRSPMGMT_H__76A6A3AE_EC19_479E_B114_910A0D526DE7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// COSRSPMgmtApp:
// See OSRSPMgmt.cpp for the implementation of this class
//

class COSRSPMgmtApp : public CWinApp
{
public:
	COSRSPMgmtApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COSRSPMgmtApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(COSRSPMgmtApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OSRSPMGMT_H__76A6A3AE_EC19_479E_B114_910A0D526DE7__INCLUDED_)
