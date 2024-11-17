#pragma once

#include "SheelTreeCtrlEx.h"

// CSheelTreeCtrlEx
class CSheelTreeCtrlEx : public CMFCShellTreeCtrl
{
	DECLARE_DYNAMIC(CSheelTreeCtrlEx)

public:
	CSheelTreeCtrlEx();
	virtual ~CSheelTreeCtrlEx();
	void				SetNotifyContainer(CWnd* pContainer);

protected:
	DECLARE_MESSAGE_MAP()
	virtual	BOOL		OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);

	// Work Ahnshy :: 다음 함수를 재정의하게 되면, 아래와 같은 에러가 발생한다.
	// error LNK2001: unresolved external symbol "class CShellManager * afxShellManager" (?afxShellManager@@3PAVCShellManager@@A)
	// VS2010에서는 수정되었다고 하는데, 일단 사용할 수 없고, 오로지 Static Library Build에서만 사용가능하다.
	//virtual HRESULT		EnumObjects(HTREEITEM hParentItem, LPSHELLFOLDER pParentFolder, LPITEMIDLIST pidlParent);

protected:
	CWnd				*m_pContainer;
	BOOL				m_bInit;
};


