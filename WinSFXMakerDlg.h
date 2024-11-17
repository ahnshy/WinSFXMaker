
// WinSFXMakerDlg.h : header file
//

#pragma once
#include "UI/DragListCtrl.h"

class CFileListCtrl : public CMFCListCtrl
{
	virtual int OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn);
	virtual COLORREF OnGetCellBkColor(int nRow, int nColum);
};


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

	CFileListCtrl	m_wndList;

	void Initialize();

	void					MoveWindow();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg					HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg					void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonPath();
};
