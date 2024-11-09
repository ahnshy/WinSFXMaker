
// SFXTemplete.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CSFXTempleteApp:
// See SFXTemplete.cpp for the implementation of this class
//

class CSFXTempleteApp : public CWinApp
{
public:
	CSFXTempleteApp();

// Overrides
public:
	virtual BOOL InitInstance();

	void CreateSFX();
	BOOL ExtractEmbeddedZip(LPCTSTR outputPath);

protected:
	INT32 Parse(LPTSTR m_lpCmdLine);

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CSFXTempleteApp theApp;