#include "stdafx.h"
#include "ZlibHelper.h"


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>

#include "../zlib-1.3.1/zlib.h"
#pragma comment(lib, "lib\\zlibstat.lib")

CZlibHelper::CZlibHelper()
{
}


CZlibHelper::~CZlibHelper()
{
}

// ���� ���� �Լ� (Decompress)
BOOL Decompress(LPCTSTR inputFilePath, LPCTSTR outputFilePath)
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
	std::vector<char> compressedData(compressedSize);
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
BOOL Compress(LPCTSTR inputFilePath, LPCTSTR outputFilePath)
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
	std::vector<char> inputData(inputSize);
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