// DlgPrgress.cpp : implementation file
//

#include "stdafx.h"

#include <afxdialogex.h>
#include "DlgPrgress.h"

#include "afxdatarecovery.h"
#include "afxglobals.h"
#include "afxwinappex.h"
#include "afxdialogex.h"


// CDlgPrgress dialog

IMPLEMENT_DYNAMIC(CDlgPrgress, CDialogEx)

CDlgPrgress::CDlgPrgress(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgPrgress::IDD, pParent)
{
	m_dwMax			= 0;
	m_pTaskbarList3 = NULL;
	::CoInitialize(NULL);
	CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pTaskbarList3));
}

CDlgPrgress::~CDlgPrgress()
{
	if (m_pTaskbarList3)
	{
		m_pTaskbarList3->Release();
		m_pTaskbarList3 = NULL;
	}
	::CoUninitialize();
}

void CDlgPrgress::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgPrgress, CDialogEx)
END_MESSAGE_MAP()

BOOL CDlgPrgress::OnInitDialog()
{
	SetBackgroundColor(RGB(255, 255, 255));

	LONG ExtendedStyle = GetWindowLong(GetSafeHwnd(),GWL_EXSTYLE);
	SetWindowLong(GetSafeHwnd(),GWL_EXSTYLE, ExtendedStyle | WS_EX_LAYERED);
	::SetLayeredWindowAttributes(GetSafeHwnd(), 0, 255, LWA_ALPHA);

	UpdateData(FALSE);

	CRect rt;
	GetClientRect(&rt);

	m_wndProgress.SetWindowPos(NULL, rt.left + 25, rt.top + (rt.Height()/2), rt.Width(),  17, SWP_SHOWWINDOW);

	return TRUE;
}

void CDlgPrgress::InitProgessRange(DWORD dwMax, DWORD dwMin)
{
	m_dwMax = dwMax;
	if (!m_wndProgress.GetSafeHwnd())
		return;
	
	m_wndProgress.SetRange((short)dwMin, (short)dwMax);
	m_wndProgress.SetStep(1);

	InitTaskProgress();
}

void CDlgPrgress::SetProgressStep(DWORD dwStep)
{
	if (!m_wndProgress.GetSafeHwnd())
		return;

	m_wndProgress.SetStep(dwStep);
	SetTaskProgressPos(dwStep);
}

void CDlgPrgress::ProgressStepIt()
{
	if (!m_wndProgress.GetSafeHwnd())
		return;

	m_wndProgress.StepIt();

	float fPrgress = (float)(m_wndProgress.GetPos() / m_dwMax);
	float fAlpha = (255 * fPrgress);
	::SetLayeredWindowAttributes(GetSafeHwnd(), 0, fAlpha > 255.0 ? 255 : (INT32)fAlpha, LWA_ALPHA);
	SetTaskProgressPos(m_wndProgress.GetPos());
}

void CDlgPrgress::UpdateText(UINT uID)
{
	TCHAR szBuffer[BUFSIZ] = { NULL, };
	::LoadString(NULL, uID, szBuffer, BUFSIZ);
	UpdateText(szBuffer);
}

void CDlgPrgress::UpdateText(LPCTSTR lpszText)
{
	CWnd *pWnd = GetDlgItem(IDC_STATIC);
	if (!pWnd || !lpszText)
		return;

	CDC* pDC = pWnd->GetWindowDC();
	CRect rt;
	pWnd->GetWindowRect(&rt);
	pDC->SetTextColor(RGB(255, 0, 0));
	pDC->DrawText(lpszText, _tcslen(lpszText), &rt, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
}

void CDlgPrgress::InitTaskProgress()
{
	if (m_pTaskbarList3)
		m_pTaskbarList3->HrInit();

	SetTaskProgressStatus(TBPF_NORMAL);
}

void CDlgPrgress::SetTaskProgressPos(int nProgressPos)
{
	if (!afxGlobalData.bIsWindows7 || !m_pTaskbarList3)
		return;

	m_pTaskbarList3->SetProgressValue(GetParent()->GetSafeHwnd(), nProgressPos, m_dwMax);
}
void CDlgPrgress::SetTaskProgressStatus(TBPFLAG tbpFlags)
{
	if (!afxGlobalData.bIsWindows7 || !m_pTaskbarList3)
		return;

	m_pTaskbarList3->SetProgressState(GetParent()->GetSafeHwnd(), tbpFlags);
}

void CDlgPrgress::CleanTaskProgress()
{
	SetTaskProgressStatus(TBPF_NOPROGRESS);
}

BOOL CDlgPrgress::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) ||
		(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN))
		return TRUE;

	return CDialogEx::PreTranslateMessage(pMsg);
}