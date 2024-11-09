#pragma once
class CZlibHelper
{
public:
	CZlibHelper();
	~CZlibHelper();


	BOOL Compress(LPCTSTR inputFilePath, LPCTSTR outputFilePath);
	BOOL Decompress(LPCTSTR inputFilePath, LPCTSTR outputFilePath);
	//BOOL Decompress(LPCTSTR inputFilePath, LPCTSTR outputFilePath);
};

