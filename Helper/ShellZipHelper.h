#pragma once

#include <afx.h>
#include <shlobj.h>
#include <comdef.h>

class CShellZipHelper {
public:
	CShellZipHelper();
	~CShellZipHelper();

	// ZIP ���� ���� (����)
	bool CreateZip(const CString& zipFilePath, const CStringArray& filesToCompress);

	// ZIP ���Ͽ��� ���� ����
	bool ExtractZip(const CString& zipFilePath, const CString& extractToFolder);

private:
	// �� ZIP ���� ����
	bool CreateEmptyZip(const CString& zipFilePath);

	// ������ ZIP�� �߰�
	bool AddFileToZip(CComPtr<Folder>& pZipFolder, const CString& filePath);
};
