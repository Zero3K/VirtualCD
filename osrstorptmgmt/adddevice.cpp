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
//		$File: osrstorptmgmt\adddevice.cpp $
//
//	ABSTRACT:
//
//      Add a SCSI Device Code.
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
// AddDevice.cpp : implementation file
//

#include "stdafx.h"
#include "osrstorptmgmt.h"
#include "adddevice.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddDevice dialog


CAddDevice::CAddDevice(CWnd* pParent /*=NULL*/)
	: CDialog(CAddDevice::IDD, pParent)
    , m_Disk(TRUE)
    , m_DiskSizeMB(5)
{
	//{{AFX_DATA_INIT(CAddDevice)
	m_PathName = _T("");
	//}}AFX_DATA_INIT
}


void CAddDevice::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAddDevice)
    DDX_Text(pDX, IDC_EDIT_PATHNAME, m_PathName);
    //}}AFX_DATA_MAP
    DDX_Text(pDX, IDC_EDIT_DISK_SIZE_MB, m_DiskSizeMB);
	DDV_MinMaxShort(pDX, m_DiskSizeMB, 5, 512);
}


BEGIN_MESSAGE_MAP(CAddDevice, CDialog)
	//{{AFX_MSG_MAP(CAddDevice)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddDevice message handlers

BOOL CAddDevice::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddDevice::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}
