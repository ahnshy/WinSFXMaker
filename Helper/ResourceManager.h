#pragma once

class CResourceManager
{
public:
	CResourceManager();
	~CResourceManager();

	BOOL ChangeExeIcon(LPCTSTR lpszExePath, LPCTSTR lpszIconFilePath);
	
	BOOL GetVersionInfo(LPCTSTR lpszExePath, CString& strFileVersion, CString& strProductName, CString& strCompanyName);
	BOOL UpdateVersionInfo(LPCTSTR szExePath, const CString& productName, const CString& fileVersion, const CString& companyName);
};

