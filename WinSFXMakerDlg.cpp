
// WinSFXMakerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WinSFXMaker.h"
#include "WinSFXMakerDlg.h"
#include "afxdialogex.h"

#include <fstream>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int CFileListCtrl::OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn)
{
		return 0;
}

COLORREF CFileListCtrl::OnGetCellBkColor(int nRow, int nColum)
{
	//if (nColum == m_iSortedColumn)
	//{
	//	//return(nRow % 2) == 0 ? RGB(233, 221, 229) : RGB(176, 218, 234);
	//}

	//return(nRow % 2) == 0 ? RGB(253, 241, 249) : RGB(196, 238, 254);
	return(nRow % 2) == 0 ? RGB(241, 244, 247) : RGB(250, 250, 250);
	//return(nRow % 2) == 0 ? RGB(250, 250, 250) : RGB(223, 230, 238);
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
}

void CWinSFXMakerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ARCHIVE, m_wndList);
}

BEGIN_MESSAGE_MAP(CWinSFXMakerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CWinSFXMakerDlg::OnBnClickedOk)
	ON_WM_CTLCOLOR()
	ON_WM_WINDOWPOSCHANGED()
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

	Initialize();

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

// SFX 템플릿 파일과 7z 파일을 합쳐 새로운 SFX 생성
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

void CWinSFXMakerDlg::OnBnClickedOk()
{

	//CString str = _T("C:\\SFXTemplate.exe");
	//SaveResourceToFile(str);


	//CreateSFXFile(_T("F:\\MFC\\WinSFXMaker\\SfxTemplate.exe"), _T("F:\\MFC\\WinSFXMaker\\bin.zip"), _T("F:\\MFC\\WinSFXMaker\\1.exe"));

	CDialogEx::OnOK();
}

void CWinSFXMakerDlg::Initialize()
{
	CRect rt;
	GetClientRect(&rt);
	//m_wndList.SetWindowPos(&wndTop, rt.left, rt.top, rt.right, rt.bottom -300, SWP_SHOWWINDOW);

	LVCOLUMN item;
	item.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	item.fmt = LVCFMT_LEFT;
	item.cx = int(rt.Width() * 0.28);
	item.pszText = _T("File");
	item.iSubItem = 0;
	m_wndList.InsertColumn(0, &item);

	item.cx = int(rt.Width() * 0.55);
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
}

void CWinSFXMakerDlg::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CDialogEx::OnWindowPosChanged(lpwndpos);
}

void CWinSFXMakerDlg::MoveWindow()
{
}

HBRUSH CWinSFXMakerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	HWND hWnd = pWnd->GetSafeHwnd();
	if (GetDlgItem(IDOK)->GetSafeHwnd() == hWnd || GetDlgItem(IDCANCEL)->GetSafeHwnd() == hWnd)
		return (HBRUSH)GetStockObject(NULL_BRUSH);

	return hbr;
}