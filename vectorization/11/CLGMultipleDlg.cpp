// CLGMultipleDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "CLGMultipleDlg.h"
#include "afxdialogex.h"
#include "NFC_Device.h"
#include <DialogOperations.h>


// CLGMultipleDlg dialog

IMPLEMENT_DYNAMIC(CLGMultipleDlg, CDialogEx)

CLGMultipleDlg::CLGMultipleDlg(CWnd* pParent /*=nullptr*/)
	: CLongGasBaseDlg(IDD_LG_MULTIPLE, pParent)
{
	bModified = 0;
}

CLGMultipleDlg::~CLGMultipleDlg()
{
}

void CLGMultipleDlg::DoDataExchange(CDataExchange* pDX)
{
	CLongGasBaseDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOGTIMEOUT, m_LogTimeout);
	DDX_Control(pDX, IDC_DEVICEMODE, m_DeviceMode);
}


BEGIN_MESSAGE_MAP(CLGMultipleDlg, CLongGasBaseDlg)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECK_LOGPERIOD, &CLGMultipleDlg::OnChecked)
	ON_BN_CLICKED(IDC_CHECK_DEVICEMODE, &CLGMultipleDlg::OnChecked)
	ON_BN_CLICKED(IDC_CHECK_O2, &CLGMultipleDlg::OnChecked)
	ON_BN_CLICKED(IDC_CHECK_CO, &CLGMultipleDlg::OnChecked)
	ON_BN_CLICKED(IDC_CHECK_H2S, &CLGMultipleDlg::OnChecked)
	ON_BN_CLICKED(IDC_CHECK_CH4, &CLGMultipleDlg::OnChecked)
	ON_BN_CLICKED(IDC_CHECK_ACCEL, &CLGMultipleDlg::OnChecked)
	ON_BN_CLICKED(IDC_CHECK_SENSORSTATUS, &CLGMultipleDlg::OnChecked)
	ON_BN_CLICKED(IDC_CHECK_VOLTDIAP, &CLGMultipleDlg::OnChecked)
	ON_BN_CLICKED(IDC_CHECK_MEASUREDIAP, &CLGMultipleDlg::OnChecked)
	ON_BN_CLICKED(IDC_CHECK_UNITS, &CLGMultipleDlg::OnChecked)
	ON_BN_CLICKED(IDC_CHECK_LIMITS, &CLGMultipleDlg::OnChecked)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &CLGMultipleDlg::OnDeltaposSpin1)
	ON_BN_CLICKED(IDC_SET_ALL, &CLGMultipleDlg::OnBnClickedSetAll)
	ON_BN_CLICKED(IDC_ALARM_OFF, &CLGMultipleDlg::OnBnClickedAlarmOff)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CLGMultipleDlg message handlers


BOOL CLGMultipleDlg::OnInitDialog()
{
	CLongGasBaseDlg::OnInitDialog();
	CString str;
	CUIntArray arr;
	GetCheckIds(arr);
	for (int i = 0; i < arr.GetSize(); i++)
	{
		str.Format("Check%u", arr[i]);
		CheckDlgButton(arr[i], AfxGetApp()->GetProfileInt("LGMultipleDlg", str, 0) ? BST_CHECKED: BST_UNCHECKED);
	}
	D_O::AddStringWithId(&m_DeviceMode, "Stop mode", 1, 1);
	D_O::AddStringWithId(&m_DeviceMode, "Transport mode", 2);
	return TRUE; 
}


void CLGMultipleDlg::OnDestroy()
{
	if (bModified)
	{
		CString str;
		CUIntArray arr;
		GetCheckIds(arr);
		for (int i = 0; i < arr.GetSize(); i++)
		{
			str.Format("Check%u", arr[i]);
			AfxGetApp()->WriteProfileInt("LGMultipleDlg", str, IsDlgButtonChecked(arr[i]) == BST_CHECKED);
		}
	}
	CLongGasBaseDlg::OnDestroy();
}


void CLGMultipleDlg::OnChecked()
{
	bModified = 1;
	CUIntArray arr;
	GetCheckIds(arr);
	CHANGEALL c;
	for (int i = 0; i < arr.GetSize(); i++)
	{
		if (IsDlgButtonChecked(arr[i]) != BST_CHECKED)continue;
		c.arr.Add(arr[i]);
	}
	main->ChangeAll(this, CHANGEALL_MULTIPLESELECT, &c);
}

bool CLGMultipleDlg::CommandDone(CNFC_Command* c)
{
	if (!c->nRet)return 1;
	switch (c->nCommand)
	{
	case GetInfDeviceByte:
	{
		DEVICE_INFO* info = &c->dev->dev_info;
		m_LogTimeout.SetWindowText(S_O::FormatUINT(info->base.nLogTimeout));
		ShowControls();
		break;
	}
	case GetAllCoefByte:
	case GetSensStatusByte:
	case GetSensVRangeByte:
	case GetGasRangeByte:
	case GetSettingsByte:
	case GetAlarmSignalingByte:
		ShowControls();
		break;
	}
	return 1;
}

void CLGMultipleDlg::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	CString str;
	m_LogTimeout.GetWindowText(str);
	int n = atoi(str) - pNMUpDown->iDelta;
	if (n < 1)n = 1;
	if (n > 255)n = 255;
	m_LogTimeout.SetWindowText(S_O::FormatUINT(n));
	*pResult = 0;
}

void CLGMultipleDlg::OnBnClickedSetAll()
{
	CString str;
	CUIntArray arr;
	GetCheckIds(arr);
	UINT nStart[] = { STARTALL_LOGTIMEOUT,STARTALL_DEVICEMODE,STARTALL_COEFF_O2,STARTALL_COEFF_CO,STARTALL_COEFF_H2S,STARTALL_COEFF_CH4,
					STARTALL_SENSOR_ACCEL,STARTALL_SENSOR_STATUS,STARTALL_SENSOR_VRANGE,STARTALL_SENSOR_GASRANGE ,STARTALL_MULTIPLEUNITS ,STARTALL_ALARMS };
	int nc = 0;
	bool bGetCoeff = 0;
	for (int i = 0; i <arr.GetSize(); i++)
	{
		if (!GetDlgItem(arr[i])->IsWindowVisible())continue;
		if (IsDlgButtonChecked(arr[i]) != BST_CHECKED)continue;
		if (arr[i] == IDC_CHECK_LOGPERIOD)
		{
			m_LogTimeout.GetWindowText(str);
			nfcCurrent->dev_info.base.nLogTimeout = atoi(str);
		}
		else if (arr[i] == IDC_CHECK_DEVICEMODE)
		{
			nfcCurrent->dev_info.btDeviceMode = D_O::GetSelectedItemData(m_DeviceMode);
		}
		if (nStart[i] == STARTALL_COEFF_O2 || nStart[i] == STARTALL_COEFF_CO
			|| nStart[i] == STARTALL_COEFF_H2S || nStart[i] == STARTALL_COEFF_CH4 || nStart[i] == STARTALL_SENSOR_ACCEL)bGetCoeff = 1;
		main->StartAll(nStart[i], nfcCurrent);
		if (nStart[i] == STARTALL_SENSOR_STATUS && nfcCurrent->IsHaveProperty(GASPOS))main->StartAll(STARTALL_GASPOS, nfcCurrent);
		nc++;
	}
	if (!nc)AfxMessageBox(S_O::LoadString(IDS_NEEDONEPARAMETER), MB_ICONINFORMATION);
	else if(bGetCoeff)main->StartAll(STARTALL_GETCOEFF, nfcCurrent);
}

void CLGMultipleDlg::OnBnClickedAlarmOff()
{
	main->StartAll(STARTALL_DISABLEALARM, nfcCurrent);
}

void CLGMultipleDlg::OnTabSelected()
{
	if (!nfcCurrent->dev_info.bLoaded)nfcCurrent->GetDeviceInfo();
	if (!nfcCurrent->sensors_info.bLoaded)nfcCurrent->GetCoeffsOnly();
	if (!nfcCurrent->sensors_info.bStatusLoaded)nfcCurrent->GetSensorStatus();
	if (!nfcCurrent->sensors_info.coVRange.bInited)nfcCurrent->GetSensorVoltRange();
	if (!nfcCurrent->sensors_info.coRange.bInited)nfcCurrent->GetGasRange();
	if (!nfcCurrent->dev_settings.bLoaded)nfcCurrent->GetSettings();
	if (!nfcCurrent->sensors_info.alarms.bLoaded)nfcCurrent->GetAlarms();
	m_LogTimeout.SetWindowText(S_O::FormatUINT(nfcCurrent->dev_info.base.nLogTimeout));
	ShowControls();
}

void CLGMultipleDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CLongGasBaseDlg::OnShowWindow(bShow, nStatus);
	if (bShow && IsWindowEnabled() && nfcCurrent)
	{		
		ShowControls();
	}
}

void CLGMultipleDlg::ShowControls()
{
	CUIntArray arr;
	GetCheckIds(arr);
	UINT bEn[] = { nfcCurrent->dev_info.bLoaded ,nfcCurrent->dev_info.bLoaded,nfcCurrent->sensors_info.bLoaded ,nfcCurrent->sensors_info.bLoaded ,nfcCurrent->sensors_info.bLoaded ,nfcCurrent->sensors_info.bLoaded ,
	nfcCurrent->sensors_info.bLoaded ,nfcCurrent->sensors_info.bStatusLoaded ,nfcCurrent->sensors_info.coVRange.bInited ,nfcCurrent->sensors_info.coRange.bInited ,nfcCurrent->dev_settings.bLoaded,nfcCurrent->sensors_info.alarms.bLoaded };
	for (int i = 0; i < arr.GetSize(); i++)
	{
		GetDlgItem(arr[i])->ShowWindow(bEn[i]);
	}
	GetDlgItem(IDC_LOGTIMEOUT)->ShowWindow(nfcCurrent->dev_info.bLoaded);
	GetDlgItem(IDC_SPIN1)->ShowWindow(nfcCurrent->dev_info.bLoaded);
	GetDlgItem(IDC_DEVICEMODE)->ShowWindow(nfcCurrent->dev_info.bLoaded);
}

void CLGMultipleDlg::GetCheckIds(CUIntArray& arr)
{
	UINT nIds[] = { IDC_CHECK_LOGPERIOD ,IDC_CHECK_DEVICEMODE ,IDC_CHECK_O2 ,IDC_CHECK_CO,IDC_CHECK_H2S,IDC_CHECK_CH4,
					IDC_CHECK_ACCEL,IDC_CHECK_SENSORSTATUS ,IDC_CHECK_VOLTDIAP ,IDC_CHECK_MEASUREDIAP ,IDC_CHECK_UNITS,IDC_CHECK_LIMITS };
	arr.RemoveAll();
	for (int i = 0; i < sizeof(nIds) / sizeof(UINT); i++)arr.Add(nIds[i]);
}

void CLGMultipleDlg::ChangeAll(CNFC_Device* dev, UINT nMode, CHANGEALL* c)
{
	if (nMode == CHANGEALL_MULTIPLESELECT)
	{
		CUIntArray arr;
		GetCheckIds(arr);
		for (int i = 0; i < arr.GetSize(); i++)
		{
			CheckDlgButton(arr[i], S_O::IsAlreadyInArray(arr[i], c->arr) ? BST_CHECKED : BST_UNCHECKED);
		}
	}
}