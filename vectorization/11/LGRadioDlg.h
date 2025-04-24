#pragma once

#include "CLongGasMainDlg.h"
// CLGRadioDlg dialog

class CLGWifiDlg;

class CLGRadioDlg : public CLongGasBaseDlg
{
	DECLARE_DYNAMIC(CLGRadioDlg)

public:
	CLGRadioDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CLGRadioDlg();

	virtual CString GetName() { return S_O::LoadStringW(IDS_LG_RADIO); }
	virtual bool CommandDone(CNFC_Command* c);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LG_RADIO };
#endif

	virtual UINT GetId() { return IDD_LG_RADIO; };

protected:
	virtual void OnTabSelected();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	BOOL m_En1;
	BOOL m_En2;
	BOOL m_En3;
	CComboBox m_SF1;
	CComboBox m_SF2;
	CComboBox m_SF3;
	CComboBox m_BW;
	CComboBox m_CR;
	CString m_Addr;
	CString m_AKey;
	CString m_Freq1;
	CString m_Freq2;
	CString m_Freq3;
	CString m_Period;
	CString m_NKey;
	CString m_LT1;
	CString m_LT2;
	CString m_LT3;
	CString m_LTAll;
	afx_msg void OnBnClickedGetRf();
	CComboBox m_Power1;
	CComboBox m_Power2;
	CComboBox m_Power3;
	afx_msg void OnBnClickedGetKeys();
	afx_msg void OnBnClickedSetKeys();
	afx_msg void OnBnClickedSetRf();
	afx_msg void OnBnClickedSetRfdef();
private:
	double CalcLifeTime(int sf, int pwr, int per);
	void UpdateLifeTimes();
	void OnSpinFreq(CString* freq, LPNMUPDOWN pNMUpDown);
	void RAK811ToControls();
	CLGWifiDlg* wifi;
	void SelectInterface();
	void EnableInterfaces();
	void InfoToControls();
	void RadioToControls();
	void OTAAToControls();
	void OTAAToControls2();
	void LoraKeyToControls();
	void LoraInfoControls();
public:
	afx_msg void OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCbnSelchangeSf1();
	afx_msg void OnEnChangePower1();
	afx_msg void OnBnClickedSendpacket();
	afx_msg void OnBnClickedCheckEn1();
	afx_msg void OnBnClickedCheckEn2();
	afx_msg void OnBnClickedCheckEn3();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	BOOL m_bABP;
	BOOL m_bOTAA;
	CString m_DevEUI;
	int m_JoinDelay1;
	int m_JoinDelay2;
	CString m_JoinEUI;
	int m_JoinNum;
	CString m_RootKey;
	int m_RX1Delay;
	int m_RX2Delay;
	CComboBox m_WindowCtrl;
	afx_msg void OnBnClickedGetOtaa();
	afx_msg void OnBnClickedSetOtaa();
	afx_msg void OnBnClickedRejoin();
	afx_msg void OnDeltaposRx1spin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposRx2spin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposJoindelayspin1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposJoindelayspin2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposJoinnumspin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedconfirm();
	afx_msg void OnBnClickedCheckAbp();
	afx_msg void OnBnClickedCheckOtaa();
	afx_msg void OnDeltaposSpinRx(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
private:
	void OnSpinDelay(int* delay, LPNMUPDOWN pNMUpDown);
public:
	
	CString m_FreqRX;
	CComboBox m_SFRX;	
protected:
	void UpdateJoinStatus();
public:
	
	CEdit m_SignalStatus;
	afx_msg void OnCbnSelchangePower1();
	afx_msg void OnBnClickedRadioWan();
	afx_msg void OnCbnSelchangeComboStandard();
	afx_msg void OnCbnSelchangeComboModule();
	CComboBox m_Module;
	CComboBox m_Protocol;
	CComboBox m_Standard;
	afx_msg void OnBnClickedSetInterface();
	afx_msg void OnBnClickedGetInterface();
	CComboBox m_Region;
	afx_msg void OnCbnSelchangeRegion();
};
