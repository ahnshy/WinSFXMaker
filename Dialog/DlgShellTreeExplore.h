/////////////////////////////////////////////////////////////////////////////
// This is a part of the ahnshy Classes C++ library.
// Copyright (C) http://blog.naver.com/ahnshy
// 
// All rights reserved.
//
// Bug FeedBack :: ahnshy@naver.com
//
/////////////////////////////////////////////////////////////////////////////
// DlgShellTreeExplore.h : header file
//
#pragma once

#include "../resource.h"
#include "../Control/SheelTreeCtrlEx.h"

// CDlgShellTreeExplore dialog
class CDlgShellTreeExplore : public CDialog
{
// Construction
public:
	CDlgShellTreeExplore(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SHELL_TREE_EXPLORE };

	public:
	void				Init();
	void				SetDescriptText(CString	strDesc)			{		m_strDescript = strDesc;		}

	CString				GetPath()									{		return m_strPath;				}

// Implementation
protected:
	CRect				m_rcPrev;
	HICON				m_hIcon;
	CSheelTreeCtrlEx	m_wndShellTree;

	CString				m_strPath;
	CString				m_strDescript;

	void				AdjustLayout(int cx, int cy);

	virtual BOOL		OnInitDialog();
	afx_msg void		OnSize(UINT nType, int cx, int cy);
	afx_msg void		OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void		OnEnChangeEditPath();
	virtual void		DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	afx_msg LRESULT		OnChangeFolder(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};
