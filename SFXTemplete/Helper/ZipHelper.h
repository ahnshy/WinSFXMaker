#pragma once

#include "../../ZipArchive/ZipArchive.h"
/////////////////////////////////////////////////////////////////////////////
// CZip

class CZipHelper
{
public:
	CZipHelper();
	~CZipHelper();

public:
	BOOL Decompress(CString strDecompressTarget, CString strDestPath);
	BOOL Compress(CString strOutputName);
	BOOL AddCompressTarget(CString strTargetPath);

private:
	BOOL IsTargetExist(LPCTSTR lpszTarget);
	CStringArray m_arrTarget;

	CZipArchive m_zip;
};
