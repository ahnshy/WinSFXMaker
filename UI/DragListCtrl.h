#pragma once


// CDragListCtrl

class CDragListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CDragListCtrl)

public:
	CDragListCtrl();
	virtual ~CDragListCtrl();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDropFiles(HDROP hDropInfo);
};


