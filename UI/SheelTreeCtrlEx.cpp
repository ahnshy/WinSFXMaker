// SheelTreeCtrlEx.cpp : implementation file
//
#include "stdafx.h"
#include "SheelTreeCtrlEx.h"
//#include "../Share/Definition.h"

//#define		WM_SELCHANGED_FOLER		307
// CSheelTreeCtrlEx
IMPLEMENT_DYNAMIC(CSheelTreeCtrlEx, CMFCShellTreeCtrl)

CSheelTreeCtrlEx::CSheelTreeCtrlEx()
{
	m_bInit		= FALSE;
	m_pContainer= NULL;
}

CSheelTreeCtrlEx::~CSheelTreeCtrlEx()
{
	m_bInit		= FALSE;
	m_pContainer= NULL;
}


BEGIN_MESSAGE_MAP(CSheelTreeCtrlEx, CMFCShellTreeCtrl)
	//ON_WM_ERASEBKGND()
	//ON_NOTIFY_REFLECT(TVN_ITEMCHANGING, &CSheelTreeCtrlEx::OnTvnSelChanged)
END_MESSAGE_MAP()

// Work Ahnshy :: VS2008 sp1에서 CMFCXXXCTRL중에 TVN_계열 메시지를 넘겨주지 않는다.
// VS2010에서는 정상적으로 이벤트 핸들링 가능!! 
// 이에 TVN_ITEMCHANGING를 OnChildNotify()받아 전달하도록 수정하였습니다.
//void CSheelTreeCtrlEx::OnTvnSelChanged(NMHDR* pNMHDR, LRESULT* pResult) 
//{
//	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
//	*pResult = 0;
//
//	CString strItem = GetItemText(pNMTreeView->itemNew.hItem);
//	AfxMessageBox(strItem);
//
//	CString strPath;
//	GetItemPath(strPath);
//	AfxMessageBox(strPath);
//}

BOOL CSheelTreeCtrlEx::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult)
{
	if (message == WM_NOTIFY)
	{
		LPNMHDR pHdr		= (LPNMHDR)lParam;
		LPNMTREEVIEW pNmtv	= (LPNMTREEVIEW)lParam;
		UNREFERENCED_PARAMETER(pNmtv);
		
		if (!pHdr || pHdr->hwndFrom != GetSafeHwnd())
			return CMFCShellTreeCtrl::OnChildNotify(message, wParam, lParam, pLResult);

		CString strPath;
		switch(pHdr->code)
		{
			case TVN_ITEMCHANGING :
				// to do
				break;

			case TVN_SELCHANGED : 
				GetItemPath(strPath);
				if ((m_pContainer && pNmtv) && (pNmtv->action == TVC_BYKEYBOARD || pNmtv->action == TVC_BYMOUSE))
					m_pContainer->SendMessage(WM_SELCHANGED_FOLER, (WPARAM)(LPCTSTR)strPath, (LPARAM)NULL);
				return FALSE;
			
			default:
				break;
		}
	}

	return CMFCShellTreeCtrl::OnChildNotify(message, wParam, lParam, pLResult);
}

void CSheelTreeCtrlEx::SetNotifyContainer(CWnd* pContainer)
{
	m_pContainer = pContainer;
}
// CSheelTreeCtrlEx message handlers

// Work Ahnshy :: 다음 함수를 재정의하게 되면, 아래와 같은 에러가 발생한다.
// error LNK2001: unresolved external symbol "class CShellManager * afxShellManager" (?afxShellManager@@3PAVCShellManager@@A)
// VS2010에서는 수정되었다고 하는데, 일단 사용할 수 없고, 오로지 Static Library Build에서만 사용가능하다.
//virtual HRESULT		EnumObjects(HTREEITEM hParentItem, LPSHELLFOLDER pParentFolder, LPITEMIDLIST pidlParent);
//HRESULT CSheelTreeCtrlEx::EnumObjects(HTREEITEM hParentItem, LPSHELLFOLDER pParentFolder, LPITEMIDLIST pidlParent)
//{
//	ASSERT_VALID(this);
//	ASSERT_VALID(afxShellManager);
//
//	LPENUMIDLIST pEnum;
//
//	HRESULT hr = pParentFolder->EnumObjects(NULL, m_dwFlags, &pEnum);
//	if (FAILED(hr))
//		return hr;
//
//	LPITEMIDLIST pidlTemp;
//	DWORD dwFetched = 1;
//
//	// Enumerate the item's PIDLs:
//	while (SUCCEEDED(pEnum->Next(1, &pidlTemp, &dwFetched)) && dwFetched)
//	{
//		TVITEM tvItem;
//		ZeroMemory(&tvItem, sizeof(tvItem));
//
//		// Fill in the TV_ITEM structure for this item:
//		tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
//
//		// AddRef the parent folder so it's pointer stays valid:
//		pParentFolder->AddRef();
//
//		// Put the private information in the lParam:
//		LPAFX_SHELLITEMINFO pItem = (LPAFX_SHELLITEMINFO)GlobalAlloc(GPTR, sizeof(AFX_SHELLITEMINFO));
//		ENSURE(pItem != NULL);
//
//		pItem->pidlRel = pidlTemp;
//		pItem->pidlFQ = afxShellManager->ConcatenateItem(pidlParent, pidlTemp);
//
//		pItem->pParentFolder = pParentFolder;
//		tvItem.lParam = (LPARAM)pItem;
//
//		CString strItem = OnGetItemText(pItem);
//		tvItem.pszText = strItem.GetBuffer(strItem.GetLength());
//		tvItem.iImage = OnGetItemIcon(pItem, FALSE);
//		tvItem.iSelectedImage = OnGetItemIcon(pItem, TRUE);
//
//		// Determine if the item has children:
//		DWORD dwAttribs = SFGAO_HASSUBFOLDER | SFGAO_FOLDER | SFGAO_DISPLAYATTRMASK | SFGAO_CANRENAME | SFGAO_STREAM;
//
//		pParentFolder->GetAttributesOf(1, (LPCITEMIDLIST*) &pidlTemp, &dwAttribs);
//		if (dwAttribs & SFGAO_STREAM)	// Work Ahnshy - ZIP 파일은 무시
//			continue;
//
//		tvItem.cChildren = (dwAttribs & SFGAO_HASSUBFOLDER);
//
//
//		// Determine if the item is shared:
//		if (dwAttribs & SFGAO_SHARE)
//		{
//			tvItem.mask |= TVIF_STATE;
//			tvItem.stateMask |= TVIS_OVERLAYMASK;
//			tvItem.state |= INDEXTOOVERLAYMASK(1); //1 is the index for the shared overlay image
//		}
//
//		// Fill in the TV_INSERTSTRUCT structure for this item:
//		TVINSERTSTRUCT tvInsert;
//
//		tvInsert.item = tvItem;
//		tvInsert.hInsertAfter = TVI_LAST;
//		tvInsert.hParent = hParentItem;
//
//		InsertItem(&tvInsert);
//		dwFetched = 0;
//	}
//
//	pEnum->Release();
//	return S_OK;
//}