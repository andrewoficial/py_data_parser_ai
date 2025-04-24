#pragma once

#include "CLongGasMainDlg.h"
// CLGSettingsDlg dialog

struct DEVICE_SETTINGS;

class CLGSettingsDlg : public CLongGasBaseDlg
{
	DECLARE_DYNAMIC(CLGSettingsDlg)

public:
	CLGSettingsDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CLGSettingsDlg();
	virtual CString GetName() { return S_O::LoadStringW(IDS_LG_SETTINGS); }
	virtual bool CommandDone(CNFC_Command* c);
	void GetSettingsAsArray(DEVICE_SETTINGS* s, CStringArray& arr);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LG_SETTINGS };
#endif
	virtual UINT GetId() { return IDD_LG_SETTINGS; };

protected:
	virtual void OnTabSelected();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg LRESULT OnCellSelected(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnCellModified(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedGetSettings();
	afx_msg void OnBnClickedSetSettings();
	afx_msg void OnBnClickedTestalarm1();
	afx_msg void OnBnClickedTestalarm2();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

};
