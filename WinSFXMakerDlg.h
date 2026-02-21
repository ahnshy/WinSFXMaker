// WinSFXMakerDlg.h : header file
#pragma once
#include "Dialog/DlgPrgress.h"
#include "UI/DragListCtrl.h"

// CDlgSFXProgress dialog
class CDlgSFXProgress : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSFXProgress)
public:
	CDlgSFXProgress(CWnd* pParent = NULL);
	virtual ~CDlgSFXProgress();
	enum { IDD = IDD_SFX_PROGRESS };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl m_ctrlProgress;
	CStatic m_ctrlStatus;
	CBrush m_brBackground;
	BOOL m_bCancel;
	void SetProgress(int nPos);
	void SetStatus(LPCTSTR lpszStatus);
	void SetRange(int nMin, int nMax);
	BOOL IsCancelled() { return m_bCancel; }
	afx_msg void OnBnClickedButtonCancel();
};

class CFileListCtrl : public CMFCListCtrl
{
	virtual int OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn);
	virtual COLORREF OnGetCellBkColor(int nRow, int nColum);
};

class CFileInfo
{
public:
	CString m_strPath;
	CString m_strFileName;
	DWORD m_dwSize;
	CFileInfo() { SetEmpty(); }
	void SetEmpty() { m_strPath.Empty(); m_strFileName.Empty(); m_dwSize = 0; }
};

// SFX creation thread parameters
struct SFX_THREAD_PARAM
{
	CString strInputPath;
	CString strOutputPath;
	CString strIconPath;
	CString strCompanyName;
	CString strProductName;
	CString strVersion;
	CString strCopyright;
	CString strDescription;
	CDlgSFXProgress* pProgressDlg;
	HWND hWndParent;
	BOOL bSuccess;
};

UINT TaskFindFilesFunc(LPVOID pParam);
UINT TaskCreateSFXFunc(LPVOID pParam);

class CWinSFXMakerDlg : public CDialogEx
{
public:
	CWinSFXMakerDlg(CWnd* pParent = NULL);
	~CWinSFXMakerDlg();
	enum { IDD = IDD_WINSFXMAKER_DIALOG };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	BOOL CreateSFXFile(LPCTSTR templatePath, LPCTSTR zipPath, LPCTSTR outputPath, CDlgSFXProgress* pProgress);
	void SaveResourceToFile(CString& strPath);
public:
	void InitFileInfo();
	void FindFiles(CString strPath);
	void AddFiles();
	void ShowResultWnd(BOOL bShow = TRUE);
	INT32 Initialize();
	INT32 UnInitialize();
	void UpdateControls();
	CString GetInputPath() { return m_strInputPath; }
	CString GetOutputPath() { return m_strOutputPath; }
	CPtrArray m_arFiles;
	CStringArray m_arPeFiles;
protected:
	HICON m_hIcon;
	HICON m_hPreviewIcon;
	CWinThread* m_pThread;
	CWinThread* m_pSFXThread;
	CFileListCtrl m_wndList;
	CString m_strInputPath;
	CString m_strOutputPath;
	CString m_strIconPath;
	BOOL m_bTaskFinish;
	CMFCButton m_btnDirectory;
	CRect m_rtTab;
	CDlgPrgress* m_pDlgProgress;
	CDlgSFXProgress* m_pSFXProgressDlg;
	SFX_THREAD_PARAM m_sfxParam;
	void InitControls();
	void MoveWindow(BOOL bShow);
	void UpdateResult();
	BOOL BeginFindFiles(CString strPath);
	void CastByte(CString& strValue);
	void UpdateIconPreview();
	void LoadDefaultIcon();
	BOOL ApplyIconToExe(LPCTSTR lpszExePath);
	BOOL ApplyVersionInfo(LPCTSTR lpszExePath);
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonPath();
	afx_msg void OnBnClickedButtonOuputPath();
	afx_msg void OnCbnEditchangeComboInputPath();
	afx_msg void OnLvnItemchangedListArchive(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnEditchangeComboOutputPath();
	afx_msg void OnBnClickedButtonIcon();
	afx_msg LRESULT OnSFXComplete(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

#define WM_SFX_COMPLETE (WM_USER + 100)
