#pragma once

/////////////////////////////////////////////////////////////////////////
// #20120118 Work Ahnshy :: File Utility
// CFileHelper class
// Bug Report :: ahnshy@naver.com

class CFileHelper
{
public:
	static	CString			GetGUID();
	static	void			CreateRandomString(TCHAR* pszBuf, int nLen);
	static	void			GetTimeBaseRandomString(TCHAR* pszBuf, int nNumOfChar);
	static	CString			GetTimeBaseFileName(LPCTSTR lpBasePath, LPCTSTR lpExt = _T("tmp"));
	static	CString			GetGuidBaseFileName(CString strPath, CString strExt);
	static	BOOL			ShowFileDialog(HWND hOwner, IN LPTSTR lpszFilter, IN LPCTSTR lpInitDir, IN OUT LPTSTR pszFilePath, IN LPCTSTR lpszTitle, IN LPCTSTR lpDefaultExt, IN int& nFilterIndex);
};