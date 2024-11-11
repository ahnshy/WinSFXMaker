#include "stdafx.h"
#include "DirectoryHelper.h"
#include <Shobjidl.h>
#include <Tlhelp32.h>

#pragma warning(disable:4995)


CString CDirectoryHelper::GetBinaryPath()
{
	TCHAR szModulePath[BUFSIZ] = { NULL };
	int nBufSiz = _countof(szModulePath);
	::GetModuleFileName(AfxGetInstanceHandle(), szModulePath, nBufSiz);

	if (szModulePath[0] != NULL)
		return MakePath(szModulePath, nBufSiz);

	return CString(_T(""));
}

CString CDirectoryHelper::MakePath(TCHAR *pszFullPath, int nBufSiz)
{
	CString strRet;
	strRet.Empty();

	if (!pszFullPath)
		return strRet;

	TCHAR	szExeFileName[MAX_PATH]	= { NULL, };
	TCHAR	szDrive[_MAX_DRIVE]		= { NULL, };
	TCHAR	szDir[_MAX_DIR]			= { NULL, };
	TCHAR	szFileName[_MAX_FNAME]	= { NULL, };
	TCHAR	szExt[_MAX_EXT]			= { NULL, };
	TCHAR	szBinDir[BUFSIZ]		= { NULL, };

	_tfullpath(szBinDir, pszFullPath, nBufSiz);
	_wsplitpath_s(szBinDir, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, szFileName, _MAX_FNAME, szExt, _MAX_EXT);
	_tmakepath_s(szBinDir, BUFSIZ-sizeof(TCHAR), szDrive, szDir, NULL, NULL);

	strRet.Format(_T("%s"), szBinDir);
	return strRet;
}

BOOL CDirectoryHelper::CreateDirectoryEx(CString strPath)
{
	if (strPath.IsEmpty())
		return FALSE;

	int nStartAt = 0;
	//CString strPath = lpszPath, strTmpPath;
	CString strTmpPath;
	CString strTok = strPath.Tokenize(_T("\\/"), nStartAt);
	while (!strTok.IsEmpty())
	{
		strTmpPath += strTok;
		::PathAddBackslash(strTmpPath.GetBuffer(_MAX_PATH));
		strTmpPath.ReleaseBuffer();

		if (!::PathIsDirectory(strTmpPath))
		{
			if (!CreateDirectory(strTmpPath, NULL))
				return FALSE;
		}
		strTok = strPath.Tokenize(_T("\\/"), nStartAt);
	}
	return TRUE;
}

BOOL CDirectoryHelper::CreateDirectoryRecursive(LPCTSTR lpszDirectory, LPSECURITY_ATTRIBUTES lpSA)
{
	TCHAR szDir[MAX_PATH];
	TCHAR *p, *pNext;
	_tcscpy_s(szDir, MAX_PATH, lpszDirectory);

	pNext = NULL;

	if (_tcslen(lpszDirectory) >= 3)
	{
		if (_T('\\') == szDir[0] && _T('\\') == szDir[1])
		{
			pNext = _tcschr(&szDir[2], _T('\\'));
		}
	}

	if (!pNext)
		pNext = _tcschr(szDir, _T('\\'));

	if (pNext)
	{
		pNext++;
		while (*pNext && (p = _tcschr(pNext, _T('\\'))) != 0)
		{
			*p = NULL;
			if (GetFileAttributes(szDir) == -1)
			{
				if (!CreateDirectory(szDir, lpSA))
				{
					return FALSE;
				}
			}
			*p = _T('\\');
			pNext = p + 1;
		}
	}

	if (-1 == GetFileAttributes(szDir))
	{
		if (!CreateDirectory(szDir, lpSA))
		{
			return FALSE;
		}
	}

	return TRUE;
}

CString CDirectoryHelper::GetTempPath()
{
	CString	strRet;
	TCHAR	szTempPath[2048] = { NULL };

	::ZeroMemory(szTempPath, 2048);
	::GetTempPath(2048, szTempPath);
	::lstrcat(szTempPath, _T("V-ceph7"));

	::CreateDirectory(szTempPath, NULL);

	if (IsExistDirectory(szTempPath))
	{
		::lstrcat(szTempPath, _T("\\"));
		TCHAR* p = szTempPath + lstrlen(szTempPath);
		for (int i = 0;; i++)
		{
			wsprintf(p, _T("%08x"), i);
			if (!IsExistDirectory(szTempPath))
			{
				CreateDirectory(szTempPath, NULL);
				break;
			}
		}
	}

	_ASSERT(szTempPath);
	strRet.Format(_T("%s"), szTempPath);
	return strRet;
}

BOOL CDirectoryHelper::IsExistDirectory(const TCHAR *pszDirPath)
{
	if (!pszDirPath)
		return FALSE;

	HANDLE			hFind;
	WIN32_FIND_DATA fData;
	BOOL			bIsFolder = FALSE;

	if ((hFind = ::FindFirstFile(pszDirPath, &fData)) != INVALID_HANDLE_VALUE)
	{
		if (fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			bIsFolder = TRUE;

		::FindClose(hFind);
	}

	return bIsFolder;
}

void CDirectoryHelper::RemoveDir(CString strDir)
{
	if (!strDir.IsEmpty())
	{
		int nLen = strDir.GetLength();

		if (strDir.GetAt(nLen - sizeof(TCHAR)) != _T('\\'))
			strDir += _T("\\");
		strDir += _T("*.*");

		CFileFind	cff;
		BOOL bWorking = cff.FindFile(strDir);
		while (bWorking)
		{
			bWorking = cff.FindNextFile();

			if (cff.IsDots())
				continue;

			if (cff.IsDirectory())
				RemoveDir(cff.GetFilePath());
			else
			{
				CString strDelFile = cff.GetFilePath();
				if (!strDelFile.IsEmpty())
				{
					::SetFileAttributes(strDelFile, FILE_ATTRIBUTE_NORMAL);
					::DeleteFile(strDelFile);
				}
			}
		}

		int nPathPos = strDir.ReverseFind(_T('\\'));
		nLen = strDir.GetLength();

		cff.Close();
		if (nLen - nPathPos > 0)
		{
			strDir.Delete(nPathPos, nLen - nPathPos);
			::SetFileAttributes(strDir, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_DIRECTORY);
			::RemoveDirectory(strDir);
		}
	}
}

CString CDirectoryHelper::GetDocumentPath()
{
	CString strPath;
	SHGetSpecialFolderPath(::AfxGetMainWnd()->GetSafeHwnd(), strPath.GetBuffer(4096), CSIDL_MYDOCUMENTS, TRUE);
	strPath.ReleaseBuffer();

	::PathAddBackslash(strPath.GetBuffer(4096));
	strPath.ReleaseBuffer();
	return strPath;
}

CString CDirectoryHelper::GetProgramFilesPath()
{
	CString strPath;
	SHGetSpecialFolderPath(::AfxGetMainWnd()->GetSafeHwnd(), strPath.GetBuffer(4096), CSIDL_PROGRAM_FILES, TRUE);
	strPath.ReleaseBuffer();

	::PathAddBackslash(strPath.GetBuffer(4096));
	strPath.ReleaseBuffer();
	return strPath;
}

CString CDirectoryHelper::GetCurrentDirectoryEx()
{
	TCHAR szBuffer[BUFSIZ] = { 0, };
	::GetCurrentDirectory(_countof(szBuffer), szBuffer);

	return CString(szBuffer);
}

CString CDirectoryHelper::GetDirectoryPath(CString strPath)
{
	CString strDir;

	TCHAR szDrive[_MAX_DRIVE] = { 0, };
	TCHAR szDir[_MAX_DIR] = { 0, };

	//_tsplitpath(strPath, szDrive, szDir, NULL, NULL);
	_wsplitpath_s(strPath, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, NULL, 0, NULL, 0);
	strDir += szDrive;
	strDir += szDir;

	return strDir;
}

BOOL CDirectoryHelper::IsSamePath(CString strPath1, CString strPath2)
{
	strPath1.Replace(_T("/"), _T("\\"));
	strPath2.Replace(_T("/"), _T("\\"));

	CString strDir1 = GetDirectoryPath(strPath1);
	CString strDir2 = GetDirectoryPath(strPath2);
	if (strDir1 == strDir2)
	{
		CString strFname1 = ::PathFindFileName(strPath1);
		CString strFname2 = ::PathFindFileName(strPath2);
		if (strFname1 == strFname2)
			return TRUE;
	}
	return FALSE;
}

BOOL CDirectoryHelper::GetParentPath(LPTSTR lpPath)
{
	if (lpPath && PathIsRoot(lpPath))	// Check Root
		return TRUE;

	TCHAR *p = lpPath;					// http://host/a/b/c/  -> http://host/a/b/
	TCHAR *pszParentSlash = NULL;		// c:\temp\dir\		   -> c:\temp\

	while (*p)
	{
		if (p[0] == ':')
		{
			if (p[1] == '/' && p[2] == '/')
			{
				p += 3;					// skip scheme separator
				continue;
			}
			if (p[1] == '\\')
			{							// skip root dir
				p += 2;
				continue;
			}
		}

		if ((p[0] == '/' || p[0] == '\\') && p[1]) 		// slash and not last slash
			pszParentSlash = p;

		p = _tcsinc(p);
	}

	if (pszParentSlash == NULL)
		return FALSE;

	pszParentSlash[1] = 0;
	return TRUE;
}

BOOL CDirectoryHelper::GetCurrentDirectory(CString& strCurrentPath)
{
	BOOL bRet = FALSE;
	DWORD dwSize = ::GetCurrentDirectory(0, NULL);
	if (!dwSize)
		return bRet;

	TCHAR* pBuffer = new TCHAR[dwSize];
	if (!pBuffer)
		return FALSE;

	if (::GetCurrentDirectory(dwSize, pBuffer))
	{
		strCurrentPath = pBuffer;
		bRet = TRUE;
	}

	delete[] pBuffer;
	pBuffer = NULL;

	return bRet;
}

BOOL CDirectoryHelper::ShellOpenDirectory(HWND hWnd, LPCTSTR lpPath)
{
	if (!lpPath)
		return FALSE;

	DWORD dwError = (DWORD)ShellExecute(hWnd, _T("open"), lpPath, NULL, NULL, SW_SHOWNORMAL);
	if (dwError <= 32)
		return FALSE;

	return TRUE;
}