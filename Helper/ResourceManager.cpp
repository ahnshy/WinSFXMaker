#include "../stdafx.h"
#include "ResourceManager.h"
#include <winver.h>
#pragma comment(lib, "Version.lib")

CResourceManager::CResourceManager()
{
}

CResourceManager::~CResourceManager()
{
}

BOOL CResourceManager::ChangeExeIcon(LPCTSTR lpszExePath, LPCTSTR lpszIconFilePath)
{
	// Open icon file
	HANDLE hFile = CreateFile(lpszIconFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD dwFileSize = GetFileSize(hFile, NULL);
	if (dwFileSize == INVALID_FILE_SIZE || dwFileSize == 0)
	{
		CloseHandle(hFile);
		return FALSE;
	}

	// Read entire icon file
	BYTE* pFileData = new BYTE[dwFileSize];
	DWORD dwBytesRead = 0;
	if (!ReadFile(hFile, pFileData, dwFileSize, &dwBytesRead, NULL) || dwBytesRead != dwFileSize)
	{
		delete[] pFileData;
		CloseHandle(hFile);
		return FALSE;
	}
	CloseHandle(hFile);

	// Verify icon directory header
	MYICONDIR* pIconDir = (MYICONDIR*)pFileData;
	if (pIconDir->idReserved != 0 || pIconDir->idType != 1 || pIconDir->idCount == 0)
	{
		delete[] pFileData;
		return FALSE;
	}

	MYICONDIRENTRY* pEntries = (MYICONDIRENTRY*)(pFileData + sizeof(MYICONDIR));
	WORD nIconCount = pIconDir->idCount;

	// Build GRPICONDIR structure
	DWORD dwGrpSize = sizeof(MYICONDIR) + sizeof(MYGRPICONDIRENTRY) * nIconCount;
	BYTE* pGrpIconDir = new BYTE[dwGrpSize];
	ZeroMemory(pGrpIconDir, dwGrpSize);

	MYICONDIR* pGrpHeader = (MYICONDIR*)pGrpIconDir;
	pGrpHeader->idReserved = 0;
	pGrpHeader->idType = 1;
	pGrpHeader->idCount = nIconCount;

	MYGRPICONDIRENTRY* pGrpEntries = (MYGRPICONDIRENTRY*)(pGrpIconDir + sizeof(MYICONDIR));
	
	int i;
	for (i = 0; i < nIconCount; i++)
	{
		pGrpEntries[i].bWidth = pEntries[i].bWidth;
		pGrpEntries[i].bHeight = pEntries[i].bHeight;
		pGrpEntries[i].bColorCount = pEntries[i].bColorCount;
		pGrpEntries[i].bReserved = 0;
		pGrpEntries[i].wPlanes = pEntries[i].wPlanes;
		pGrpEntries[i].wBitCount = pEntries[i].wBitCount;
		pGrpEntries[i].dwBytesInRes = pEntries[i].dwBytesInRes;
		pGrpEntries[i].nID = (WORD)(i + 1);
	}

	// Begin resource update
	HANDLE hResource = BeginUpdateResource(lpszExePath, FALSE);
	if (hResource == NULL)
	{
		delete[] pGrpIconDir;
		delete[] pFileData;
		return FALSE;
	}

	BOOL bResult = TRUE;
	bResult = UpdateResource(hResource, RT_GROUP_ICON, MAKEINTRESOURCE(1), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), pGrpIconDir, dwGrpSize);

	if (!bResult)
	{
		EndUpdateResource(hResource, TRUE);
		delete[] pGrpIconDir;
		delete[] pFileData;
		return FALSE;
	}

	// Add each icon image
	for (i = 0; i < nIconCount && bResult; i++)
	{
		BYTE* pIconData = pFileData + pEntries[i].dwImageOffset;
		DWORD dwIconSize = pEntries[i].dwBytesInRes;
		bResult = UpdateResource(hResource, RT_ICON, MAKEINTRESOURCE(i + 1), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), pIconData, dwIconSize);
	}

	if (!bResult)
	{
		EndUpdateResource(hResource, TRUE);
		delete[] pGrpIconDir;
		delete[] pFileData;
		return FALSE;
	}

	bResult = EndUpdateResource(hResource, FALSE);
	delete[] pGrpIconDir;
	delete[] pFileData;
	return bResult;
}

// Helper function: Add WORD to byte array
static void AddWORDToArray(CByteArray& arr, WORD w)
{
	arr.Add((BYTE)(w & 0xFF));
	arr.Add((BYTE)((w >> 8) & 0xFF));
}

// Helper function: Align array to DWORD boundary
static void AlignDWORDArray(CByteArray& arr)
{
	while (arr.GetSize() % 4 != 0)
		arr.Add(0);
}

// Helper function: Add string to byte array (including null terminator)
static void AddStringToArray(CByteArray& arr, LPCTSTR str)
{
	int len = (int)_tcslen(str) + 1;
	int i;
	for (i = 0; i < len; i++)
	{
		TCHAR ch = str[i];
#ifdef UNICODE
		arr.Add((BYTE)(ch & 0xFF));
		arr.Add((BYTE)((ch >> 8) & 0xFF));
#else
		arr.Add((BYTE)ch);
#endif
	}
}

// Helper function: Write version string entry
static void WriteStringEntry(CByteArray& arr, LPCTSTR szKey, LPCTSTR szValue)
{
	int nStartPos = (int)arr.GetSize();
	AddWORDToArray(arr, 0); // wLength - will be filled later
	AddWORDToArray(arr, (WORD)(_tcslen(szValue) + 1)); // wValueLength
	AddWORDToArray(arr, 1); // wType = 1 (text)
	AddStringToArray(arr, szKey);
	AlignDWORDArray(arr);
	AddStringToArray(arr, szValue);
	AlignDWORDArray(arr);
	// Update wLength
	int nLen = (int)arr.GetSize() - nStartPos;
	arr[nStartPos] = (BYTE)(nLen & 0xFF);
	arr[nStartPos + 1] = (BYTE)((nLen >> 8) & 0xFF);
}

BOOL CResourceManager::UpdateVersionInfo(LPCTSTR lpszExePath, LPCTSTR lpszCompanyName, LPCTSTR lpszProductName, LPCTSTR lpszFileVersion, LPCTSTR lpszCopyright, LPCTSTR lpszDescription)
{
	// Parse version string - extract only digits for binary version
	WORD wMajor = 1, wMinor = 0, wBuild = 0, wRevision = 0;
	CString strVersionForParse;
	if (lpszFileVersion != NULL && _tcslen(lpszFileVersion) > 0)
	{
		strVersionForParse = lpszFileVersion;
		// Replace comma with dot and remove spaces for parsing only
		strVersionForParse.Replace(_T(","), _T("."));
		strVersionForParse.Replace(_T(" "), _T(""));
		int nPos = 0;
		CString strToken;
		int nIndex = 0;
		while (nPos >= 0 && nIndex < 4)
		{
			strToken = strVersionForParse.Tokenize(_T("."), nPos);
			if (strToken.IsEmpty()) break;
			int nValue = _ttoi(strToken);
			switch (nIndex)
			{
			case 0: wMajor = (WORD)nValue; break;
			case 1: wMinor = (WORD)nValue; break;
			case 2: wBuild = (WORD)nValue; break;
			case 3: wRevision = (WORD)nValue; break;
			}
			nIndex++;
		}
	}

	// Use original version string for display (keep comma and space as entered)
	CString strFileVer = (lpszFileVersion && _tcslen(lpszFileVersion) > 0) ? lpszFileVersion : _T("1.0.0.0");
	CString strProductVer = strFileVer;

	// Prepare strings
	CString strCompanyName = (lpszCompanyName && _tcslen(lpszCompanyName) > 0) ? lpszCompanyName : _T(" ");
	CString strProductName = (lpszProductName && _tcslen(lpszProductName) > 0) ? lpszProductName : _T(" ");
	CString strCopyrightStr = (lpszCopyright && _tcslen(lpszCopyright) > 0) ? lpszCopyright : _T(" ");
	CString strDescStr = (lpszDescription && _tcslen(lpszDescription) > 0) ? lpszDescription : _T(" ");

	// Prepare VS_FIXEDFILEINFO structure
	VS_FIXEDFILEINFO vsFixedInfo;
	ZeroMemory(&vsFixedInfo, sizeof(vsFixedInfo));
	vsFixedInfo.dwSignature = 0xFEEF04BD;
	vsFixedInfo.dwStrucVersion = 0x00010000;
	vsFixedInfo.dwFileVersionMS = MAKELONG(wMinor, wMajor);
	vsFixedInfo.dwFileVersionLS = MAKELONG(wRevision, wBuild);
	vsFixedInfo.dwProductVersionMS = MAKELONG(wMinor, wMajor);
	vsFixedInfo.dwProductVersionLS = MAKELONG(wRevision, wBuild);
	vsFixedInfo.dwFileFlagsMask = 0x3F;
	vsFixedInfo.dwFileFlags = 0;
	vsFixedInfo.dwFileOS = VOS_NT_WINDOWS32;
	vsFixedInfo.dwFileType = VFT_APP;
	vsFixedInfo.dwFileSubtype = 0;
	vsFixedInfo.dwFileDateMS = 0;
	vsFixedInfo.dwFileDateLS = 0;

	// Build version resource binary
	CByteArray arrVersionData;
	
	// VS_VERSION_INFO start
	int nVersionInfoStart = (int)arrVersionData.GetSize();
	AddWORDToArray(arrVersionData, 0); // wLength
	AddWORDToArray(arrVersionData, sizeof(VS_FIXEDFILEINFO)); // wValueLength
	AddWORDToArray(arrVersionData, 0); // wType = 0 (binary)
	AddStringToArray(arrVersionData, _T("VS_VERSION_INFO"));
	AlignDWORDArray(arrVersionData);

	// Add VS_FIXEDFILEINFO
	BYTE* pFixed = (BYTE*)&vsFixedInfo;
	int j;
	for (j = 0; j < sizeof(VS_FIXEDFILEINFO); j++)
		arrVersionData.Add(pFixed[j]);
	AlignDWORDArray(arrVersionData);

	// StringFileInfo start
	int nStringFileInfoStart = (int)arrVersionData.GetSize();
	AddWORDToArray(arrVersionData, 0); // wLength
	AddWORDToArray(arrVersionData, 0); // wValueLength
	AddWORDToArray(arrVersionData, 1); // wType
	AddStringToArray(arrVersionData, _T("StringFileInfo"));
	AlignDWORDArray(arrVersionData);

	// StringTable start
	int nStringTableStart = (int)arrVersionData.GetSize();
	AddWORDToArray(arrVersionData, 0); // wLength
	AddWORDToArray(arrVersionData, 0); // wValueLength
	AddWORDToArray(arrVersionData, 1); // wType
	AddStringToArray(arrVersionData, _T("040904B0"));
	AlignDWORDArray(arrVersionData);

	// Add strings
	WriteStringEntry(arrVersionData, _T("CompanyName"), strCompanyName);
	WriteStringEntry(arrVersionData, _T("FileDescription"), strDescStr);
	WriteStringEntry(arrVersionData, _T("FileVersion"), strFileVer);
	WriteStringEntry(arrVersionData, _T("InternalName"), strProductName);
	WriteStringEntry(arrVersionData, _T("LegalCopyright"), strCopyrightStr);
	WriteStringEntry(arrVersionData, _T("OriginalFilename"), strProductName);
	WriteStringEntry(arrVersionData, _T("ProductName"), strProductName);
	WriteStringEntry(arrVersionData, _T("ProductVersion"), strProductVer);

	// Update StringTable length
	int nStringTableLen = (int)arrVersionData.GetSize() - nStringTableStart;
	arrVersionData[nStringTableStart] = (BYTE)(nStringTableLen & 0xFF);
	arrVersionData[nStringTableStart + 1] = (BYTE)((nStringTableLen >> 8) & 0xFF);

	// Update StringFileInfo length
	int nStringFileInfoLen = (int)arrVersionData.GetSize() - nStringFileInfoStart;
	arrVersionData[nStringFileInfoStart] = (BYTE)(nStringFileInfoLen & 0xFF);
	arrVersionData[nStringFileInfoStart + 1] = (BYTE)((nStringFileInfoLen >> 8) & 0xFF);

	AlignDWORDArray(arrVersionData);

	// VarFileInfo start
	int nVarFileInfoStart = (int)arrVersionData.GetSize();
	AddWORDToArray(arrVersionData, 0); // wLength
	AddWORDToArray(arrVersionData, 0); // wValueLength
	AddWORDToArray(arrVersionData, 1); // wType
	AddStringToArray(arrVersionData, _T("VarFileInfo"));
	AlignDWORDArray(arrVersionData);

	// Var (Translation)
	int nVarStart = (int)arrVersionData.GetSize();
	AddWORDToArray(arrVersionData, 0); // wLength
	AddWORDToArray(arrVersionData, 4); // wValueLength
	AddWORDToArray(arrVersionData, 0); // wType = 0 (binary)
	AddStringToArray(arrVersionData, _T("Translation"));
	AlignDWORDArray(arrVersionData);
	AddWORDToArray(arrVersionData, 0x0409);
	AddWORDToArray(arrVersionData, 0x04B0);

	// Update Var length
	int nVarLen = (int)arrVersionData.GetSize() - nVarStart;
	arrVersionData[nVarStart] = (BYTE)(nVarLen & 0xFF);
	arrVersionData[nVarStart + 1] = (BYTE)((nVarLen >> 8) & 0xFF);

	// Update VarFileInfo length
	int nVarFileInfoLen = (int)arrVersionData.GetSize() - nVarFileInfoStart;
	arrVersionData[nVarFileInfoStart] = (BYTE)(nVarFileInfoLen & 0xFF);
	arrVersionData[nVarFileInfoStart + 1] = (BYTE)((nVarFileInfoLen >> 8) & 0xFF);

	// Update VS_VERSION_INFO total length
	int nVersionInfoLen = (int)arrVersionData.GetSize() - nVersionInfoStart;
	arrVersionData[nVersionInfoStart] = (BYTE)(nVersionInfoLen & 0xFF);
	arrVersionData[nVersionInfoStart + 1] = (BYTE)((nVersionInfoLen >> 8) & 0xFF);

	// Update resource
	HANDLE hResource = BeginUpdateResource(lpszExePath, FALSE);
	if (hResource == NULL)
		return FALSE;

	BOOL bResult = UpdateResource(hResource, RT_VERSION, MAKEINTRESOURCE(1),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		arrVersionData.GetData(), (DWORD)arrVersionData.GetSize());

	if (!bResult)
	{
		EndUpdateResource(hResource, TRUE);
		return FALSE;
	}

	bResult = EndUpdateResource(hResource, FALSE);
	return bResult;
}
