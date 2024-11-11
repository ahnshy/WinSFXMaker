#pragma once

class CResourceManager
{
public:
	CResourceManager();
	~CResourceManager();

	BOOL ChangeExeIcon(LPCTSTR szExePath, LPCTSTR szIconFilePath);
};

