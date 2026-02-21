#pragma once
#include <windows.h>
#include <tchar.h>

// Icon group header structures (2-byte alignment for icon format)
#pragma pack(push, 2)
typedef struct tagMYICONDIR
{
	WORD idReserved;
	WORD idType;
	WORD idCount;
} MYICONDIR;

typedef struct tagMYGRPICONDIRENTRY
{
	BYTE  bWidth;
	BYTE  bHeight;
	BYTE  bColorCount;
	BYTE  bReserved;
	WORD  wPlanes;
	WORD  wBitCount;
	DWORD dwBytesInRes;
	WORD  nID;
} MYGRPICONDIRENTRY;

typedef struct tagMYICONDIRENTRY
{
	BYTE  bWidth;
	BYTE  bHeight;
	BYTE  bColorCount;
	BYTE  bReserved;
	WORD  wPlanes;
	WORD  wBitCount;
	DWORD dwBytesInRes;
	DWORD dwImageOffset;
} MYICONDIRENTRY;
#pragma pack(pop)

class CResourceManager
{
public:
	CResourceManager();
	~CResourceManager();
	BOOL ChangeExeIcon(LPCTSTR lpszExePath, LPCTSTR lpszIconFilePath);
	BOOL UpdateVersionInfo(LPCTSTR lpszExePath, LPCTSTR lpszCompanyName, LPCTSTR lpszProductName, LPCTSTR lpszFileVersion, LPCTSTR lpszCopyright, LPCTSTR lpszDescription);
};
