#pragma once

#include <afx.h>
#include <shlobj.h>
#include <comdef.h>

class CShellZipHelper {
public:
	CShellZipHelper();
	~CShellZipHelper();

	// ZIP 파일 생성 (압축)
	bool CreateZip(const CString& zipFilePath, const CStringArray& filesToCompress);

	// ZIP 파일에서 압축 해제
	bool ExtractZip(const CString& zipFilePath, const CString& extractToFolder);

private:
	// 빈 ZIP 파일 생성
	bool CreateEmptyZip(const CString& zipFilePath);

	// 파일을 ZIP에 추가
	bool AddFileToZip(CComPtr<Folder>& pZipFolder, const CString& filePath);
};
