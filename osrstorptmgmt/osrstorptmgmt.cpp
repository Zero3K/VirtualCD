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
//		$File: //depot/tools/osrvmMEMsample/OsrVmSampleMgmt/OSRSPMgmt.cpp $
//
//	ABSTRACT:
//
//      OSR SCSI PORT MANAGEMENT Code
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
// OSRSPMgmt.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "OSRSPMgmt.h"
#include "OSRSPMgmtDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COSRSPMgmtApp

BEGIN_MESSAGE_MAP(COSRSPMgmtApp, CWinApp)
	//{{AFX_MSG_MAP(COSRSPMgmtApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COSRSPMgmtApp construction

COSRSPMgmtApp::COSRSPMgmtApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only COSRSPMgmtApp object

COSRSPMgmtApp theApp;

/////////////////////////////////////////////////////////////////////////////
// COSRSPMgmtApp initialization

BOOL COSRSPMgmtApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
//	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	COSRSPMgmtDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
