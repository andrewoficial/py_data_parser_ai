#pragma once

#include "CLongGasMainDlg.h"
#include <XTipComboBox.h>

// CLGSensorsDlg dialog

class CLGSensorsDlg : public CLongGasBaseDlg
{
	DECLARE_DYNAMIC(CLGSensorsDlg)

public:
	CLGSensorsDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CLGSensorsDlg();
	virtual CString GetName() { return S_O::LoadStringW(IDS_LG_SENSORS); }
	virtual bool CommandDone(CNFC_Command* c);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LG_SENSORS };
#endif
	virtual UINT GetId() { return IDD_LG_SENSORS; };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedGetall();
	afx_msg void OnBnClickedGetVrange();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedSetO2coeff();
	afx_msg void OnBnClickedSetCocoeff();
	afx_msg void OnBnClickedSetH2scoeff();
	afx_msg void OnBnClickedSetCh4coeff();
	afx_msg void OnBnClickedSetO2coeff2();
	afx_msg void OnBnClickedSetVrange();
	afx_msg void OnBnClickedGetSensorstatus();
	BOOL m_CH4En;
	BOOL m_H2SEn;
	BOOL m_O2En;
	BOOL m_COEn;
	afx_msg void OnBnClickedSetSensorstatus();
	afx_msg void OnBnClickedSetO2coeffdef();
	afx_msg void OnBnClickedSetCocoeffdef();
	afx_msg void OnBnClickedSetH2scoeffdef();
	afx_msg void OnBnClickedSetCh4coeffdef();
	afx_msg void OnBnClickedSetO2coeffdef2();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	BOOL m_bSetAll;
	afx_msg void OnBnClickedSetall();
	virtual void ChangeAll(CNFC_Device* dev, UINT nMode, CHANGEALL* c);
//	BOOL m_OChem;
	afx_msg void OnBnClickedSetCoeff();
	afx_msg void OnBnClickedCheckH2sstatus();
	afx_msg void OnBnClickedCoStatus();
	virtual void OnTabSelected();
	afx_msg void OnBnClickedCheckMipexstatus();
	CXTipComboBox m_ComboMipex;
	CXTipComboBox m_ComboO2;
	afx_msg void OnBnClickedCheckO2status();
	CComboBox m_ComboCO;
	afx_msg void OnCbnSelchangeComboCo();
	BOOL m_bPressure;
	afx_msg void OnBnClickedPressure();
	CComboBox m_CO_Pos;
	CComboBox m_ComboH2S;
	CComboBox m_H2S_Pos;
	CComboBox m_MIPEX_Pos;
	CComboBox m_O2_Pos;
	afx_msg void OnBnClickedSetCh4mult();
private:
	void MultToControls();
	void SettingToControls();
	void CoeffToControls();
	void VRangeToControls();
	void SensStatusControls();
};
