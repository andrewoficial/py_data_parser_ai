#pragma once
#include "CLongGasMainDlg.h"

// CLGMultipleDlg dialog

class CLGMultipleDlg : public CLongGasBaseDlg
{
	DECLARE_DYNAMIC(CLGMultipleDlg)

public:
	CLGMultipleDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CLGMultipleDlg();
	virtual CString GetName() { return S_O::LoadStringW(IDS_LG_MULTIPLE); };
	virtual UINT GetId() { return IDD_LG_MULTIPLE; };
	virtual bool CommandDone(CNFC_Command* c);
	virtual void ChangeAll(CNFC_Device* dev, UINT nMode, CHANGEALL* c);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LG_MULTIPLE };
#endif

protected:
	void GetCheckIds(CUIntArray& arr);
	void ShowControls();
	bool bModified;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnChecked();	
	CEdit m_LogTimeout;
	afx_msg void OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult);
	CComboBox m_DeviceMode;
	afx_msg void OnBnClickedSetAll();
	afx_msg void OnBnClickedAlarmOff();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual void OnTabSelected();
};
