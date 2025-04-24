#pragma once

#include "CLongGasMainDlg.h"

// CLGAlarmsDlg dialog

class CLGAlarmsDlg : public CLongGasBaseDlg
{
	DECLARE_DYNAMIC(CLGAlarmsDlg)

public:
	CLGAlarmsDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CLGAlarmsDlg();

	virtual CString GetName() { return S_O::LoadStringW(IDS_LG_ALARMS); }
	virtual bool CommandDone(CNFC_Command* c);
	virtual bool OperationStarted(CNFC_Device* dev, UINT nType);
	virtual bool OperationEnded(CNFC_Device* dev, UINT nType);
	virtual void OnTabSelected();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LG_ALARMS };
#endif
	virtual UINT GetId() { return IDD_LG_ALARMS; };

protected:
	void OperationToControls(CNFC_Device* nfc, UINT nType, bool bOperationStarted);
	void StartCalibration(bool bAll);
	void StartZeroCalibration(bool bAll);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CMyComboBox m_CO_Units;
	CMyComboBox m_CH4_Units;
	CMyComboBox m_H2S_Units;
	CMyComboBox m_O2_Units;
	CComboBox m_VibroPower;
	afx_msg void OnBnClickedGetUnits();
	afx_msg void OnBnClickedSetVibropower();
	afx_msg void OnBnClickedGetVibropower();
	afx_msg void OnBnClickedSetUnits();
	afx_msg void OnBnClickedGetAlarms();
	afx_msg void OnBnClickedSetAlarms();
	afx_msg void OnBnClickedSetAlarmsdef();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedStartzerocalibration();
	afx_msg void OnBnClickedStartcalibration();
	afx_msg void OnBnClickedCancelcalibration();
	BOOL m_CH4Zero;
	BOOL m_O2Zero;
	BOOL m_H2SZero;
	BOOL m_COZero;
	BOOL m_CH4Calib;
	BOOL m_O2Calib;
	BOOL m_H2SCalib;
	BOOL m_COCalib;
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedStartzerocalibrationall();
	afx_msg void OnBnClickedStartcalibrationall();
	UINT m_WaitTime;
	void SaveSelection();
	afx_msg void OnBnClickedFactoryreset();
	BOOL m_bCH4Factory;
	BOOL m_bH2SFactory;
	BOOL m_bO2Factory;
	BOOL m_bCOFactory;
	afx_msg void OnBnClickedFactoryAll();
	void FactoryReset(bool bAll);
	afx_msg void OnBnClickedSetUnitsall();
	void SetUnits(bool bAll);
	void SetPrecisions(bool bAll);
	void SetAutozero(bool bAll);
protected:
	bool SetCalibrationLimits();
	LRESULT OnGridDblClk(WPARAM wp, LPARAM lp);	
	void ShowControls();
	bool SetGasRange(bool bAll);
	void UpdateUnits();
	void EnablePrecision();
	void SetExtendedUnits(UINT nUnits, UINT nGas);
	void GasRangeToControls();
	void SettingsToControls();
	void AlarmSignalingToControls();
public:
	afx_msg void OnBnClickedCancelfactoryreset();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedSetPrecisions();
	afx_msg void OnBnClickedSetPrecisionsall();
	afx_msg void OnBnClickedGetPrecisions();
	afx_msg void OnBnClickedSetAutozero();
	afx_msg void OnBnClickedSetAutozeroall();
	afx_msg void OnBnClickedGetAutozero();
	afx_msg void OnBnClickedSetGasrange();
	afx_msg void OnBnClickedGetGasrange();
	afx_msg void OnBnClickedSetAllgasrange();
	afx_msg void OnBnClickedGetScale();
	afx_msg void OnBnClickedSetScale();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedScaleAutozero();
};
