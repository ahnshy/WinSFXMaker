#include "../stdafx.h"
#include "ZlibHelper.h"


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>

#include <direct.h>

#include "../zlib-1.3.1/zlib.h"
#pragma comment(lib, "lib\\zlibstat.lib")

CZlibHelper::CZlibHelper()
{
}


CZlibHelper::~CZlibHelper()
{
}

// ���� ���� �Լ� (Decompress)
BOOL CZlibHelper::Decompress(LPCTSTR inputFilePath, LPCTSTR outputFilePath)
{
	// �Է� ���� ����
	std::ifstream inputFile(inputFilePath, std::ios::binary);
	if (!inputFile.is_open()) {
		//std::cerr << "�Է� ������ �� �� �����ϴ�: " << inputFilePath << std::endl;
		return FALSE;
	}

	// ��� ���� ����
	std::ofstream outputFile(outputFilePath, std::ios::binary);
	if (!outputFile.is_open()) {
		//std::cerr << "��� ������ �� �� �����ϴ�: " << outputFilePath << std::endl;
		return FALSE;
	}

	// ���� ũ�� ���ϱ�
	inputFile.seekg(0, std::ios::end);
	std::streamsize compressedSize = inputFile.tellg();
	inputFile.seekg(0, std::ios::beg);

	// �Է� ������ ����� �����͸� �޸𸮿� �б�
	std::vector<char> compressedData((int)compressedSize);
	inputFile.read(compressedData.data(), compressedSize);

	// zlib ��Ʈ�� �غ�
	z_stream strm;
	memset(&strm, 0, sizeof(strm));  // ��Ʈ�� �ʱ�ȭ
	if (inflateInit(&strm) != Z_OK) {
		//std::cerr << "zlib �ʱ�ȭ ����" << std::endl;
		return FALSE;
	}

	// ���� ������ ���� �غ�
	const size_t bufferSize = 1024 * 8;  // 8KB ũ���� ����
	std::vector<char> buffer(bufferSize);

	// zlib ���� ���� ó��
	strm.avail_in = static_cast<uInt>(compressedSize);
	strm.next_in = reinterpret_cast<Bytef*>(compressedData.data());
	strm.avail_out = static_cast<uInt>(bufferSize);
	strm.next_out = reinterpret_cast<Bytef*>(buffer.data());

	int ret = Z_OK;
	while (strm.avail_in > 0) {
		ret = inflate(&strm, Z_NO_FLUSH);
		if (ret == Z_ERRNO) {
			//std::cerr << "���� ���� ����" << std::endl;
			inflateEnd(&strm);
			return FALSE;
		}

		// ���� ������ �����Ͱ� ������ ��� ���Ͽ� ����.
		if (strm.avail_out == 0 || ret == Z_STREAM_END) {
			outputFile.write(buffer.data(), bufferSize - strm.avail_out);
			strm.avail_out = bufferSize;
			strm.next_out = reinterpret_cast<Bytef*>(buffer.data());
		}
	}

	// ��� �����Ͱ� ������ �� ó��
	while (ret != Z_STREAM_END) {
		ret = inflate(&strm, Z_FINISH);
		if (ret == Z_ERRNO) {
			//std::cerr << "���� ���� ����" << std::endl;
			inflateEnd(&strm);
			return FALSE;
		}

		// ���� �����͸� ��� ���Ͽ� ����.
		outputFile.write(buffer.data(), bufferSize - strm.avail_out);
		strm.avail_out = bufferSize;
		strm.next_out = reinterpret_cast<Bytef*>(buffer.data());
	}

	// zlib ��Ʈ�� ����
	inflateEnd(&strm);

	//std::cout << "���� ���� �Ϸ�: " << outputFilePath << std::endl;
	return TRUE;
}

// ���� �Լ� (Compress)
BOOL CZlibHelper::Compress(LPCTSTR inputFilePath, LPCTSTR outputFilePath)
{
	// �Է� ���� ����
	std::ifstream inputFile(inputFilePath, std::ios::binary);
	if (!inputFile.is_open()) {
		//std::cerr << "�Է� ������ �� �� �����ϴ�: " << inputFilePath << std::endl;
		return FALSE;
	}

	// ��� ���� ����
	std::ofstream outputFile(outputFilePath, std::ios::binary);
	if (!outputFile.is_open()) {
		//std::cerr << "��� ������ �� �� �����ϴ�: " << outputFilePath << std::endl;
		return FALSE;
	}

	// ���� ũ�� ���ϱ�
	inputFile.seekg(0, std::ios::end);
	std::streamsize inputSize = inputFile.tellg();
	inputFile.seekg(0, std::ios::beg);

	// �Է� ������ �����͸� �޸𸮿� �б�
	std::vector<char> inputData((int)inputSize);
	inputFile.read(inputData.data(), inputSize);

	// zlib ��Ʈ�� �غ�
	z_stream strm;
	memset(&strm, 0, sizeof(strm));  // ��Ʈ�� �ʱ�ȭ
	if (deflateInit(&strm, Z_DEFAULT_COMPRESSION) != Z_OK) {
		//std::cerr << "zlib �ʱ�ȭ ����" << std::endl;
		return FALSE;
	}

	// ������ ���� �غ�
	const size_t bufferSize = 1024 * 8;  // 8KB ũ���� ����
	std::vector<char> buffer(bufferSize);

	// ����
	strm.avail_in = static_cast<uInt>(inputSize);
	strm.next_in = reinterpret_cast<Bytef*>(inputData.data());
	strm.avail_out = static_cast<uInt>(bufferSize);
	strm.next_out = reinterpret_cast<Bytef*>(buffer.data());

	int ret = Z_OK;
	while (strm.avail_in > 0) {
		ret = deflate(&strm, Z_NO_FLUSH);
		if (ret == Z_ERRNO) {
			//std::cerr << "���� ����" << std::endl;
			deflateEnd(&strm);
			return FALSE;
		}

		// ����� �����Ͱ� ������ ��� ���Ͽ� ����.
		if (strm.avail_out == 0 || ret == Z_STREAM_END) {
			outputFile.write(buffer.data(), bufferSize - strm.avail_out);
			strm.avail_out = bufferSize;
			strm.next_out = reinterpret_cast<Bytef*>(buffer.data());
		}
	}

	// ��� �����Ͱ� ����� �� ó��
	while (ret != Z_STREAM_END) {
		ret = deflate(&strm, Z_FINISH);
		if (ret == Z_ERRNO) {
			//std::cerr << "���� ����" << std::endl;
			deflateEnd(&strm);
			return FALSE;
		}

		// ���� ����� �����͸� ��� ���Ͽ� ����.
		outputFile.write(buffer.data(), bufferSize - strm.avail_out);
		strm.avail_out = bufferSize;
		strm.next_out = reinterpret_cast<Bytef*>(buffer.data());
	}

	// zlib ��Ʈ�� ����
	deflateEnd(&strm);

	//std::cout << "���� �Ϸ�: " << outputFilePath << std::endl;
	return TRUE;
}

// ������ ������ ZIP �������� ����
BOOL CZlibHelper::CompressFile(const CString& strPath, gzFile& gzFile)
{
	FILE* pFile = NULL;
	_wfopen_s(&pFile, (LPCTSTR)strPath, _T("rb"));
	if (pFile == nullptr)
	{
		//std::cerr << "���� ���� ����: " << filePath.GetString() << std::endl;
		return FALSE;
	}

	const size_t bufferSize = 1024;
	char buffer[bufferSize];
	int bytesRead;

	while ((bytesRead = fread(buffer, 1, bufferSize, pFile)) > 0)
	{
		if (gzwrite(gzFile, buffer, bytesRead) != bytesRead)
		{
			//std::cerr << "���� ���� ����: " << filePath.GetString() << std::endl;
			fclose(pFile);
			return FALSE;
		}
	}

	fclose(pFile);
	return TRUE;
}

// ���丮�� ���������� ��������� Ž���ϸ鼭 ������ ����
void CZlibHelper::AddFilesToZip(const CString& dirPath, const CString& baseDir, gzFile& gzFile)
{
	CFileFind finder;
	CString strFileName;

	// ���� �� ��� ���ϰ� ���������� ã�� ���� ����
	strFileName = dirPath;
	strFileName += "\\*.*";

	BOOL bWorking = finder.FindFile(strFileName);
	while (bWorking) {
		bWorking = finder.FindNextFile();
		if (finder.IsDots()) {
			continue; // "." �� ".."�� �ǳʶݴϴ�.
		}

		// ���������� ��� ��������� Ž��
		if (finder.IsDirectory()) {
			CString subDir = finder.GetFilePath();
			AddFilesToZip(subDir, baseDir, gzFile); // ��� ȣ��
		}
		else {
			CString filePath = finder.GetFilePath();
			CString relativePath = filePath.Right(filePath.GetLength() - baseDir.GetLength() - 1); // baseDir ��� ���

			std::wcout << L"���� ��: " << (LPCWSTR)relativePath << std::endl;
			CompressFile(relativePath, gzFile); // ������ ����
		}
	}
}

bool CZlibHelper::CreateDirectoryRecursively(const CString& path)
{
	int result = _wmkdir(path);
	if (result == 0 || errno == EEXIST) {
		return true;  // ���丮 ���� ���� �Ǵ� �̹� ����
	}
	else {
		std::cerr << "���丮 ���� ����: " << path.GetString() << std::endl;
		return false;
	}
}

// .gz ������ Ǯ� ������ ��ο� �����ϴ� �Լ�
bool CZlibHelper::DecompressGZFile(const CString& gzFilePath, const CString& outputFilePath)
{
	// .gz ���� ����
	gzFile gzFile = gzopen(CT2A(gzFilePath.GetString()), "rb");
	if (gzFile == nullptr) {
		std::cerr << "���� ���� ����: " << gzFilePath.GetString() << std::endl;
		return false;
	}

	// ��� ������ ���� ���� ���丮 Ȯ�� �� ����
	CString outputDir = outputFilePath.Left(outputFilePath.ReverseFind('\\'));
	if (!CreateDirectoryRecursively(outputDir)) {
		gzclose(gzFile);
		return false;
	}

	// ��� ���� ����
	std::ofstream outFile(outputFilePath.GetString(), std::ios::binary);
	if (!outFile.is_open()) {
		std::cerr << "��� ���� ���� ����: " << outputFilePath.GetString() << std::endl;
		gzclose(gzFile);
		return false;
	}

	// ���� ����
	const size_t bufferSize = 4096;
	char buffer[bufferSize];
	int bytesRead = 0;

	// ���� Ǯ��
	while ((bytesRead = gzread(gzFile, buffer, bufferSize)) > 0) {
		outFile.write(buffer, bytesRead);
	}

	// ���� �ݱ�
	outFile.close();
	gzclose(gzFile);

	return true;
}

// ����
//{
//	CString inputDir = _T("C:\\TestFolder"); // ������ ���� ���
//	CString outputZip = _T("C:\\output.gz"); // ��� ���� ���� ��� (gz ����)
//
//	gzFile gzFile = gzopen(CT2A(outputZip.GetString()), "wb");
//	if (gzFile == nullptr) {
//		std::cerr << "���� ���� ���� ����: " << (LPCWSTR)outputZip << std::endl;
//		return -1;
//	}
//
//	AddFilesToZip(inputDir, inputDir, gzFile); // ���� �� ���� �߰�
//
//	gzclose(gzFile); // ���� ���� �ݱ�
//	std::wcout << L"���� �Ϸ�: " << (LPCWSTR)outputZip << std::endl;
//
//	return 0;
//}