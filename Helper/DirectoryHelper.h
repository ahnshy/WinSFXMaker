#pragma once

class CDirectoryHelper
{
public:
	static	CString			GetTempPath();
	static	CString			GetBinaryPath();
	static	CString			GetDocumentPath();
	static	CString			GetProgramFilesPath();
	static	BOOL			GetCurrentDirectory(CString& strCurrentPath);
	static	void			RemoveDir(CString strDir);
	static	BOOL			IsExistDirectory(const TCHAR *pszDirPath);
	static	CString			MakePath(TCHAR *pszFullPath, int nBufSiz);
	static	BOOL			CreateDirectoryEx(CString strPath);
	static	BOOL			CreateDirectoryRecursive(LPCTSTR lpszDirectory, LPSECURITY_ATTRIBUTES lpSA = NULL);
	static	CString			GetCurrentDirectoryEx();
	static	CString			GetDirectoryPath(CString strPath);
	static	BOOL			IsSamePath(CString strPath1, CString strPath2);
	static	BOOL			GetParentPath(LPTSTR lpPath);
	static	BOOL			ShellOpenDirectory(HWND hWnd, LPCTSTR lpPath);
};