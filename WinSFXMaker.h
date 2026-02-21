// WinSFXMaker.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "Helper/CLIHelper.h"

// CWinSFXMakerApp:
// See WinSFXMaker.cpp for the implementation of this class
//

class CWinSFXMakerApp : public CWinAppEx
{
public:
	CWinSFXMakerApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	DECLARE_MESSAGE_MAP()

protected:
	// CLI support
	CCLIHelper m_cliHelper;
	CLI_PARAMS m_cliParams;
	BOOL ProcessCommandLine();
};

extern CWinSFXMakerApp theApp;
