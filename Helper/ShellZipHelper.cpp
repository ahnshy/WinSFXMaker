#include "../stdafx.h"

#include <afx.h>
#include "ShellZipHelper.h"

CShellZipHelper::CShellZipHelper()
{
	CoInitialize(nullptr);
}

CShellZipHelper::~CShellZipHelper()
{
	CoUninitialize();
}

BOOL CShellZipHelper::CreateZip(const CString& zipFilePath, const CStringArray& filesToCompress)
{
		if (filesToCompress.IsEmpty()) {
			AfxMessageBox(_T("������ ������ �����ϴ�."));
			return FALSE;
		}

		// ZIP ���� ����
		if (!CreateEmptyZip(zipFilePath)) {
			AfxMessageBox(_T("ZIP ������ �������� ���߽��ϴ�."));
			return FALSE;
		}

		// ���� �߰�
		CComPtr<IShellDispatch> pShellDispatch;
		HRESULT hr = CoCreateInstance(CLSID_Shell, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pShellDispatch));
		if (FAILED(hr) || !pShellDispatch) {
			AfxMessageBox(_T("IShellDispatch �ʱ�ȭ ����"));
			return FALSE;
		}

		CComVariant zipFile(zipFilePath);
		CComPtr<Folder> pZipFolder;
		hr = pShellDispatch->NameSpace(zipFile, &pZipFolder);
		if (FAILED(hr) || !pZipFolder) {
			AfxMessageBox(_T("ZIP ���� ���� ����"));
			return FALSE;
		}

		for (int i = 0; i < filesToCompress.GetSize(); ++i) {
			CString filePath = filesToCompress[i];
			if (!AddFileToZip(pZipFolder, filePath)) {
				CString msg;
				msg.Format(_T("���� �߰� ����: %s"), filePath);
				AfxMessageBox(msg);
				return FALSE;
			}
		}

		return TRUE;
}

BOOL CShellZipHelper::ExtractZip(const CString& zipFilePath, const CString& extractToFolder)
{
	CComPtr<IShellDispatch> pShellDispatch;
	HRESULT hr = CoCreateInstance(CLSID_Shell, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pShellDispatch));
	if (FAILED(hr) || !pShellDispatch) {
		AfxMessageBox(_T("IShellDispatch �ʱ�ȭ ����"));
		return FALSE;
	}

	CComVariant zipFile(zipFilePath);
	CComPtr<Folder> pZipFolder;
	hr = pShellDispatch->NameSpace(zipFile, &pZipFolder);
	if (FAILED(hr) || !pZipFolder) {
		AfxMessageBox(_T("ZIP ���� ���� ����"));
		return FALSE;
	}

	CComVariant extractFolder(extractToFolder);
	CComPtr<Folder> pExtractFolder;
	hr = pShellDispatch->NameSpace(extractFolder, &pExtractFolder);
	if (FAILED(hr) || !pExtractFolder) {
		AfxMessageBox(_T("������ ���� ���� ����"));
		return FALSE;
	}

	CComVariant options(0); // �� ��° �Ű������� �⺻ �ɼ� ����
	hr = pExtractFolder->CopyHere(CComVariant(pZipFolder), options);
	return SUCCEEDED(hr);
}

BOOL CShellZipHelper::CreateEmptyZip(const CString& zipFilePath)
{
		HANDLE hFile = CreateFile(zipFilePath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		BYTE zipHeader[] = { 0x50, 0x4B, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00 };
		DWORD bytesWritten;
		WriteFile(hFile, zipHeader, sizeof(zipHeader), &bytesWritten, nullptr);
		CloseHandle(hFile);

		return bytesWritten == sizeof(zipHeader);
}

BOOL CShellZipHelper::AddFileToZip(CComPtr<Folder>& pZipFolder, const CString& filePath)
{
	CComVariant file(filePath);   // ù ��° �Ű�����: �߰��� ������ ���
	CComVariant options(0);       // �� ��° �Ű�����: �ɼ� (0�� �⺻ �ɼ�)

	HRESULT hr = pZipFolder->CopyHere(file, options); // CopyHere ȣ��
	return SUCCEEDED(hr);         // �۾� ���� ���� ��ȯ
}