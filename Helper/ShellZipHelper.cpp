#include "../stdafx.h"

#include <afx.h>
#include "ShellZipHelper.h"

CShellZipHelper::CShellZipHelper()
{
	CoInitialize(nullptr);
}

CShellZipHelper::~CShellZipHelper()
{
	CoUninitialize();
}

bool CShellZipHelper::CreateZip(const CString& zipFilePath, const CStringArray& filesToCompress)
{
		if (filesToCompress.IsEmpty()) {
			AfxMessageBox(_T("압축할 파일이 없습니다."));
			return false;
		}

		// ZIP 파일 생성
		if (!CreateEmptyZip(zipFilePath)) {
			AfxMessageBox(_T("ZIP 파일을 생성하지 못했습니다."));
			return false;
		}

		// 압축 추가
		CComPtr<IShellDispatch> pShellDispatch;
		HRESULT hr = CoCreateInstance(CLSID_Shell, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pShellDispatch));
		if (FAILED(hr) || !pShellDispatch) {
			AfxMessageBox(_T("IShellDispatch 초기화 실패"));
			return false;
		}

		CComVariant zipFile(zipFilePath);
		CComPtr<Folder> pZipFolder;
		hr = pShellDispatch->NameSpace(zipFile, &pZipFolder);
		if (FAILED(hr) || !pZipFolder) {
			AfxMessageBox(_T("ZIP 파일 열기 실패"));
			return false;
		}

		for (int i = 0; i < filesToCompress.GetSize(); ++i) {
			CString filePath = filesToCompress[i];
			if (!AddFileToZip(pZipFolder, filePath)) {
				CString msg;
				msg.Format(_T("파일 추가 실패: %s"), filePath);
				AfxMessageBox(msg);
				return false;
			}
		}

		return true;
}

bool CShellZipHelper::ExtractZip(const CString& zipFilePath, const CString& extractToFolder)
{
	CComPtr<IShellDispatch> pShellDispatch;
	HRESULT hr = CoCreateInstance(CLSID_Shell, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pShellDispatch));
	if (FAILED(hr) || !pShellDispatch) {
		AfxMessageBox(_T("IShellDispatch 초기화 실패"));
		return false;
	}

	CComVariant zipFile(zipFilePath);
	CComPtr<Folder> pZipFolder;
	hr = pShellDispatch->NameSpace(zipFile, &pZipFolder);
	if (FAILED(hr) || !pZipFolder) {
		AfxMessageBox(_T("ZIP 파일 열기 실패"));
		return false;
	}

	CComVariant extractFolder(extractToFolder);
	CComPtr<Folder> pExtractFolder;
	hr = pShellDispatch->NameSpace(extractFolder, &pExtractFolder);
	if (FAILED(hr) || !pExtractFolder) {
		AfxMessageBox(_T("추출할 폴더 열기 실패"));
		return false;
	}

	CComVariant options(0); // 두 번째 매개변수에 기본 옵션 전달
	hr = pExtractFolder->CopyHere(CComVariant(pZipFolder), options);
	return SUCCEEDED(hr);
}

bool CShellZipHelper::CreateEmptyZip(const CString& zipFilePath)
{
		HANDLE hFile = CreateFile(zipFilePath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		BYTE zipHeader[] = { 0x50, 0x4B, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00 };
		DWORD bytesWritten;
		WriteFile(hFile, zipHeader, sizeof(zipHeader), &bytesWritten, nullptr);
		CloseHandle(hFile);

		return bytesWritten == sizeof(zipHeader);
}

bool CShellZipHelper::AddFileToZip(CComPtr<Folder>& pZipFolder, const CString& filePath)
{
	CComVariant file(filePath);   // 첫 번째 매개변수: 추가할 파일의 경로
	CComVariant options(0);       // 두 번째 매개변수: 옵션 (0은 기본 옵션)

	HRESULT hr = pZipFolder->CopyHere(file, options); // CopyHere 호출
	return SUCCEEDED(hr);         // 작업 성공 여부 반환
}