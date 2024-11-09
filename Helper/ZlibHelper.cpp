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

// 압축 해제 함수 (Decompress)
BOOL Decompress(LPCTSTR inputFilePath, LPCTSTR outputFilePath)
{
	// 입력 파일 열기
	std::ifstream inputFile(inputFilePath, std::ios::binary);
	if (!inputFile.is_open()) {
		//std::cerr << "입력 파일을 열 수 없습니다: " << inputFilePath << std::endl;
		return FALSE;
	}

	// 출력 파일 열기
	std::ofstream outputFile(outputFilePath, std::ios::binary);
	if (!outputFile.is_open()) {
		//std::cerr << "출력 파일을 열 수 없습니다: " << outputFilePath << std::endl;
		return FALSE;
	}

	// 파일 크기 구하기
	inputFile.seekg(0, std::ios::end);
	std::streamsize compressedSize = inputFile.tellg();
	inputFile.seekg(0, std::ios::beg);

	// 입력 파일의 압축된 데이터를 메모리에 읽기
	std::vector<char> compressedData(compressedSize);
	inputFile.read(compressedData.data(), compressedSize);

	// zlib 스트림 준비
	z_stream strm;
	memset(&strm, 0, sizeof(strm));  // 스트림 초기화
	if (inflateInit(&strm) != Z_OK) {
		//std::cerr << "zlib 초기화 실패" << std::endl;
		return FALSE;
	}

	// 압축 해제할 버퍼 준비
	const size_t bufferSize = 1024 * 8;  // 8KB 크기의 버퍼
	std::vector<char> buffer(bufferSize);

	// zlib 압축 해제 처리
	strm.avail_in = static_cast<uInt>(compressedSize);
	strm.next_in = reinterpret_cast<Bytef*>(compressedData.data());
	strm.avail_out = static_cast<uInt>(bufferSize);
	strm.next_out = reinterpret_cast<Bytef*>(buffer.data());

	int ret = Z_OK;
	while (strm.avail_in > 0) {
		ret = inflate(&strm, Z_NO_FLUSH);
		if (ret == Z_ERRNO) {
			//std::cerr << "압축 해제 실패" << std::endl;
			inflateEnd(&strm);
			return FALSE;
		}

		// 압축 해제된 데이터가 있으면 출력 파일에 쓴다.
		if (strm.avail_out == 0 || ret == Z_STREAM_END) {
			outputFile.write(buffer.data(), bufferSize - strm.avail_out);
			strm.avail_out = bufferSize;
			strm.next_out = reinterpret_cast<Bytef*>(buffer.data());
		}
	}

	// 모든 데이터가 해제된 후 처리
	while (ret != Z_STREAM_END) {
		ret = inflate(&strm, Z_FINISH);
		if (ret == Z_ERRNO) {
			//std::cerr << "압축 해제 실패" << std::endl;
			inflateEnd(&strm);
			return FALSE;
		}

		// 남은 데이터를 출력 파일에 쓴다.
		outputFile.write(buffer.data(), bufferSize - strm.avail_out);
		strm.avail_out = bufferSize;
		strm.next_out = reinterpret_cast<Bytef*>(buffer.data());
	}

	// zlib 스트림 종료
	inflateEnd(&strm);

	//std::cout << "압축 해제 완료: " << outputFilePath << std::endl;
	return TRUE;
}

// 압축 함수 (Compress)
BOOL Compress(LPCTSTR inputFilePath, LPCTSTR outputFilePath)
{
	// 입력 파일 열기
	std::ifstream inputFile(inputFilePath, std::ios::binary);
	if (!inputFile.is_open()) {
		//std::cerr << "입력 파일을 열 수 없습니다: " << inputFilePath << std::endl;
		return FALSE;
	}

	// 출력 파일 열기
	std::ofstream outputFile(outputFilePath, std::ios::binary);
	if (!outputFile.is_open()) {
		//std::cerr << "출력 파일을 열 수 없습니다: " << outputFilePath << std::endl;
		return FALSE;
	}

	// 파일 크기 구하기
	inputFile.seekg(0, std::ios::end);
	std::streamsize inputSize = inputFile.tellg();
	inputFile.seekg(0, std::ios::beg);

	// 입력 파일의 데이터를 메모리에 읽기
	std::vector<char> inputData(inputSize);
	inputFile.read(inputData.data(), inputSize);

	// zlib 스트림 준비
	z_stream strm;
	memset(&strm, 0, sizeof(strm));  // 스트림 초기화
	if (deflateInit(&strm, Z_DEFAULT_COMPRESSION) != Z_OK) {
		//std::cerr << "zlib 초기화 실패" << std::endl;
		return FALSE;
	}

	// 압축할 버퍼 준비
	const size_t bufferSize = 1024 * 8;  // 8KB 크기의 버퍼
	std::vector<char> buffer(bufferSize);

	// 압축
	strm.avail_in = static_cast<uInt>(inputSize);
	strm.next_in = reinterpret_cast<Bytef*>(inputData.data());
	strm.avail_out = static_cast<uInt>(bufferSize);
	strm.next_out = reinterpret_cast<Bytef*>(buffer.data());

	int ret = Z_OK;
	while (strm.avail_in > 0) {
		ret = deflate(&strm, Z_NO_FLUSH);
		if (ret == Z_ERRNO) {
			//std::cerr << "압축 실패" << std::endl;
			deflateEnd(&strm);
			return FALSE;
		}

		// 압축된 데이터가 있으면 출력 파일에 쓴다.
		if (strm.avail_out == 0 || ret == Z_STREAM_END) {
			outputFile.write(buffer.data(), bufferSize - strm.avail_out);
			strm.avail_out = bufferSize;
			strm.next_out = reinterpret_cast<Bytef*>(buffer.data());
		}
	}

	// 모든 데이터가 압축된 후 처리
	while (ret != Z_STREAM_END) {
		ret = deflate(&strm, Z_FINISH);
		if (ret == Z_ERRNO) {
			//std::cerr << "압축 실패" << std::endl;
			deflateEnd(&strm);
			return FALSE;
		}

		// 남은 압축된 데이터를 출력 파일에 쓴다.
		outputFile.write(buffer.data(), bufferSize - strm.avail_out);
		strm.avail_out = bufferSize;
		strm.next_out = reinterpret_cast<Bytef*>(buffer.data());
	}

	// zlib 스트림 종료
	deflateEnd(&strm);

	//std::cout << "압축 완료: " << outputFilePath << std::endl;
	return TRUE;
}