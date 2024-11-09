
// SFXTempleteDlg.h : header file
//

#pragma once


// CSFXTempleteDlg dialog
class CSFXTempleteDlg : public CDialogEx
{
// Construction
public:
	CSFXTempleteDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SFXTEMPLETE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};
