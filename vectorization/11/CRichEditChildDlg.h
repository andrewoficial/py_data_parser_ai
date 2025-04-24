#pragma once
#include <LayoutHelper.h>


// CRichEditChildDlg dialog

class CRichEditChildDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRichEditChildDlg)

public:
	CRichEditChildDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CRichEditChildDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RICHEDIT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CRichEditCtrl m_Rich;
private:
	CLayoutHelper layout;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
