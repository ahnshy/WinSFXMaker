
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

	SetRegistryKey(_T("SFX Templete App"));

	return FALSE;
}

