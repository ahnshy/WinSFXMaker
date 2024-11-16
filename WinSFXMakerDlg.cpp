
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
	DDX_Control(pDX, IDC_LIST_ARCHIVE, m_listFiles);
}

BEGIN_MESSAGE_MAP(CWinSFXMakerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CWinSFXMakerDlg::OnBnClickedOk)
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

	m_listFiles.SetBkColor(RGB(0xe0, 0xff, 0xff));
	m_listFiles.SetTextBkColor(RGB(0xe0, 0xff, 0xff));
	m_listFiles.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES);

	m_listFiles.InsertColumn(0, _T("��ü ���"), LVCFMT_LEFT, 300, -1);

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
	// ���ҽ����� ���̳ʸ� �����͸� �б� ���� �۾�

	// ���ҽ� �ڵ� ���
	HRSRC hRes = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_TEMPLATE_BINARY), _T("BINARY"));
	if (hRes == NULL)
	{
		AfxMessageBox(_T("���ҽ��� ã�� �� �����ϴ�."));
		return;
	}

	// ���ҽ� ũ�� ���ϱ�
	DWORD dwSize = SizeofResource(nullptr, hRes);
	if (dwSize == 0)
	{
		AfxMessageBox(_T("���ҽ� ũ�⸦ �� �� �����ϴ�."));
		return;
	}

	// ���ҽ��� �޸𸮿� �ε��ϱ�
	HGLOBAL hGlobal = LoadResource(nullptr, hRes);
	if (hGlobal == nullptr)
	{
		AfxMessageBox(_T("���ҽ��� �޸𸮷� �ε��� �� �����ϴ�."));
		return;
	}

	// �޸𸮿��� �����͸� �б� ���� ������ ���
	LPVOID pData = LockResource(hGlobal);
	if (pData == nullptr)
	{
		AfxMessageBox(_T("���ҽ� �����͸� ��� �� �����ϴ�."));
		return;
	}

	// ���� ���� ��ȭ���ڸ� ���� ���� ��� �ޱ�
	{
		//CString filePath = fileDlg.GetPathName();

		// ���õ� ���Ͽ� ���̳ʸ� ������ ����
		CFile file;
		if (file.Open(strPath, CFile::modeCreate | CFile::modeWrite))
		{
			file.Write(pData, dwSize);
			file.Close();
			AfxMessageBox(_T("���� ������ �Ϸ�Ǿ����ϴ�."));
		}
		else
		{
			AfxMessageBox(_T("������ ������ �� �����ϴ�."));
		}
	}

	// ���ҽ� ��� ����
	UnlockResource(hGlobal);
}

// SFX ���ø� ���ϰ� 7z ������ ���� ���ο� SFX ����
BOOL CWinSFXMakerDlg::CreateSFXFile(LPCTSTR templatePath, LPCTSTR zipPath, LPCTSTR outputPath) {
	// ���ø� ���� �б�
	std::ifstream templateFile(templatePath, std::ios::binary | std::ios::ate);
	if (!templateFile) {
		AfxMessageBox(_T("SFX ���ø� ������ �� �� �����ϴ�."));
		return FALSE;
	}
	std::streamsize templateSize = templateFile.tellg();
	templateFile.seekg(0, std::ios::beg);
	std::vector<char> templateData((int)templateSize);
	templateFile.read(templateData.data(), templateSize);
	templateFile.close();

	// ZIP ���� �б�
	std::ifstream zipFile(zipPath, std::ios::binary | std::ios::ate);
	if (!zipFile) {
		AfxMessageBox(_T("ZIP ������ �� �� �����ϴ�."));
		return FALSE;
	}
	std::streamsize zipSize = (UINT)zipFile.tellg();
	zipFile.seekg(0, std::ios::beg);
	std::vector<char> zipData((int)zipSize);
	zipFile.read(zipData.data(), zipSize);
	zipFile.close();

	// ���ø� ���� + ZIP ���� + ZIP ũ�� ����Ͽ� ���ο� SFX ���� ����
	std::ofstream outFile(outputPath, std::ios::binary);
	if (!outFile) {
		AfxMessageBox(_T("��� SFX ������ ������ �� �����ϴ�."));
		return FALSE;
	}
	outFile.write(templateData.data(), templateSize);
	outFile.write(zipData.data(), zipSize);
	outFile.write(reinterpret_cast<char*>(&zipSize), sizeof(zipSize));  // ZIP ���� ũ�� ���
	outFile.close();

	CString msg;
	msg.Format(_T("���ο� SFX ������ �����Ǿ����ϴ�: %s"), outputPath);
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