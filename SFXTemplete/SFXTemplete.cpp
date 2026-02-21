
// SFXTemplete.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "SFXTemplete.h"

#include <fstream>
#include <vector>

#include "Helper/ZipHelper.h"
#include "Helper/DirectoryHelper.h"
#include "Helper/Logger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSFXTempleteApp
BEGIN_MESSAGE_MAP(CSFXTempleteApp, CWinApp)
END_MESSAGE_MAP()

// CSFXTempleteApp construction
CSFXTempleteApp::CSFXTempleteApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CSFXTempleteApp object
CSFXTempleteApp theApp;

// CSFXTempleteApp initialization
BOOL CSFXTempleteApp::InitInstance()
{
	CWinApp::InitInstance();

	// Initialize logger
	LOG_INIT(NULL);
	LOG_INFO(_T("=== SFX Extractor Started ==="));

	SetRegistryKey(_T("SFX Templete App"));

	CreateSFX();

	LOG_INFO(_T("=== SFX Extractor Finished ==="));
	return FALSE;
}

void CSFXTempleteApp::CreateSFX()
{
	LPCTSTR outputPath = _T("extracted_data.zip");
	ExtractEmbeddedZip(outputPath);
}

BOOL CSFXTempleteApp::ExtractEmbeddedZip(LPCTSTR outputPath)
{
	// Get current executable path
	TCHAR exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH);
	
	LOG_INFO(_T("Executable path: %s"), exePath);

	// Open executable in binary mode
	std::ifstream exeFile(exePath, std::ios::binary | std::ios::ate);
	if (!exeFile) {
		LOG_ERROR(_T("Cannot open executable file"));
		AfxMessageBox(_T("Cannot open executable file."));
		return FALSE;
	}

	// Get file size
	std::streamsize exeSize = exeFile.tellg();
	LOG_INFO(_T("Executable size: %lld bytes"), (long long)exeSize);
	
	// Read run-after info from end of file (if exists)
	CString strRunAfter;
	BOOL bHasRunAfter = FALSE;
	
	// Check for "END!" marker at the very end
	DWORD dwEndMarker = 0;
	exeFile.seekg(exeSize - sizeof(DWORD), std::ios::beg);
	exeFile.read(reinterpret_cast<char*>(&dwEndMarker), sizeof(dwEndMarker));
	
	std::streamsize dataEndPos = exeSize;
	
	if (dwEndMarker == 0x454E4421) // "END!"
	{
		LOG_INFO(_T("Found END! marker, reading run-after info"));
		
		// Has run-after info, read it
		// Structure: [ZIP data][ZIP size 8 bytes][RUNA marker 4 bytes][path length 4 bytes][path string][END! marker 4 bytes]
		
		// Read path length (before END! marker)
		int nPathLen = 0;
		exeFile.seekg(exeSize - sizeof(DWORD) - sizeof(int), std::ios::beg);
		
		// First, find RUNA marker to calculate positions
		// Scan backwards to find RUNA marker
		for (std::streamsize pos = exeSize - 20; pos > exeSize - 1024 && pos > 0; pos--)
		{
			exeFile.seekg(pos, std::ios::beg);
			DWORD dwMarker = 0;
			exeFile.read(reinterpret_cast<char*>(&dwMarker), sizeof(dwMarker));
			if (dwMarker == 0x52554E41) // "RUNA"
			{
				LOG_INFO(_T("Found RUNA marker at position %lld"), (long long)pos);
				
				// Found RUNA marker, read path length
				exeFile.read(reinterpret_cast<char*>(&nPathLen), sizeof(nPathLen));
				
				if (nPathLen > 0 && nPathLen < MAX_PATH)
				{
					// Read path string
					std::vector<TCHAR> pathBuffer(nPathLen + 1);
					exeFile.read(reinterpret_cast<char*>(pathBuffer.data()), nPathLen * sizeof(TCHAR));
					pathBuffer[nPathLen] = _T('\0');
					strRunAfter = pathBuffer.data();
					bHasRunAfter = TRUE;
					
					LOG_INFO(_T("Run-after executable: %s"), strRunAfter);
					
					// Data ends before RUNA marker
					dataEndPos = pos;
				}
				break;
			}
		}
	}

	// Read ZIP size (8 bytes before run-after data or at end)
	std::streamsize zipSizePos = dataEndPos - sizeof(std::streamsize);
	exeFile.seekg(zipSizePos, std::ios::beg);

	std::streamsize zipSize;
	exeFile.read(reinterpret_cast<char*>(&zipSize), sizeof(zipSize));
	
	LOG_INFO(_T("ZIP data size: %lld bytes"), (long long)zipSize);

	// Move to ZIP data position and read
	exeFile.seekg(dataEndPos - zipSize - sizeof(zipSize), std::ios::beg);
	if (zipSize <= 0)
	{
		LOG_ERROR(_T("Invalid ZIP size"));
		exeFile.close();
		return FALSE;
	}

	std::vector<char> zipData((int)zipSize);
	exeFile.read(zipData.data(), zipSize);
	exeFile.close();

	// Write ZIP file
	std::ofstream outFile(outputPath, std::ios::binary);
	if (!outFile) {
		LOG_ERROR(_T("Cannot save extracted file"));
		AfxMessageBox(_T("Cannot save extracted file."));
		return FALSE;
	}
	outFile.write(zipData.data(), zipSize);
	outFile.close();
	
	LOG_INFO(_T("Temporary ZIP file created: %s"), outputPath);

	// Get temp path for extraction
	CString strPath = CDirectoryHelper::GetTempPath();
	PathAddBackslash(strPath.GetBuffer(BUFSIZ));
	strPath.ReleaseBuffer();
	
	LOG_INFO(_T("Extraction path: %s"), strPath);

	// Decompress ZIP file
	CZipHelper helper;
	helper.Decompress(outputPath, strPath);
	DeleteFile(outputPath);
	
	LOG_INFO(_T("ZIP file decompressed and deleted"));

	// Show extraction complete message
	//CString msg;
	//msg.Format(_T("Files extracted to: %s"), strPath);

	// Run executable after extraction if specified
	if (bHasRunAfter && !strRunAfter.IsEmpty())
	{
		CString strFullPath = strPath + strRunAfter;
		LOG_INFO(_T("Attempting to run: %s"), strFullPath);
		
		// Check if file exists
		if (PathFileExists(strFullPath))
		{
			// Get directory of the executable
			CString strWorkDir = strFullPath;
			PathRemoveFileSpec(strWorkDir.GetBuffer(MAX_PATH));
			strWorkDir.ReleaseBuffer();
			
			LOG_INFO(_T("Working directory: %s"), strWorkDir);
			
			// Execute the file
			SHELLEXECUTEINFO sei = { 0 };
			sei.cbSize = sizeof(sei);
			sei.fMask = SEE_MASK_NOCLOSEPROCESS;
			sei.lpVerb = _T("open");
			sei.lpFile = strFullPath;
			sei.lpDirectory = strWorkDir;
			sei.nShow = SW_SHOWNORMAL;
			
			if (!ShellExecuteEx(&sei))
			{
				DWORD dwError = GetLastError();
				LOG_ERROR(_T("Failed to execute: %s, Error code: %d"), strFullPath, dwError);
				
				CString errMsg;
				errMsg.Format(_T("Failed to execute: %s\nError code: %d"), strFullPath, dwError);
				AfxMessageBox(errMsg, MB_ICONERROR);
			}
			else
			{
				LOG_INFO(_T("Successfully launched: %s"), strFullPath);
			}
		}
		else
		{
			LOG_ERROR(_T("Executable not found: %s"), strFullPath);
			
			CString errMsg;
			errMsg.Format(_T("Executable not found: %s"), strFullPath);
			AfxMessageBox(errMsg, MB_ICONWARNING);
		}
	}

	return TRUE;
}
