#include "../stdafx.h"
#include "ResourceManager.h"


CResourceManager::CResourceManager()
{
}


CResourceManager::~CResourceManager()
{
}

BOOL CResourceManager::ChangeExeIcon(LPCTSTR szExePath, LPCTSTR szIconFilePath)
{
	// EXE 파일 열기
	HANDLE hUpdate = BeginUpdateResource(szExePath, FALSE);
	if (hUpdate == NULL) {
		//MessageBox(NULL, _T("EXE 파일을 열 수 없습니다."), _T("Error"), MB_OK);
		return FALSE;
	}

	// 아이콘 리소스 변경
	HICON hIcon = (HICON)LoadImage(NULL, szIconFilePath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	if (hIcon == NULL) {
		//MessageBox(NULL, _T("아이콘 파일을 로드할 수 없습니다."), _T("Error"), MB_OK);
		EndUpdateResource(hUpdate, TRUE);
		return FALSE;
	}

	// 리소스 업데이트
	BOOL result = UpdateResource(hUpdate, RT_ICON, MAKEINTRESOURCE(101), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), hIcon, sizeof(HICON));

	if (!result) {
		//MessageBox(NULL, _T("리소스를 업데이트할 수 없습니다."), _T("Error"), MB_OK);
		EndUpdateResource(hUpdate, TRUE);
		return FALSE;
	}

	// 리소스 업데이트 완료
	EndUpdateResource(hUpdate, FALSE);
	return TRUE;
}