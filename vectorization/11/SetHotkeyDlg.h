#pragma once


#include "CLongGasMainDlg.h"

class CSetHotkeyDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSetHotkeyDlg)

public:
	CSetHotkeyDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CSetHotkeyDlg();
	HOTKEY hk;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETHOTKEYDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CHotKeyCtrl m_Hotkey;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
