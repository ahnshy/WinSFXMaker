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

// 압축 해제 함수 (Decompress)
BOOL CZlibHelper::Decompress(LPCTSTR inputFilePath, LPCTSTR outputFilePath)
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
	std::vector<char> compressedData((int)compressedSize);
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
BOOL CZlibHelper::Compress(LPCTSTR inputFilePath, LPCTSTR outputFilePath)
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
	std::vector<char> inputData((int)inputSize);
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

// 압축할 파일을 ZIP 형식으로 저장
BOOL CZlibHelper::CompressFile(const CString& strPath, gzFile& gzFile)
{
	FILE* pFile = NULL;
	_wfopen_s(&pFile, (LPCTSTR)strPath, _T("rb"));
	if (pFile == nullptr)
	{
		//std::cerr << "파일 열기 실패: " << filePath.GetString() << std::endl;
		return FALSE;
	}

	const size_t bufferSize = 1024;
	char buffer[bufferSize];
	int bytesRead;

	while ((bytesRead = fread(buffer, 1, bufferSize, pFile)) > 0)
	{
		if (gzwrite(gzFile, buffer, bytesRead) != bytesRead)
		{
			//std::cerr << "압축 쓰기 오류: " << filePath.GetString() << std::endl;
			fclose(pFile);
			return FALSE;
		}
	}

	fclose(pFile);
	return TRUE;
}

// 디렉토리와 서브폴더를 재귀적으로 탐색하면서 파일을 압축
void CZlibHelper::AddFilesToZip(const CString& dirPath, const CString& baseDir, gzFile& gzFile)
{
	CFileFind finder;
	CString strFileName;

	// 폴더 내 모든 파일과 서브폴더를 찾기 위한 패턴
	strFileName = dirPath;
	strFileName += "\\*.*";

	BOOL bWorking = finder.FindFile(strFileName);
	while (bWorking) {
		bWorking = finder.FindNextFile();
		if (finder.IsDots()) {
			continue; // "." 및 ".."는 건너뜁니다.
		}

		// 서브폴더일 경우 재귀적으로 탐색
		if (finder.IsDirectory()) {
			CString subDir = finder.GetFilePath();
			AddFilesToZip(subDir, baseDir, gzFile); // 재귀 호출
		}
		else {
			CString filePath = finder.GetFilePath();
			CString relativePath = filePath.Right(filePath.GetLength() - baseDir.GetLength() - 1); // baseDir 상대 경로

			std::wcout << L"압축 중: " << (LPCWSTR)relativePath << std::endl;
			CompressFile(relativePath, gzFile); // 파일을 압축
		}
	}
}

bool CZlibHelper::CreateDirectoryRecursively(const CString& path)
{
	int result = _wmkdir(path);
	if (result == 0 || errno == EEXIST) {
		return true;  // 디렉토리 생성 성공 또는 이미 존재
	}
	else {
		std::cerr << "디렉토리 생성 실패: " << path.GetString() << std::endl;
		return false;
	}
}

// .gz 파일을 풀어서 지정된 경로에 저장하는 함수
bool CZlibHelper::DecompressGZFile(const CString& gzFilePath, const CString& outputFilePath)
{
	// .gz 파일 열기
	gzFile gzFile = gzopen(CT2A(gzFilePath.GetString()), "rb");
	if (gzFile == nullptr) {
		std::cerr << "파일 열기 실패: " << gzFilePath.GetString() << std::endl;
		return false;
	}

	// 출력 파일을 열기 전에 디렉토리 확인 및 생성
	CString outputDir = outputFilePath.Left(outputFilePath.ReverseFind('\\'));
	if (!CreateDirectoryRecursively(outputDir)) {
		gzclose(gzFile);
		return false;
	}

	// 출력 파일 열기
	std::ofstream outFile(outputFilePath.GetString(), std::ios::binary);
	if (!outFile.is_open()) {
		std::cerr << "출력 파일 열기 실패: " << outputFilePath.GetString() << std::endl;
		gzclose(gzFile);
		return false;
	}

	// 버퍼 설정
	const size_t bufferSize = 4096;
	char buffer[bufferSize];
	int bytesRead = 0;

	// 압축 풀기
	while ((bytesRead = gzread(gzFile, buffer, bufferSize)) > 0) {
		outFile.write(buffer, bytesRead);
	}

	// 파일 닫기
	outFile.close();
	gzclose(gzFile);

	return true;
}

// 사용법
//{
//	CString inputDir = _T("C:\\TestFolder"); // 압축할 폴더 경로
//	CString outputZip = _T("C:\\output.gz"); // 출력 압축 파일 경로 (gz 파일)
//
//	gzFile gzFile = gzopen(CT2A(outputZip.GetString()), "wb");
//	if (gzFile == nullptr) {
//		std::cerr << "압축 파일 열기 실패: " << (LPCWSTR)outputZip << std::endl;
//		return -1;
//	}
//
//	AddFilesToZip(inputDir, inputDir, gzFile); // 파일 및 폴더 추가
//
//	gzclose(gzFile); // 압축 파일 닫기
//	std::wcout << L"압축 완료: " << (LPCWSTR)outputZip << std::endl;
//
//	return 0;
//}