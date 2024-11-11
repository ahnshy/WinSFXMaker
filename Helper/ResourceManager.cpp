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
	// EXE ���� ����
	HANDLE hUpdate = BeginUpdateResource(szExePath, FALSE);
	if (hUpdate == NULL) {
		//MessageBox(NULL, _T("EXE ������ �� �� �����ϴ�."), _T("Error"), MB_OK);
		return FALSE;
	}

	// ������ ���ҽ� ����
	HICON hIcon = (HICON)LoadImage(NULL, szIconFilePath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	if (hIcon == NULL) {
		//MessageBox(NULL, _T("������ ������ �ε��� �� �����ϴ�."), _T("Error"), MB_OK);
		EndUpdateResource(hUpdate, TRUE);
		return FALSE;
	}

	// ���ҽ� ������Ʈ
	BOOL result = UpdateResource(hUpdate, RT_ICON, MAKEINTRESOURCE(101), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), hIcon, sizeof(HICON));

	if (!result) {
		//MessageBox(NULL, _T("���ҽ��� ������Ʈ�� �� �����ϴ�."), _T("Error"), MB_OK);
		EndUpdateResource(hUpdate, TRUE);
		return FALSE;
	}

	// ���ҽ� ������Ʈ �Ϸ�
	EndUpdateResource(hUpdate, FALSE);
	return TRUE;
}

BOOL CResourceManager::GetVersionInfo(LPCTSTR szExePath, CString& fileVersion, CString& productName, CString& companyName)
{
	// EXE ������ ���� ���ҽ� �ε�
	DWORD dwHandle;
	UINT uLen = GetFileVersionInfoSize(szExePath, &dwHandle);
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
	if (!GetFileVersionInfo(szExePath, dwHandle, uLen, pData)) {
		free(pData);
		//MessageBox(NULL, _T("���� ���� �б� ����"), _T("Error"), MB_OK);
		return FALSE;
	}

	// ���� ���� �Ľ�
	LPVOID lpVersionInfo;
	UINT uSize;
	if (VerQueryValue(pData, _T("\\StringFileInfo\\040904b0\\FileVersion"), &lpVersionInfo, &uSize)) {
		fileVersion = CString(static_cast<TCHAR*>(lpVersionInfo), uSize);
	}

	if (VerQueryValue(pData, _T("\\StringFileInfo\\040904b0\\ProductName"), &lpVersionInfo, &uSize)) {
		productName = CString(static_cast<TCHAR*>(lpVersionInfo), uSize);
	}

	if (VerQueryValue(pData, _T("\\StringFileInfo\\040904b0\\CompanyName"), &lpVersionInfo, &uSize)) {
		companyName = CString(static_cast<TCHAR*>(lpVersionInfo), uSize);
	}

	// �޸� ����
	free(pData);

	return TRUE;
}

BOOL CResourceManager::UpdateVersionInfo(LPCTSTR szExePath, const CString& productName, const CString& fileVersion, const CString& companyName)
{
	// EXE ���� ����
	HANDLE hUpdate = BeginUpdateResource(szExePath, FALSE);
	if (hUpdate == NULL) {
		//MessageBox(NULL, _T("EXE ������ �� �� �����ϴ�."), _T("Error"), MB_OK);
		return FALSE;
	}

	// ���� ���ҽ� ������Ʈ (VS_VERSION_INFO)
	struct {
		VS_FIXEDFILEINFO vsFileInfo;
		TCHAR szFileVersion[128];
		TCHAR szProductName[128];
		TCHAR szCompanyName[128];
	} versionData = {};

	// ���� ���� ä��� (CString�� TCHAR �迭�� ����)
	_tcscpy_s(versionData.szFileVersion, (LPCTSTR)fileVersion);
	_tcscpy_s(versionData.szProductName, (LPCTSTR)productName);
	_tcscpy_s(versionData.szCompanyName, (LPCTSTR)companyName);

	// VS_VERSION_INFO ���ҽ� ������Ʈ
	BOOL bResult = UpdateResource(hUpdate, RT_VERSION, MAKEINTRESOURCE(1), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		&versionData, sizeof(versionData));
	if (!bResult) {
		//MessageBox(NULL, _T("���� ���ҽ��� ������Ʈ�� �� �����ϴ�."), _T("Error"), MB_OK);
		EndUpdateResource(hUpdate, TRUE);
		return FALSE;
	}

	// ���ҽ� ������Ʈ �Ϸ�
	if (EndUpdateResource(hUpdate, FALSE)) {
		//MessageBox(NULL, _T("���� ���� �� ��ǰ ���� ������Ʈ �Ϸ�"), _T("Success"), MB_OK);
		return TRUE;
	}
	else {
		//MessageBox(NULL, _T("���ҽ� ������Ʈ ����."), _T("Error"), MB_OK);
		return FALSE;
	}
}
