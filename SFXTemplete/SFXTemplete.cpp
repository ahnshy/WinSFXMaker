
// SFXTemplete.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "SFXTemplete.h"

#include <fstream>
#include <vector>

#include "Helper/ZipHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSFXTempleteApp
BEGIN_MESSAGE_MAP(CSFXTempleteApp, CWinApp)
	//ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// CSFXTempleteApp construction
CSFXTempleteApp::CSFXTempleteApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CSFXTempleteApp object
CSFXTempleteApp theApp;

// CSFXTempleteApp initialization
BOOL CSFXTempleteApp::InitInstance()
{
	CWinApp::InitInstance();

	//if (Parse(m_lpCmdLine) == 1)
	//{
		// to do...
	//}

	SetRegistryKey(_T("SFX Templete App"));

	CreateSFX();

	return FALSE;
}

void CSFXTempleteApp::CreateSFX()
{
	// ���� ������ ������ ��� ���� (LPCTSTR)
	LPCTSTR outputPath = _T("extracted_data.zip");

	// CreateZipFile �Լ� ȣ��
	ExtractEmbeddedZip(outputPath);
	//if (ExtractEmbeddedZip(outputPath))
	//	AfxMessageBox(_T("���� ���� ���� ����."));
	//else
	//	AfxMessageBox(_T("���� ���� ���⿡ �����߽��ϴ�."));
}

BOOL CSFXTempleteApp::ExtractEmbeddedZip(LPCTSTR outputPath)
{
	// ���� ���� ���� ��� ���ϱ�
	TCHAR exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH);

	// ���� ������ ���̳ʸ� ���� ����
	std::ifstream exeFile(exePath, std::ios::binary | std::ios::ate);
	if (!exeFile) {
		AfxMessageBox(_T("���� ������ �� �� �����ϴ�."));
		return FALSE;
	}

	// ���� ũ�� �� ���� ������ ũ�� �б�
	std::streamsize exeSize = exeFile.tellg();
	exeFile.seekg(exeSize - sizeof(std::streamsize), std::ios::beg);

	std::streamsize zipSize;
	exeFile.read(reinterpret_cast<char*>(&zipSize), sizeof(zipSize));

	// ���� ������ ��ġ�� �̵��Ͽ� �б�
	exeFile.seekg(exeSize - zipSize - sizeof(zipSize), std::ios::beg);
	if (zipSize <= 0)
	{
		exeFile.close();
		return FALSE;
	}

	std::vector<char> zipData((int)zipSize);
	exeFile.read(zipData.data(), zipSize);
	exeFile.close();

	// ���� ������ outputPath�� ����
	std::ofstream outFile(outputPath, std::ios::binary);
	if (!outFile) {
		AfxMessageBox(_T("���� ������ ������ �� �����ϴ�."));
		return FALSE;
	}
	outFile.write(zipData.data(), zipSize);
	outFile.close();

	CString msg;
	msg.Format(_T("���� ������ ����Ǿ����ϴ�: %s"), outputPath);
	AfxMessageBox(msg);

	return TRUE;
}