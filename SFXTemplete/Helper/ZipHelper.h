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

private:
	CZipArchive m_zip;
};
