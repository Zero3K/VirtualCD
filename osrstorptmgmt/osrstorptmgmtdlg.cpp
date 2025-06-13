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
//		$File: osrstorptmgmt\osrstorptmgmtdlg.cpp $
//
//	ABSTRACT:
//
//      SCSI Port Management Application Dialog
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
// OSRSPMgmtDlg.cpp : implementation file
//

#include "stdafx.h"
#include "osrstorptmgmt.h"
#include "osrstorptmgmtdlg.h"
#include "adddevice.h"
#include <htmlhelp.h>
#include <osrstorptuserioctl.h>
#include <osrspintf.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef struct _FILE_ALLOCATION_INFORMATION {
    LARGE_INTEGER AllocationSize;
} FILE_ALLOCATION_INFORMATION, *PFILE_ALLOCATION_INFORMATION;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COSRSPMgmtDlg dialog

COSRSPMgmtDlg::COSRSPMgmtDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COSRSPMgmtDlg::IDD, pParent)
	, m_pActiveList(NULL)
{
	//{{AFX_DATA_INIT(COSRSPMgmtDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

COSRSPMgmtDlg::~COSRSPMgmtDlg()
{
	delete []m_pActiveList;
}

void COSRSPMgmtDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(COSRSPMgmtDlg)
    DDX_Control(pDX, IDOK, m_OkButton);
    DDX_Control(pDX, IDCANCEL, m_CancelButton);
    DDX_Control(pDX, IDC_BUTTON_ADD, m_AddButton);
    DDX_Control(pDX, ID_HELP, m_HelpButton);
    DDX_Control(pDX, IDC_STATIC_TITLE, m_Title);
    DDX_Control(pDX, IDC_CONNECTION_LIST, m_ConnectionList);
    DDX_Control(pDX, IDC_BUTTON_DELETE, m_DeleteButton);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COSRSPMgmtDlg, CDialog)
	//{{AFX_MSG_MAP(COSRSPMgmtDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_BN_CLICKED(ID_HELP, OnHelp)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COSRSPMgmtDlg message handlers

BOOL COSRSPMgmtDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

    HANDLE osrSPDriverHandle = ConnectToScsiPort();

    if(osrSPDriverHandle == INVALID_HANDLE_VALUE) {
        AfxMessageBox(L"OSRScsi Driver Not Found. Exiting.",MB_OK|MB_ICONSTOP);
        OnCancel();
        return FALSE;
    }
    CloseHandle(osrSPDriverHandle);

    m_nTimer = -1;

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
    m_ConnectionList.SetExtendedStyle(LVS_EX_FULLROWSELECT);	

    CRect rect;
    CSize size;
	TEXTMETRIC textMetrics;
	CDC* pDc = m_ConnectionList.GetDC();

	if(!pDc)
		return FALSE;

    m_ConnectionList.GetClientRect(rect);
    pDc->GetTextMetrics(&textMetrics);

    size = rect.Size();

    UINT portion = size.cx/4;

    UINT portion1 = wcslen(L"Bus,Target,Lun") * textMetrics.tmAveCharWidth;
    UINT portion2 = wcslen(L"DiskSizeMB") * textMetrics.tmAveCharWidth;
    UINT portion3 = wcslen(L"Disconnected") * textMetrics.tmAveCharWidth;

    m_ConnectionList.InsertColumn(0,L"Instance Name",LVCFMT_CENTER,size.cx - portion1 - portion2 - portion3);
    m_ConnectionList.InsertColumn(1,L"Bus, Target, Lun",LVCFMT_CENTER,portion1);
    m_ConnectionList.InsertColumn(2,L"DiskSizeMB",LVCFMT_CENTER,portion2);
    m_ConnectionList.InsertColumn(3,L"Status",LVCFMT_CENTER,portion3);

	m_ConnectionList.ReleaseDC(pDc);

    UpdateDisplay();

    UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void COSRSPMgmtDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

void COSRSPMgmtDlg::OnDestroy()
{
    if(m_nTimer != -1) {
        KillTimer(1234);
        m_nTimer = -1;
    }
	WinHelp(0L, HELP_QUIT);
	CDialog::OnDestroy();
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void COSRSPMgmtDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR COSRSPMgmtDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void COSRSPMgmtDlg::OnButtonAdd() 
{
    CAddDevice dlg;

    if(dlg.DoModal() == IDOK) {


        DWORD status = OSRSPConnect((LPCTSTR) dlg.m_PathName,(USHORT) dlg.m_DiskSizeMB);

        if(status == ERROR_SUCCESS) {

            UpdateDisplay();

        } else {

            CString message;

            message.Format(L"The connection could not be configured due to the following error %x.",status);
            AfxMessageBox(message,MB_OK|MB_ICONSTOP);

        }

        HandleDeleteButton();
    }
	
}

void COSRSPMgmtDlg::OnButtonDelete() 
{
    DWORD   status;
    int     listLimit = m_ConnectionList.GetItemCount();

    if(!m_ConnectionList.GetSelectedCount()) {
        return;
    }

    //
    // Look for a selected item in the list.  If none
    // are found, then exit.
    //

    for(int index = 0; index < listLimit; index++) {

        if(m_ConnectionList.GetItemState(index,LVIS_SELECTED)) {

            //
            // Check to see if the user really meant to do this.....
            //

            if(IDOK != AfxMessageBox(L"Are you sure you want to remove this device?",MB_OKCANCEL|MB_ICONQUESTION)) {
                break;
            }

            //
            // Retrieve the information from the list item.
            //
			status = OSRSPDisconnect((PWCHAR) (LPCTSTR) m_ConnectionList.GetItemText(index,0));

            if(status != ERROR_SUCCESS) {

                AfxMessageBox(L"Could not delete the connection.",MB_OK|MB_ICONSTOP);
                return;

            }

            //
            // Remove the entry from the list.
            //

            m_ConnectionList.DeleteItem(index);

            break;
        }

    }

    HandleDeleteButton();

}

DWORD COSRSPMgmtDlg::AddEntryToList(PACTIVELIST_ENTRY PAEntry)
{
    int     insertIndex = 0;
    CString text;

	insertIndex = m_ConnectionList.InsertItem(insertIndex,(LPCTSTR) PAEntry->InstanceName);

    UpdateEntryInList(insertIndex,PAEntry);
    
    return ERROR_SUCCESS;

}

void COSRSPMgmtDlg::OnHelp() 
{
#if 0
    WCHAR buffer[512];
    memset(buffer,0,sizeof(buffer));
    GetWindowsDirectory(buffer,sizeof(buffer)/sizeof(WCHAR));
    CString helpfilename;
    helpfilename = buffer;
    helpfilename += L"\\system32\\OsrSpMgmt.chm";
	HtmlHelp(NULL,(LPCTSTR) helpfilename,HH_DISPLAY_TOPIC,0);
#endif
}

void COSRSPMgmtDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
    if(nIDEvent == 1234) {
        UpdateDisplay();
    }
	
	CDialog::OnTimer(nIDEvent);
}

void COSRSPMgmtDlg::HandleDeleteButton()
{
    if(!m_ConnectionList.GetItemCount()) {

        m_DeleteButton.EnableWindow(FALSE);
        if(m_nTimer != -1) {
            KillTimer(1234);
            m_nTimer = -1;
        }

    } else {

        m_DeleteButton.EnableWindow(TRUE);
        if(m_nTimer == -1) {
            m_nTimer = SetTimer(1234,10000,NULL);
        }
    }

}

void COSRSPMgmtDlg::UpdateDisplay()
{
    PACTIVELIST_ENTRY pActiveList = NULL;
    ULONG             activeCount = 0;
    CWaitCursor       wait;


    DWORD status = OSRSPGetActiveList(&pActiveList,&activeCount);

    if(status == ERROR_SUCCESS) {

		delete []m_pActiveList;

		m_pActiveList = pActiveList;

        for(DWORD index = 0; ((index < activeCount) && pActiveList); index++) {

            int findIndex;
            BOOLEAN found = FALSE;

            for(findIndex = 0; findIndex < m_ConnectionList.GetItemCount(); findIndex++) {
                CString text = m_ConnectionList.GetItemText(findIndex,0);
                if(text.Compare(pActiveList[index].InstanceName) == 0) {
                    found = TRUE;
                    break;
                }
            }

            if(found) {
                UpdateEntryInList(findIndex,&pActiveList[index]);
            } else {
                AddEntryToList(&pActiveList[index]);
            }

        }
    }

    HandleDeleteButton();

}

DWORD COSRSPMgmtDlg::UpdateEntryInList(int InsertIndex, PACTIVELIST_ENTRY PAEntry)
{
    CString text;

    text.Format(L"%d,%d,%d",PAEntry->BusNumber,PAEntry->TargetId,PAEntry->Lun);
	m_ConnectionList.SetItemText(InsertIndex,1,(LPCTSTR) text);

    text.Format(L"%dMB",PAEntry->DiskSizeMB);
    m_ConnectionList.SetItemText(InsertIndex,2,(LPCTSTR) text);

    if(PAEntry->Connected) {

	    m_ConnectionList.SetItemText(InsertIndex,3,L"Connected");

    } else {

	    m_ConnectionList.SetItemText(InsertIndex,3,L"Disconnected");
    }

	m_ConnectionList.SetItemData(InsertIndex,(DWORD_PTR) PAEntry);

    return ERROR_SUCCESS;

}

void COSRSPMgmtDlg::OnCancel() 
{
	CDialog::OnCancel();
}

