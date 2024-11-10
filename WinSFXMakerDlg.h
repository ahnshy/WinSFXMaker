
// WinSFXMakerDlg.h : header file
//

#pragma once


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
	BOOL ChangeExeIcon(LPCTSTR szExePath, LPCTSTR szIconFilePath);
	BOOL UpdateVersionInfo(LPCTSTR szExePath, const CString& productName, const CString& fileVersion, const CString& companyName);
	BOOL GetVersionInfo(LPCTSTR szExePath, CString& fileVersion, CString& productName, CString& companyName);

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};
