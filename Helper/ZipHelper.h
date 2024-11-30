// Zip.h : Declaration of the CZip

#ifndef __ZIPWRAPPER_H_
#define __ZIPWRAPPER_H_

#include "resource.h"       // main symbols
#include "ZipArchive.h"
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

#endif //__ZIP_H_
