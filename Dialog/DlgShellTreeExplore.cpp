/////////////////////////////////////////////////////////////////////////////
// This is a part of the ahnshy Classes C++ library.
// Copyright (C) http://blog.naver.com/ahnshy
// 
// All rights reserved.
//
// Bug FeedBack :: ahnshy@naver.com
//
/////////////////////////////////////////////////////////////////////////////

// DlgShellTreeExplore.cpp : implementation file
//

#include "stdafx.h"
#include "DlgShellTreeExplore.h"

//#include "../../Share/Definition.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDlgShellTreeExplore dialog
CDlgShellTreeExplore::CDlgShellTreeExplore(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgShellTreeExplore::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_strPath.Empty();
	m_strDescript.Empty();
	m_strDescript = _T("Select a Folder...");
}

void CDlgShellTreeExplore::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgShellTreeExplore, CDialog)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_EN_CHANGE(IDC_EDIT_PATH, &CDlgShellTreeExplore::OnEnChangeEditPath)
	ON_MESSAGE(WM_SELCHANGED_FOLER, OnChangeFolder)
END_MESSAGE_MAP()


// CDlgShellTreeExplore message handlers

BOOL CDlgShellTreeExplore::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	Init();
	CenterWindow();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgShellTreeExplore::Init()
{

	CRect rt(0, 0, 0, 0);
	CWnd *pWnd = GetDlgItem(IDC_TREE);
	if (pWnd)
	{
		pWnd->GetWindowRect(&rt);
		ScreenToClient(&rt);
		pWnd->ShowWindow(SW_HIDE);
	}

	const DWORD dwViewStyle =	WS_CHILD | WS_CLIPCHILDREN | TVS_SHOWSELALWAYS | WS_BORDER | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;
	VERIFY(m_wndShellTree.Create(dwViewStyle, rt, this, 2027));
	if (m_strPath.IsEmpty())
	{
		GetCurrentDirectory(BUFSIZ, m_strPath.GetBuffer(BUFSIZ));
		m_strPath.ReleaseBuffer();
	}

	//DWORD dwFlag = m_wndShellTree.GetFlags();
	//dwFlag = dwFlag & ~SFGAO_COMPRESSED;
	//m_wndShellTree.SetFlags(m_wndShellTree.GetFlags() | SFGAO_COMPRESSED | );
	//m_wndShellTree.SetFlags(SHCONTF_FOLDERS);

	m_wndShellTree.SetNotifyContainer(this);
	m_wndShellTree.SelectPath(m_strPath);

	pWnd = GetDlgItem(IDC_EDIT_PATH);
	if (!pWnd || !pWnd->GetSafeHwnd())
		return;

	pWnd->SetWindowText(m_strPath);


	pWnd = GetDlgItem(IDC_TEXT_DESCRIPCTION);
	if (!pWnd || !pWnd->GetSafeHwnd())
		return;

	pWnd->SetWindowText(m_strDescript);
}

void CDlgShellTreeExplore::AdjustLayout(int cx, int cy)
{
	int nX = cx - m_rcPrev.Width();
	int nY = cy - m_rcPrev.Height();

	UINT nID[] = { IDOK, IDCANCEL };
	CWnd *pWnd = NULL;
	CRect rcClient, rcWindow;
	for (int nIdx = 0 ; nIdx < _countof(nID) ; nIdx++)
	{
		pWnd = GetDlgItem(nID[nIdx]);
		if (!pWnd || !pWnd->GetSafeHwnd())
			continue;

		pWnd->GetClientRect(&rcClient);
		pWnd->GetWindowRect(&rcWindow);
		ScreenToClient(&rcWindow);

		pWnd->SetWindowPos(NULL, rcWindow.left + nX, rcWindow.top + nY, rcClient.Width(), rcClient.Height(), SWP_SHOWWINDOW);
		pWnd->InvalidateRect(NULL);
	}

	// Resize with Width & Height
	UINT nID2[] = { IDC_EDIT_PATH, IDC_STATIC_FOLDER };
	for (int nIdx = 0 ; nIdx < _countof(nID2) ; nIdx++)
	{
		pWnd = GetDlgItem(nID2[nIdx]);
		if (!pWnd || !pWnd->GetSafeHwnd())
			continue;

		pWnd->GetClientRect(&rcClient);
		pWnd->GetWindowRect(&rcWindow);
		ScreenToClient(&rcWindow);

		pWnd->SetWindowPos(NULL, rcWindow.left, rcWindow.top + nY, rcWindow.Width() + nX, rcWindow.Height(), SWP_SHOWWINDOW);
		pWnd->InvalidateRect(NULL);
	}

	GetClientRect(&m_rcPrev);

	pWnd = GetDlgItem(IDC_TEXT_DESCRIPCTION);
	if (!pWnd || !pWnd->GetSafeHwnd())
		return;

	pWnd->GetWindowRect(&rcWindow);
	ScreenToClient(&rcWindow);

	pWnd = GetDlgItem(IDC_EDIT_PATH);
	if (!pWnd || !pWnd->GetSafeHwnd())
		return;

	pWnd->GetWindowRect(&rcWindow);
	ScreenToClient(&rcWindow);

	m_wndShellTree.GetClientRect(&rcClient);
	m_wndShellTree.GetWindowRect(&rcWindow);
	ScreenToClient(&rcWindow);

	m_wndShellTree.SetWindowPos(NULL, rcWindow.left, rcWindow.top, rcWindow.Width() + nX, rcWindow.Height() + nY, SWP_SHOWWINDOW);
}

void CDlgShellTreeExplore::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	AdjustLayout(cx, cy);
}

void CDlgShellTreeExplore::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	RECT rc;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
	lpMMI->ptMaxSize.x		= rc.right;
	lpMMI->ptMaxSize.y		= rc.bottom;
	lpMMI->ptMaxPosition.x	= 0;
	lpMMI->ptMaxPosition.y	= 0;
	lpMMI->ptMinTrackSize.x = 351;
	lpMMI->ptMinTrackSize.y = 379;
	lpMMI->ptMaxTrackSize.x = GetSystemMetrics(SM_CXSCREEN);
	lpMMI->ptMaxTrackSize.y = GetSystemMetrics(SM_CYSCREEN);

	CDialog::OnGetMinMaxInfo(lpMMI);
}

void CDlgShellTreeExplore::OnEnChangeEditPath()
{
	CWnd *pWnd = GetDlgItem(IDC_EDIT_PATH);
	if (!pWnd || !pWnd->GetSafeHwnd())
		return;

	pWnd->GetWindowText(m_strPath);

	if (m_wndShellTree.GetSafeHwnd())
		m_wndShellTree.SelectPath(m_strPath);
}

LRESULT CDlgShellTreeExplore::OnChangeFolder(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	LPCTSTR lpParam = (LPCTSTR)wParam;
	if (!lpParam || !lpParam[0])
		return -1;

	m_strPath = lpParam;

	CWnd *pWnd = GetDlgItem(IDC_EDIT_PATH);
	if (!pWnd || !pWnd->GetSafeHwnd())
		return -1;

	pWnd->SetWindowText(m_strPath);
	return 0;
}