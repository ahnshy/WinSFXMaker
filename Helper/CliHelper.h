#pragma once

// CLI command line parameters structure
struct CLI_PARAMS
{
	BOOL bCLIMode;          // TRUE if running in CLI mode
	CString strInputPath;   // -p: Path to compress
	CString strIconPath;    // -i: Icon file path
	CString strRunAfter;    // -r: Executable to run after extraction
	CString strOutputPath;  // -o: Output SFX file path (optional)
	CString strVersion;     // -v: Version string (optional)
	CString strProductName; // -n: Product name (optional)
	CString strCopyright;   // -c: Copyright (optional)
	CString strDescription; // -d: Description (optional)
	BOOL bSilent;           // -s: Silent mode (no console output)
	BOOL bHelp;             // -h or --help: Show help

	CLI_PARAMS()
	{
		bCLIMode = FALSE;
		bSilent = FALSE;
		bHelp = FALSE;
	}
};

class CCLIHelper
{
public:
	CCLIHelper();
	~CCLIHelper();

	// Parse command line arguments
	BOOL ParseCommandLine(int argc, TCHAR* argv[], CLI_PARAMS& params);
	BOOL ParseCommandLine(LPCTSTR lpCmdLine, CLI_PARAMS& params);

	// Execute CLI mode (create SFX without GUI)
	int ExecuteCLI(const CLI_PARAMS& params);

	// Show help message
	void ShowHelp();

	// Show error message
	void ShowError(LPCTSTR lpszError);

	// Show message
	void ShowMessage(LPCTSTR lpszMessage);

private:
	// Attach console for output
	void AttachConsole();
	void DetachConsole();

	BOOL m_bConsoleAttached;
};
