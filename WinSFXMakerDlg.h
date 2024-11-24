
// WinSFXMakerDlg.h : header file
//

#pragma once
#include "UI/DragListCtrl.h"

class CFileListCtrl : public CMFCListCtrl
{
	virtual int OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn);
	virtual COLORREF OnGetCellBkColor(int nRow, int nColum);
};

class CFileInfo
{
public:
	CString				m_strPath;
	CString				m_strFileName;
	DWORD				m_dwSize;

	CFileInfo::CFileInfo()
	{
		SetEmpty();
	}

	void CFileInfo::SetEmpty()
	{
		m_strPath.Empty();
		m_strFileName.Empty();
		m_dwSize = 0;
	}
};

UINT TaskFindFilesFunc(LPVOID pParam);
// CWinSFXMakerDlg dialog
class CWinSFXMakerDlg : public CDialogEx
{
// Construction
public:
	CWinSFXMakerDlg(CWnd* pParent = NULL);	// standard constructor
	~CWinSFXMakerDlg();

// Dialog Data
	enum { IDD = IDD_WINSFXMAKER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	BOOL CreateSFXFile(LPCTSTR templatePath, LPCTSTR zipPath, LPCTSTR outputPath);
	void SaveResourceToFile(CString& strPath);
	
public:
	void					InitFileInfo();
	void					FindFiles(CString strPath);
	void					AddFiles();
	
	CString			GetPath()							{ return m_strPath; }

	CPtrArray		m_arFiles;

// Implementation
protected:
	HICON m_hIcon;

	CWinThread*		m_pThread;
	CFileListCtrl	m_wndList;
	CString			m_strPath;
	BOOL			m_bTaskFinish;
	CMFCButton				m_btnDirectory;

	void Initialize();

	void					MoveWindow();
	void					UpdateResult();
	BOOL					BeginFindFiles(CString strPath);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg					void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg					HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg					void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonPath();
};
