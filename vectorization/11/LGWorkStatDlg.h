#pragma once

#include "CLongGasMainDlg.h"
// CLGWorkStatDlg dialog

class CLGWorkStatDlg : public CLongGasBaseDlg
{
	DECLARE_DYNAMIC(CLGWorkStatDlg)

public:
	CLGWorkStatDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CLGWorkStatDlg();

	virtual CString GetName() { return S_O::LoadStringW(IDS_LG_WORKSTAT); }
	virtual bool CommandDone(CNFC_Command* c);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LG_WORKSTAT };
#endif

	virtual UINT GetId() { return IDD_LG_WORKSTAT; };

protected:
	void FillGrid();
	virtual void OnTabSelected();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedGet();
	afx_msg void OnBnClickedReset();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
