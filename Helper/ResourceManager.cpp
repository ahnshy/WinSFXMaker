#include "../stdafx.h"
#include "ResourceManager.h"

#include <winver.h>
#pragma comment(lib, "Version.lib")

CResourceManager::CResourceManager()
{
}


CResourceManager::~CResourceManager()
{
}

BOOL CResourceManager::ChangeExeIcon(LPCTSTR lpszExePath, LPCTSTR lpszIconFilePath)
{
	// EXE 파일 열기
	HANDLE hResource = BeginUpdateResource(lpszExePath, FALSE);
	if (hResource == NULL) {
		//MessageBox(NULL, _T("EXE 파일을 열 수 없습니다."), _T("Error"), MB_OK);
		return FALSE;
	}

	// 아이콘 리소스 변경
	HICON hIcon = (HICON)LoadImage(NULL, lpszIconFilePath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	if (hIcon == NULL) {
		//MessageBox(NULL, _T("아이콘 파일을 로드할 수 없습니다."), _T("Error"), MB_OK);
		EndUpdateResource(hResource, TRUE);
		return FALSE;
	}

	// 리소스 업데이트
	BOOL bResult = UpdateResource(hResource, RT_ICON, MAKEINTRESOURCE(101), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), hIcon, sizeof(HICON));

	if (!bResult) {
		//MessageBox(NULL, _T("리소스를 업데이트할 수 없습니다."), _T("Error"), MB_OK);
		EndUpdateResource(hResource, TRUE);
		return FALSE;
	}

	// 리소스 업데이트 완료
	EndUpdateResource(hResource, FALSE);
	return TRUE;
}

BOOL CResourceManager::GetVersionInfo(LPCTSTR lpszExePath, CString& strFileVersion, CString& strProductName, CString& strCompanyName)
{
	// EXE 파일의 버전 리소스 로드
	DWORD dwHandle = 0;
	UINT uLen = GetFileVersionInfoSize(lpszExePath, &dwHandle);
	if (uLen == 0) {
		//MessageBox(NULL, _T("버전 정보 읽기 실패"), _T("Error"), MB_OK);
		return FALSE;
	}

	// 버전 정보 메모리 할당
	LPVOID pData = malloc(uLen);
	if (pData == NULL) {
		//MessageBox(NULL, _T("메모리 할당 실패"), _T("Error"), MB_OK);
		return FALSE;
	}

	// 버전 리소스 읽기
	if (!GetFileVersionInfo(lpszExePath, dwHandle, uLen, pData)) {
		free(pData);
		//MessageBox(NULL, _T("버전 정보 읽기 실패"), _T("Error"), MB_OK);
		return FALSE;
	}

	// 버전 정보 파싱
	LPVOID lpVersionInfo = NULL;
	UINT uSize = 0;
	if (VerQueryValue(pData, _T("\\StringFileInfo\\040904b0\\FileVersion"), &lpVersionInfo, &uSize))
		strFileVersion = CString(static_cast<TCHAR*>(lpVersionInfo), uSize);

	if (VerQueryValue(pData, _T("\\StringFileInfo\\040904b0\\ProductName"), &lpVersionInfo, &uSize))
		strProductName = CString(static_cast<TCHAR*>(lpVersionInfo), uSize);

	if (VerQueryValue(pData, _T("\\StringFileInfo\\040904b0\\CompanyName"), &lpVersionInfo, &uSize))
		strCompanyName = CString(static_cast<TCHAR*>(lpVersionInfo), uSize);

	// 메모리 해제
	free(pData);

	return TRUE;
}

BOOL CResourceManager::UpdateVersionInfo(LPCTSTR lpszExePath, const CString& strFileVersion, const CString& strProductName, const CString& strCompanyName)
{
	// EXE 파일 열기
	HANDLE hResource = BeginUpdateResource(lpszExePath, FALSE);
	if (hResource == NULL) {
		//MessageBox(NULL, _T("EXE 파일을 열 수 없습니다."), _T("Error"), MB_OK);
		return FALSE;
	}

	// 버전 리소스 업데이트 (VS_VERSION_INFO)
	struct {
		VS_FIXEDFILEINFO fileinfo;
		TCHAR szFileVersion[128];
		TCHAR szProductName[128];
		TCHAR szCompanyName[128];
	} versioninfo = { NULL, };

	// 버전 정보 채우기 (CString을 TCHAR 배열로 복사)
	_tcscpy_s(versioninfo.szFileVersion, (LPCTSTR)strFileVersion);
	_tcscpy_s(versioninfo.szProductName, (LPCTSTR)strProductName);
	_tcscpy_s(versioninfo.szCompanyName, (LPCTSTR)strCompanyName);

	// VS_VERSION_INFO 리소스 업데이트
	BOOL bResult = UpdateResource(hResource, RT_VERSION, MAKEINTRESOURCE(1), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		&versioninfo, sizeof(versioninfo));
	if (!bResult) {
		//MessageBox(NULL, _T("버전 리소스를 업데이트할 수 없습니다."), _T("Error"), MB_OK);
		EndUpdateResource(hResource, TRUE);
		return FALSE;
	}

	// 리소스 업데이트 완료
	if (EndUpdateResource(hResource, FALSE)) {
		//MessageBox(NULL, _T("버전 정보 및 제품 정보 업데이트 완료"), _T("Success"), MB_OK);
		return TRUE;
	}
	else {
		//MessageBox(NULL, _T("리소스 업데이트 실패."), _T("Error"), MB_OK);
		return FALSE;
	}
}
