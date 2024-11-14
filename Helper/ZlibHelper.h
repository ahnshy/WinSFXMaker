#pragma once

#include "../zlib-1.3.1/zlib.h"
class CZlibHelper
{
public:
	CZlibHelper();
	~CZlibHelper();


	BOOL Compress(LPCTSTR lpszInputPath, LPCTSTR lpszOutputPath);
	BOOL Decompress(LPCTSTR lpszInputPath, LPCTSTR lpszOutputPath);
	//BOOL Decompress(LPCTSTR inputFilePath, LPCTSTR outputFilePath);

	void AddFilesToZip(const CString& dirPath, const CString& baseDir, gzFile& gzFile);
	BOOL CompressFile(const CString& strPath, gzFile& gzFile);

	BOOL DecompressGZFile(const CString& gzFilePath, const CString& outputFilePath);
	BOOL CreateDirectoryRecursively(const CString& path);
};

