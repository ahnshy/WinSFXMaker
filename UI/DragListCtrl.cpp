#include "../stdafx.h"
#include "DragListCtrl.h"

// CDragListCtrl
IMPLEMENT_DYNAMIC(CDragListCtrl, CListCtrl)
CDragListCtrl::CDragListCtrl()
{
}

CDragListCtrl::~CDragListCtrl()
{
}


BEGIN_MESSAGE_MAP(CDragListCtrl, CListCtrl)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()

void CDragListCtrl::OnDropFiles(HDROP hDropInfo)
{
	TCHAR szPath[BUFSIZ] = { NULL, };
	UINT uCount = 0;

	uCount = DragQueryFile(hDropInfo, 0xffffffff, NULL, 0);
	for (UINT i = 0; i < uCount ; i++)
	{
		DragQueryFile(hDropInfo, i, (LPTSTR)szPath, BUFSIZ-sizeof(TCHAR));
		InsertItem(i, szPath);
	}

	CListCtrl::OnDropFiles(hDropInfo);
}
