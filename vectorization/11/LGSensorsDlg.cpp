// LGSensorsDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "LGSensorsDlg.h"
#include "afxdialogex.h"
#include "NFC_Device.h"
#include "GridCtrl.h"
#include <DialogOperations.h>


// CLGSensorsDlg dialog

IMPLEMENT_DYNAMIC(CLGSensorsDlg, CLongGasBaseDlg)

CLGSensorsDlg::CLGSensorsDlg(CWnd* pParent /*=nullptr*/)
	: CLongGasBaseDlg(IDD_LG_SENSORS, pParent)
	, m_CH4En(FALSE)
	, m_H2SEn(FALSE)
	, m_O2En(FALSE)
	, m_COEn(FALSE)
	, m_bSetAll(FALSE)
	, m_bPressure(FALSE)
{

}

CLGSensorsDlg::~CLGSensorsDlg()
{
}

void CLGSensorsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_MIPEXSTATUS, m_CH4En);
	DDX_Check(pDX, IDC_CHECK_H2SSTATUS, m_H2SEn);
	DDX_Check(pDX, IDC_CHECK_O2STATUS, m_O2En);
	DDX_Check(pDX, IDC_CO_STATUS, m_COEn);
	DDX_Check(pDX, IDC_SETALL, m_bSetAll);
	DDX_Control(pDX, IDC_COMBO_MIPEX, m_ComboMipex);
	DDX_Control(pDX, IDC_COMBO_O2, m_ComboO2);
	DDX_Control(pDX, IDC_COMBO_CO, m_ComboCO);
	DDX_Check(pDX, IDC_PRESSURE, m_bPressure);
	DDX_Control(pDX, IDC_COMBO_CO_POS, m_CO_Pos);
	DDX_Control(pDX, IDC_COMBO_H2S, m_ComboH2S);
	DDX_Control(pDX, IDC_COMBO_H2S_POS, m_H2S_Pos);
	DDX_Control(pDX, IDC_COMBO_MIPEX_POS, m_MIPEX_Pos);
	DDX_Control(pDX, IDC_COMBO_O2_POS, m_O2_Pos);
}


BEGIN_MESSAGE_MAP(CLGSensorsDlg, CLongGasBaseDlg)
	ON_BN_CLICKED(IDC_GETALL, &CLGSensorsDlg::OnBnClickedGetall)
	ON_BN_CLICKED(IDC_GET_VRANGE, &CLGSensorsDlg::OnBnClickedGetVrange)
	ON_BN_CLICKED(IDC_SET_O2COEFF, &CLGSensorsDlg::OnBnClickedSetO2coeff)
	ON_BN_CLICKED(IDC_SET_COCOEFF, &CLGSensorsDlg::OnBnClickedSetCocoeff)
	ON_BN_CLICKED(IDC_SET_H2SCOEFF, &CLGSensorsDlg::OnBnClickedSetH2scoeff)
	ON_BN_CLICKED(IDC_SET_CH4COEFF, &CLGSensorsDlg::OnBnClickedSetCh4coeff)
	ON_BN_CLICKED(IDC_SET_O2COEFF2, &CLGSensorsDlg::OnBnClickedSetO2coeff2)
	ON_BN_CLICKED(IDC_SET_VRANGE, &CLGSensorsDlg::OnBnClickedSetVrange)
	ON_BN_CLICKED(IDC_GET_SENSORSTATUS, &CLGSensorsDlg::OnBnClickedGetSensorstatus)
	ON_BN_CLICKED(IDC_SET_SENSORSTATUS, &CLGSensorsDlg::OnBnClickedSetSensorstatus)
	ON_BN_CLICKED(IDC_SET_O2COEFFDEF, &CLGSensorsDlg::OnBnClickedSetO2coeffdef)
	ON_BN_CLICKED(IDC_SET_COCOEFFDEF, &CLGSensorsDlg::OnBnClickedSetCocoeffdef)
	ON_BN_CLICKED(IDC_SET_H2SCOEFFDEF, &CLGSensorsDlg::OnBnClickedSetH2scoeffdef)
	ON_BN_CLICKED(IDC_SET_CH4COEFFDEF, &CLGSensorsDlg::OnBnClickedSetCh4coeffdef)
	ON_BN_CLICKED(IDC_SET_O2COEFFDEF2, &CLGSensorsDlg::OnBnClickedSetO2coeffdef2)
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_SETALL, &CLGSensorsDlg::OnBnClickedSetall)
	ON_BN_CLICKED(IDC_SET_COEFF, &CLGSensorsDlg::OnBnClickedSetCoeff)
	ON_BN_CLICKED(IDC_CHECK_H2SSTATUS, &CLGSensorsDlg::OnBnClickedCheckH2sstatus)
	ON_BN_CLICKED(IDC_CO_STATUS, &CLGSensorsDlg::OnBnClickedCoStatus)
	ON_BN_CLICKED(IDC_CHECK_MIPEXSTATUS, &CLGSensorsDlg::OnBnClickedCheckMipexstatus)
	ON_BN_CLICKED(IDC_CHECK_O2STATUS, &CLGSensorsDlg::OnBnClickedCheckO2status)
	ON_CBN_SELCHANGE(IDC_COMBO_CO, &CLGSensorsDlg::OnCbnSelchangeComboCo)
	ON_BN_CLICKED(IDC_PRESSURE, &CLGSensorsDlg::OnBnClickedPressure)
	ON_BN_CLICKED(IDC_SET_CH4MULT, &CLGSensorsDlg::OnBnClickedSetCh4mult)
END_MESSAGE_MAP()

void CLGSensorsDlg::MultToControls()
{
	for (int i = 0; i < 6; i++)
	{
		GetDlgItem(IDC_EDIT59 + i * 2)->SetWindowText(S_O::FormatAllDigits(nfcCurrent->sensors_info.fCoefCH4_Mult[i]));
	}
	for (int i = 0; i < 6; i++)
	{
		GetDlgItem(IDC_EDIT65 + i * 2 + (i < 2 ? 0 : 4))->SetWindowText(S_O::FormatAllDigits(nfcCurrent->sensors_info.fCoefCH4_Mult2[i]));
	}
	GetDlgItem(IDC_EDIT71)->SetWindowText(S_O::Format("%d", nfcCurrent->sensors_info.nSwitchConc));
}

void CLGSensorsDlg::SettingToControls()
{
	DEVICE_SETTINGS* s = &nfcCurrent->dev_settings;
	RM_DATABASEIDLIST l;
	nfcCurrent->GetAvailableGas(GAS_CO, l);
	l.FillCombo(&m_ComboCO, nfcCurrent->sensors_info.bCOEn);
	nfcCurrent->GetAvailableGas(GAS_H2S, l);
	l.FillCombo(&m_ComboH2S, nfcCurrent->sensors_info.bH2SEn);
	OnBnClickedCheckO2status();
	UINT ids[] = { IDC_STATIC_COPPMTOMG,IDC_STATIC_H2SPPMTOMG,IDC_STATIC_VOLTOLEL, IDC_STATIC_O2PPMTOMG };
	CString g[] = { "CO ","H2S ","","CHEM " };
	for (int i = 0; i < 4; i++)
	{
		UINT nGas = 1 << i, nSid = 0;
		if (nGas == GAS_MIPEX)nSid = (nfcCurrent->GetUnits(nGas) > 1) ? IDS_PPMTOMG : IDS_VOLTOLEL;
		else nSid = (nfcCurrent->GetUnits(nGas) > 1) ? IDS_VOLTOLEL : IDS_PPMTOMG;
		D_O::SetWindowTextW(GetDlgItem(ids[i]), S_O::Format("%s%s", g[i], nfcCurrent->LoadString(nSid, 1)));
	}
	GetDlgItem(IDC_COEFFVOLTOLEL)->SetWindowText(S_O::FormatUINT(s->base.CoefVolToLEL));
	GetDlgItem(IDC_COEFFCOPPMTOMG)->SetWindowText(S_O::FormatUINT(s->base.CoefCOppmToMg));
	GetDlgItem(IDC_COEFFH2SPPMTOMG)->SetWindowText(S_O::FormatUINT(s->base.CoefH2SppmToMg));
	GetDlgItem(IDC_COEFFCHEMPPMTOMG)->SetWindowText(S_O::FormatUINT(s->base.CoefCHEMppmToMg));
	GetDlgItem(IDC_SET_COEFF)->EnableWindow(1);
	GetDlgItem(IDC_PRESSURE)->EnableWindow(nfcCurrent->IsHaveProperty(CHPRESSURE));
	UpdateData();
	m_bPressure = s->Options & DEVOPT_CH4PRESSURE;
	UpdateData(0);
	CComboBox* c[] = { &m_O2_Pos,&m_CO_Pos,&m_H2S_Pos,&m_MIPEX_Pos };
	for (int i = 0; i < 4; i++)
	{
		c[i]->ShowWindow(nfcCurrent->IsHaveProperty(GASPOS));
		BYTE n = (s->O2ChemScrPos >> (i * 2)) & 3;
		D_O::SelectIDInCombo(*c[i], n);
	}
}

void CLGSensorsDlg::CoeffToControls()
{
	CString str;
	float* f = &nfcCurrent->sensors_info.fO2Coeff[0];
	for (int i = 0; i < 47; i++)
	{
		//str.Format("%f", f[i]);
		GetDlgItem(IDC_EDIT1 + i * 2)->SetWindowText(S_O::FormatAllDigits(f[i]));
	}
	f = &nfcCurrent->sensors_info.fAccO2;
	UINT nIds[] = { IDC_ACC_O2,IDC_ACC_CO,IDC_ACC_H2S,IDC_ACC_MIPEX };
	for (int i = 0; i < 4; i++)
	{
		//str.Format("%f", f[i]);
		GetDlgItem(nIds[i])->SetWindowText(S_O::FormatAllDigits(f[i]));
	}
	f = (float*)&nfcCurrent->sensors_info.fCH4Threshold;
	for (int i = 0; i < 7; i++)
	{
		if (i)str = S_O::FormatAllDigits(f[i]);// str.Format("%f", f[i]);
		else str = S_O::FormatUINT(nfcCurrent->sensors_info.fCH4Threshold);
		GetDlgItem(IDC_EDIT52 + i * 2)->SetWindowText(str);
	}
	bool bEn = nfcCurrent->IsHaveProperty(CH4_MULT);
	UINT Ids[] = { IDC_GROUP_CH4_MULT ,IDC_STATIC_59 ,IDC_STATIC_60,IDC_STATIC_61,IDC_STATIC_62,IDC_STATIC_63,IDC_STATIC_64,
		IDC_EDIT59 ,IDC_EDIT60,IDC_EDIT61,IDC_EDIT62,IDC_EDIT63,IDC_EDIT64,IDC_SET_CH4MULT,
		IDC_STATIC_65,IDC_STATIC_66,IDC_STATIC_67,IDC_STATIC_68,IDC_STATIC_69,IDC_STATIC_70,IDC_STATIC_71,
		IDC_EDIT65,IDC_EDIT66,IDC_EDIT67,IDC_EDIT68,IDC_EDIT69,IDC_EDIT70,IDC_EDIT71 };
	for (int i = 0; i < sizeof(Ids) / sizeof(UINT); i++)GetDlgItem(Ids[i])->ShowWindow(bEn);
	if (bEn && !nfcCurrent->IsHaveProperty(CH4_MULT_NEW))
	{
		MultToControls();
	}
}

void CLGSensorsDlg::VRangeToControls()
{
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_VRANGE);
	GAS_RANGE* r[] = { &nfcCurrent->sensors_info.o2VRange,&nfcCurrent->sensors_info.coVRange,&nfcCurrent->sensors_info.h2sVRange };
	for (int i = 0; i < 3; i++)
	{
		grid->names[i * 3 + 1] = S_O::FormatUINT(r[i]->wFrom);
		grid->names[i * 3 + 2] = S_O::FormatUINT(r[i]->wTo);
	}
	grid->FillData();
}

void CLGSensorsDlg::SensStatusControls()
{
	UpdateData();
	BYTE* en[] = { &nfcCurrent->sensors_info.bO2En,&nfcCurrent->sensors_info.bCOEn,&nfcCurrent->sensors_info.bH2SEn,&nfcCurrent->sensors_info.bCH4En };
	BOOL* c[] = { &m_O2En,&m_COEn,&m_H2SEn ,&m_CH4En };
	for (int i = 0; i < 4; i++)
	{
		*(c[i]) = *(en[i]) != 0;
	}
	//		if (nfc->sensors_info.bCOEn == 1)m_H2SEn = 1;
	UpdateData(0);
	if (m_CH4En)D_O::SelectIDInCombo(m_ComboMipex, nfcCurrent->sensors_info.bCH4En);
	else D_O::SelectIDInCombo(m_ComboMipex, 1);
	if (m_O2En)D_O::SelectIDInCombo(m_ComboO2, nfcCurrent->sensors_info.bO2En);
	else D_O::SelectIDInCombo(m_ComboO2, 1);
	if (m_COEn)D_O::SelectIDInCombo(m_ComboCO, nfcCurrent->sensors_info.bCOEn);
	else D_O::SelectIDInCombo(m_ComboCO, CO_CO);
	if (m_H2SEn)D_O::SelectIDInCombo(m_ComboH2S, nfcCurrent->sensors_info.bH2SEn);
	else D_O::SelectIDInCombo(m_ComboH2S, H2S_H2S);
	OnBnClickedCheckMipexstatus();
	OnBnClickedCoStatus();
	OnCbnSelchangeComboCo();
	OnBnClickedCheckO2status();

	CString str;
	str.Format("%s %s", S_O::LoadString(IDS_COEFFS), nfcCurrent->GetGasString(GAS_MIPEX));
	GetDlgItem(IDC_GROUP_MIPEX)->SetWindowText(str);
	str.Format("%s %s", S_O::LoadString(IDS_COEFFS), nfcCurrent->GetGasString(GAS_O2));
	GetDlgItem(IDC_GROUP_O2)->SetWindowText(str);
	str.Format("%s %s", S_O::LoadString(IDS_COEFFS), nfcCurrent->GetGasString(GAS_CO));
	GetDlgItem(IDC_GROUP_CO)->SetWindowText(str);
	str.Format("%s %s", S_O::LoadString(IDS_COEFFS), nfcCurrent->GetGasString(GAS_H2S));
	GetDlgItem(IDC_GROUP_H2S)->SetWindowText(str);
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_VRANGE);
	grid->names[0] = nfcCurrent->GetGasString(GAS_O2);
	grid->names[grid->columnnames.GetSize()] = nfcCurrent->GetGasString(GAS_CO);
	grid->names[2 * grid->columnnames.GetSize()] = nfcCurrent->GetGasString(GAS_H2S);
	grid->FillData();
}

bool CLGSensorsDlg::CommandDone(CNFC_Command* c)
{
	if (!c->nRet)return 1;
	switch (c->nCommand)
	{
	case GetSettingsByte:
	{
		if (c->dev == nfcCurrent)SettingToControls();
		break;
	}
	case GetCH4Poly:
	{
		if (c->dev == nfcCurrent)MultToControls();
		break;
	}
	case GetAllCoefByte:
	{
		if (c->dev == nfcCurrent)CoeffToControls();		
		break;
	}
	
	case GetSensVRangeByte:
	{
		if (c->dev == nfcCurrent)VRangeToControls();
		break;
	}
	case GetSensStatusByte:
	{
		if (c->dev == nfcCurrent)SensStatusControls();		
		break;
	}
	}
	return 1;
}

void CLGSensorsDlg::OnBnClickedGetall()
{
	nfcCurrent->GetAllCoeffs();
}

void CLGSensorsDlg::OnBnClickedGetVrange()
{
	nfcCurrent->GetSensorVoltRange();
}

BOOL CLGSensorsDlg::OnInitDialog()
{
	CLongGasBaseDlg::OnInitDialog();
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_VRANGE);
	grid->bFirstColNarrow = 0;
	grid->columnnames.Add(S_O::LoadString(IDS_GAS));
	grid->columnnames.Add("ADC Min");
	grid->columnnames.Add("ADC Max");
	char* g[] = {"Î2/Chem","CO","H2S"};
	for (int i = 0; i < 3; i++)
	{
		grid->names.Add(g[i]);
		grid->names.Add("");
		grid->names.Add("");
	}
	grid->FillData();
	CComboBox* c[] = { &m_O2_Pos,&m_CO_Pos,&m_H2S_Pos,&m_MIPEX_Pos };
	for (int i = 0; i < 4; i++)
	{
		for (int k = 0; k < 4; k++)
		{
			D_O::AddStringWithId(c[k], S_O::FormatUINT(i + 1), i);
		}
	}
	UINT ids[] = { IDC_STATIC_COPPMTOMG, IDC_STATIC_H2SPPMTOMG, IDC_STATIC_O2PPMTOMG };
	UINT sid[] = { IDS_COPPMTOMG, IDS_H2SPPMTOMG, IDS_O2PPMTOMG };
	for (int i = 0; i < 3; i++)
	{
		D_O::SetWindowTextW(GetDlgItem(ids[i]), nfcCurrent->LoadString(sid[i], 1));
	}
	OnBnClickedSetO2coeffdef();
	OnBnClickedSetCocoeffdef();
	OnBnClickedSetH2scoeffdef();
	OnBnClickedSetCh4coeffdef();
	RM_DATABASEIDLIST l;
	nfcCurrent->GetAvailableGas(GAS_O2, l);
	l.FillCombo(&m_ComboO2,1);
	nfcCurrent->GetAvailableGas(GAS_MIPEX, l);
	l.FillCombo(&m_ComboMipex,1);
	nfcCurrent->GetAvailableGas(GAS_CO, l);
	l.FillCombo(&m_ComboCO, CO_CO);
	nfcCurrent->GetAvailableGas(GAS_H2S, l);
	l.FillCombo(&m_ComboH2S, H2S_H2S);
	return TRUE;  
}


void CLGSensorsDlg::OnBnClickedSetO2coeff()
{
	CString str;
	float* f = &nfcCurrent->sensors_info.fO2Coeff[0];
	for (int i = 0; i < 19; i++)
	{
		GetDlgItem(IDC_EDIT1 + i * 2)->GetWindowText(str);
		f[i] = (float)S_O::ToDouble(str);
	}
	if (m_bSetAll)main->StartAll(STARTALL_COEFF_O2, nfcCurrent);
	else nfcCurrent->SetO2Coeff();
}

void CLGSensorsDlg::OnBnClickedSetCocoeff()
{
	CString str;
	float* f = &nfcCurrent->sensors_info.fO2Coeff[0];
	for (int i = 19; i < 33; i++)
	{
		GetDlgItem(IDC_EDIT1 + i * 2)->GetWindowText(str);
		f[i] = (float)S_O::ToDouble(str);
	}
	if (m_bSetAll)main->StartAll(STARTALL_COEFF_CO, nfcCurrent);
	else nfcCurrent->SetCOCoeff();
}

void CLGSensorsDlg::OnBnClickedSetH2scoeff()
{
	CString str;
	float* f = &nfcCurrent->sensors_info.fO2Coeff[0];
	for (int i = 33; i < 47; i++)
	{
		GetDlgItem(IDC_EDIT1 + i * 2)->GetWindowText(str);
		f[i] = (float)S_O::ToDouble(str);
	}
	if (m_bSetAll)main->StartAll(STARTALL_COEFF_H2S, nfcCurrent);
	else nfcCurrent->SetH2SCoeff();
}

void CLGSensorsDlg::OnBnClickedSetCh4coeff()
{
	if (!nfcCurrent->sensors_info.bLoaded)return;
	CString str;
	float* f = (float *)&nfcCurrent->sensors_info.fCH4Threshold;
	for (int i = 0; i < 7; i++)
	{		
		GetDlgItem(IDC_EDIT52 + i * 2)->GetWindowText(str);
		if (i)f[i] = (float)S_O::ToDouble(str);
		else nfcCurrent->sensors_info.fCH4Threshold = atoi(str);
	}
	if (m_bSetAll)main->StartAll(STARTALL_COEFF_CH4, nfcCurrent);
	else
	{
		nfcCurrent->SetCH4Coeff();
	}
}

void CLGSensorsDlg::OnBnClickedSetO2coeff2()
{
	CString str;
	float *f = &nfcCurrent->sensors_info.fAccO2;
	UINT nIds[] = { IDC_ACC_O2,IDC_ACC_CO,IDC_ACC_H2S,IDC_ACC_MIPEX };
	for (int i = 0; i < 4; i++)
	{
		GetDlgItem(nIds[i])->GetWindowText(str);
		f[i] = (float)S_O::ToDouble(str);
	}
	if (m_bSetAll)main->StartAll(STARTALL_SENSOR_ACCEL, nfcCurrent);
	else nfcCurrent->SetSensorAccel();
}

void CLGSensorsDlg::OnBnClickedSetVrange()
{
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_VRANGE);
	grid->SaveModifyed();
	GAS_RANGE* r[] = { &nfcCurrent->sensors_info.o2VRange,&nfcCurrent->sensors_info.coVRange,&nfcCurrent->sensors_info.h2sVRange };
	for (int i = 0; i < 3; i++)
	{
		r[i]->wFrom=atoi(grid->names[i * 3 + 1]);
		r[i]->wTo = atoi(grid->names[i * 3 + 2]);
	}
	if (m_bSetAll)main->StartAll(STARTALL_SENSOR_VRANGE, nfcCurrent);
	else nfcCurrent->SetSensorVRange();
}

void CLGSensorsDlg::OnBnClickedGetSensorstatus()
{
	nfcCurrent->GetSensorStatus();
	nfcCurrent->GetSettings();
}

void CLGSensorsDlg::OnBnClickedSetSensorstatus()
{
	UpdateData();
	if (nfcCurrent->dev_settings.bLoaded && nfcCurrent->IsHaveProperty(GASPOS))
	{
		CUIntArray arr;
		CComboBox* c[] = { &m_O2_Pos,&m_CO_Pos,&m_H2S_Pos,&m_MIPEX_Pos };
		BYTE btPos = 0;
		for (int i = 0; i < 4; i++)
		{
			UINT nd = D_O::GetSelectedItemData(*c[i]);
			if (S_O::IsAlreadyInArray(nd, arr))
			{
				AfxMessageBox(S_O::LoadString(IDS_POSMUSTBEDIFF),MB_ICONERROR);
				return;
			}
			btPos |= (nd << (i * 2));
			arr.Add(nd);
		}
		if (nfcCurrent->dev_settings.O2ChemScrPos != btPos)
		{
			nfcCurrent->dev_settings.O2ChemScrPos = btPos;
			if (m_bSetAll)main->StartAll(STARTALL_GASPOS, nfcCurrent);
			else
			{
				nfcCurrent->SetSettings(1);				
			}
		}
	}
	BYTE* en[] = { &nfcCurrent->sensors_info.bO2En,&nfcCurrent->sensors_info.bCOEn,&nfcCurrent->sensors_info.bH2SEn,&nfcCurrent->sensors_info.bCH4En };
	BOOL* c[] = { &m_O2En,&m_COEn,&m_H2SEn ,&m_CH4En };
	CComboBox* cb[] = { &m_ComboO2,&m_ComboCO,&m_ComboH2S,&m_ComboMipex };
	UINT nCO = D_O::GetSelectedItemData(m_ComboCO);
	for (int i = 0; i < 4; i++)
	{
		*(en[i]) = *c[i];
		if (*(en[i]))
		{
			*en[i] = D_O::GetSelectedItemData(*cb[i]);
			//if (i == 2 && nCO != CO_MPC)*en[i] = 1;//H2S
		}		
	}
	if (m_bSetAll)main->StartAll(STARTALL_SENSOR_STATUS, nfcCurrent);
	else
	{
		nfcCurrent->SetSensorStatus();
		nfcCurrent->GetSensorStatus();
	}
}

void CLGSensorsDlg::OnBnClickedSetO2coeffdef()
{
	CString str;
	double f[] = {0,1.0147,0,1,0,-1.3,0.0129,-7.8e-7,0,0,0,0,0,0,0,0,0,0,0};
	for (int i = 0; i < 19; i++)
	{
		str.Format("%g", f[i]);
		GetDlgItem(IDC_EDIT1 + i * 2)->SetWindowText(str);
	}
}

void CLGSensorsDlg::OnBnClickedSetCocoeffdef()
{
	CString str;
	double f[] = { -3,1.036,0,1,0,-180,0.358,0,0,0,0,0,0,0 };
	for (int i = 0; i < 14; i++)
	{
		str.Format("%g", f[i]);
		GetDlgItem(IDC_EDIT20 + i * 2)->SetWindowText(str);
	}
}

void CLGSensorsDlg::OnBnClickedSetH2scoeffdef()
{
	CString str;
	double f[] = { 0,0.92,0,1,0,-85.1,0.1674,0,0,0,0,0,0,0 };
	for (int i = 0; i < 14; i++)
	{
		str.Format("%g", f[i]);
		GetDlgItem(IDC_EDIT34 + i * 2)->SetWindowText(str);
	}
}

void CLGSensorsDlg::OnBnClickedSetCh4coeffdef()
{
	CString str;
	float f[] = { 0,1,0,0,0,0,0,0};
	for (int i = 0; i < 7; i++)
	{
		str.Format("%g", f[i]);
		GetDlgItem(IDC_EDIT52 + i * 2)->SetWindowText(str);
	}
}

void CLGSensorsDlg::OnBnClickedSetO2coeffdef2()
{
}

void CLGSensorsDlg::OnTabSelected()
{	
	if (!nfcCurrent->dev_settings.bLoaded)nfcCurrent->GetSettings();
	if (!nfcCurrent->sensors_info.bLoaded)nfcCurrent->GetAllCoeffs(); //PostCommand(IDC_GETALL);		
	SettingToControls();
	CoeffToControls();
	VRangeToControls();
	SensStatusControls();
	MultToControls();
}

void CLGSensorsDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CLongGasBaseDlg::OnShowWindow(bShow, nStatus);
	if (bShow && IsWindowEnabled() && nfcCurrent)
	{				
		if (!nfcCurrent->dev_settings.bLoaded)GetDlgItem(IDC_SET_COEFF)->EnableWindow(0);
	}
}

void CLGSensorsDlg::OnSize(UINT nType, int cx, int cy)
{
	CLongGasBaseDlg::OnSize(nType, cx, cy);
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_VRANGE);
	if (grid)grid->FillData();
}

void CLGSensorsDlg::OnBnClickedSetall()
{
	UpdateData();
	if (m_bSetAll)
	{
		if (AfxMessageBox(S_O::LoadString(IDS_ENABLESETALL_Q), MB_Q) == IDNO)
		{
			m_bSetAll = 0;
			UpdateData(0);
			return;
		}
	}
	
}

void CLGSensorsDlg::ChangeAll(CNFC_Device* dev, UINT nMode, CHANGEALL* c)
{
	
}

void CLGSensorsDlg::OnBnClickedSetCoeff()
{
	UpdateData();
	DEVICE_SETTINGS* s = &nfcCurrent->dev_settings;
	s->base.CoefVolToLEL = atoi(D_O::GetWindowText(GetDlgItem(IDC_COEFFVOLTOLEL)));
	s->base.CoefCOppmToMg = atoi(D_O::GetWindowText(GetDlgItem(IDC_COEFFCOPPMTOMG)));
	s->base.CoefH2SppmToMg = atoi(D_O::GetWindowText(GetDlgItem(IDC_COEFFH2SPPMTOMG)));
	s->base.CoefCHEMppmToMg = atoi(D_O::GetWindowText(GetDlgItem(IDC_COEFFCHEMPPMTOMG)));
	if (m_bSetAll)main->StartAll(STARTALL_COEFF, nfcCurrent);
	else nfcCurrent->SetSettings(1);
	OnBnClickedCheckO2status();
}

void CLGSensorsDlg::OnBnClickedCheckH2sstatus()
{
	UpdateData();
	UINT nCO = D_O::GetSelectedItemData(m_ComboCO);
	UINT nH2S = D_O::GetSelectedItemData(m_ComboH2S);
	if  (m_H2SEn && m_COEn && nCO!= CO_COH2S && nCO!=CO_MPC)
	{
		//D_O::SelectIDInCombo(m_ComboCO, CO_COH2S);
	}
	else if (nCO == CO_COH2S && (!m_H2SEn || !m_COEn))
	{
		//D_O::SelectIDInCombo(m_ComboCO, CO_CO);
	}
	m_ComboH2S.EnableWindow(m_H2SEn);
	//m_ComboH2S.ShowWindow(nCO == CO_MPC);
}


void CLGSensorsDlg::OnBnClickedCoStatus()
{
	UpdateData();
	m_ComboCO.EnableWindow(m_COEn);
	OnBnClickedCheckH2sstatus();
}


void CLGSensorsDlg::OnBnClickedCheckMipexstatus()
{
	UpdateData();
	m_ComboMipex.EnableWindow(m_CH4En);
}


void CLGSensorsDlg::OnBnClickedCheckO2status()
{
	UpdateData();
	m_ComboO2.EnableWindow(m_O2En);
}


void CLGSensorsDlg::OnCbnSelchangeComboCo()
{
	UINT nCO = D_O::GetSelectedItemData(m_ComboCO);
	if (m_H2SEn && m_COEn && nCO != CO_COH2S && nCO != CO_MPC && nCO!= SO2_H2S)
	{
		m_H2SEn = 0;
		UpdateData(0);
	}
	//m_ComboH2S.ShowWindow(nCO == CO_MPC);
	if (nCO == CO_MPC && m_ComboH2S.GetCurSel() == -1)m_ComboH2S.SetCurSel(0);
	OnBnClickedCheckH2sstatus();
}


void CLGSensorsDlg::OnBnClickedPressure()
{
	if (!nfcCurrent->IsHaveProperty(CHPRESSURE) || !nfcCurrent->dev_settings.bLoaded)return;
	UpdateData();
	if (m_bPressure)nfcCurrent->dev_settings.Options |= DEVOPT_CH4PRESSURE;
	else nfcCurrent->dev_settings.Options &= ~DEVOPT_CH4PRESSURE;
	nfcCurrent->SetSettings(1);	
}

void CLGSensorsDlg::OnBnClickedSetCh4mult()
{
	if (!nfcCurrent->sensors_info.bLoaded)return;
	CString str;
	for (int i = 0; i < 6; i++)
	{
		GetDlgItem(IDC_EDIT59 + i * 2)->GetWindowText(str);
		nfcCurrent->sensors_info.fCoefCH4_Mult[i] = S_O::ToDouble(str);
	}
	for (int i = 0; i < 6; i++)
	{
		GetDlgItem(IDC_EDIT65 + i * 2 + (i < 2 ? 0 : 4))->GetWindowText(str);
		nfcCurrent->sensors_info.fCoefCH4_Mult2[i] = S_O::ToDouble(str);
	}
	GetDlgItem(IDC_EDIT71)->GetWindowText(str);
	nfcCurrent->sensors_info.nSwitchConc = atoi(str);
//	if (m_bSetAll)main->StartAll(STARTALL_COEFF_CH4, nfc);
//	else
	{
		nfcCurrent->SetCH4CoeffMult();
		nfcCurrent->GetAllCoeffs();
	}
}
