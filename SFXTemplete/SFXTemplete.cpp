
// SFXTemplete.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "SFXTemplete.h"

#include <fstream>
#include <vector>

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

	SetRegistryKey(_T("SFX Templete App"));

	CreateSFX();

	return FALSE;
}

void CSFXTempleteApp::CreateSFX()
{
	// 압축 파일을 추출할 경로 설정 (LPCTSTR)
	LPCTSTR outputPath = _T("extracted_data.zip");

	// CreateZipFile 함수 호출
	ExtractEmbeddedZip(outputPath);
	//if (ExtractEmbeddedZip(outputPath))
	//	AfxMessageBox(_T("압축 파일 추출 성공."));
	//else
	//	AfxMessageBox(_T("압축 파일 추출에 실패했습니다."));
}

BOOL CSFXTempleteApp::ExtractEmbeddedZip(LPCTSTR outputPath)
{
	// 현재 실행 파일 경로 구하기
	TCHAR exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH);

	// 실행 파일을 바이너리 모드로 열기
	std::ifstream exeFile(exePath, std::ios::binary | std::ios::ate);
	if (!exeFile) {
		AfxMessageBox(_T("실행 파일을 열 수 없습니다."));
		return FALSE;
	}

	// 파일 크기 및 압축 데이터 크기 읽기
	std::streamsize exeSize = exeFile.tellg();
	exeFile.seekg(exeSize - sizeof(std::streamsize), std::ios::beg);

	std::streamsize zipSize;
	exeFile.read(reinterpret_cast<char*>(&zipSize), sizeof(zipSize));

	// 압축 데이터 위치로 이동하여 읽기
	exeFile.seekg(exeSize - zipSize - sizeof(zipSize), std::ios::beg);
	if (zipSize <= 0)
	{
		exeFile.close();
		return FALSE;
	}

	std::vector<char> zipData(zipSize);
	exeFile.read(zipData.data(), zipSize);
	exeFile.close();

	// 압축 파일을 outputPath에 저장
	std::ofstream outFile(outputPath, std::ios::binary);
	if (!outFile) {
		AfxMessageBox(_T("압축 파일을 저장할 수 없습니다."));
		return FALSE;
	}
	outFile.write(zipData.data(), zipSize);
	outFile.close();

	CString msg;
	msg.Format(_T("7z 파일이 추출되었습니다: %s"), outputPath);
	AfxMessageBox(msg);

	return TRUE;
}

INT32 CSFXTempleteApp::Parse(LPTSTR m_lpCmdLine)
{
	if (m_lpCmdLine == NULL || m_lpCmdLine[0] == NULL)
		return 0;

	{
		TCHAR seps[] = _T(" ");
		TCHAR* pToken = NULL;
		TCHAR* p = NULL;
		pToken = _tcstok(m_lpCmdLine, seps); // establish first token            
		while (pToken != NULL)
		{
			do
			{
				if (((p = _tcsstr(wcsupr(pToken), _T("/w"))) != NULL) || ((p = _tcsstr(wcsupr(pToken), _T("/W"))) != NULL))
				{

					//m_strPath = _tcstok(NULL, _T("\""));
					//PathAddBackslash(m_strPath.GetBuffer(512));
					//m_strPath.ReleaseBuffer();
					break;
				}

				if (((p = _tcsstr(wcsupr(pToken), _T("/p"))) != NULL) || ((p = _tcsstr(wcsupr(pToken), _T("/P"))) != NULL))
				{
					//m_strPatientID = _tcstok(NULL, _T("\""));
					break;
				}

				if (((p = _tcsstr(wcsupr(pToken), _T("/n"))) != NULL) || ((p = _tcsstr(wcsupr(pToken), _T("/N"))) != NULL))
				{
					//m_strName = _tcstok(NULL, _T("\""));
					break;
				}

				if (((p = _tcsstr(wcsupr(pToken), _T("/r"))) != NULL) || ((p = _tcsstr(wcsupr(pToken), _T("/R"))) != NULL))
				{
					//m_strRecordID = _tcstok(NULL, _T("\""));
					break;
				}

			} while (0);
			pToken = _tcstok(NULL, seps);
		}
	}

	//if (m_strPath.IsEmpty() || m_strPatientID.IsEmpty() || m_strName.IsEmpty() || m_strRecordID.IsEmpty())
	//	return 0;

	return 1;
}