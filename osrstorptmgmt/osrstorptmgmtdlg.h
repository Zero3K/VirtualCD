// OSRSPMgmtDlg.h : header file
//

#if !defined(AFX_OSRSPMGMTDLG_H__EA981EFC_ED39_47C7_AFA1_7C5506DFDB22__INCLUDED_)
#define AFX_OSRSPMGMTDLG_H__EA981EFC_ED39_47C7_AFA1_7C5506DFDB22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct _ACTIVELIST_ENTRY;

/////////////////////////////////////////////////////////////////////////////
// COSRSPMgmtDlg dialog

class COSRSPMgmtDlg : public CDialog
{
// Construction
public:
	DWORD AddEntryToList(struct _ACTIVELIST_ENTRY* PAEntry);
	DWORD UpdateEntryInList(int InsertIndex,struct _ACTIVELIST_ENTRY* PAEntry);
	void UpdateDisplay();
	void HandleDeleteButton();
	UINT_PTR m_nTimer;
	COSRSPMgmtDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~COSRSPMgmtDlg();

// Dialog Data
	//{{AFX_DATA(COSRSPMgmtDlg)
	enum { IDD = IDD_OSRSPMGMT_DIALOG };
	CButton	m_OkButton;
	CButton	m_CancelButton;
	CButton	m_AddButton;
	CButton	m_HelpButton;
	CStatic	m_Title;
	CListCtrl	m_ConnectionList;
	CButton	m_DeleteButton;
	struct _ACTIVELIST_ENTRY*	m_pActiveList;

	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COSRSPMgmtDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(COSRSPMgmtDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonCreateDisk();
	afx_msg void OnButtonDelete();
	afx_msg void OnHelp();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OSRSPMGMTDLG_H__EA981EFC_ED39_47C7_AFA1_7C5506DFDB22__INCLUDED_)
