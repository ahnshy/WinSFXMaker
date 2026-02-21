#include "../stdafx.h"
#include "../WinSFXMaker.h"
#include "DlgSFXProgress.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CDlgSFXProgress, CDialogEx)

CDlgSFXProgress::CDlgSFXProgress(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgSFXProgress::IDD, pParent)
{
	m_bCancel = FALSE;
}

CDlgSFXProgress::~CDlgSFXProgress()
{
}

void CDlgSFXProgress::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_SFX, m_ctrlProgress);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_ctrlStatus);
}

BEGIN_MESSAGE_MAP(CDlgSFXProgress, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CDlgSFXProgress::OnBnClickedButtonCancel)
END_MESSAGE_MAP()

BOOL CDlgSFXProgress::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_bCancel = FALSE;
	m_ctrlProgress.SetRange(0, 100);
	m_ctrlProgress.SetPos(0);
	m_ctrlStatus.SetWindowText(_T("Preparing..."));
	return TRUE;
}

void CDlgSFXProgress::SetProgress(int nPos)
{
	if (m_ctrlProgress.GetSafeHwnd())
		m_ctrlProgress.SetPos(nPos);
}

void CDlgSFXProgress::SetStatus(LPCTSTR lpszStatus)
{
	if (m_ctrlStatus.GetSafeHwnd())
		m_ctrlStatus.SetWindowText(lpszStatus);
}

void CDlgSFXProgress::SetRange(int nMin, int nMax)
{
	if (m_ctrlProgress.GetSafeHwnd())
		m_ctrlProgress.SetRange(nMin, nMax);
}

void CDlgSFXProgress::OnBnClickedButtonCancel()
{
	m_bCancel = TRUE;
}
