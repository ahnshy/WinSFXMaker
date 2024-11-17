#pragma once

#include <afxshelltreectrl.h>

// CShellTreeCtrlEx
class CShellTreeCtrlEx : public CMFCShellTreeCtrl
{
	DECLARE_DYNAMIC(CShellTreeCtrlEx)

public:
	CShellTreeCtrlEx();
	virtual ~CShellTreeCtrlEx();
	void				SetNotifyContainer(CWnd* pContainer);

protected:
	DECLARE_MESSAGE_MAP()
	virtual	BOOL		OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);

	// Work Ahnshy :: ���� �Լ��� �������ϰ� �Ǹ�, �Ʒ��� ���� ������ �߻��Ѵ�.
	// error LNK2001: unresolved external symbol "class CShellManager * afxShellManager" (?afxShellManager@@3PAVCShellManager@@A)
	// VS2010������ �����Ǿ��ٰ� �ϴµ�, �ϴ� ����� �� ����, ������ Static Library Build������ ��밡���ϴ�.
	//virtual HRESULT		EnumObjects(HTREEITEM hParentItem, LPSHELLFOLDER pParentFolder, LPITEMIDLIST pidlParent);

protected:
	CWnd				*m_pContainer;
	BOOL				m_bInit;
};


