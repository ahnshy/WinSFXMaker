
// WinSFXMakerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WinSFXMaker.h"
#include "WinSFXMakerDlg.h"
#include "afxdialogex.h"

#include <fstream>
#include <vector>

#include "Helper/FileHelper.h"
#include "Helper/DirectoryHelper.h"
#include "Helper/ZipHelper.h"
#include "Dialog/DlgShellTreeExplore.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT TaskFindFilesFunc(LPVOID pParam)
{
	CWinSFXMakerDlg *pDlg = (CWinSFXMakerDlg*)pParam;
	if (pDlg == NULL)
		return 0;

	pDlg->EnableWindow(FALSE);
	pDlg->InitFileInfo();
	pDlg->FindFiles(pDlg->GetInputPath());
	pDlg->AddFiles();

	pDlg->UpdateControls();
	pDlg->EnableWindow(TRUE);
	pDlg->ShowResultWnd(FALSE);

	return 1;
}

BOOL CWinSFXMakerDlg::BeginFindFiles(CString strPath)
{
	if (strPath.IsEmpty())
		return FALSE;

	if (m_pThread)
		m_pThread = NULL;

	m_pThread = (CWinThread*)AfxBeginThread(TaskFindFilesFunc, (LPVOID)this);
	return TRUE;
}

int CFileListCtrl::OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn)
{
		return 0;
}

COLORREF CFileListCtrl::OnGetCellBkColor(int nRow, int nColum)
{
	//if (nColum == m_iSortedColumn)
	//{
		//return(nRow % 2) == 0 ? RGB(233, 221, 229) : RGB(176, 218, 234);
	//}

	//return(nRow % 2) == 0 ? RGB(253, 241, 249) : RGB(196, 238, 254);
	//return(nRow % 2) == 0 ? RGB(241, 244, 247) : RGB(250, 250, 250);
	return(nRow % 2) == 0 ? RGB(249, 249, 249) : RGB(223, 230, 238);
}

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CWinSFXMakerDlg dialog



CWinSFXMakerDlg::CWinSFXMakerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWinSFXMakerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	m_rtTab.SetRectEmpty();
	m_pDlgProgress = NULL;

	m_bTaskFinish = FALSE;
	m_pThread = NULL;
	m_strInputPath.Empty();
	m_strOutputPath.Empty();

	InitFileInfo();
}

CWinSFXMakerDlg::~CWinSFXMakerDlg()
{
	InitFileInfo();
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
END_MESSAGE_MAP()


// CWinSFXMakerDlg message handlers

BOOL CWinSFXMakerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	SetBackgroundColor(RGB(255, 255, 255));

	//m_listFiles.SetBkColor(RGB(0xe0, 0xff, 0xff));
	//m_listFiles.SetTextBkColor(RGB(0xe0, 0xff, 0xff));
	//m_listFiles.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES);

	InitControls();
	UpdateControls();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWinSFXMakerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWinSFXMakerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWinSFXMakerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CWinSFXMakerDlg::SaveResourceToFile(CString& strPath)
{
	// 리소스에서 바이너리 데이터를 읽기 위한 작업

	// 리소스 핸들 얻기
	HRSRC hRes = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_TEMPLATE_BINARY), _T("BINARY"));
	if (hRes == NULL)
	{
		AfxMessageBox(_T("리소스를 찾을 수 없습니다."));
		return;
	}

	// 리소스 크기 구하기
	DWORD dwSize = SizeofResource(nullptr, hRes);
	if (dwSize == 0)
	{
		AfxMessageBox(_T("리소스 크기를 알 수 없습니다."));
		return;
	}

	// 리소스를 메모리에 로드하기
	HGLOBAL hGlobal = LoadResource(nullptr, hRes);
	if (hGlobal == nullptr)
	{
		AfxMessageBox(_T("리소스를 메모리로 로드할 수 없습니다."));
		return;
	}

	// 메모리에서 데이터를 읽기 위한 포인터 얻기
	LPVOID pData = LockResource(hGlobal);
	if (pData == nullptr)
	{
		AfxMessageBox(_T("리소스 데이터를 잠글 수 없습니다."));
		return;
	}

	// 파일 저장 대화상자를 열어 파일 경로 받기
	{
		//CString filePath = fileDlg.GetPathName();

		// 선택된 파일에 바이너리 데이터 저장
		CFile file;
		if (file.Open(strPath, CFile::modeCreate | CFile::modeWrite))
		{
			file.Write(pData, dwSize);
			file.Close();
			AfxMessageBox(_T("파일 저장이 완료되었습니다."));
		}
		else
		{
			AfxMessageBox(_T("파일을 저장할 수 없습니다."));
		}
	}

	// 리소스 잠금 해제
	UnlockResource(hGlobal);
}

BOOL CWinSFXMakerDlg::CreateSFXFile(LPCTSTR templatePath, LPCTSTR zipPath, LPCTSTR outputPath) {
	// 템플릿 파일 읽기
	std::ifstream templateFile(templatePath, std::ios::binary | std::ios::ate);
	if (!templateFile) {
		AfxMessageBox(_T("SFX 템플릿 파일을 열 수 없습니다."));
		return FALSE;
	}
	std::streamsize templateSize = templateFile.tellg();
	templateFile.seekg(0, std::ios::beg);
	std::vector<char> templateData((int)templateSize);
	templateFile.read(templateData.data(), templateSize);
	templateFile.close();

	// ZIP 파일 읽기
	std::ifstream zipFile(zipPath, std::ios::binary | std::ios::ate);
	if (!zipFile) {
		AfxMessageBox(_T("ZIP 파일을 열 수 없습니다."));
		return FALSE;
	}
	std::streamsize zipSize = (UINT)zipFile.tellg();
	zipFile.seekg(0, std::ios::beg);
	std::vector<char> zipData((int)zipSize);
	zipFile.read(zipData.data(), zipSize);
	zipFile.close();

	// 템플릿 파일 + ZIP 파일 + ZIP 크기 기록하여 새로운 SFX 파일 생성
	std::ofstream outFile(outputPath, std::ios::binary);
	if (!outFile) {
		AfxMessageBox(_T("출력 SFX 파일을 생성할 수 없습니다."));
		return FALSE;
	}
	outFile.write(templateData.data(), templateSize);
	outFile.write(zipData.data(), zipSize);
	outFile.write(reinterpret_cast<char*>(&zipSize), sizeof(zipSize));  // ZIP 파일 크기 기록
	outFile.close();

	CString msg;
	msg.Format(_T("새로운 SFX 파일이 생성되었습니다: %s"), outputPath);
	AfxMessageBox(msg);

	return TRUE;
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
	//m_wndList.SetWindowPos(&wndTop, rt.left, rt.top, rt.right, rt.bottom -300, SWP_SHOWWINDOW);

	LVCOLUMN item;
	item.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	item.fmt = LVCFMT_LEFT;
	item.cx = int(rt.Width() * 0.25);
	item.pszText = _T("File");
	item.iSubItem = 0;
	m_wndList.InsertColumn(0, &item);

	item.cx = int(rt.Width() * 0.52);
	item.pszText = _T("Path");
	item.iSubItem = 1;
	m_wndList.InsertColumn(1, &item);

	item.fmt = LVCFMT_RIGHT;
	item.cx = int(rt.Width() * 0.14);
	item.pszText = _T("Size");
	item.iSubItem = 2;
	m_wndList.InsertColumn(2, &item);

	m_wndList.GetHeaderCtrl().SetFont(&afxGlobalData.fontRegular);
	m_wndList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);

	if (m_pDlgProgress)
	{
		delete m_pDlgProgress;
		m_pDlgProgress = NULL;
	}

	m_pDlgProgress = new CDlgPrgress;
	m_pDlgProgress->Create(CDlgPrgress::IDD);
	
	//CRect rtScreen;
	//GetClientRect(&m_rtTab);
	//rtScreen.CopyRect(m_rtTab);
	//this->ClientToScreen(rtScreen);
	//m_pDlgProgress->SetWindowPos(&wndTop, rtScreen.left, rtScreen.top, m_rtTab.right, m_rtTab.bottom, SWP_HIDEWINDOW);
	MoveWindow(FALSE);
}

void CWinSFXMakerDlg::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	MoveWindow(FALSE);
	CDialogEx::OnWindowPosChanged(lpwndpos);
}

void CWinSFXMakerDlg::MoveWindow(BOOL bShow)
{
	CRect rtScreen;
	GetClientRect(&m_rtTab);
	rtScreen.CopyRect(m_rtTab);
	this->ClientToScreen(rtScreen);

	DWORD dwFlag = SWP_HIDEWINDOW;
	if (bShow)
		dwFlag = SWP_SHOWWINDOW;

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
	if (IDOK != dlg.DoModal())
		return;

	m_strInputPath = dlg.GetPath();
	if (m_strInputPath.IsEmpty())
		return;

	EnableWindow(FALSE);
	ShowResultWnd(TRUE);
	SetTimer(IDT_UPDATE_SCREEN, 5, NULL);
	GetDlgItem(IDC_COMBO_INPUT_PATH)->SetWindowText(m_strInputPath);
	BeginFindFiles(m_strInputPath);
}

void CWinSFXMakerDlg::ShowResultWnd(BOOL bShow)
{
	if (!m_pDlgProgress)
	{
		m_pDlgProgress = new CDlgPrgress();
		m_pDlgProgress->Create(IDD_PROGRESS, this);
	}

	if (bShow)
		m_pDlgProgress->ShowWindow(SW_SHOW);
	else
	{
		m_pDlgProgress->ShowWindow(SW_HIDE);
		return;
	}

	CRect rt;
	GetClientRect(&rt);
	ClientToScreen(&rt);
	m_pDlgProgress->MoveWindow(rt);
}

void CWinSFXMakerDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == IDT_UPDATE_SCREEN)
		UpdateResult();

	CDialogEx::OnTimer(nIDEvent);
}

void CWinSFXMakerDlg::UpdateResult()
{
}

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

		GetDlgItem(IDC_EDIT_COMPANY_NAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_PRODUCT_NAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_VERSION)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_COPYRIGHT)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_DESCRIPT)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC_MAIN_ICON)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_COMBO_OUTPUT_PATH)->EnableWindow(TRUE);
		GetDlgItem(ID_BUTTON_OUPUT_PATH)->EnableWindow(TRUE);

		GetDlgItem(IDC_EDIT_COMPANY_NAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_PRODUCT_NAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_VERSION)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_COPYRIGHT)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_DESCRIPT)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_MAIN_ICON)->EnableWindow(TRUE);
	}

	strPath.Empty();
	GetDlgItemText(IDC_COMBO_OUTPUT_PATH, strPath);

	if (strPath.IsEmpty())
		GetDlgItem(IDOK)->EnableWindow(FALSE);
	else
		GetDlgItem(IDOK)->EnableWindow(TRUE);
}

void CWinSFXMakerDlg::CastByte(CString& strValue)
{
	DWORD dwSize = _ttoi(strValue);
	if (dwSize < 1024)
	{
		CString strBytes;
		strBytes.Format(_T("%d"), dwSize);
		strValue.Format(_T("%s B"), strBytes);
	}
	else if (dwSize < 1048576)
	{
		CString strKiloBytes;
		strKiloBytes.Format(_T("%0.1f"), dwSize / 1024.0);
		strValue.Format(_T("%s KB"), strKiloBytes);
	}
	else if (dwSize < 1073741824.0)
	{
		CString strMegaBytes;
		strMegaBytes.Format(_T("%0.2f"), dwSize / 1048576.0);
		strValue.Format(_T("%s MB"), strMegaBytes);
	}
	else
	{
		CString strGigaBytes;
		strGigaBytes.Format(_T("%0.2f"), dwSize / 1073741824.0);
		strValue.Format(_T("%s GB"), strGigaBytes);
	}

}

void CWinSFXMakerDlg::FindFiles(CString strPath)
{
	if (strPath.IsEmpty() || m_bTaskFinish)
		return;

	if (m_bTaskFinish)
		return;

	PathAddBackslash(strPath.GetBuffer(BUFSIZ));
	strPath.ReleaseBuffer();
	strPath.Append(_T("*.*"));

	CTime ct;
	CFileFind ff;

	BOOL bContinue = ff.FindFile(strPath);
	CString strExt, strBuffer;
	CFileInfo* pItem = NULL;
	while (bContinue)
	{
		if (m_bTaskFinish)
			return;

		bContinue = ff.FindNextFile();
		if (ff.IsDots())
			continue;

		if (ff.IsDirectory())
		{
			FindFiles(ff.GetFilePath());
			continue;
		}

		pItem = new CFileInfo;
		if (!pItem)
			continue;

		pItem->m_strPath = ff.GetFilePath();
		pItem->m_strFileName = ff.GetFileName();

		LPTSTR lpszExtension = PathFindExtension(pItem->m_strFileName);
		if (lpszExtension != NULL && (_tcsicmp(lpszExtension, _T(".exe")) == 0 || _tcsicmp(lpszExtension, _T(".bat")) == 0 || _tcsicmp(lpszExtension, _T(".com")) == 0))
			m_arPeFiles.Add(pItem->m_strPath);

		HANDLE hFile = NULL;
		hFile = CreateFile(pItem->m_strPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			pItem->m_dwSize = GetFileSize(hFile, NULL);
			CloseHandle(hFile);
		}

		//ff.GetLastAccessTime(ct);
		//pItem->m_strModifiedTime = ct.Format(_T("%Y-%m-%d %p %I:%M"));

		m_arFiles.Add(pItem);
	}
}

void CWinSFXMakerDlg::InitFileInfo()
{
	CFileInfo* pItem = NULL;
	for (int nCnt = 0; nCnt < (int)m_arFiles.GetCount(); nCnt++)
	{
		pItem = (CFileInfo*)m_arFiles.GetAt(nCnt);
		if (!pItem)
			continue;

		delete pItem;
		pItem = NULL;
	}
	m_arFiles.RemoveAll();
	m_arPeFiles.RemoveAll();
}

void CWinSFXMakerDlg::AddFiles()
{
	if (!m_wndList.GetSafeHwnd())
		return;

	m_wndList.DeleteAllItems();

	CString strBuffer;
	DWORD dwColumn = 0;

	CFileInfo* pItem = NULL;
	for (int nCnt = 0; nCnt < (int)m_arFiles.GetCount(); nCnt++)
	{
		if (m_bTaskFinish)
			return;

		pItem = (CFileInfo*)m_arFiles.GetAt(nCnt);
		if (!pItem)
			continue;

		dwColumn = 0;

		LVITEM item;
		item.iItem = nCnt;
		item.iSubItem = dwColumn;
		item.pszText = (LPTSTR)(LPCTSTR)pItem->m_strFileName;
		item.mask = LVIF_TEXT;
		if (m_wndList.GetSafeHwnd())
			m_wndList.InsertItem(&item);

		strBuffer = pItem->m_strPath;
		PathRemoveFileSpec(strBuffer.GetBuffer(BUFSIZ));
		strBuffer.ReleaseBuffer();
		item.iSubItem = ++dwColumn;
		item.pszText = (LPTSTR)(LPCTSTR)strBuffer;
		if (m_wndList.GetSafeHwnd())
			m_wndList.SetItem(&item);

		item.iSubItem = ++dwColumn;
		strBuffer.Format(_T("%d"), pItem->m_dwSize);
		CastByte(strBuffer);
		item.pszText = (LPTSTR)(LPCTSTR)strBuffer;
		if (m_wndList.GetSafeHwnd())
			m_wndList.SetItem(&item);

		if (m_bTaskFinish)
			return;
	}

	//CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_OUTPUT_PATH);
	//if (pCombo == NULL || !pCombo->GetSafeHwnd())
	//	return;

	//pCombo->ResetContent();
	//for (int nIdnex = 0; nIdnex < m_arPeFiles.GetSize(); ++nIdnex)
	//{
	//	strBuffer = m_arPeFiles.GetAt(nIdnex);
	//	if (strBuffer.IsEmpty())
	//		continue;

	//	pCombo->AddString(strBuffer);
	//}
}

INT32 CWinSFXMakerDlg::Initialize()
{
	return 0;
}

INT32 CWinSFXMakerDlg::UnInitialize()
{
	return 0;
}

void CWinSFXMakerDlg::OnBnClickedOk()
{
	CZipHelper zipHelper;
	zipHelper.AddCompressTarget(m_strInputPath);

	CString strPath = CDirectoryHelper::GetTempPath();
	PathAddBackslash(strPath.GetBuffer(BUFSIZ));
	strPath.ReleaseBuffer();

	CString strZipFile = CFileHelper::GetTimeBaseFileName(strPath, _T("zip"));
	zipHelper.Compress(strZipFile);

	CString strTemplateFile = CFileHelper::GetTimeBaseFileName(strPath, _T("exe"));
	SaveResourceToFile(strTemplateFile);

	CreateSFXFile(strTemplateFile, strZipFile, m_strOutputPath);

	if (!PathIsRoot(strPath))
		CDirectoryHelper::ShellRemoveDirectory(strPath);


	//CStringArray filesToCompress;
	//CFileInfo* pItem = NULL;
	//for (int nCnt = 0; nCnt < (int)m_arFiles.GetCount(); nCnt++)
	//{
	//	pItem = (CFileInfo*)m_arFiles.GetAt(nCnt);
	//	if (!pItem)
	//		continue;

	//	filesToCompress.Add(pItem->m_strPath);
	//}


	//CreateSFXFile(_T("F:\\MFC\\WinSFXMaker\\SfxTemplate.exe"), _T("F:\\MFC\\WinSFXMaker\\bin.zip"), _T("F:\\MFC\\WinSFXMaker\\1.exe"));
	//CDialogEx::OnOK();
}


void CWinSFXMakerDlg::OnBnClickedButtonOuputPath()
{
	CWaitCursor cw;

	m_strOutputPath.Empty();
	GetDlgItem(IDC_COMBO_OUTPUT_PATH)->SetWindowText(m_strOutputPath);

	CFileDialog dlg(FALSE, _T("exe"), _T("*.exe"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("Executable File(*.exe)|*.exe|All Files(*.*)|*.*||"));

	TCHAR szPath[BUFSIZ] = { NULL, };
	GetCurrentDirectory(BUFSIZ - sizeof(TCHAR), szPath);
	dlg.m_ofn.lpstrInitialDir = szPath;
	
	if (IDOK != dlg.DoModal())
	{
		UpdateControls();
		return;
	}

	m_strOutputPath = dlg.GetPathName();
	if (m_strOutputPath.IsEmpty())
	{
		UpdateControls();
		return;
	}

	GetDlgItem(IDC_COMBO_OUTPUT_PATH)->SetWindowText(m_strOutputPath);
	UpdateControls();
}

void CWinSFXMakerDlg::OnCbnEditchangeComboInputPath()
{
	UpdateControls();
}

void CWinSFXMakerDlg::OnLvnItemchangedListArchive(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	UpdateControls();
	*pResult = 0;
}


void CWinSFXMakerDlg::OnCbnEditchangeComboOutputPath()
{
	// TODO: Add your control notification handler code here
	UpdateControls();
}
