#include "../stdafx.h"
#include "CLIHelper.h"
#include "FileHelper.h"
#include "DirectoryHelper.h"
#include "ZipHelper.h"
#include "ResourceManager.h"
#include "../resource.h"
#include <iostream>
#include <fcntl.h>
#include <io.h>

CCLIHelper::CCLIHelper()
{
	m_bConsoleAttached = FALSE;
}

CCLIHelper::~CCLIHelper()
{
	DetachConsole();
}

void CCLIHelper::AttachConsole()
{
	if (m_bConsoleAttached) return;

	// Try to attach to parent console (if launched from cmd)
	if (::AttachConsole(ATTACH_PARENT_PROCESS))
	{
		m_bConsoleAttached = TRUE;
	}
	else
	{
		// Allocate new console if no parent console
		if (::AllocConsole())
		{
			m_bConsoleAttached = TRUE;
		}
	}

	if (m_bConsoleAttached)
	{
		// Redirect stdout
		FILE* fp;
		freopen_s(&fp, "CONOUT$", "w", stdout);
		freopen_s(&fp, "CONOUT$", "w", stderr);
		freopen_s(&fp, "CONIN$", "r", stdin);

		std::cout.clear();
		std::cerr.clear();
		std::cin.clear();
	}
}

void CCLIHelper::DetachConsole()
{
	if (m_bConsoleAttached)
	{
		// Send Enter to release the console
		INPUT ip;
		ip.type = INPUT_KEYBOARD;
		ip.ki.wScan = 0;
		ip.ki.time = 0;
		ip.ki.dwExtraInfo = 0;
		ip.ki.wVk = VK_RETURN;
		ip.ki.dwFlags = 0;
		SendInput(1, &ip, sizeof(INPUT));
		ip.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, &ip, sizeof(INPUT));

		::FreeConsole();
		m_bConsoleAttached = FALSE;
	}
}

BOOL CCLIHelper::ParseCommandLine(LPCTSTR lpCmdLine, CLI_PARAMS& params)
{
	CString strCmdLine = lpCmdLine;
	strCmdLine.Trim();

	if (strCmdLine.IsEmpty())
		return FALSE;

	// Parse command line into argc/argv style
	int argc = 0;

	// Use CommandLineToArgvW for proper parsing
	LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argvW == NULL || argc <= 1)
	{
		if (argvW) LocalFree(argvW);
		return FALSE;
	}

	// Any argument means CLI mode
	params.bCLIMode = TRUE;

	// Parse arguments
	for (int i = 1; i < argc; i++)
	{
		CString strArg = argvW[i];

		if (strArg == _T("-p") || strArg == _T("/p") || strArg == _T("--path"))
		{
			if (i + 1 < argc)
			{
				params.strInputPath = argvW[++i];
				params.strInputPath.Trim(_T("\""));
			}
		}
		else if (strArg == _T("-i") || strArg == _T("/i") || strArg == _T("--icon"))
		{
			if (i + 1 < argc)
			{
				params.strIconPath = argvW[++i];
				params.strIconPath.Trim(_T("\""));
			}
		}
		else if (strArg == _T("-r") || strArg == _T("/r") || strArg == _T("--run"))
		{
			if (i + 1 < argc)
			{
				params.strRunAfter = argvW[++i];
				params.strRunAfter.Trim(_T("\""));
			}
		}
		else if (strArg == _T("-o") || strArg == _T("/o") || strArg == _T("--output"))
		{
			if (i + 1 < argc)
			{
				params.strOutputPath = argvW[++i];
				params.strOutputPath.Trim(_T("\""));
			}
		}
		else if (strArg == _T("-v") || strArg == _T("/v") || strArg == _T("--version"))
		{
			if (i + 1 < argc)
			{
				params.strVersion = argvW[++i];
				params.strVersion.Trim(_T("\""));
			}
		}
		else if (strArg == _T("-n") || strArg == _T("/n") || strArg == _T("--name"))
		{
			if (i + 1 < argc)
			{
				params.strProductName = argvW[++i];
				params.strProductName.Trim(_T("\""));
			}
		}
		else if (strArg == _T("-c") || strArg == _T("/c") || strArg == _T("--copyright"))
		{
			if (i + 1 < argc)
			{
				params.strCopyright = argvW[++i];
				params.strCopyright.Trim(_T("\""));
			}
		}
		else if (strArg == _T("-d") || strArg == _T("/d") || strArg == _T("--description"))
		{
			if (i + 1 < argc)
			{
				params.strDescription = argvW[++i];
				params.strDescription.Trim(_T("\""));
			}
		}
		else if (strArg == _T("-s") || strArg == _T("/s") || strArg == _T("--silent"))
		{
			params.bSilent = TRUE;
		}
		else if (strArg == _T("-h") || strArg == _T("/h") || strArg == _T("--help") || strArg == _T("/?"))
		{
			params.bHelp = TRUE;
		}
	}

	LocalFree(argvW);
	return params.bCLIMode;
}

void CCLIHelper::ShowHelp()
{
	AttachConsole();

	std::cout << std::endl;
	std::cout << "WinSFXMaker - Self-Extracting Archive Creator" << std::endl;
	std::cout << "==============================================" << std::endl;
	std::cout << std::endl;
	std::cout << "Usage: WinSFXMaker.exe [options]" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -p, --path <path>        Path to directory to compress (required)" << std::endl;
	std::cout << "  -o, --output <file>      Output SFX file path (optional, auto-generated if not specified)" << std::endl;
	std::cout << "  -i, --icon <file>        Icon file (.ico) for the SFX executable" << std::endl;
	std::cout << "  -r, --run <file>         Executable to run after extraction (relative to extracted path)" << std::endl;
	std::cout << "  -v, --version <ver>      Version string (e.g., \"1.0.0.0\")" << std::endl;
	std::cout << "  -n, --name <name>        Product name" << std::endl;
	std::cout << "  -c, --copyright <text>   Copyright text" << std::endl;
	std::cout << "  -d, --description <text> File description" << std::endl;
	std::cout << "  -s, --silent             Silent mode (no output)" << std::endl;
	std::cout << "  -h, --help               Show this help message" << std::endl;
	std::cout << std::endl;
	std::cout << "Examples:" << std::endl;
	std::cout << "  WinSFXMaker.exe -p \"C:\\MyApp\" -i \"app.ico\" -r \"MyApp.exe\"" << std::endl;
	std::cout << "  WinSFXMaker.exe -p \"C:\\MyApp\" -o \"C:\\Output\\Setup.exe\" -r \"Setup.exe\"" << std::endl;
	std::cout << std::endl;
}

void CCLIHelper::ShowError(LPCTSTR lpszError)
{
	AttachConsole();
	std::wcerr << L"Error: " << lpszError << std::endl;
}

void CCLIHelper::ShowMessage(LPCTSTR lpszMessage)
{
	AttachConsole();
	std::wcout << lpszMessage << std::endl;
}

int CCLIHelper::ExecuteCLI(const CLI_PARAMS& params)
{
	if (!params.bSilent)
	{
		AttachConsole();
		std::cout << std::endl;
		std::cout << "WinSFXMaker - CLI Mode" << std::endl;
		std::cout << "======================" << std::endl;
	}

	// Validate required parameters
	if (params.strInputPath.IsEmpty())
	{
		if (!params.bSilent)
			ShowError(_T("Input path (-p) is required."));
		return 1;
	}

	if (!PathIsDirectory(params.strInputPath))
	{
		if (!params.bSilent)
			ShowError(_T("Input path does not exist or is not a directory."));
		return 1;
	}

	// Validate icon file if specified
	if (!params.strIconPath.IsEmpty() && !PathFileExists(params.strIconPath))
	{
		if (!params.bSilent)
			ShowError(_T("Icon file does not exist."));
		return 1;
	}

	// Generate output path if not specified
	CString strOutputPath = params.strOutputPath;
	if (strOutputPath.IsEmpty())
	{
		strOutputPath = params.strInputPath;
		PathAddBackslash(strOutputPath.GetBuffer(MAX_PATH));
		strOutputPath.ReleaseBuffer();
		strOutputPath += PathFindFileName(params.strInputPath);
		strOutputPath += _T(".exe");
	}

	if (!params.bSilent)
	{
		std::wcout << L"Input Path:  " << (LPCTSTR)params.strInputPath << std::endl;
		std::wcout << L"Output File: " << (LPCTSTR)strOutputPath << std::endl;
		if (!params.strIconPath.IsEmpty())
			std::wcout << L"Icon:        " << (LPCTSTR)params.strIconPath << std::endl;
		if (!params.strRunAfter.IsEmpty())
			std::wcout << L"Run After:   " << (LPCTSTR)params.strRunAfter << std::endl;
		std::cout << std::endl;
	}

	// Step 1: Create ZIP file
	if (!params.bSilent)
		std::cout << "Compressing files..." << std::endl;

	CString strTempPath = CDirectoryHelper::GetTempPath();
	PathAddBackslash(strTempPath.GetBuffer(MAX_PATH));
	strTempPath.ReleaseBuffer();

	CString strZipFile = CFileHelper::GetTimeBaseFileName(strTempPath, _T("zip"));

	CZipHelper zipHelper;
	zipHelper.AddCompressTarget(params.strInputPath);
	if (!zipHelper.Compress(strZipFile))
	{
		if (!params.bSilent)
			ShowError(_T("Failed to compress files."));
		return 2;
	}

	// Step 2: Extract template file
	if (!params.bSilent)
		std::cout << "Preparing template..." << std::endl;

	CString strTemplateFile = CFileHelper::GetTimeBaseFileName(strTempPath, _T("exe"));

	HRSRC hRes = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_TEMPLATE_BINARY), _T("BINARY"));
	if (!hRes)
	{
		DeleteFile(strZipFile);
		if (!params.bSilent)
			ShowError(_T("Failed to find template resource."));
		return 3;
	}

	DWORD dwSize = SizeofResource(NULL, hRes);
	HGLOBAL hGlobal = LoadResource(NULL, hRes);
	if (!hGlobal || dwSize == 0)
	{
		DeleteFile(strZipFile);
		if (!params.bSilent)
			ShowError(_T("Failed to load template resource."));
		return 3;
	}

	LPVOID pData = LockResource(hGlobal);
	if (!pData)
	{
		DeleteFile(strZipFile);
		if (!params.bSilent)
			ShowError(_T("Failed to lock template resource."));
		return 3;
	}

	CFile templateOutFile;
	if (!templateOutFile.Open(strTemplateFile, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
	{
		DeleteFile(strZipFile);
		if (!params.bSilent)
			ShowError(_T("Failed to create template file."));
		return 3;
	}
	templateOutFile.Write(pData, dwSize);
	templateOutFile.Close();
	UnlockResource(hGlobal);

	// Step 3: Apply icon if specified
	if (!params.strIconPath.IsEmpty())
	{
		if (!params.bSilent)
			std::cout << "Applying icon..." << std::endl;

		CResourceManager resMgr;
		if (!resMgr.ChangeExeIcon(strTemplateFile, params.strIconPath))
		{
			if (!params.bSilent)
				std::cout << "Warning: Failed to apply icon." << std::endl;
		}
	}

	// Step 4: Apply version info
	if (!params.strVersion.IsEmpty() || !params.strProductName.IsEmpty() ||
		!params.strCopyright.IsEmpty() || !params.strDescription.IsEmpty())
	{
		if (!params.bSilent)
			std::cout << "Applying version info..." << std::endl;

		CResourceManager resMgr;
		resMgr.UpdateVersionInfo(strTemplateFile, _T(""), params.strProductName,
			params.strVersion, params.strCopyright, params.strDescription);
	}

	// Step 5: Create SFX file
	if (!params.bSilent)
		std::cout << "Creating SFX file..." << std::endl;

	CFile templateFile, zipFile, outFile;
	BOOL bSuccess = FALSE;

	if (templateFile.Open(strTemplateFile, CFile::modeRead | CFile::typeBinary))
	{
		ULONGLONG templateSize = templateFile.GetLength();
		BYTE* pTemplateData = new BYTE[(size_t)templateSize];
		templateFile.Read(pTemplateData, (UINT)templateSize);
		templateFile.Close();

		if (zipFile.Open(strZipFile, CFile::modeRead | CFile::typeBinary))
		{
			ULONGLONG zipSize = zipFile.GetLength();
			BYTE* pZipData = new BYTE[(size_t)zipSize];
			zipFile.Read(pZipData, (UINT)zipSize);
			zipFile.Close();

			if (outFile.Open(strOutputPath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
			{
				outFile.Write(pTemplateData, (UINT)templateSize);
				outFile.Write(pZipData, (UINT)zipSize);

				// Write ZIP size for extraction
				LONGLONG llZipSize = (LONGLONG)zipSize;
				outFile.Write(&llZipSize, sizeof(llZipSize));

				// Write run-after executable path if specified
				if (!params.strRunAfter.IsEmpty())
				{
					// Convert to relative path from input folder
					CString strRunAfterFile = params.strRunAfter;
					
					// If it's an absolute path, make it relative to input path
					if (PathIsRelative(strRunAfterFile) == FALSE)
					{
						// Get input path with backslash
						CString strInputBase = params.strInputPath;
						PathAddBackslash(strInputBase.GetBuffer(MAX_PATH));
						strInputBase.ReleaseBuffer();
						
						// Check if run-after path starts with input path
						CString strRunAfterLower = strRunAfterFile;
						CString strInputBaseLower = strInputBase;
						strRunAfterLower.MakeLower();
						strInputBaseLower.MakeLower();
						
						if (strRunAfterLower.Find(strInputBaseLower) == 0)
						{
							// Extract relative path (remove input base path)
							strRunAfterFile = params.strRunAfter.Mid(strInputBase.GetLength());
						}
						else
						{
							// Not under input path, use filename only
							strRunAfterFile = PathFindFileName(params.strRunAfter);
						}
					}
					
					// Write marker for run-after info
					DWORD dwMarker = 0x52554E41; // "RUNA"
					outFile.Write(&dwMarker, sizeof(dwMarker));

					// Write run-after path length and string
					int nLen = strRunAfterFile.GetLength();
					outFile.Write(&nLen, sizeof(nLen));
					outFile.Write((LPCTSTR)strRunAfterFile, nLen * sizeof(TCHAR));

					// Write end marker
					DWORD dwEndMarker = 0x454E4421; // "END!"
					outFile.Write(&dwEndMarker, sizeof(dwEndMarker));
				}

				outFile.Close();
				bSuccess = TRUE;
			}
			delete[] pZipData;
		}
		delete[] pTemplateData;
	}

	// Cleanup temp files
	DeleteFile(strZipFile);
	DeleteFile(strTemplateFile);

	if (bSuccess)
	{
		if (!params.bSilent)
		{
			std::cout << std::endl;
			std::cout << "SFX file created successfully!" << std::endl;
			std::wcout << L"Output: " << (LPCTSTR)strOutputPath << std::endl;
		}
		return 0;
	}
	else
	{
		if (!params.bSilent)
			ShowError(_T("Failed to create SFX file."));
		return 4;
	}
}
