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

// ZIP 파일 추출 함수
//BOOL CZlibHelper::Decompress(LPCTSTR inputFilePath, LPCTSTR outputFilePath)
//{
//	// 입력 파일을 바이너리 모드로 엽니다.
//	std::ifstream inputFile(inputFilePath, std::ios::binary);
//	if (!inputFile) {
//		std::cerr << "입력 파일을 열 수 없습니다: " << inputFilePath << std::endl;
//		return FALSE;
//	}
//
//	// 압축 해제할 출력 파일을 엽니다.
//	std::ofstream outputFile(outputFilePath, std::ios::binary);
//	if (!outputFile) {
//		std::cerr << "출력 파일을 열 수 없습니다: " << outputFilePath << std::endl;
//		return FALSE;
//	}
//
//	// 입력 파일 크기 구하기
//	inputFile.seekg(0, std::ios::end);
//	size_t inputFileSize = inputFile.tellg();
//	inputFile.seekg(0, std::ios::beg);
//
//	// Gzip 헤더와 데이터의 압축을 푼 후 저장할 버퍼 생성
//	std::vector<char> inputBuffer(inputFileSize);
//	inputFile.read(inputBuffer.data(), inputFileSize);
//	inputFile.close();
//
//	// zlib 상태 초기화
//	z_stream strm;
//	memset(&strm, 0, sizeof(strm));
//
//	// 압축 해제 스트림 초기화
//	if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK) {
//		std::cerr << "zlib 초기화 실패!" << std::endl;
//		return FALSE;
//	}
//
//	// 입력 데이터 설정
//	strm.avail_in = inputBuffer.size();
//	strm.next_in = reinterpret_cast<Bytef*>(inputBuffer.data());
//
//	// 출력 버퍼 설정
//	std::vector<char> outputBuffer(1024);  // 출력 버퍼 크기
//
//	// 압축 해제
//	while (strm.avail_in > 0) {
//		strm.avail_out = outputBuffer.size();
//		strm.next_out = reinterpret_cast<Bytef*>(outputBuffer.data());
//
//		int ret = inflate(&strm, Z_NO_FLUSH);
//		if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
//			std::cerr << "압축 해제 실패!" << std::endl;
//			return FALSE;
//		}
//
//		// 출력 파일에 데이터를 씁니다.
//		size_t bytesWritten = outputBuffer.size() - strm.avail_out;
//		outputFile.write(outputBuffer.data(), bytesWritten);
//	}
//
//	// 압축 해제 스트림 종료
//	inflateEnd(&strm);
//
//	outputFile.close();
//	std::cout << "파일 추출 완료!" << std::endl;
//	return true;
//}