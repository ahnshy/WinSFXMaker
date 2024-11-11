#pragma once

#include "../zlib-1.3.1/zlib.h"
class CZlibHelper
{
public:
	CZlibHelper();
	~CZlibHelper();


	BOOL Compress(LPCTSTR inputFilePath, LPCTSTR outputFilePath);
	BOOL Decompress(LPCTSTR inputFilePath, LPCTSTR outputFilePath);
	//BOOL Decompress(LPCTSTR inputFilePath, LPCTSTR outputFilePath);

	void AddFilesToZip(const CString& dirPath, const CString& baseDir, gzFile& gzFile);
	BOOL CompressFile(const CString& strPath, gzFile& gzFile);

	bool DecompressGZFile(const CString& gzFilePath, const CString& outputFilePath);
		bool CreateDirectoryRecursively(const CString& path);
};

