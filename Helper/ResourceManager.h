#pragma once

class CResourceManager
{
public:
	CResourceManager();
	~CResourceManager();

	BOOL ChangeExeIcon(LPCTSTR lpszExePath, LPCTSTR lpszIconFilePath);
	
	BOOL GetVersionInfo(LPCTSTR szExePath, CString& fileVersion, CString& productName, CString& companyName);
	BOOL UpdateVersionInfo(LPCTSTR szExePath, const CString& productName, const CString& fileVersion, const CString& companyName);
};

