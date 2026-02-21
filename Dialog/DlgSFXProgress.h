#pragma once
#include "afxcmn.h"

// CDlgSFXProgress dialog
class CDlgSFXProgress : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSFXProgress)
public:
	CDlgSFXProgress(CWnd* pParent = NULL);
	virtual ~CDlgSFXProgress();
	enum { IDD = IDD_SFX_PROGRESS };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl m_ctrlProgress;
	CStatic m_ctrlStatus;
	BOOL m_bCancel;
	void SetProgress(int nPos);
	void SetStatus(LPCTSTR lpszStatus);
	void SetRange(int nMin, int nMax);
	BOOL IsCancelled() { return m_bCancel; }
	afx_msg void OnBnClickedButtonCancel();
};
