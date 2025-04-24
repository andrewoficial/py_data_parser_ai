// LGAlarmsDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "LGAlarmsDlg.h"
#include "afxdialogex.h"
#include "NFC_Device.h"
#include "GridCtrl.h"
#include <DialogOperations.h>

extern CLongGasApp theApp;
extern VEGA_PREFERENCES g_pref;
// CLGAlarmsDlg dialog

IMPLEMENT_DYNAMIC(CLGAlarmsDlg, CLongGasBaseDlg)

CLGAlarmsDlg::CLGAlarmsDlg(CWnd* pParent /*=nullptr*/)
	: CLongGasBaseDlg(IDD_LG_ALARMS, pParent)
	, m_CH4Zero(FALSE)
	, m_WaitTime()
	, m_bCH4Factory(FALSE)
{
	m_WaitTime = AfxGetApp()->GetProfileInt("LGAlarms", "CalibrationWaitTime", 25);
}

CLGAlarmsDlg::~CLGAlarmsDlg()
{
}

void CLGAlarmsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COUNITS, m_CO_Units);
	DDX_Control(pDX, IDC_CH4UNITS, m_CH4_Units);
	DDX_Control(pDX, IDC_H2SUNITS, m_H2S_Units);
	DDX_Control(pDX, IDC_O2UNITS, m_O2_Units);
	DDX_Control(pDX, IDC_VIBROPOWER, m_VibroPower);
	DDX_Check(pDX, IDC_CHECK_CH4_ZERO, m_CH4Zero);
	DDX_Check(pDX, IDC_CHECK_CO_ZERO, m_COZero);
	DDX_Check(pDX, IDC_CHECK_H2S_ZERO, m_H2SZero);
	DDX_Check(pDX, IDC_CHECK_O2_ZERO, m_O2Zero);
	DDX_Check(pDX, IDC_CHECK_CH4_CALIB, m_CH4Calib);
	DDX_Check(pDX, IDC_CHECK_CO_CALIB, m_COCalib);
	DDX_Check(pDX, IDC_CHECK_H2S_CALIB, m_H2SCalib);
	DDX_Check(pDX, IDC_CHECK_O2_CALIB, m_O2Calib);
	DDX_Text(pDX, IDC_WAITTIME, m_WaitTime);
	DDX_Check(pDX, IDC_CHECK_CH4_FACTORY, m_bCH4Factory);
	DDX_Check(pDX, IDC_CO_FACTORY, m_bCOFactory);
	DDX_Check(pDX, IDC_CHECK_O2FACTORY, m_bO2Factory);
	DDX_Check(pDX, IDC_CHECK_H2FACTORY, m_bH2SFactory);
}

BEGIN_MESSAGE_MAP(CLGAlarmsDlg, CLongGasBaseDlg)
	ON_BN_CLICKED(IDC_GET_UNITS, &CLGAlarmsDlg::OnBnClickedGetUnits)
	ON_BN_CLICKED(IDC_SET_VIBROPOWER, &CLGAlarmsDlg::OnBnClickedSetVibropower)
	ON_BN_CLICKED(IDC_GET_VIBROPOWER, &CLGAlarmsDlg::OnBnClickedGetVibropower)
	ON_BN_CLICKED(IDC_SET_UNITS, &CLGAlarmsDlg::OnBnClickedSetUnits)
	ON_BN_CLICKED(IDC_GET_ALARMS, &CLGAlarmsDlg::OnBnClickedGetAlarms)
	ON_BN_CLICKED(IDC_SET_ALARMS, &CLGAlarmsDlg::OnBnClickedSetAlarms)
	ON_BN_CLICKED(IDC_SET_ALARMSDEF, &CLGAlarmsDlg::OnBnClickedSetAlarmsdef)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_STARTZEROCALIBRATION, &CLGAlarmsDlg::OnBnClickedStartzerocalibration)
	ON_BN_CLICKED(IDC_CANCELZEROCALIBRATION, &CLGAlarmsDlg::OnBnClickedCancelcalibration)
	ON_BN_CLICKED(IDC_STARTCALIBRATION, &CLGAlarmsDlg::OnBnClickedStartcalibration)
	ON_BN_CLICKED(IDC_CANCELCALIBRATION, &CLGAlarmsDlg::OnBnClickedCancelcalibration)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_STARTZEROCALIBRATIONALL, &CLGAlarmsDlg::OnBnClickedStartzerocalibrationall)
	ON_BN_CLICKED(IDC_STARTCALIBRATIONALL, &CLGAlarmsDlg::OnBnClickedStartcalibrationall)
	ON_BN_CLICKED(IDC_FACTORYRESET, &CLGAlarmsDlg::OnBnClickedFactoryreset)
	ON_BN_CLICKED(IDC_FACTORY_ALL, &CLGAlarmsDlg::OnBnClickedFactoryAll)
	ON_BN_CLICKED(IDC_SET_UNITSALL, &CLGAlarmsDlg::OnBnClickedSetUnitsall)
	ON_BN_CLICKED(IDC_CANCELFACTORYRESET, &CLGAlarmsDlg::OnBnClickedCancelfactoryreset)
	ON_WM_SIZE()
	ON_MESSAGE(WM_NOTIFYLDBLCLK, &CLGAlarmsDlg::OnGridDblClk)
	ON_BN_CLICKED(IDC_SET_PRECISIONS, &CLGAlarmsDlg::OnBnClickedSetPrecisions)
	ON_BN_CLICKED(IDC_SET_PRECISIONSALL, &CLGAlarmsDlg::OnBnClickedSetPrecisionsall)
	ON_BN_CLICKED(IDC_GET_PRECISIONS, &CLGAlarmsDlg::OnBnClickedGetPrecisions)
	ON_BN_CLICKED(IDC_SET_AUTOZERO, &CLGAlarmsDlg::OnBnClickedSetAutozero)
	ON_BN_CLICKED(IDC_SET_AUTOZEROALL, &CLGAlarmsDlg::OnBnClickedSetAutozeroall)
	ON_BN_CLICKED(IDC_GET_AUTOZERO, &CLGAlarmsDlg::OnBnClickedGetAutozero)
	ON_BN_CLICKED(IDC_SET_GASRANGE, &CLGAlarmsDlg::OnBnClickedSetGasrange)
	ON_BN_CLICKED(IDC_GET_GASRANGE, &CLGAlarmsDlg::OnBnClickedGetGasrange)
	ON_BN_CLICKED(IDC_SET_ALLGASRANGE, &CLGAlarmsDlg::OnBnClickedSetAllgasrange)
	ON_BN_CLICKED(IDC_GET_SCALE, &CLGAlarmsDlg::OnBnClickedGetScale)
	ON_BN_CLICKED(IDC_SET_SCALE, &CLGAlarmsDlg::OnBnClickedSetScale)
	ON_BN_CLICKED(IDC_SCALE_AUTOZERO, &CLGAlarmsDlg::OnBnClickedScaleAutozero)
END_MESSAGE_MAP()


// CLGAlarmsDlg message handlers


BOOL CLGAlarmsDlg::OnInitDialog()
{
	CLongGasBaseDlg::OnInitDialog();
	if (PRIMARYLANGID(g_pref.nLang) != LANG_RUSSIAN)GetDlgItem(IDC_STATIC_INFO)->ShowWindow(0);
	CString str;
	UINT ids[] = {IDC_EDIT_O2,IDC_EDIT_CO,IDC_EDIT_H2S,IDC_EDIT_CH4 };
	for (int i = 0; i < 4; i++)
	{
		str.Format("CalRef%d", i);
		GetDlgItem(ids[i])->SetWindowText(AfxGetApp()->GetProfileString("LGAlarmsDlg", str, ""));
	}
	DIALOG_OPERATIONS::AddStringWithId(&m_VibroPower, "1", 1, 1);
	DIALOG_OPERATIONS::AddStringWithId(&m_VibroPower, "2", 2);
	DIALOG_OPERATIONS::AddStringWithId(&m_VibroPower, "3", 3);
	RM_DATABASEIDLIST l;
	CMyComboBox* c[] = { &m_CO_Units,&m_H2S_Units,&m_CH4_Units,&m_O2_Units };
	for (int i = 0; i < 4; i++)
	{
		c[i]->nParams = MCB__PAR_UNICODE;
		c[i]->SetFontSize(15);
	}
	BOOL * b[] = {&m_O2Zero,&m_COZero,&m_H2SZero,&m_CH4Zero,&m_O2Calib,&m_COCalib,&m_H2SCalib,&m_CH4Calib };
	UINT nSel=AfxGetApp()->GetProfileInt("LGAlarmsDlg", "CalSel", 0);
	for (int i = 0; i < 8; i++)*b[i] = (nSel & (1 << i)) != 0;
	UpdateData(0);
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_ALARMS);
	grid->nGridId = GRIDID_LIMITS;
	grid->FillDataLimits(nfcCurrent);
	grid = (CGridCtrl*)GetDlgItem(IDC_GRID_SCALE);
	grid->nGridId = GRIDID_SCALEPOINT;
	UpdateUnits();
	GetDlgItem(IDC_ZERO_CH4)->SetWindowText(S_O::FormatUINT(minCH4));
	GetDlgItem(IDC_ZERO_O2)->SetWindowText(S_O::FormatUINT(minO2));
	GetDlgItem(IDC_ZERO_H2S)->SetWindowText(S_O::FormatUINT(minH2S));
	GetDlgItem(IDC_ZERO_CO)->SetWindowText(S_O::FormatUINT(minCO));
	GetDlgItem(IDC_MAX_CH4)->SetWindowText(S_O::FormatValue(maxCH4));
	GetDlgItem(IDC_MAX_O2)->SetWindowText(S_O::FormatValue(maxO2));
	GetDlgItem(IDC_MAX_H2S)->SetWindowText(S_O::FormatValue(maxH2S));
	GetDlgItem(IDC_MAX_CO)->SetWindowText(S_O::FormatValue(maxCO));
	return TRUE;
}

void CLGAlarmsDlg::ShowControls()
{
	UINT nPrecIds[] = { IDC_CO_PREC,IDC_H2S_PREC,IDC_CH4_PREC,IDC_O2_PREC};
	UINT nAutoZeroIds[] = { IDC_CO_AUTOZERO,IDC_H2S_AUTOZERO,IDC_CH4_AUTOZERO,IDC_O2_AUTOZERO };
	UINT nCalibIds[] = { IDC_CHECK_CO_CALIB,IDC_CHECK_H2S_CALIB,IDC_CHECK_CH4_CALIB,IDC_CHECK_O2_CALIB };
	UINT nCalibEdit[] = { IDC_EDIT_CO,IDC_EDIT_H2S,IDC_EDIT_CH4,IDC_EDIT_O2 };
	UINT nZeroIds[] = { IDC_CHECK_CO_ZERO,IDC_CHECK_H2S_ZERO,IDC_CHECK_CH4_ZERO,IDC_CHECK_O2_ZERO };
	UINT nFactIds[] = { IDC_CO_FACTORY,IDC_CHECK_H2FACTORY,IDC_CHECK_CH4_FACTORY,IDC_CHECK_O2FACTORY };
	UINT nUnits[] = { IDC_COUNITS,IDC_H2SUNITS,IDC_CH4UNITS, IDC_O2UNITS };
	UINT nUnitsStatic[] = { IDC_STATIC_COUNITS,IDC_STATIC_H2SUNITS,IDC_STATIC_CH4UNITS, IDC_STATIC_O2UNITS };
	UINT nCalUnitsStatic[] = { IDC_STATIC_CALCOUNITS,IDC_STATIC_CALH2SUNITS,IDC_STATIC_CALCH4UNITS, IDC_STATIC_CALO2UNITS };
	UINT* nArr[] = { nPrecIds ,nAutoZeroIds ,nCalibIds ,nZeroIds,nCalibEdit,nFactIds,nUnits,nUnitsStatic,nCalUnitsStatic };
	BYTE en[] = { nfcCurrent->sensors_info.bCOEn,nfcCurrent->sensors_info.bH2SEn,nfcCurrent->sensors_info.bCH4En,nfcCurrent->sensors_info.bO2En };
	for (int i = 0; i < sizeof(nArr) / sizeof(UINT*); i++)
	{
		for (int k = 0; k < 4; k++)
		{
			GetDlgItem(nArr[i][k])->ShowWindow(nfcCurrent->bManufact || en[k]);
		}
	}
	for (int i = 0; i < 4; i++)
	{
		D_O::SetWindowTextW(GetDlgItem(nCalUnitsStatic[i]), nfcCurrent->GetUnitsString(1 << i, 1));
	}
	/*if (!nfc->IsHaveProperty(AUTOZERO))GetDlgItem(IDC_CO_AUTOZERO)->ShowWindow(0);
	else
	{
		if (nfc->GetSelectedGas(GAS_CO) == CO_MPC)
		{
			if (nfc->IsGasSelected(GAS_MIPEX))GetDlgItem(IDC_CO_AUTOZERO)->ShowWindow(0);
			else GetDlgItem(IDC_CO_AUTOZERO)->EnableWindow(1);
		}
		else GetDlgItem(IDC_CO_AUTOZERO)->ShowWindow(0);
	}*/
	GetDlgItem(IDC_H2S_AUTOZERO)->ShowWindow(0);
	GetDlgItem(IDC_CO_AUTOZERO)->ShowWindow(0);
	GetDlgItem(IDC_O2UNITS)->ShowWindow(nfcCurrent->dev_settings.base.O2Chem);
	GetDlgItem(IDC_STATIC_O2UNITS)->ShowWindow(nfcCurrent->dev_settings.base.O2Chem);
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_ALARMS);
	grid->FillDataLimits(nfcCurrent);
	UINT idsg[] = { IDC_STATIC_CO,IDC_STATIC_10,IDC_FROM_CO ,IDC_TO_CO,
		IDC_STATIC_H2S,IDC_STATIC_11,IDC_FROM_H2S ,IDC_TO_H2S,
		IDC_STATIC_CH4,IDC_STATIC_09,IDC_FROM_CH4,IDC_TO_CH4 ,
		IDC_STATIC_O2,IDC_STATIC_12,IDC_FROM_O2 ,IDC_TO_O2};
	for (int i = 0; i < sizeof(idsg) / sizeof(UINT*); i++)
		GetDlgItem(idsg[i])->ShowWindow(nfcCurrent->bManufact && en[i/4]);
	CString strMIPEX = nfcCurrent->GetGasString(GAS_MIPEX);
	UINT nMIPEX[] = { IDC_STATIC_CH4UNITS, IDC_CH4_PREC ,IDC_CH4_AUTOZERO ,IDC_CHECK_CH4_FACTORY ,
		IDC_CHECK_CH4_ZERO ,IDC_CHECK_CH4_CALIB ,IDC_STATIC_CH4 };
	for (int i = 0; i < sizeof(nMIPEX) / sizeof(UINT); i++)
	{
		GetDlgItem(nMIPEX[i])->SetWindowText(strMIPEX);
	}
	CString strO2 = nfcCurrent->GetGasString(GAS_O2);
	UINT nO2[] = { IDC_STATIC_O2 ,IDC_STATIC_O2UNITS ,IDC_O2_PREC ,IDC_O2_AUTOZERO ,IDC_CHECK_O2FACTORY ,
		IDC_CHECK_O2_ZERO ,IDC_CHECK_O2_CALIB };
	for (int i = 0; i < sizeof(nO2) / sizeof(UINT); i++)
	{
		GetDlgItem(nO2[i])->SetWindowText(strO2);
	}
	CString strCO = nfcCurrent->GetGasString(GAS_CO);
	UINT nCO[] = { IDC_STATIC_CO ,IDC_STATIC_COUNITS ,IDC_CO_PREC ,IDC_CO_AUTOZERO ,IDC_CO_FACTORY ,
		IDC_CHECK_CO_ZERO ,IDC_CHECK_CO_CALIB };
	for (int i = 0; i < sizeof(nCO) / sizeof(UINT); i++)
	{
		GetDlgItem(nCO[i])->SetWindowText(strCO);
	}
	CString strH2S = nfcCurrent->GetGasString(GAS_H2S);
	UINT nH2S[] = { IDC_STATIC_H2S ,IDC_STATIC_H2SUNITS ,IDC_H2S_PREC ,IDC_H2S_AUTOZERO ,IDC_CHECK_H2FACTORY ,
		IDC_CHECK_H2S_ZERO ,IDC_CHECK_H2S_CALIB };
	for (int i = 0; i < sizeof(nH2S) / sizeof(UINT); i++)
	{
		GetDlgItem(nH2S[i])->SetWindowText(strH2S);
	}
	UINT nScale[] = { IDC_STATIC_SCALE ,IDC_GRID_SCALE ,IDC_SET_SCALE ,IDC_GET_SCALE };
	for (int i = 0; i < sizeof(nScale) / sizeof(UINT); i++)
	{
		GetDlgItem(nScale[i])->ShowWindow(nfcCurrent->IsHaveProperty(SCALEPOINT));
	}
	GetDlgItem(IDC_MAX_O2)->SetWindowText(S_O::FormatValue((nfcCurrent->sensors_info.bO2En!=O2_O2)?maxCO:maxO2));
}

void CLGAlarmsDlg::UpdateUnits()
{
	RM_DATABASEIDLIST l;
	CMyComboBox* c[] = { &m_CO_Units,&m_H2S_Units,&m_CH4_Units,&m_O2_Units };
	for (int i = 0; i < 4; i++)
	{
		UINT nGas = 1 << i;
		nfcCurrent->GetAvailableUnits(nGas, l, 1);
		l.FillCombo(c[i], nfcCurrent->GetUnits(nGas), 1);
		if(nGas==GAS_O2)D_O::SetWindowTextW(GetDlgItem(IDC_STATIC_CALO2UNITS), l.Get(0));
	}
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_SCALE);
	grid->FillDataScalePoints(nfcCurrent);
	//nfc->GetAvailableUnits(GAS_MIPEX, list,1);
	//list.FillCombo(&m_CH4_Units, nfc->GetUnits(GAS_MIPEX), 1);
	//nfc->GetAvailableUnits(GAS_O2, list, 1);
	//list.FillCombo(&m_O2_Units, nfc->GetUnits(GAS_O2),1);

}

void CLGAlarmsDlg::EnablePrecision()
{
	if (!nfcCurrent->IsHaveProperty(PRECISION))return;
	bool bEnCO = !nfcCurrent->calibration_settings.bInCalibration, bEnCH4 = !nfcCurrent->calibration_settings.bInCalibration;
	if (nfcCurrent->GetSelectedGas(GAS_CO) == CO_MPC)bEnCO = 0;
	if (nfcCurrent->GetUnits(GAS_MIPEX) > 1)bEnCH4 = 0;
	GetDlgItem(IDC_CO_PREC)->EnableWindow(bEnCO);
	GetDlgItem(IDC_CH4_PREC)->EnableWindow(bEnCH4);
	if (!bEnCO)CheckDlgButton(IDC_CO_PREC, BST_UNCHECKED);
	if (!bEnCH4)CheckDlgButton(IDC_CH4_PREC, BST_UNCHECKED);
}

void CLGAlarmsDlg::GasRangeToControls()
{
	GAS_RANGE* r[] = { &nfcCurrent->sensors_info.ch4Range,&nfcCurrent->sensors_info.coRange,&nfcCurrent->sensors_info.h2sRange,&nfcCurrent->sensors_info.o2Range };
	UINT gas[] = { GAS_MIPEX,GAS_CO,GAS_H2S,GAS_O2 };
	for (int i = 0; i < 4; i++)
	{
		float dDev = nfcCurrent->GetGasDevider(gas[i], 1);
		double f = (float)r[i]->wFrom / dDev;
		GetDlgItem(IDC_FROM_CH4 + i * 4)->SetWindowText(S_O::FormatValue(f));
		f = (float)r[i]->wTo / dDev;
		GetDlgItem(IDC_TO_CH4 + i * 4)->SetWindowText(S_O::FormatValue(f));
	}
}

void CLGAlarmsDlg::SettingsToControls()
{
	UpdateUnits();
	if (nfcCurrent->IsHaveProperty(PRECISION))
	{
		UINT nIds[] = { IDC_CO_PREC,IDC_H2S_PREC,IDC_CH4_PREC,IDC_O2_PREC,IDC_SET_PRECISIONS,IDC_SET_PRECISIONSALL,IDC_GET_PRECISIONS };
		for (int i = 0; i < sizeof(nIds) / sizeof(UINT*); i++)
		{
			GetDlgItem(nIds[i])->EnableWindow(!nfcCurrent->calibration_settings.bInCalibration);
		}
		for (int i = 0; i < 4; i++)
		{
			bool bSet = 1;
			if (nfcCurrent->dev_settings.SensorsPrecisions & (1 << i))bSet = 0;
			CheckDlgButton(nIds[i], bSet ? BST_CHECKED : BST_UNCHECKED);
		}
		EnablePrecision();
	}
	if (nfcCurrent->IsHaveProperty(AUTOZERO))
	{
		UINT nIds[] = { IDC_CO_AUTOZERO,IDC_H2S_AUTOZERO,IDC_CH4_AUTOZERO,IDC_O2_AUTOZERO,IDC_SET_AUTOZERO,
		IDC_SET_AUTOZEROALL, IDC_GET_AUTOZERO };
		for (int i = 0; i < sizeof(nIds) / sizeof(UINT*); i++)
		{
			if (i < 2)continue;//dont work for co and h2s
			GetDlgItem(nIds[i])->EnableWindow(!nfcCurrent->calibration_settings.bInCalibration);
		}
		if (nfcCurrent->dev_settings.base.O2Chem)GetDlgItem(IDC_O2_AUTOZERO)->EnableWindow(0);
		for (int i = 0; i < 4; i++)
		{
			if (i < 2)continue;
			bool bSet = 0;
			if (nfcCurrent->dev_settings.SensorsAutoZero & (1 << i))bSet = 1;
			CheckDlgButton(nIds[i], bSet ? BST_CHECKED : BST_UNCHECKED);
		}
	}
	bool bScaleSet = 0;
	if (nfcCurrent->IsHaveProperty(SCALEPOINT))
	{
		if (nfcCurrent->dev_settings.SensorsAutoZero & (1 << 4))bScaleSet = 1;
	}
	CheckDlgButton(IDC_SCALE_AUTOZERO, bScaleSet ? BST_CHECKED : BST_UNCHECKED);
	ShowControls();
}

void CLGAlarmsDlg::AlarmSignalingToControls()
{
	GAS_RANGE* r[] = { &nfcCurrent->sensors_info.alarms.ch4Alarm,&nfcCurrent->sensors_info.alarms.o2Alarm,
				&nfcCurrent->sensors_info.alarms.coAlarm,&nfcCurrent->sensors_info.alarms.h2sAlarm };
	UINT gas[] = { GAS_MIPEX,GAS_O2,GAS_CO,GAS_H2S };
	for (int i = 0; i < 4; i++)
	{
		float dDev = nfcCurrent->GetGasDevider(gas[i], 1);
		double f = (float)r[i]->wFrom / dDev;
		nfcCurrent->limits[i].strLimit1 = S_O::FormatValue(f);
		f = (float)r[i]->wTo / dDev;
		nfcCurrent->limits[i].strLimit2 = S_O::FormatValue(f);
	}
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_ALARMS);
	grid->FillDataLimits(nfcCurrent);
}

bool CLGAlarmsDlg::CommandDone(CNFC_Command* c)
{
	if (!c->nRet)return 1;
	switch (c->nCommand)
	{
	case GetGasRangeByte:
	{
		if (c->dev == nfcCurrent)GasRangeToControls();
		break;
	}
	case GetSensStatusByte:
	{
		if (c->dev == nfcCurrent)
		{
			ShowControls();
			UpdateUnits();
			EnablePrecision();
		}
		break;
	}
	case SetSensUnitsByte:
		if(!nfcCurrent->calibration_settings.bInCalibration)nfcCurrent->GetAlarms();
		nfcCurrent->GetGasRange();
		break;
	case GetSettingsByte:
	{
		if (c->dev == nfcCurrent)SettingsToControls();
		break;
	}
	case GetRegVibroByte:
	{
		if (c->dev == nfcCurrent)
		{
			D_O::SelectIDInCombo(m_VibroPower, nfcCurrent->dev_info.base.btVibroPower);
		}
		break;
	}
	case GetAlarmSignalingByte:
	{
		if (c->dev == nfcCurrent)AlarmSignalingToControls();
		break;
	}
	}
	return 1;
}

void CLGAlarmsDlg::OnBnClickedGetUnits()
{
	nfcCurrent->GetSettings();
}

void CLGAlarmsDlg::OnBnClickedSetVibropower()
{
	nfcCurrent->dev_info.base.btVibroPower = DIALOG_OPERATIONS::GetSelectedItemData(m_VibroPower);
	nfcCurrent->SetVibroPower();
}

void CLGAlarmsDlg::OnBnClickedGetVibropower()
{
	nfcCurrent->GetVibroPower();
}

void CLGAlarmsDlg::OnBnClickedSetUnits()
{
	SetUnits(0);
}

void CLGAlarmsDlg::OnBnClickedGetAlarms()
{
	nfcCurrent->GetAlarms();
}

void CLGAlarmsDlg::OnBnClickedSetAlarms()
{
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_ALARMS);
	grid->SaveModifyed();
	SENSORS_INFO save = nfcCurrent->sensors_info;
	GAS_RANGE* r[] = { &nfcCurrent->sensors_info.alarms.ch4Alarm,&nfcCurrent->sensors_info.alarms.o2Alarm,
		& nfcCurrent->sensors_info.alarms.coAlarm,& nfcCurrent->sensors_info.alarms.h2sAlarm };
	GAS_RANGE* range[] = { &nfcCurrent->sensors_info.ch4Range,&nfcCurrent->sensors_info.o2Range,
		& nfcCurrent->sensors_info.coRange,& nfcCurrent->sensors_info.h2sRange };
	UINT gas[] = { GAS_MIPEX,GAS_O2,GAS_CO,GAS_H2S };
	CString str;
	int nOutOfRange = -1;
	for (int i = 0; i < 4; i++)
	{
		if (!nfcCurrent->IsGasSelected(gas[i]))continue;
		if (S_O::ToDouble(nfcCurrent->limits[i].strLimit1) > S_O::ToDouble(nfcCurrent->limits[i].strLimit2))
		{
			str.Format("%s - %s", nfcCurrent->GetGasString(gas[i]), S_O::LoadString(IDS_LIMITSMUSTBEINRANGE));
			AfxMessageBox(str, MB_ICONERROR);
			return;
		}
		float dDev = nfcCurrent->GetGasDevider(gas[i],1);
		double f = S_O::ToDouble(nfcCurrent->limits[i].strLimit1) * dDev;
		if (gas[i] != GAS_O2 || nfcCurrent->sensors_info.bO2En!=O2_O2)
		{
			if (f < min(range[i]->wFrom, range[i]->wTo) || f >  max(range[i]->wFrom, range[i]->wTo))
			{
				nOutOfRange = i;
				break;
			}
		}
		else if (gas[i] == GAS_O2)
		{
			if (f < 0 || f > 30 * dDev)
			{
				nOutOfRange = i;
				break;
			}
		}
		r[i]->wFrom = (WORD)f;
		f = S_O::ToDouble(nfcCurrent->limits[i].strLimit2) * dDev;
		if (gas[i] != GAS_O2 || nfcCurrent->sensors_info.bO2En != O2_O2)
		{
			if (f < min(range[i]->wFrom, range[i]->wTo) || f >  max(range[i]->wFrom, range[i]->wTo))
			{
				nOutOfRange = i;
				break;
			}
		}
		else if (gas[i] == GAS_O2 )
		{
			if (f < 0 || f > 30 * dDev)
			{
				nOutOfRange = i;
				break;
			}
		}
		r[i]->wTo = (WORD)f;
	}
	if (nOutOfRange!=-1)
	{
		nfcCurrent->sensors_info = save;
		str.Format("%s - %s", nfcCurrent->GetGasString(gas[nOutOfRange]), S_O::LoadString(IDS_LIMITSMUSTBEINRANGE));
		AfxMessageBox(str, MB_ICONERROR);
		return;
	}
	nfcCurrent->SetAlarms();
}


void CLGAlarmsDlg::OnBnClickedSetAlarmsdef()
{
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_ALARMS);
	double def[] = {10,20,19.5,23.5,20,40,10,20};
	for (int i = 0; i < 4; i++)
	{
		nfcCurrent->limits[i].strLimit1 = S_O::FormatValue(def[i * 2]);
		nfcCurrent->limits[i].strLimit2 = S_O::FormatValue(def[i * 2+1]);
	}
	grid->RedrawWindow();
}

void CLGAlarmsDlg::OnTabSelected()
{
	if (!nfcCurrent->sensors_info.bStatusLoaded)nfcCurrent->GetSensorStatus();
	if (!nfcCurrent->sensors_info.alarms.bLoaded)nfcCurrent->GetAlarms();
	if (!nfcCurrent->dev_settings.bLoaded)nfcCurrent->GetSettings();
	if (!nfcCurrent->sensors_info.bGasRangeLoaded)nfcCurrent->GetGasRange();
	if (!nfcCurrent->dev_info.bRegVibroLoaded)nfcCurrent->GetVibroPower();
	UINT ids[] = { IDC_COUNITS,IDC_H2SUNITS,IDC_CH4UNITS, IDC_O2UNITS,IDC_SET_UNITS,IDC_SET_UNITSALL };
	for (int i = 0; i < sizeof(ids) / sizeof(UINT*); i++)
		GetDlgItem(ids[i])->EnableWindow(nfcCurrent->bManufact);
	UINT ids2[] = { IDC_STATIC_DIAP,IDC_STATIC_CH4,IDC_STATIC_CO,IDC_STATIC_H2S,IDC_STATIC_O2,IDC_STATIC_MIN,IDC_STATIC_MAX ,IDC_ZERO_CH4 ,IDC_ZERO_CO,IDC_ZERO_O2,IDC_ZERO_H2S,
	IDC_MAX_CH4 ,IDC_MAX_CO,IDC_MAX_H2S,IDC_MAX_O2,IDC_STATIC_FROM,IDC_STATIC_TO,IDC_STATIC_1 ,IDC_STATIC_02,IDC_STATIC_03,IDC_STATIC_04,
	IDC_STATIC_05,IDC_STATIC_06,IDC_STATIC_07,IDC_STATIC_08,IDC_STATIC_09,IDC_STATIC_10,IDC_STATIC_11,IDC_STATIC_12,
		IDC_FROM_CH4 ,IDC_TO_CH4 ,IDC_FROM_CO ,IDC_TO_CO,IDC_FROM_O2 ,IDC_TO_O2,IDC_FROM_H2S ,IDC_TO_H2S,
		IDC_SET_GASRANGE ,IDC_GET_GASRANGE,IDC_SET_ALLGASRANGE,
		IDC_SCALE_AUTOZERO ,IDC_STATIC_SCALE ,IDC_GRID_SCALE ,IDC_SET_SCALE ,IDC_GET_SCALE };
	for (int i = 0; i < sizeof(ids2) / sizeof(UINT*); i++)
		GetDlgItem(ids2[i])->ShowWindow(nfcCurrent->bManufact);
	GasRangeToControls();
	SettingsToControls();
	AlarmSignalingToControls();
	D_O::SelectIDInCombo(m_VibroPower, nfcCurrent->dev_info.base.btVibroPower);
	OperationToControls(nfcCurrent, nfcCurrent->nCurrentOperation, nfcCurrent->nCurrentOperation!=-1);
}


void CLGAlarmsDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CLongGasBaseDlg::OnShowWindow(bShow, nStatus);
	if (bShow && IsWindowEnabled() && nfcCurrent)
	{
		if (!nfcCurrent->IsHaveProperty(PRECISION))
		{
			UINT nIds[] = { IDC_CO_PREC,IDC_H2S_PREC,IDC_CH4_PREC,IDC_O2_PREC,IDC_SET_PRECISIONS,IDC_SET_PRECISIONSALL,IDC_GET_PRECISIONS };
			for (int i = 0; i < sizeof(nIds) / sizeof(UINT*); i++)
				GetDlgItem(nIds[i])->EnableWindow(0);
		}
		if (!nfcCurrent->IsHaveProperty(AUTOZERO))
		{
			UINT nIds[] = { IDC_CO_AUTOZERO,IDC_H2S_AUTOZERO,IDC_CH4_AUTOZERO,IDC_O2_AUTOZERO,IDC_SET_AUTOZERO,
				IDC_SET_AUTOZEROALL,IDC_GET_AUTOZERO };
			for (int i = 0; i < sizeof(nIds) / sizeof(UINT*); i++)
				GetDlgItem(nIds[i])->EnableWindow(0);
		}
		GetDlgItem(IDC_SCALE_AUTOZERO)->EnableWindow(nfcCurrent->IsHaveProperty(SCALEPOINT));
		ShowControls();
	}
}

void CLGAlarmsDlg::OperationToControls(CNFC_Device* nfc, UINT nType, bool bOperationStarted)
{
	if (nfc != nfcCurrent)return;
	if (bOperationStarted)
	{
		if (nType == STARTALL_CALIBRATION)
		{
			EnableWindow(1);
			D_O::EnableAllChilds(this, 0);
			GetDlgItem(IDC_CANCELZEROCALIBRATION)->EnableWindow(1);
			GetDlgItem(IDC_CANCELCALIBRATION)->EnableWindow(1);
			main->SetFocus();
		}
		else if (nType == STARTALL_FACTORYRESET)
		{
			EnableWindow(1);
			D_O::EnableAllChilds(this, 0);
			GetDlgItem(IDC_CANCELFACTORYRESET)->EnableWindow(1);
			main->SetFocus();
		}
		else EnableWindow(0);
	}
	else
	{
		D_O::EnableAllChilds(this, 1);
		GetDlgItem(IDC_CANCELZEROCALIBRATION)->EnableWindow(0);
		GetDlgItem(IDC_CANCELCALIBRATION)->EnableWindow(0);
		GetDlgItem(IDC_CANCELFACTORYRESET)->EnableWindow(0);
		EnableWindow(1);
	}
}

bool CLGAlarmsDlg::OperationStarted(CNFC_Device* nfc, UINT nType)
{
	OperationToControls(nfc,nType,1);
	return 1;
}

bool CLGAlarmsDlg::OperationEnded(CNFC_Device* nfc, UINT nType)
{
	OperationToControls(nfc, nType, 0);
	return 1;
}

void CLGAlarmsDlg::OnBnClickedStartzerocalibration()
{
	StartZeroCalibration(0);
}

void CLGAlarmsDlg::StartCalibration(bool bAll)
{
	UpdateData();
	SaveSelection();
	nfcCurrent->calibration_settings.bCalibZero = 0;
	nfcCurrent->calibration_settings.bAll = bAll;
	nfcCurrent->calibration_settings.dParamWaitTime = m_WaitTime;
	SetCalibrationLimits();
	AfxGetApp()->WriteProfileInt("LGAlarms", "CalibrationWaitTime", m_WaitTime);
	UINT ids[] = { 0,IDC_EDIT_O2,IDC_EDIT_CO,IDC_EDIT_H2S,IDC_EDIT_CH4 };
	BOOL b[] = { 0,m_O2Calib,m_COCalib,m_H2SCalib,m_CH4Calib };
	UINT gas[] = { 0,GAS_O2,GAS_CO,GAS_H2S,GAS_MIPEX };
	int nc = 0;
	CString str, buf;
	for (int i = 1; i <= 4; i++)
	{
		CWnd* w = GetDlgItem(ids[i]);
		nfcCurrent->calibration_settings.GasCalbSens[i] = b[i] && w->IsWindowVisible();
		nc += nfcCurrent->calibration_settings.GasCalbSens[i];
		w->GetWindowText(str);
		S_O::Trim(str);
		if (nfcCurrent->calibration_settings.GasCalbSens[i] && str == "")
		{
			AfxMessageBox(S_O::LoadString(IDS_NEEDSETCONCENTRATION));
			return;
		}
		buf.Format("CalRef%d", i - 1);
		AfxGetApp()->WriteProfileString("LGAlarmsDlg", buf, str);
		nfcCurrent->calibration_settings.GasCalbRefVal[i] = nfcCurrent->RecalcForCalibration(gas[i], S_O::ToDouble(str));
	}
	if (!nc)
	{
		AfxMessageBox(S_O::LoadString(IDS_NEEDONEGAS));
		return;
	}
	if (bAll)main->StartAll(STARTALL_CALIBRATION, nfcCurrent);
	else nfcCurrent->Calibrate();
}

void CLGAlarmsDlg::StartZeroCalibration(bool bAll)
{
	UpdateData();
	SaveSelection();
	nfcCurrent->calibration_settings.bCalibZero = 1;
	nfcCurrent->calibration_settings.bAll = bAll;
	nfcCurrent->calibration_settings.dParamWaitTime = m_WaitTime;
	SetCalibrationLimits();
	AfxGetApp()->WriteProfileInt("LGAlarms", "CalibrationWaitTime", m_WaitTime);
	BOOL b[] = { 0,m_O2Zero,m_COZero,m_H2SZero,m_CH4Zero };
	UINT nZeroIds[] = { 0, IDC_CHECK_O2_ZERO,IDC_CHECK_CO_ZERO,IDC_CHECK_H2S_ZERO,IDC_CHECK_CH4_ZERO };
	int nc = 0;
	for (int i = 1; i <= 4; i++)
	{
		nfcCurrent->calibration_settings.GasCalbSens[i] = b[i] && GetDlgItem(nZeroIds[i])->IsWindowVisible();
		nc += nfcCurrent->calibration_settings.GasCalbSens[i];
	}
	if (!nc)
	{
		AfxMessageBox(S_O::LoadString(IDS_NEEDONEGAS));
		return;
	}
	if (bAll)main->StartAll(STARTALL_CALIBRATION, nfcCurrent);
	else nfcCurrent->Calibrate();
}

void CLGAlarmsDlg::OnBnClickedStartcalibration()
{
	StartCalibration(0);
}

void CLGAlarmsDlg::OnBnClickedCancelcalibration()
{
	main->BreakAll(nfcCurrent);
}

void CLGAlarmsDlg::OnDestroy()
{
	CLongGasBaseDlg::OnDestroy();
}

void CLGAlarmsDlg::OnBnClickedStartzerocalibrationall()
{
	StartZeroCalibration(1);
}

void CLGAlarmsDlg::OnBnClickedStartcalibrationall()
{
	StartCalibration(1);
}

void CLGAlarmsDlg::SaveSelection()
{
	BOOL b[] = { m_O2Zero,m_COZero,m_H2SZero,m_CH4Zero,m_O2Calib,m_COCalib,m_H2SCalib,m_CH4Calib };
	UINT nSel = 0;
	for (int i = 0; i < 8; i++)if (b[i])nSel |= (1 << i);
	AfxGetApp()->WriteProfileInt("LGAlarmsDlg", "CalSel", nSel);
}

void CLGAlarmsDlg::OnBnClickedFactoryreset()
{
	FactoryReset(0);
}

void CLGAlarmsDlg::OnBnClickedFactoryAll()
{
	FactoryReset(1);
}

void CLGAlarmsDlg::FactoryReset(bool bAll)
{
	UINT nSel = 0;
	UpdateData();
	BOOL b[] = { m_bCOFactory,m_bH2SFactory,m_bCH4Factory,m_bO2Factory };
	UINT nFactIds[] = { IDC_CO_FACTORY,IDC_CHECK_H2FACTORY,IDC_CHECK_CH4_FACTORY,IDC_CHECK_O2FACTORY };
	for (int i = 0; i < 4; i++)
	{
		if (b[i] && GetDlgItem(nFactIds[i])->IsWindowVisible())nSel |= (1<<i);
	}
	if (!nSel)
	{
		AfxMessageBox(S_O::LoadString(IDS_NEEDONEGAS), MB_ICONINFORMATION);
		return;
	}
	nfcCurrent->additional_pars.nFactoryResetGasSel = nSel;
	if(bAll)main->StartAll(STARTALL_FACTORYRESET, nfcCurrent);
	else nfcCurrent->FactoryReset();
}


void CLGAlarmsDlg::OnBnClickedSetUnitsall()
{
	SetUnits(1);
}

void CLGAlarmsDlg::SetExtendedUnits(UINT nUnits, UINT nGas)
{
	if (nUnits == 1)nfcCurrent->dev_settings.SensorsUnits |= nGas;
	else if (nUnits > 1 && nfcCurrent->IsExtendedUnits(nGas))
	{
		UINT nMask = nfcCurrent->GetExtendedUnitsMask(nGas);
		if (nUnits == 2)nfcCurrent->dev_settings.SensorsUnits |= nMask;
		else if (nUnits == 3)nfcCurrent->dev_settings.SensorsUnits |= (nGas | nMask);
	}
}

void CLGAlarmsDlg::SetUnits(bool bAll)
{
	nfcCurrent->dev_settings.SensorsUnits = 0;
	CMyComboBox* c[] = { &m_CO_Units,&m_H2S_Units,&m_CH4_Units,&m_O2_Units };
	for (int i = 0; i < 4; i++)
	{
		UINT nGas = 1 << i;
		SetExtendedUnits(D_O::GetSelectedItemData(*c[i]), nGas);
	}
	/*if (DIALOG_OPERATIONS::GetSelectedItemData(m_CO_Units))nfc->dev_settings.SensorsUnits |= GAS_CO;
	if (DIALOG_OPERATIONS::GetSelectedItemData(m_H2S_Units))nfc->dev_settings.SensorsUnits |= GAS_H2S;
	UINT nCH4 = DIALOG_OPERATIONS::GetSelectedItemData(m_CH4_Units);
	if (nCH4==1)nfc->dev_settings.SensorsUnits |= GAS_MIPEX;
	else if (nCH4>1 && nfc->IsHaveProperty(CH4_PDK_UNITS))
	{
		if (nCH4 == 2)nfc->dev_settings.SensorsUnits |= CH4_UNITS_PDK;
		else if (nCH4 == 3)nfc->dev_settings.SensorsUnits |= GAS_MIPEX|CH4_UNITS_PDK;
	}
	if (DIALOG_OPERATIONS::GetSelectedItemData(m_O2_Units))nfc->dev_settings.SensorsUnits |= GAS_O2;*/
	if (bAll)main->StartAll(STARTALL_UNITS, nfcCurrent);
	else nfcCurrent->SetSensorUnits();
}

bool CLGAlarmsDlg::SetCalibrationLimits()
{
	double dMinOffs = theApp.GetProfileDouble("Calibration", "MinOffs", -100);
	double dMaxOffs = theApp.GetProfileDouble("Calibration", "MaxOffs", 100);
	double dMinAmp = theApp.GetProfileDouble("Calibration", "MinAmp", 0.7);
	double dMaxAmp = theApp.GetProfileDouble("Calibration", "MaxAmp", 1.3);
	double dMin[] = { dMinOffs,dMinAmp,dMinOffs,dMinAmp};
	double dMax[] = { dMaxOffs,dMaxAmp,dMaxOffs,dMaxAmp};
	for (int i = 0; i < 4; i++)
	{
		nfcCurrent->calibration_settings.dCoeffMin[i] = dMin[i];
		nfcCurrent->calibration_settings.dCoeffMax[i] = dMax[i];
	}
	return 1;
}

void CLGAlarmsDlg::OnBnClickedCancelfactoryreset()
{
	main->BreakAll(nfcCurrent);
}

void CLGAlarmsDlg::OnSize(UINT nType, int cx, int cy)
{
	CLongGasBaseDlg::OnSize(nType, cx, cy);
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_ALARMS);
	if (grid)grid->FillData();
}

LRESULT CLGAlarmsDlg::OnGridDblClk(WPARAM wp, LPARAM lp)
{
	CGridCtrl* gridFrom = (CGridCtrl*)wp, * grid = (CGridCtrl*)GetDlgItem(IDC_GRID_ALARMS);
	if (grid == gridFrom)
	{
		int nRow = grid->GetActiveRow(), nCol = grid->GetActiveCol();
		if (nCol < 3)return 1;
		if (!nfcCurrent->bManufact || nfcCurrent->limits[nRow].nGas == GAS_MIPEX)return 1;
		nfcCurrent->TestLimit(nfcCurrent->limits[nRow].nGas, nCol == 4);
	}
	return 1;
}


void CLGAlarmsDlg::SetPrecisions(bool bAll)
{
	nfcCurrent->dev_settings.SensorsPrecisions = 0;
	UINT nIds[] = { IDC_CO_PREC,IDC_H2S_PREC,IDC_CH4_PREC,IDC_O2_PREC };
	for (int i = 0; i < 4; i++)
	{
		UINT n = IsDlgButtonChecked(nIds[i]) == BST_CHECKED;
		if (!n)nfcCurrent->dev_settings.SensorsPrecisions |= (1 << i);
	}
	if (bAll)main->StartAll(STARTALL_PRECISIONS, nfcCurrent);
	else nfcCurrent->SetSettings(1);
}

void CLGAlarmsDlg::OnBnClickedSetPrecisions()
{
	SetPrecisions(0);
}

void CLGAlarmsDlg::OnBnClickedSetPrecisionsall()
{
	SetPrecisions(1);
}

void CLGAlarmsDlg::OnBnClickedGetPrecisions()
{
	nfcCurrent->GetSettings();
}

void CLGAlarmsDlg::SetAutozero(bool bAll)
{
	nfcCurrent->dev_settings.SensorsAutoZero = 0;
	UINT nIds[] = { IDC_CO_AUTOZERO,IDC_H2S_AUTOZERO,IDC_CH4_AUTOZERO,IDC_O2_AUTOZERO,IDC_SCALE_AUTOZERO };
	for (int i = 0; i < 5; i++)
	{
		UINT n = IsDlgButtonChecked(nIds[i]) == BST_CHECKED;
		if (n)nfcCurrent->dev_settings.SensorsAutoZero |= (1 << i);
	}
	if (bAll)main->StartAll(STARTALL_AUTOZERO, nfcCurrent);
	else nfcCurrent->SetSettings(1);
}

void CLGAlarmsDlg::OnBnClickedSetAutozero()
{
	SetAutozero(0);
}

void CLGAlarmsDlg::OnBnClickedSetAutozeroall()
{
	SetAutozero(1);
}

void CLGAlarmsDlg::OnBnClickedGetAutozero()
{
	nfcCurrent->GetSettings();
}

bool CLGAlarmsDlg::SetGasRange(bool bAll)
{
	CString str;
	GAS_RANGE* r[] = { &nfcCurrent->sensors_info.ch4Range,&nfcCurrent->sensors_info.coRange,
		& nfcCurrent->sensors_info.h2sRange,& nfcCurrent->sensors_info.o2Range };
	UINT gas[] = { GAS_MIPEX,GAS_CO,GAS_H2S,GAS_O2 };
	for (int i = 0; i < 4; i++)
	{
		float dDev = nfcCurrent->GetGasDevider(gas[i],1);
		GetDlgItem(IDC_FROM_CH4 + i * 4)->GetWindowText(str);
		double f = S_O::ToDouble(str) * dDev;
		r[i]->wFrom = (WORD)f;
		GetDlgItem(IDC_TO_CH4 + i * 4)->GetWindowText(str);
		f = S_O::ToDouble(str) * dDev;
		r[i]->wTo = (WORD)f;
	}
	if (bAll)main->StartAll(STARTALL_SENSOR_GASRANGE, nfcCurrent);
	else nfcCurrent->SetGasRange();
	return 1;
}

void CLGAlarmsDlg::OnBnClickedSetGasrange()
{
	SetGasRange(0);
}

void CLGAlarmsDlg::OnBnClickedGetGasrange()
{
	nfcCurrent->GetGasRange();
}

void CLGAlarmsDlg::OnBnClickedSetAllgasrange()
{
	SetGasRange(1);
}


void CLGAlarmsDlg::OnBnClickedGetScale()
{
	nfcCurrent->GetSettings();
}

void CLGAlarmsDlg::OnBnClickedSetScale()
{
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_SCALE);
	grid->SaveModifyed();
	nfcCurrent->SetSettings(1);
}

BOOL CLGAlarmsDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_CHAR)
	{
		return !IsNumber(pMsg);
	}
	return CLongGasBaseDlg::PreTranslateMessage(pMsg);
}

void CLGAlarmsDlg::OnBnClickedScaleAutozero()
{
	if (!nfcCurrent->dev_settings.bLoaded)return;
	UpdateData();
	bool bSet = IsDlgButtonChecked(IDC_SCALE_AUTOZERO) == BST_CHECKED;
	if (bSet)nfcCurrent->dev_settings.SensorsAutoZero |= (1 << 4);
	else nfcCurrent->dev_settings.SensorsAutoZero &= ~(1 << 4);
	nfcCurrent->SetSettings(1);
}
