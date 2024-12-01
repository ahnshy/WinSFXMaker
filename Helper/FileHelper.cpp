/////////////////////////////////////////////////////////////////////////
// #20120118 Work Ahnshy :: File Utility
// CFileHelper class
// Bug Report :: ahnshy@naver.com

#include "../stdafx.h"
#include "FileHelper.h"

#include <Rpc.h>
#include <Shobjidl.h>

#pragma warning(disable:4995)
#pragma comment(lib,"Rpcrt4.lib")

CString CFileHelper::GetGUID()
{
	CString strGUID;

	::CoInitialize(NULL);

	GUID guid = GUID_NULL;
	::UuidCreate(&guid);

	if (guid != GUID_NULL)
	{
		strGUID.Format(_T("%08lX-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X"),
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
			guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7],
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
			guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
	}

	::CoUninitialize();

	strGUID.MakeLower();
	return strGUID;
}

void CFileHelper::CreateRandomString(TCHAR* pszBuf, int nLen)
{
	int nRand = 0;
	const static TCHAR szTab[] = _T("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	for (; nLen > 0; nLen -= 2)
	{
		_ASSERTE(RAND_MAX == 32767);
		nRand = rand();
		*pszBuf = szTab[nRand % 36];
		pszBuf++;
		nRand >>= 6;
		*pszBuf = szTab[nRand % 36];
		pszBuf++;
	}
}

void CFileHelper::GetTimeBaseRandomString(TCHAR* pszBuf, int nNumOfChar)
{
	SYSTEMTIME curtime;
	GetLocalTime(&curtime);
	const int RANDOM_NAME_SIZE = 8;

	_sntprintf(pszBuf, nNumOfChar, _T("%04d%02d%02d%02d%02d%03d_"), curtime.wYear, curtime.wMonth, curtime.wDay, curtime.wHour, curtime.wMinute, curtime.wMilliseconds);
	int timeStrSize = (int)_tcslen(pszBuf);

	CreateRandomString(pszBuf + timeStrSize, RANDOM_NAME_SIZE);
}

CString CFileHelper::GetTimeBaseFileName(LPCTSTR lpBasePath, LPCTSTR lpExt)
{	
	TCHAR szFullPath[BUFSIZ]		= {	0, };	// Result OutBuffer

	TCHAR szPath[BUFSIZ]			= { 0, }; 
	TCHAR szFileName[_MAX_FNAME]	= { 0, };
	TCHAR szDrive[_MAX_DRIVE]		= {	0, };
	TCHAR szDir[_MAX_DIR]			= {	0, };
	TCHAR szExt[_MAX_EXT]			= {	0, };

	_tfullpath(szFullPath, lpBasePath, BUFSIZ);
	_tsplitpath(szFullPath, szDrive, szDir, szFileName, szExt);

	GetTimeBaseRandomString(szFileName, _MAX_FNAME);
	_tmakepath(szFullPath, szDrive, szDir, szFileName, lpExt);

	return CString(szFullPath);
}

CString CFileHelper::GetGuidBaseFileName(CString strPath, CString strExt)
{
	strPath.Append(GetGUID());
	strPath.Append(strExt);

	return strPath;
}

BOOL CFileHelper::ShowFileDialog(HWND hOwner, IN LPTSTR lpszFilter, IN LPCTSTR lpInitDir, IN OUT LPTSTR pszFilePath, IN LPCTSTR lpszTitle, IN LPCTSTR lpDefaultExt, IN int& nFilterIndex)
{
	if (!pszFilePath)
		return FALSE;

#ifdef _UNICODE || #ifdef UNICODE
	for (int nIndex = 0 ; nIndex < (int)_tcslen(lpszFilter);  nIndex++)
		if (lpszFilter[nIndex] == _T('|'))
			lpszFilter[nIndex] = _T('\0');
#endif

	TCHAR lpszFile[2048]		= {0};
	TCHAR lpszFileTitle[2048]	= {0};
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));

	ofn.lStructSize				= sizeof(OPENFILENAME);
	ofn.hwndOwner				= hOwner;
	ofn.lpstrFilter				= lpszFilter;
	ofn.lpstrFile				= lpszFile;
	ofn.nMaxFile				= sizeof(lpszFile)/sizeof(TCHAR);
	ofn.lpstrFileTitle			= lpszFileTitle;
	ofn.nMaxFileTitle			= sizeof(lpszFileTitle)/sizeof(TCHAR);
	ofn.Flags					= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_ENABLESIZING;
	ofn.lpstrInitialDir			= lpInitDir;
	ofn.nFilterIndex			= nFilterIndex;
	ofn.hInstance				= ::AfxGetResourceHandle();
	ofn.lpstrDefExt				= lpDefaultExt;

	//if (!::GetOpenFileName(&ofn))
	if (!::GetSaveFileName(&ofn))
		return FALSE;

	nFilterIndex = ofn.nFilterIndex;
	CString strPath = ofn.lpstrFile;
	if (!strPath.IsEmpty())
	{
		TCHAR szCurrentPath[BUFSIZ] = {_T('\0')};
		_tcscpy(pszFilePath, strPath);
		_tcscpy(szCurrentPath, pszFilePath);

		LPTSTR lpPath = (LPTSTR)_tcsrchr(szCurrentPath, _T('\\'));
		int nDest = lpPath - szCurrentPath + 1; // Include '\'
		if (nDest > 0)
		{
			*(szCurrentPath+nDest) = NULL;
			::SetCurrentDirectory(szCurrentPath);
		}
	}

	return TRUE;
}