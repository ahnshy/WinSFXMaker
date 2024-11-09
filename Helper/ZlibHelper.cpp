#include "stdafx.h"
#include "ZlibHelper.h"

#include <iostream>
#include <fstream>
#include <vector>
#include "../zlib-1.3.1/zlib.h"

#pragma comment(lib, "lib\\zlibstat.lib")

CZlibHelper::CZlibHelper()
{
}


CZlibHelper::~CZlibHelper()
{
}

// ZIP ���� ���� �Լ�
//BOOL CZlibHelper::Decompress(LPCTSTR inputFilePath, LPCTSTR outputFilePath)
//{
//	// �Է� ������ ���̳ʸ� ���� ���ϴ�.
//	std::ifstream inputFile(inputFilePath, std::ios::binary);
//	if (!inputFile) {
//		std::cerr << "�Է� ������ �� �� �����ϴ�: " << inputFilePath << std::endl;
//		return FALSE;
//	}
//
//	// ���� ������ ��� ������ ���ϴ�.
//	std::ofstream outputFile(outputFilePath, std::ios::binary);
//	if (!outputFile) {
//		std::cerr << "��� ������ �� �� �����ϴ�: " << outputFilePath << std::endl;
//		return FALSE;
//	}
//
//	// �Է� ���� ũ�� ���ϱ�
//	inputFile.seekg(0, std::ios::end);
//	size_t inputFileSize = inputFile.tellg();
//	inputFile.seekg(0, std::ios::beg);
//
//	// Gzip ����� �������� ������ Ǭ �� ������ ���� ����
//	std::vector<char> inputBuffer(inputFileSize);
//	inputFile.read(inputBuffer.data(), inputFileSize);
//	inputFile.close();
//
//	// zlib ���� �ʱ�ȭ
//	z_stream strm;
//	memset(&strm, 0, sizeof(strm));
//
//	// ���� ���� ��Ʈ�� �ʱ�ȭ
//	if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK) {
//		std::cerr << "zlib �ʱ�ȭ ����!" << std::endl;
//		return FALSE;
//	}
//
//	// �Է� ������ ����
//	strm.avail_in = inputBuffer.size();
//	strm.next_in = reinterpret_cast<Bytef*>(inputBuffer.data());
//
//	// ��� ���� ����
//	std::vector<char> outputBuffer(1024);  // ��� ���� ũ��
//
//	// ���� ����
//	while (strm.avail_in > 0) {
//		strm.avail_out = outputBuffer.size();
//		strm.next_out = reinterpret_cast<Bytef*>(outputBuffer.data());
//
//		int ret = inflate(&strm, Z_NO_FLUSH);
//		if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
//			std::cerr << "���� ���� ����!" << std::endl;
//			return FALSE;
//		}
//
//		// ��� ���Ͽ� �����͸� ���ϴ�.
//		size_t bytesWritten = outputBuffer.size() - strm.avail_out;
//		outputFile.write(outputBuffer.data(), bytesWritten);
//	}
//
//	// ���� ���� ��Ʈ�� ����
//	inflateEnd(&strm);
//
//	outputFile.close();
//	std::cout << "���� ���� �Ϸ�!" << std::endl;
//	return true;
//}