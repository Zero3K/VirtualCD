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
//		$File: //depot/tools/osrvmMEMsample/OsrVmSampleMgmt/AddDevice.h $
//
//	ABSTRACT:
//
//      This file contains AddDevice definitions
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
#if !defined(AFX_ADDDEVICE_H__57705060_9FAD_4D64_953B_65FBA1137CEF__INCLUDED_)
#define AFX_ADDDEVICE_H__57705060_9FAD_4D64_953B_65FBA1137CEF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddDevice.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddDevice dialog

class CAddDevice : public CDialog
{
// Construction
public:
	CAddDevice(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddDevice)
	enum { IDD = IDD_DIALOG_ADD_DEVICE };
	CString	m_PathName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddDevice)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAddDevice)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnButtonBrowse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    BOOL m_Disk;
    short m_DiskSizeMB;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDDEVICE_H__57705060_9FAD_4D64_953B_65FBA1137CEF__INCLUDED_)
