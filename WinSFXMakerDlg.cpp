// WinSFXMakerDlg.cpp : implementation file
#include "stdafx.h"
#include "WinSFXMaker.h"
#include "WinSFXMakerDlg.h"
#include "afxdialogex.h"
#include "Helper/FileHelper.h"
#include "Helper/DirectoryHelper.h"
#include "Helper/ZipHelper.h"
#include "Helper/ResourceManager.h"
#include "Dialog/DlgShellTreeExplore.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDlgSFXProgress implementation
IMPLEMENT_DYNAMIC(CDlgSFXProgress, CDialogEx)

CDlgSFXProgress::CDlgSFXProgress(CWnd* pParent)
	: CDialogEx(CDlgSFXProgress::IDD, pParent)
{
	m_bCancel = FALSE;
	m_brBackground.CreateSolidBrush(RGB(0, 0, 0));
}

CDlgSFXProgress::~CDlgSFXProgress()
{
}

void CDlgSFXProgress::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_SFX, m_ctrlProgress);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_ctrlStatus);
}

BEGIN_MESSAGE_MAP(CDlgSFXProgress, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CDlgSFXProgress::OnBnClickedButtonCancel)
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CDlgSFXProgress::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_bCancel = FALSE;
	m_ctrlProgress.SetRange(0, 100);
	m_ctrlProgress.SetPos(0);
	m_ctrlStatus.SetWindowText(_T("Preparing..."));
	
	// Apply layered window style for semi-transparency with black background
	ModifyStyleEx(0, WS_EX_LAYERED);
	SetLayeredWindowAttributes(RGB(0, 0, 0), 200, LWA_ALPHA);
	SetBackgroundColor(RGB(0, 0, 0));
	
	// Center controls will be done in OnSize
	
	return TRUE;
}

void CDlgSFXProgress::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	
	if (cx <= 0 || cy <= 0) return;
	
	// Control dimensions - progress bar 30% wider
	int nControlWidth = 370;  // 222 * 1.3 = ~290
	int nControlX = (cx - nControlWidth) / 2;
	int nCenterY = cy / 2;
	
	// Spacing between controls
	int nSpacing = 10;
	
	// Status label - above progress bar with 10px spacing
	if (m_ctrlStatus.GetSafeHwnd())
	{
		m_ctrlStatus.SetWindowPos(NULL, nControlX, nCenterY - 14 - nSpacing - 10, nControlWidth, 16, SWP_NOZORDER);
	}
	
	// Progress bar - at center
	if (m_ctrlProgress.GetSafeHwnd())
	{
		m_ctrlProgress.SetWindowPos(NULL, nControlX, nCenterY - 7, nControlWidth, 21, SWP_NOZORDER);
	}
	
	// Cancel button - below progress bar with 10px spacing
	CWnd* pBtnCancel = GetDlgItem(IDC_BUTTON_CANCEL);
	if (pBtnCancel && pBtnCancel->GetSafeHwnd())
	{
		int nBtnWidth = 75;
		int nBtnX = (cx - nBtnWidth) / 2;
		pBtnCancel->SetWindowPos(NULL, nBtnX, nCenterY + 7 + nSpacing*4, nBtnWidth, 23, SWP_NOZORDER);
	}
	
	Invalidate();
}

HBRUSH CDlgSFXProgress::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUS)
	{
		pDC->SetTextColor(RGB(135, 206, 250)); // Sky blue color
		pDC->SetBkColor(RGB(0, 0, 0)); // Black background
		pDC->SetBkMode(OPAQUE);
		return m_brBackground;
	}
	
	return hbr;
}

void CDlgSFXProgress::SetProgress(int nPos)
{
	if (m_ctrlProgress.GetSafeHwnd())
		m_ctrlProgress.SetPos(nPos);
}

void CDlgSFXProgress::SetStatus(LPCTSTR lpszStatus)
{
	if (m_ctrlStatus.GetSafeHwnd())
	{
		m_ctrlStatus.SetWindowText(lpszStatus);
		m_ctrlStatus.Invalidate();
		m_ctrlStatus.UpdateWindow();
	}
}

void CDlgSFXProgress::SetRange(int nMin, int nMax)
{
	if (m_ctrlProgress.GetSafeHwnd())
		m_ctrlProgress.SetRange(nMin, nMax);
}

void CDlgSFXProgress::OnBnClickedButtonCancel()
{
	m_bCancel = TRUE;
}

// End of CDlgSFXProgress implementation

UINT TaskFindFilesFunc(LPVOID pParam)
{
	CWinSFXMakerDlg *pDlg = (CWinSFXMakerDlg*)pParam;
	if (pDlg == NULL) return 0;
	pDlg->EnableWindow(FALSE);
	pDlg->InitFileInfo();
	pDlg->FindFiles(pDlg->GetInputPath());
	pDlg->AddFiles();
	pDlg->ShowResultWnd(FALSE);
	pDlg->UpdateControls();
	pDlg->EnableWindow(TRUE);
	return 1;
}

UINT TaskCreateSFXFunc(LPVOID pParam)
{
	SFX_THREAD_PARAM* pSfxParam = (SFX_THREAD_PARAM*)pParam;
	if (pSfxParam == NULL) return 0;

	CDlgSFXProgress* pProgress = pSfxParam->pProgressDlg;
	pSfxParam->bSuccess = FALSE;

	CString strTempPath;
	CString strZipFile;
	CString strTemplateFile;
	BOOL bCancelled = FALSE;

	// Step 1: ZIP compression (30%)
	if (pProgress) { pProgress->SetStatus(_T("Compressing files...")); pProgress->SetProgress(0); }
	if (pProgress && pProgress->IsCancelled()) { bCancelled = TRUE; }

	if (!bCancelled)
	{
		CZipHelper zipHelper;
		zipHelper.AddCompressTarget(pSfxParam->strInputPath);

		strTempPath = CDirectoryHelper::GetTempPath();
		PathAddBackslash(strTempPath.GetBuffer(MAX_PATH));
		strTempPath.ReleaseBuffer();

		strZipFile = CFileHelper::GetTimeBaseFileName(strTempPath, _T("zip"));
		
		if (pProgress) pProgress->SetProgress(10);
		if (pProgress && pProgress->IsCancelled()) { bCancelled = TRUE; }
		
		if (!bCancelled)
		{
			zipHelper.Compress(strZipFile);
			if (pProgress) pProgress->SetProgress(30);
			if (pProgress && pProgress->IsCancelled()) { bCancelled = TRUE; }
		}
	}

	// Step 2: Extract template file (40%)
	if (!bCancelled)
	{
		if (pProgress) pProgress->SetStatus(_T("Preparing template..."));
		
		strTemplateFile = CFileHelper::GetTimeBaseFileName(strTempPath, _T("exe"));
		HRSRC hRes = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_TEMPLATE_BINARY), _T("BINARY"));
		if (hRes)
		{
			DWORD dwSize = SizeofResource(NULL, hRes);
			HGLOBAL hGlobal = LoadResource(NULL, hRes);
			if (hGlobal && dwSize > 0)
			{
				LPVOID pData = LockResource(hGlobal);
				if (pData)
				{
					CFile file;
					if (file.Open(strTemplateFile, CFile::modeCreate | CFile::modeWrite))
					{
						file.Write(pData, dwSize);
						file.Close();
					}
					UnlockResource(hGlobal);
				}
			}
		}

		if (pProgress) pProgress->SetProgress(40);
		if (pProgress && pProgress->IsCancelled()) { bCancelled = TRUE; }
	}

	// Step 3: Apply icon (50%)
	if (!bCancelled && !pSfxParam->strIconPath.IsEmpty())
	{
		if (pProgress) pProgress->SetStatus(_T("Applying icon..."));
		CResourceManager resMgr;
		resMgr.ChangeExeIcon(strTemplateFile, pSfxParam->strIconPath);

		if (pProgress) pProgress->SetProgress(50);
		if (pProgress && pProgress->IsCancelled()) { bCancelled = TRUE; }
	}
	else if (!bCancelled)
	{
		if (pProgress) pProgress->SetProgress(50);
	}

	// Step 4: Apply version info (60%)
	if (!bCancelled)
	{
		if (!pSfxParam->strCompanyName.IsEmpty() || !pSfxParam->strProductName.IsEmpty() ||
			!pSfxParam->strVersion.IsEmpty() || !pSfxParam->strCopyright.IsEmpty() || !pSfxParam->strDescription.IsEmpty())
		{
			if (pProgress) pProgress->SetStatus(_T("Applying version info..."));
			CResourceManager resMgr;
			resMgr.UpdateVersionInfo(strTemplateFile, pSfxParam->strCompanyName, pSfxParam->strProductName,
				pSfxParam->strVersion, pSfxParam->strCopyright, pSfxParam->strDescription);
		}

		if (pProgress) pProgress->SetProgress(60);
		if (pProgress && pProgress->IsCancelled()) { bCancelled = TRUE; }
	}

	// Step 5: Create SFX file (90%)
	if (!bCancelled)
	{
		if (pProgress) pProgress->SetStatus(_T("Creating SFX file..."));
		
		CFile templateFile, zipFile, outFile;
		if (templateFile.Open(strTemplateFile, CFile::modeRead | CFile::typeBinary))
		{
			ULONGLONG templateSize = templateFile.GetLength();
			BYTE* pTemplateData = new BYTE[(size_t)templateSize];
			templateFile.Read(pTemplateData, (UINT)templateSize);
			templateFile.Close();

			if (zipFile.Open(strZipFile, CFile::modeRead | CFile::typeBinary))
			{
				ULONGLONG zipSize = zipFile.GetLength();
				BYTE* pZipData = new BYTE[(size_t)zipSize];
				zipFile.Read(pZipData, (UINT)zipSize);
				zipFile.Close();

				if (outFile.Open(pSfxParam->strOutputPath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
				{
					outFile.Write(pTemplateData, (UINT)templateSize);
					outFile.Write(pZipData, (UINT)zipSize);
					LONGLONG llZipSize = (LONGLONG)zipSize;
					outFile.Write(&llZipSize, sizeof(llZipSize));
					outFile.Close();
					pSfxParam->bSuccess = TRUE;
				}
				delete[] pZipData;
			}
			delete[] pTemplateData;
		}

		if (pProgress) pProgress->SetProgress(90);
	}

	// Step 6: Cleanup temp files (100%)
	if (pProgress) pProgress->SetStatus(_T("Cleaning up..."));
	if (!strZipFile.IsEmpty()) DeleteFile(strZipFile);
	if (!strTemplateFile.IsEmpty()) DeleteFile(strTemplateFile);

	if (pProgress) pProgress->SetProgress(100);

	// Send completion message
	if (pSfxParam->hWndParent)
		::PostMessage(pSfxParam->hWndParent, WM_SFX_COMPLETE, (WPARAM)pSfxParam->bSuccess, 0);

	return pSfxParam->bSuccess ? 1 : 0;
}

BOOL CWinSFXMakerDlg::BeginFindFiles(CString strPath)
{
	if (strPath.IsEmpty()) return FALSE;
	if (m_pThread) m_pThread = NULL;
	m_pThread = (CWinThread*)AfxBeginThread(TaskFindFilesFunc, (LPVOID)this);
	return TRUE;
}

int CFileListCtrl::OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn) { return 0; }
COLORREF CFileListCtrl::OnGetCellBkColor(int nRow, int nColum) { return (nRow % 2) == 0 ? RGB(249, 249, 249) : RGB(223, 230, 238); }

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();
	enum { IDD = IDD_ABOUTBOX };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()
};
CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD) {}
void CAboutDlg::DoDataExchange(CDataExchange* pDX) { CDialogEx::DoDataExchange(pDX); }
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

CWinSFXMakerDlg::CWinSFXMakerDlg(CWnd* pParent)
	: CDialogEx(CWinSFXMakerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hPreviewIcon = NULL;
	m_rtTab.SetRectEmpty();
	m_pDlgProgress = NULL;
	m_pSFXProgressDlg = NULL;
	m_pSFXThread = NULL;
	m_bTaskFinish = FALSE;
	m_pThread = NULL;
	m_strInputPath.Empty();
	m_strOutputPath.Empty();
	m_strIconPath.Empty();
	InitFileInfo();
}

CWinSFXMakerDlg::~CWinSFXMakerDlg()
{
	InitFileInfo();
	if (m_hPreviewIcon != NULL) { DestroyIcon(m_hPreviewIcon); m_hPreviewIcon = NULL; }
	if (m_pSFXProgressDlg != NULL) { delete m_pSFXProgressDlg; m_pSFXProgressDlg = NULL; }
}

void CWinSFXMakerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ARCHIVE, m_wndList);
	DDX_Control(pDX, ID_BUTTON_INPUT_PATH, m_btnDirectory);
}

BEGIN_MESSAGE_MAP(CWinSFXMakerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CWinSFXMakerDlg::OnBnClickedOk)
	ON_WM_CTLCOLOR()
	ON_WM_WINDOWPOSCHANGED()
	ON_BN_CLICKED(ID_BUTTON_INPUT_PATH, &CWinSFXMakerDlg::OnBnClickedButtonPath)
	ON_BN_CLICKED(ID_BUTTON_OUPUT_PATH, &CWinSFXMakerDlg::OnBnClickedButtonOuputPath)
	ON_CBN_EDITCHANGE(IDC_COMBO_INPUT_PATH, &CWinSFXMakerDlg::OnCbnEditchangeComboInputPath)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ARCHIVE, &CWinSFXMakerDlg::OnLvnItemchangedListArchive)
	ON_CBN_EDITCHANGE(IDC_COMBO_OUTPUT_PATH, &CWinSFXMakerDlg::OnCbnEditchangeComboOutputPath)
	ON_BN_CLICKED(IDC_BUTTON_ICON, &CWinSFXMakerDlg::OnBnClickedButtonIcon)
	ON_MESSAGE(WM_SFX_COMPLETE, &CWinSFXMakerDlg::OnSFXComplete)
END_MESSAGE_MAP()

BOOL CWinSFXMakerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);
	SetBackgroundColor(RGB(255, 255, 255));
	
	// Hide Company Name controls if they exist
	CWnd* pCompanyLabel = GetDlgItem(IDC_STATIC_COMPANY_NAME);
	CWnd* pCompanyEdit = GetDlgItem(IDC_EDIT_COMPANY_NAME);
	if (pCompanyLabel) pCompanyLabel->ShowWindow(SW_HIDE);
	if (pCompanyEdit) pCompanyEdit->ShowWindow(SW_HIDE);
	
	InitControls();
	UpdateControls();
	LoadDefaultIcon();
	return TRUE;
}

void CWinSFXMakerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX) { CAboutDlg dlgAbout; dlgAbout.DoModal(); }
	else { CDialogEx::OnSysCommand(nID, lParam); }
}

void CWinSFXMakerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
	}
	else { CDialogEx::OnPaint(); }
}

HCURSOR CWinSFXMakerDlg::OnQueryDragIcon() { return static_cast<HCURSOR>(m_hIcon); }

void CWinSFXMakerDlg::SaveResourceToFile(CString& strPath)
{
	HRSRC hRes = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_TEMPLATE_BINARY), _T("BINARY"));
	if (hRes == NULL) { AfxMessageBox(_T("Resource not found.")); return; }
	DWORD dwSize = SizeofResource(NULL, hRes);
	if (dwSize == 0) { AfxMessageBox(_T("Resource size is zero.")); return; }
	HGLOBAL hGlobal = LoadResource(NULL, hRes);
	if (hGlobal == NULL) { AfxMessageBox(_T("Failed to load resource.")); return; }
	LPVOID pData = LockResource(hGlobal);
	if (pData == NULL) { AfxMessageBox(_T("Failed to lock resource.")); return; }
	CFile file;
	if (file.Open(strPath, CFile::modeCreate | CFile::modeWrite)) { file.Write(pData, dwSize); file.Close(); }
	else { AfxMessageBox(_T("Failed to save file.")); }
	UnlockResource(hGlobal);
}

BOOL CWinSFXMakerDlg::CreateSFXFile(LPCTSTR templatePath, LPCTSTR zipPath, LPCTSTR outputPath, CDlgSFXProgress* pProgress)
{
	return TRUE;
}

BOOL CWinSFXMakerDlg::ApplyIconToExe(LPCTSTR lpszExePath)
{
	if (m_strIconPath.IsEmpty()) return FALSE;
	CResourceManager resMgr;
	return resMgr.ChangeExeIcon(lpszExePath, (LPCTSTR)m_strIconPath);
}

BOOL CWinSFXMakerDlg::ApplyVersionInfo(LPCTSTR lpszExePath)
{
	CString strProductName, strVersion, strCopyright, strDescription;
	GetDlgItemText(IDC_EDIT_PRODUCT_NAME, strProductName);
	GetDlgItemText(IDC_EDIT_VERSION, strVersion);
	GetDlgItemText(IDC_EDIT_COPYRIGHT, strCopyright);
	GetDlgItemText(IDC_EDIT_DESCRIPT, strDescription);
	if (strProductName.IsEmpty() && strVersion.IsEmpty() && strCopyright.IsEmpty() && strDescription.IsEmpty())
		return TRUE;
	CResourceManager resMgr;
	return resMgr.UpdateVersionInfo(lpszExePath, _T(""), (LPCTSTR)strProductName, (LPCTSTR)strVersion, (LPCTSTR)strCopyright, (LPCTSTR)strDescription);
}

void CWinSFXMakerDlg::UpdateIconPreview()
{
	if (m_hPreviewIcon != NULL) { DestroyIcon(m_hPreviewIcon); m_hPreviewIcon = NULL; }
	CWnd* pIconWnd = GetDlgItem(IDC_STATIC_MAIN_ICON);
	if (pIconWnd == NULL) return;
	if (!m_strIconPath.IsEmpty())
		m_hPreviewIcon = (HICON)LoadImage(NULL, m_strIconPath, IMAGE_ICON, 48, 48, LR_LOADFROMFILE);
	CStatic* pStatic = (CStatic*)pIconWnd;
	if (m_hPreviewIcon != NULL) pStatic->SetIcon(m_hPreviewIcon);
	else pStatic->SetIcon(NULL);
	pIconWnd->Invalidate();
	pIconWnd->UpdateWindow();
}

void CWinSFXMakerDlg::LoadDefaultIcon()
{
	// Extract template exe to temp file
	CString strTempPath = CDirectoryHelper::GetTempPath();
	PathAddBackslash(strTempPath.GetBuffer(MAX_PATH));
	strTempPath.ReleaseBuffer();
	
	CString strTemplateFile = strTempPath + _T("_sfx_template_icon.exe");
	
	HRSRC hRes = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_TEMPLATE_BINARY), _T("BINARY"));
	if (hRes)
	{
		DWORD dwSize = SizeofResource(NULL, hRes);
		HGLOBAL hGlobal = LoadResource(NULL, hRes);
		if (hGlobal && dwSize > 0)
		{
			LPVOID pData = LockResource(hGlobal);
			if (pData)
			{
				CFile file;
				if (file.Open(strTemplateFile, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
				{
					file.Write(pData, dwSize);
					file.Close();
					
					// Extract icon from template exe
					if (m_hPreviewIcon != NULL) { DestroyIcon(m_hPreviewIcon); m_hPreviewIcon = NULL; }
					m_hPreviewIcon = ExtractIcon(AfxGetInstanceHandle(), strTemplateFile, 0);
					
					CWnd* pIconWnd = GetDlgItem(IDC_STATIC_MAIN_ICON);
					if (pIconWnd)
					{
						CStatic* pStatic = (CStatic*)pIconWnd;
						if (m_hPreviewIcon && m_hPreviewIcon != (HICON)1)
						{
							pStatic->SetIcon(m_hPreviewIcon);
						}
						else
						{
							// Fallback: use app icon
							pStatic->SetIcon(m_hIcon);
						}
						pIconWnd->Invalidate();
						pIconWnd->UpdateWindow();
					}
					
					DeleteFile(strTemplateFile);
				}
				UnlockResource(hGlobal);
			}
		}
	}
}

void CWinSFXMakerDlg::OnBnClickedButtonIcon()
{
	CFileDialog dlg(TRUE, _T("ico"), _T("*.ico"), OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, _T("Icon Files (*.ico)|*.ico|All Files (*.*)|*.*||"));
	TCHAR szPath[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH - 1, szPath);
	dlg.m_ofn.lpstrInitialDir = szPath;
	if (dlg.DoModal() == IDOK) { m_strIconPath = dlg.GetPathName(); UpdateIconPreview(); }
}

void CWinSFXMakerDlg::InitControls()
{
	InitFileInfo();
	m_btnDirectory.SetImage(IDB_BITMAP_DIRECTORY, IDB_BITMAP_DIRECTORY_HOT);
	m_btnDirectory.SetMouseCursorHand();
	m_btnDirectory.SetWindowText(_T(""));
	m_btnDirectory.Invalidate();

	CRect rt;
	GetClientRect(&rt);
	LVCOLUMN item;
	item.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	item.fmt = LVCFMT_LEFT;
	item.cx = (int)(rt.Width() * 0.25);
	item.pszText = _T("File");
	item.iSubItem = 0;
	m_wndList.InsertColumn(0, &item);
	item.cx = (int)(rt.Width() * 0.52);
	item.pszText = _T("Path");
	item.iSubItem = 1;
	m_wndList.InsertColumn(1, &item);
	item.fmt = LVCFMT_RIGHT;
	item.cx = (int)(rt.Width() * 0.14);
	item.pszText = _T("Size");
	item.iSubItem = 2;
	m_wndList.InsertColumn(2, &item);
	m_wndList.GetHeaderCtrl().SetFont(&afxGlobalData.fontRegular);
	m_wndList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);

	if (m_pDlgProgress) { delete m_pDlgProgress; m_pDlgProgress = NULL; }
	m_pDlgProgress = new CDlgPrgress;
	m_pDlgProgress->Create(CDlgPrgress::IDD);
	MoveWindow(FALSE);
}

void CWinSFXMakerDlg::OnWindowPosChanged(WINDOWPOS* lpwndpos) { MoveWindow(FALSE); CDialogEx::OnWindowPosChanged(lpwndpos); }

void CWinSFXMakerDlg::MoveWindow(BOOL bShow)
{
	CRect rtScreen;
	GetClientRect(&m_rtTab);
	rtScreen.CopyRect(m_rtTab);
	this->ClientToScreen(rtScreen);
	DWORD dwFlag = bShow ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
	if (m_pDlgProgress && m_pDlgProgress->GetSafeHwnd())
		m_pDlgProgress->SetWindowPos(&wndTop, rtScreen.left, rtScreen.top, m_rtTab.right, m_rtTab.bottom, dwFlag);
}

HBRUSH CWinSFXMakerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	HWND hWnd = pWnd->GetSafeHwnd();
	if (GetDlgItem(IDOK)->GetSafeHwnd() == hWnd || GetDlgItem(IDCANCEL)->GetSafeHwnd() == hWnd)
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	return hbr;
}

void CWinSFXMakerDlg::OnBnClickedButtonPath()
{
	KillTimer(IDT_UPDATE_SCREEN);
	CWaitCursor cw;
	m_strInputPath.Empty();
	GetDlgItem(IDC_COMBO_INPUT_PATH)->SetWindowText(m_strInputPath);
	CDlgShellTreeExplore dlg;
	if (IDOK != dlg.DoModal()) return;
	m_strInputPath = dlg.GetPath();
	if (m_strInputPath.IsEmpty()) return;
	EnableWindow(FALSE);
	ShowResultWnd(TRUE);
	SetTimer(IDT_UPDATE_SCREEN, 10, NULL);
	GetDlgItem(IDC_COMBO_INPUT_PATH)->SetWindowText(m_strInputPath);
	BeginFindFiles(m_strInputPath);
}

void CWinSFXMakerDlg::ShowResultWnd(BOOL bShow)
{
	if (!m_pDlgProgress) { m_pDlgProgress = new CDlgPrgress(); m_pDlgProgress->Create(IDD_PROGRESS, this); }
	if (bShow) { m_pDlgProgress->InitProgessRange(0); m_pDlgProgress->ShowWindow(SW_SHOW); }
	else { m_pDlgProgress->ShowWindow(SW_HIDE); return; }
	CRect rt;
	GetClientRect(&rt);
	ClientToScreen(&rt);
	m_pDlgProgress->MoveWindow(rt);
}

void CWinSFXMakerDlg::OnTimer(UINT_PTR nIDEvent) { CDialogEx::OnTimer(nIDEvent); }
void CWinSFXMakerDlg::UpdateResult() { if (m_pDlgProgress && m_pDlgProgress->GetSafeHwnd()) m_pDlgProgress->UpdateWindow(); }

void CWinSFXMakerDlg::UpdateControls()
{
	CString strPath;
	GetDlgItemText(IDC_COMBO_INPUT_PATH, strPath);
	if (strPath.IsEmpty() || !PathIsDirectory(strPath) || PathIsRoot(strPath))
	{
		GetDlgItem(IDC_COMBO_EXECUTABLE_PATH)->EnableWindow(FALSE);
		GetDlgItem(IDC_LIST_ARCHIVE)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_COMBO_EXECUTABLE_PATH)->EnableWindow(TRUE);
		GetDlgItem(IDC_LIST_ARCHIVE)->EnableWindow(TRUE);
	}

	if (m_wndList.GetItemCount() <= 0 || m_wndList.IsWindowEnabled() == FALSE)
	{
		GetDlgItem(IDC_COMBO_OUTPUT_PATH)->EnableWindow(FALSE);
		GetDlgItem(ID_BUTTON_OUPUT_PATH)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_PRODUCT_NAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_VERSION)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_COPYRIGHT)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_DESCRIPT)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC_MAIN_ICON)->EnableWindow(FALSE);
		CWnd* pBtnIcon = GetDlgItem(IDC_BUTTON_ICON);
		if (pBtnIcon) pBtnIcon->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_COMBO_OUTPUT_PATH)->EnableWindow(TRUE);
		GetDlgItem(ID_BUTTON_OUPUT_PATH)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_PRODUCT_NAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_VERSION)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_COPYRIGHT)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_DESCRIPT)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_MAIN_ICON)->EnableWindow(TRUE);
		CWnd* pBtnIcon = GetDlgItem(IDC_BUTTON_ICON);
		if (pBtnIcon) pBtnIcon->EnableWindow(TRUE);
		GetDlgItemText(IDC_COMBO_OUTPUT_PATH, m_strOutputPath);
	}

	strPath.Empty();
	GetDlgItemText(IDC_COMBO_OUTPUT_PATH, strPath);
	if (strPath.IsEmpty()) GetDlgItem(IDOK)->EnableWindow(FALSE);
	else GetDlgItem(IDOK)->EnableWindow(TRUE);
}

void CWinSFXMakerDlg::CastByte(CString& strValue)
{
	DWORD dwSize = _ttoi(strValue);
	if (dwSize < 1024) { strValue.Format(_T("%d B"), dwSize); }
	else if (dwSize < 1048576) { strValue.Format(_T("%0.1f KB"), dwSize / 1024.0); }
	else if (dwSize < 1073741824.0) { strValue.Format(_T("%0.2f MB"), dwSize / 1048576.0); }
	else { strValue.Format(_T("%0.2f GB"), dwSize / 1073741824.0); }
}

void CWinSFXMakerDlg::FindFiles(CString strPath)
{
	if (strPath.IsEmpty() || m_bTaskFinish) return;
	PathAddBackslash(strPath.GetBuffer(BUFSIZ));
	strPath.ReleaseBuffer();
	strPath.Append(_T("*.*"));

	CFileFind ff;
	BOOL bContinue = ff.FindFile(strPath);
	CFileInfo* pItem = NULL;
	while (bContinue)
	{
		if (m_bTaskFinish) return;
		bContinue = ff.FindNextFile();
		if (ff.IsDots()) continue;
		if (ff.IsDirectory()) { FindFiles(ff.GetFilePath()); continue; }
		pItem = new CFileInfo;
		if (!pItem) continue;
		pItem->m_strPath = ff.GetFilePath();
		pItem->m_strFileName = ff.GetFileName();
		LPTSTR lpszExtension = PathFindExtension(pItem->m_strFileName);
		if (lpszExtension != NULL && (_tcsicmp(lpszExtension, _T(".exe")) == 0 || _tcsicmp(lpszExtension, _T(".bat")) == 0 || _tcsicmp(lpszExtension, _T(".com")) == 0))
			m_arPeFiles.Add(pItem->m_strPath);
		HANDLE hFile = CreateFile(pItem->m_strPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE) { pItem->m_dwSize = GetFileSize(hFile, NULL); CloseHandle(hFile); }
		m_arFiles.Add(pItem);
	}
}

void CWinSFXMakerDlg::InitFileInfo()
{
	CFileInfo* pItem = NULL;
	int nCnt;
	for (nCnt = 0; nCnt < (int)m_arFiles.GetCount(); nCnt++)
	{
		pItem = (CFileInfo*)m_arFiles.GetAt(nCnt);
		if (pItem) { delete pItem; pItem = NULL; }
	}
	m_arFiles.RemoveAll();
	m_arPeFiles.RemoveAll();
}

void CWinSFXMakerDlg::AddFiles()
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_EXECUTABLE_PATH);
	if (pCombo) { pCombo->ResetContent(); pCombo->AddString(_T("")); }
	if (m_pDlgProgress && m_pDlgProgress->GetSafeHwnd()) m_pDlgProgress->InitProgessRange((int)m_arFiles.GetCount());
	if (!m_wndList.GetSafeHwnd()) return;
	m_wndList.DeleteAllItems();

	CString strBuffer;
	DWORD dwColumn = 0;
	CFileInfo* pItem = NULL;
	int nCnt;
	for (nCnt = 0; nCnt < (int)m_arFiles.GetCount(); nCnt++)
	{
		if (m_bTaskFinish) return;
		pItem = (CFileInfo*)m_arFiles.GetAt(nCnt);
		if (!pItem) continue;
		dwColumn = 0;
		LVITEM item;
		item.iItem = nCnt;
		item.iSubItem = dwColumn;
		item.pszText = (LPTSTR)(LPCTSTR)pItem->m_strFileName;
		item.mask = LVIF_TEXT;
		if (m_wndList.GetSafeHwnd()) m_wndList.InsertItem(&item);
		if (m_pDlgProgress && m_pDlgProgress->GetSafeHwnd()) { m_pDlgProgress->ProgressStepIt(); m_pDlgProgress->UpdateText(pItem->m_strFileName); }
		strBuffer = pItem->m_strPath;
		PathRemoveFileSpec(strBuffer.GetBuffer(BUFSIZ));
		strBuffer.ReleaseBuffer();
		item.iSubItem = ++dwColumn;
		item.pszText = (LPTSTR)(LPCTSTR)strBuffer;
		if (m_wndList.GetSafeHwnd()) m_wndList.SetItem(&item);
		item.iSubItem = ++dwColumn;
		strBuffer.Format(_T("%d"), pItem->m_dwSize);
		CastByte(strBuffer);
		item.pszText = (LPTSTR)(LPCTSTR)strBuffer;
		if (m_wndList.GetSafeHwnd()) m_wndList.SetItem(&item);
		if (m_bTaskFinish) return;
	}

	for (nCnt = 0; nCnt < (int)m_arPeFiles.GetCount(); nCnt++)
	{
		if (m_bTaskFinish) return;
		strBuffer = m_arPeFiles.GetAt(nCnt);
		if (strBuffer.IsEmpty()) continue;
		if (pCombo) pCombo->AddString(strBuffer);
		if (m_bTaskFinish) return;
	}
	
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_OUTPUT_PATH);
	if (pCombo == NULL || !pCombo->GetSafeHwnd()) return;
	strBuffer = m_strInputPath;
	PathAddBackslash(strBuffer.GetBuffer(BUFSIZ));
	strBuffer.ReleaseBuffer();
	strBuffer.Append(PathFindFileName(m_strInputPath));
	PathAddExtension(strBuffer.GetBuffer(BUFSIZ), _T(".exe"));
	strBuffer.ReleaseBuffer();
	pCombo->AddString(strBuffer);
	pCombo->SetCurSel(0);
}

INT32 CWinSFXMakerDlg::Initialize() { return 0; }
INT32 CWinSFXMakerDlg::UnInitialize() { return 0; }

void CWinSFXMakerDlg::OnBnClickedOk()
{
	// Set parameters
	m_sfxParam.strInputPath = m_strInputPath;
	GetDlgItemText(IDC_COMBO_OUTPUT_PATH, m_sfxParam.strOutputPath);
	m_sfxParam.strIconPath = m_strIconPath;
	m_sfxParam.strCompanyName = _T("");
	GetDlgItemText(IDC_EDIT_PRODUCT_NAME, m_sfxParam.strProductName);
	GetDlgItemText(IDC_EDIT_VERSION, m_sfxParam.strVersion);
	GetDlgItemText(IDC_EDIT_COPYRIGHT, m_sfxParam.strCopyright);
	GetDlgItemText(IDC_EDIT_DESCRIPT, m_sfxParam.strDescription);
	m_sfxParam.hWndParent = GetSafeHwnd();
	m_sfxParam.bSuccess = FALSE;

	// Create progress dialog
	if (m_pSFXProgressDlg == NULL)
	{
		m_pSFXProgressDlg = new CDlgSFXProgress(this);
		m_pSFXProgressDlg->Create(IDD_SFX_PROGRESS, this);
	}
	m_sfxParam.pProgressDlg = m_pSFXProgressDlg;
	m_pSFXProgressDlg->SetProgress(0);
	m_pSFXProgressDlg->SetStatus(_T("Preparing..."));
	m_pSFXProgressDlg->m_bCancel = FALSE;
	
	// Resize progress dialog to match parent window
	CRect rtParent;
	GetWindowRect(&rtParent);
	m_pSFXProgressDlg->SetWindowPos(&wndTop, rtParent.left, rtParent.top, rtParent.Width(), rtParent.Height(), SWP_SHOWWINDOW);
	
	EnableWindow(FALSE);

	// Start thread
	m_pSFXThread = AfxBeginThread(TaskCreateSFXFunc, &m_sfxParam);
}

LRESULT CWinSFXMakerDlg::OnSFXComplete(WPARAM wParam, LPARAM lParam)
{
	EnableWindow(TRUE);
	if (m_pSFXProgressDlg) m_pSFXProgressDlg->ShowWindow(SW_HIDE);

	BOOL bSuccess = (BOOL)wParam;
	if (bSuccess)
	{
		CString strMsg;
		strMsg.Format(_T("SFX file created successfully.\n\nDo you want to open the folder containing the file?"));
		if (AfxMessageBox(strMsg, MB_ICONINFORMATION | MB_YESNO) == IDYES)
		{
			// Open explorer and select the file
			CString strParam;
			strParam.Format(_T("/select,\"%s\""), m_sfxParam.strOutputPath);
			ShellExecute(NULL, _T("open"), _T("explorer.exe"), strParam, NULL, SW_SHOWNORMAL);
		}
	}
	else if (!m_pSFXProgressDlg || !m_pSFXProgressDlg->IsCancelled())
		AfxMessageBox(_T("Failed to create SFX file."), MB_ICONERROR);

	return 0;
}

void CWinSFXMakerDlg::OnBnClickedButtonOuputPath()
{
	CWaitCursor cw;
	m_strOutputPath.Empty();
	GetDlgItem(IDC_COMBO_OUTPUT_PATH)->SetWindowText(m_strOutputPath);
	CFileDialog dlg(FALSE, _T("exe"), _T("*.exe"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("Executable File(*.exe)|*.exe|All Files(*.*)|*.*||"));
	TCHAR szPath[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH - 1, szPath);
	dlg.m_ofn.lpstrInitialDir = szPath;
	if (IDOK != dlg.DoModal()) { UpdateControls(); return; }
	m_strOutputPath = dlg.GetPathName();
	if (m_strOutputPath.IsEmpty()) { UpdateControls(); return; }
	GetDlgItem(IDC_COMBO_OUTPUT_PATH)->SetWindowText(m_strOutputPath);
	UpdateControls();
}

void CWinSFXMakerDlg::OnCbnEditchangeComboInputPath() { UpdateControls(); }
void CWinSFXMakerDlg::OnLvnItemchangedListArchive(NMHDR *pNMHDR, LRESULT *pResult) { UpdateControls(); *pResult = 0; }
void CWinSFXMakerDlg::OnCbnEditchangeComboOutputPath() { UpdateControls(); }
