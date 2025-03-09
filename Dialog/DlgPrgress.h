#pragma once

#include "../resource.h"

// CDlgPrgress dialog
class CDlgPrgress : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPrgress)

public:
	CDlgPrgress(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgPrgress();

	void			InitProgessRange(DWORD dwMax, DWORD dwMin = 0);
	void			SetProgressStep(DWORD dwStep = 1);
	void			ProgressStepIt();
	void			UpdateText(UINT uID);
	void			UpdateText(LPCTSTR lpszText);
	void			UpdateWindow();

	void			CleanTaskProgress();

// Dialog Data
	enum { IDD = IDD_PROGRESS };

protected:
	ITaskbarList3	*m_pTaskbarList3;
	CProgressCtrl	m_wndProgress;
	DWORD			m_dwMax;

	void			InitTaskProgress();
	void			SetTaskProgressStatus(TBPFLAG tbpFlags);
	void			SetTaskProgressPos(int nProgressPos);

	virtual BOOL	OnInitDialog();
	virtual void	DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL	PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
};
