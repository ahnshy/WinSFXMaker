
// SFXTemplete.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "SFXTemplete.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSFXTempleteApp

BEGIN_MESSAGE_MAP(CSFXTempleteApp, CWinApp)
	//ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
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

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("SFX Templete App"));

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

