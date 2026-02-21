// WinSFXMaker.cpp : Defines the class behaviors for the application.
//
#include "stdafx.h"
#include "WinSFXMaker.h"
#include "WinSFXMakerDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// CWinSFXMakerApp
BEGIN_MESSAGE_MAP(CWinSFXMakerApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()
// CWinSFXMakerApp construction
CWinSFXMakerApp::CWinSFXMakerApp()
{
	// support Restart Manager
	//m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}
// The one and only CWinSFXMakerApp object
CWinSFXMakerApp theApp;
// CWinSFXMakerApp initialization
BOOL CWinSFXMakerApp::ProcessCommandLine()
{
	CString strCmdLine = m_lpCmdLine;
	strCmdLine.Trim();
	// No command line arguments - run GUI mode
	if (strCmdLine.IsEmpty())
		return FALSE;
	// Parse command line
	m_cliHelper.ParseCommandLine(m_lpCmdLine, m_cliParams);
	// If any CLI argument is present, run in CLI mode
	if (m_cliParams.bCLIMode)
	{
		// Show help if requested
		if (m_cliParams.bHelp)
		{
			m_cliHelper.ShowHelp();
			return TRUE;
		}
		// Execute CLI mode
		int nResult = m_cliHelper.ExecuteCLI(m_cliParams);
		return TRUE;
	}
	// No valid CLI arguments - run GUI mode
	return FALSE;
}
BOOL CWinSFXMakerApp::InitInstance()
{
	// Check for command line arguments FIRST (before any GUI initialization)
	CString strCmdLine = m_lpCmdLine;
	strCmdLine.Trim();
	
	if (!strCmdLine.IsEmpty())
	{
		// Has command line arguments - try CLI mode
		if (m_cliHelper.ParseCommandLine(m_lpCmdLine, m_cliParams))
		{
			// CLI mode
			if (m_cliParams.bHelp)
			{
				m_cliHelper.ShowHelp();
			}
			else
			{
				m_cliHelper.ExecuteCLI(m_cliParams);
			}
			// Exit without GUI
			return FALSE;
		}
	}
	// GUI mode - continue with normal initialization
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	CWinAppEx::InitInstance();
	//AfxEnableControlContainer();
	// Create the shell manager only if it doesn't exist
	CShellManager *pShellManager = NULL;
	if (afxShellManager == NULL)
		pShellManager = new CShellManager;
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
	SetRegistryKey(_T("WinSFXMaker"));
	// GUI mode - show dialog
	CWinSFXMakerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
	}
	// Delete the shell manager only if we created it
	if (pShellManager != NULL && pShellManager != afxShellManager)
	{
		delete pShellManager;
	}
	return FALSE;
}
