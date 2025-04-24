#pragma once

#include "CLongGasMainDlg.h"
// CLGMainDlg dialog

class CLGMainDlg : public CLongGasBaseDlg
{
	DECLARE_DYNAMIC(CLGMainDlg)

public:
	CLGMainDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CLGMainDlg();
	virtual CString GetName() { return S_O::LoadStringW(IDS_LG_MAIN); }
	virtual bool CommandDone(CNFC_Command* c);
	

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LG_MAIN };
#endif
	virtual UINT GetId() { return IDD_LG_MAIN; };

protected:
	void OnLogSpin(NMHDR* pNMHDR, CEdit* edit);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void ShowControls();
	void SetLogType();
	CString strPrevSerial;
	void InfToControls();
	void SettingsToControls();
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnTabSelected();
	afx_msg void OnBnClickedBlink();
	afx_msg void OnBnClickedBeep();
	afx_msg void OnBnClickedReboot();
	afx_msg void OnBnClickedReplacebattery();
	afx_msg void OnBnClickedRefreshdevinfo();
	afx_msg void OnBnClickedSetserialno();
	CEdit m_SerialNo;
	CEdit m_LogTimeout;
	afx_msg void OnBnClickedSetlogtimeout();
	afx_msg void OnBnClickedCheckBeep();
	BOOL m_bBeep;
	BOOL m_bVibro;
	BOOL m_bAlarm;
	afx_msg void OnBnClickedCheckVibro();
	afx_msg void OnBnClickedCheckAlarm();
	afx_msg void OnBnClickedSetdevicemode();
	CComboBox m_DeviceMode;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedNfcpowerdownenable();
	afx_msg void OnBnClickedNfcpowerdowndisable();
	afx_msg void OnBnClickedSetdatetime();
	afx_msg void OnBnClickedSetdatetimecomp();
	BOOL m_bDaylight;
	CDateTimeCtrl m_Date;
	CDateTimeCtrl m_Time;
	CString m_SWVer;
	CString m_IDSensor;
	CString m_CPUId;
	afx_msg void OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	CEdit m_LogAlarmTimeout;
	afx_msg void OnBnClickedSetlogtimeout2();
	afx_msg void OnDeltaposSpin7(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedCheckSkipselftest();
	BOOL m_bSkipSelfTest;
	CEdit m_WeekToScale;
	afx_msg void OnBnClickedSetweektoscale();
	afx_msg void OnDeltaposSpinWeektoscale(NMHDR* pNMHDR, LRESULT* pResult);
	BOOL m_ShortLog;
	afx_msg void OnBnClickedShortlog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	BOOL m_bMipexLog;
	afx_msg void OnBnClickedMipexlog();
	BOOL m_bSkipSelfTestTransport;
	afx_msg void OnBnClickedCheckSkipselftesttransport();
	BOOL m_Accum;
	afx_msg void OnBnClickedAccum();
};
