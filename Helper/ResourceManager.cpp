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
	// EXE ���� ����
	HANDLE hResource = BeginUpdateResource(lpszExePath, FALSE);
	if (hResource == NULL) {
		//MessageBox(NULL, _T("EXE ������ �� �� �����ϴ�."), _T("Error"), MB_OK);
		return FALSE;
	}

	// ������ ���ҽ� ����
	HICON hIcon = (HICON)LoadImage(NULL, lpszIconFilePath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	if (hIcon == NULL) {
		//MessageBox(NULL, _T("������ ������ �ε��� �� �����ϴ�."), _T("Error"), MB_OK);
		EndUpdateResource(hResource, TRUE);
		return FALSE;
	}

	// ���ҽ� ������Ʈ
	BOOL bResult = UpdateResource(hResource, RT_ICON, MAKEINTRESOURCE(101), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), hIcon, sizeof(HICON));

	if (!bResult) {
		//MessageBox(NULL, _T("���ҽ��� ������Ʈ�� �� �����ϴ�."), _T("Error"), MB_OK);
		EndUpdateResource(hResource, TRUE);
		return FALSE;
	}

	// ���ҽ� ������Ʈ �Ϸ�
	EndUpdateResource(hResource, FALSE);
	return TRUE;
}

BOOL CResourceManager::GetVersionInfo(LPCTSTR lpszExePath, CString& strFileVersion, CString& strProductName, CString& strCompanyName)
{
	// EXE ������ ���� ���ҽ� �ε�
	DWORD dwHandle = 0;
	UINT uLen = GetFileVersionInfoSize(lpszExePath, &dwHandle);
	if (uLen == 0) {
		//MessageBox(NULL, _T("���� ���� �б� ����"), _T("Error"), MB_OK);
		return FALSE;
	}

	// ���� ���� �޸� �Ҵ�
	LPVOID pData = malloc(uLen);
	if (pData == NULL) {
		//MessageBox(NULL, _T("�޸� �Ҵ� ����"), _T("Error"), MB_OK);
		return FALSE;
	}

	// ���� ���ҽ� �б�
	if (!GetFileVersionInfo(lpszExePath, dwHandle, uLen, pData)) {
		free(pData);
		//MessageBox(NULL, _T("���� ���� �б� ����"), _T("Error"), MB_OK);
		return FALSE;
	}

	// ���� ���� �Ľ�
	LPVOID lpVersionInfo = NULL;
	UINT uSize = 0;
	if (VerQueryValue(pData, _T("\\StringFileInfo\\040904b0\\FileVersion"), &lpVersionInfo, &uSize))
		strFileVersion = CString(static_cast<TCHAR*>(lpVersionInfo), uSize);

	if (VerQueryValue(pData, _T("\\StringFileInfo\\040904b0\\ProductName"), &lpVersionInfo, &uSize))
		strProductName = CString(static_cast<TCHAR*>(lpVersionInfo), uSize);

	if (VerQueryValue(pData, _T("\\StringFileInfo\\040904b0\\CompanyName"), &lpVersionInfo, &uSize))
		strCompanyName = CString(static_cast<TCHAR*>(lpVersionInfo), uSize);

	// �޸� ����
	free(pData);

	return TRUE;
}

BOOL CResourceManager::UpdateVersionInfo(LPCTSTR lpszExePath, const CString& strFileVersion, const CString& strProductName, const CString& strCompanyName)
{
	// EXE ���� ����
	HANDLE hResource = BeginUpdateResource(lpszExePath, FALSE);
	if (hResource == NULL) {
		//MessageBox(NULL, _T("EXE ������ �� �� �����ϴ�."), _T("Error"), MB_OK);
		return FALSE;
	}

	// ���� ���ҽ� ������Ʈ (VS_VERSION_INFO)
	struct {
		VS_FIXEDFILEINFO fileinfo;
		TCHAR szFileVersion[128];
		TCHAR szProductName[128];
		TCHAR szCompanyName[128];
	} versioninfo = { NULL, };

	// ���� ���� ä��� (CString�� TCHAR �迭�� ����)
	_tcscpy_s(versioninfo.szFileVersion, (LPCTSTR)strFileVersion);
	_tcscpy_s(versioninfo.szProductName, (LPCTSTR)strProductName);
	_tcscpy_s(versioninfo.szCompanyName, (LPCTSTR)strCompanyName);

	// VS_VERSION_INFO ���ҽ� ������Ʈ
	BOOL bResult = UpdateResource(hResource, RT_VERSION, MAKEINTRESOURCE(1), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		&versioninfo, sizeof(versioninfo));
	if (!bResult) {
		//MessageBox(NULL, _T("���� ���ҽ��� ������Ʈ�� �� �����ϴ�."), _T("Error"), MB_OK);
		EndUpdateResource(hResource, TRUE);
		return FALSE;
	}

	// ���ҽ� ������Ʈ �Ϸ�
	if (EndUpdateResource(hResource, FALSE)) {
		//MessageBox(NULL, _T("���� ���� �� ��ǰ ���� ������Ʈ �Ϸ�"), _T("Success"), MB_OK);
		return TRUE;
	}
	else {
		//MessageBox(NULL, _T("���ҽ� ������Ʈ ����."), _T("Error"), MB_OK);
		return FALSE;
	}
}
