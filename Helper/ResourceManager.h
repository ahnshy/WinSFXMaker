#pragma once

class CResourceManager
{
public:
	CResourceManager();
	~CResourceManager();

	BOOL ChangeExeIcon(LPCTSTR lpszExePath, LPCTSTR lpszIconFilePath);
	
	BOOL GetVersionInfo(LPCTSTR lpszExePath, CString& strFileVersion, CString& strProductName, CString& strCompanyName);
	BOOL UpdateVersionInfo(LPCTSTR lpszExePath, const CString& strFileVersion, const CString& strProductName, const CString& strCompanyName);
};

