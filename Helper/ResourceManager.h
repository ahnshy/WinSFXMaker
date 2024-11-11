#pragma once

class CResourceManager
{
public:
	CResourceManager();
	~CResourceManager();

	BOOL ChangeExeIcon(LPCTSTR szExePath, LPCTSTR szIconFilePath);
	BOOL GetVersionInfo(LPCTSTR szExePath, CString& fileVersion, CString& productName, CString& companyName);
};

