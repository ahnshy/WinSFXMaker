
// WinSFXMakerDlg.h : header file
//

#pragma once
#include "UI/DragListCtrl.h"

// CWinSFXMakerDlg dialog
class CWinSFXMakerDlg : public CDialogEx
{
// Construction
public:
	CWinSFXMakerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WINSFXMAKER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	BOOL CreateSFXFile(LPCTSTR templatePath, LPCTSTR zipPath, LPCTSTR outputPath);
	void SaveResourceToFile(CString& strPath);

// Implementation
protected:
	HICON m_hIcon;

	CDragListCtrl	m_listFiles;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};
