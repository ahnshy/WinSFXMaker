#pragma once

#include <afx.h>
#include <shlobj.h>
#include <comdef.h>

class CShellZipHelper {
public:
	CShellZipHelper();
	~CShellZipHelper();

	// ZIP ���� ���� (����)
	BOOL CreateZip(const CString& zipFilePath, const CStringArray& filesToCompress);

	// ZIP ���Ͽ��� ���� ����
	BOOL ExtractZip(const CString& zipFilePath, const CString& extractToFolder);

private:
	// �� ZIP ���� ����
	BOOL CreateEmptyZip(const CString& zipFilePath);

	// ������ ZIP�� �߰�
	BOOL AddFileToZip(CComPtr<Folder>& pZipFolder, const CString& filePath);
};
