// LGMainDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "LGMainDlg.h"
#include "afxdialogex.h"
#include "NFC_Device.h"
#include <DialogOperations.h>


// CLGMainDlg dialog

IMPLEMENT_DYNAMIC(CLGMainDlg, CLongGasBaseDlg)

CLGMainDlg::CLGMainDlg(CWnd* pParent /*=nullptr*/)
	: CLongGasBaseDlg(IDD_LG_MAIN, pParent)
	, m_bBeep(FALSE)
	, m_bVibro(FALSE)
	, m_bAlarm(FALSE)
	, m_bDaylight(FALSE)
	, m_SWVer(_T(""))
	, m_IDSensor(_T(""))
	, m_CPUId(_T(""))
	, m_bSkipSelfTest(FALSE)
	, m_ShortLog(FALSE)
	, m_bMipexLog(FALSE)
	, m_bSkipSelfTestTransport(FALSE)
	, m_Accum(FALSE)
{
	strPrevSerial = "-1";
}

CLGMainDlg::~CLGMainDlg()
{
}

void CLGMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SERIALNO, m_SerialNo);
	DDX_Control(pDX, IDC_LOGTIMEOUT, m_LogTimeout);
	DDX_Check(pDX, IDC_CHECK_BEEP, m_bBeep);
	DDX_Check(pDX, IDC_CHECK_VIBRO, m_bVibro);
	DDX_Check(pDX, IDC_CHECK_ALARM, m_bAlarm);
	DDX_Control(pDX, IDC_DEVICEMODE, m_DeviceMode);
	DDX_Check(pDX, IDC_CHECK_DAYLIGHT, m_bDaylight);
	DDX_Control(pDX, IDC_DATE, m_Date);
	DDX_Control(pDX, IDC_TIME, m_Time);
	DDX_Text(pDX, IDC_EDIT_SWVER, m_SWVer);
	DDX_Text(pDX, IDC_EDIT_IDSENSOR, m_IDSensor);
	DDX_Text(pDX, IDC_EDIT_IDCPU, m_CPUId);
	DDX_Control(pDX, IDC_LOGTIMEOUT2, m_LogAlarmTimeout);
	DDX_Check(pDX, IDC_CHECK_SKIPSELFTEST, m_bSkipSelfTest);
	DDX_Control(pDX, IDC_WEEKTOSCALE, m_WeekToScale);
	DDX_Check(pDX, IDC_SHORTLOG, m_ShortLog);
	DDX_Check(pDX, IDC_MIPEXLOG, m_bMipexLog);
	DDX_Check(pDX, IDC_CHECK_SKIPSELFTESTTRANSPORT, m_bSkipSelfTestTransport);
	DDX_Check(pDX, IDC_ACCUM, m_Accum);
}


BEGIN_MESSAGE_MAP(CLGMainDlg, CLongGasBaseDlg)
	ON_BN_CLICKED(IDC_BLINK, &CLGMainDlg::OnBnClickedBlink)
	ON_BN_CLICKED(IDC_BEEP, &CLGMainDlg::OnBnClickedBeep)
	ON_BN_CLICKED(IDC_REBOOT, &CLGMainDlg::OnBnClickedReboot)
	ON_BN_CLICKED(IDC_REPLACEBATTERY, &CLGMainDlg::OnBnClickedReplacebattery)
	ON_BN_CLICKED(IDC_REFRESHDEVINFO, &CLGMainDlg::OnBnClickedRefreshdevinfo)
	ON_BN_CLICKED(IDC_SETSERIALNO, &CLGMainDlg::OnBnClickedSetserialno)
	ON_BN_CLICKED(IDC_SETLOGTIMEOUT, &CLGMainDlg::OnBnClickedSetlogtimeout)
	ON_BN_CLICKED(IDC_CHECK_BEEP, &CLGMainDlg::OnBnClickedCheckBeep)
	ON_BN_CLICKED(IDC_CHECK_VIBRO, &CLGMainDlg::OnBnClickedCheckVibro)
	ON_BN_CLICKED(IDC_CHECK_ALARM, &CLGMainDlg::OnBnClickedCheckAlarm)
	ON_BN_CLICKED(IDC_SETDEVICEMODE, &CLGMainDlg::OnBnClickedSetdevicemode)
	ON_BN_CLICKED(IDC_NFCPOWERDOWNENABLE, &CLGMainDlg::OnBnClickedNfcpowerdownenable)
	ON_BN_CLICKED(IDC_NFCPOWERDOWNDISABLE, &CLGMainDlg::OnBnClickedNfcpowerdowndisable)
	ON_BN_CLICKED(IDC_SETDATETIME, &CLGMainDlg::OnBnClickedSetdatetime)
	ON_BN_CLICKED(IDC_SETDATETIMECOMP, &CLGMainDlg::OnBnClickedSetdatetimecomp)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &CLGMainDlg::OnDeltaposSpin1)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_SETLOGTIMEOUT2, &CLGMainDlg::OnBnClickedSetlogtimeout2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN7, &CLGMainDlg::OnDeltaposSpin7)
	ON_BN_CLICKED(IDC_CHECK_SKIPSELFTEST, &CLGMainDlg::OnBnClickedCheckSkipselftest)
	ON_BN_CLICKED(IDC_SETWEEKTOSCALE, &CLGMainDlg::OnBnClickedSetweektoscale)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_WEEKTOSCALE, &CLGMainDlg::OnDeltaposSpinWeektoscale)
	ON_BN_CLICKED(IDC_SHORTLOG, &CLGMainDlg::OnBnClickedShortlog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_MIPEXLOG, &CLGMainDlg::OnBnClickedMipexlog)
	ON_BN_CLICKED(IDC_CHECK_SKIPSELFTESTTRANSPORT, &CLGMainDlg::OnBnClickedCheckSkipselftesttransport)
	ON_BN_CLICKED(IDC_ACCUM, &CLGMainDlg::OnBnClickedAccum)
END_MESSAGE_MAP()


// CLGMainDlg message handlers
void CLGMainDlg::OnBnClickedBlink()
{
	nfcCurrent->Blink();
}

void CLGMainDlg::OnBnClickedBeep()
{
	nfcCurrent->Beep();
}

void CLGMainDlg::OnBnClickedReboot()
{
	nfcCurrent->Reboot();
}

void CLGMainDlg::OnBnClickedReplacebattery()
{
	if (AfxMessageBox(S_O::LoadString(IDS_REPLACEBATTERY_Q), MB_YESNO | MB_ICONQUESTION) == IDNO)return;
	nfcCurrent->ReplaceBattery();
}

void CLGMainDlg::OnBnClickedRefreshdevinfo()
{
	nfcCurrent->GetDeviceInfo();
	nfcCurrent->GetSettings();
}

void CLGMainDlg::OnBnClickedSetserialno()
{
	CString str;
	m_SerialNo.GetWindowText(str);
	nfcCurrent->SetSerialNumber(atoi(str));
}

void CLGMainDlg::OnBnClickedSetlogtimeout()
{
	CString str;
	m_LogTimeout.GetWindowText(str);
	nfcCurrent->SetLogTimeout(atoi(str));
}

void CLGMainDlg::OnBnClickedCheckBeep()
{
	UpdateData();
	nfcCurrent->EnableBeep(m_bBeep);
}

void CLGMainDlg::OnBnClickedCheckVibro()
{
	UpdateData();
	nfcCurrent->EnableVibro(m_bVibro);
}

void CLGMainDlg::OnBnClickedCheckAlarm()
{
	UpdateData();
	nfcCurrent->EnableAlarm(m_bAlarm);
}

void CLGMainDlg::OnBnClickedSetdevicemode()
{
	nfcCurrent->SetDeviceMode(DIALOG_OPERATIONS::GetSelectedItemData(m_DeviceMode));
}

BOOL CLGMainDlg::OnInitDialog()
{
	CLongGasBaseDlg::OnInitDialog();
	DIALOG_OPERATIONS::AddStringWithId(&m_DeviceMode, "Stop mode", 1,1);
	DIALOG_OPERATIONS::AddStringWithId(&m_DeviceMode, "Transport mode", 2);
	m_Date.SetTime(&CTime::GetCurrentTime());
	m_Time.SetTime(&CTime::GetCurrentTime());
	return TRUE;
}

void CLGMainDlg::OnBnClickedNfcpowerdownenable()
{
	nfcCurrent->EnableNFCPowerDown(1);
}

void CLGMainDlg::OnBnClickedNfcpowerdowndisable()
{
	nfcCurrent->EnableNFCPowerDown(0);
}

void CLGMainDlg::OnBnClickedSetdatetime()
{
	UpdateData();
	SYSTEMTIME st1, st2;
	m_Date.GetTime(&st1);
	m_Time.GetTime(&st2);
	st1.wHour = st2.wHour;
	st1.wMinute = st2.wMinute;
	st1.wSecond = st2.wSecond;
	nfcCurrent->SetDateTime((UINT)nfcCurrent->SystemTimeToTime(st1), m_bDaylight);
	KillTimer(0);
}

void CLGMainDlg::OnBnClickedSetdatetimecomp()
{
	UpdateData();
	SYSTEMTIME st;
	GetLocalTime(&st);
	nfcCurrent->SetDateTime((UINT)nfcCurrent->SystemTimeToTime(st), m_bDaylight);
	SetTimer(0, 500, 0);
}

void CLGMainDlg::InfToControls()
{
	DEVICE_INFO* info = &nfcCurrent->dev_info;
	m_SWVer = info->strSoftwareVer;
	m_IDSensor = info->strSensorId;
	m_CPUId = info->strCPUId;
	m_bAlarm = info->base.bAlarm != 0;
	m_bBeep = info->base.bBeep != 0;
	m_bVibro = info->base.bVibro != 0;
	m_bDaylight = info->base.bDaylight != 0;
	UpdateData(0);
	m_SerialNo.SetWindowText(nfcCurrent->GetSerialNumber());
	m_LogTimeout.SetWindowText(S_O::FormatUINT(info->base.nLogTimeout));
	SYSTEMTIME st;
	CNFC_Device::TimeToSystemTIME(info->nTime, st);
	CDateTime tm(st);
	m_Date.SetTime(tm);
	m_Time.SetTime(tm);
	if (strPrevSerial != nfcCurrent->GetSerialNumber())KillTimer(0);
	strPrevSerial = nfcCurrent->GetSerialNumber();
}

void CLGMainDlg::SettingsToControls()
{
	m_LogAlarmTimeout.SetWindowText(S_O::FormatUINT(nfcCurrent->dev_settings.LogAlarmTimeOut));
	UpdateData();
	m_bSkipSelfTest = (nfcCurrent->dev_settings.SkipSelfTest & SKIPSELFTEST_STANDART) != 0;
	if (nfcCurrent->IsHaveProperty(SHORTLOG))
	{
		m_ShortLog = (nfcCurrent->dev_settings.Options & DEVOPT_SHORTLOG) != 0;
		m_bMipexLog = (nfcCurrent->dev_settings.Options & DEVOPT_MIPEXLOG) != 0;
	}
	m_bSkipSelfTestTransport= nfcCurrent->IsHaveProperty(SKIPSELFTTESTTRANSPORT)?(nfcCurrent->dev_settings.SkipSelfTest & SKIPSELFTEST_TRANSPORT) != 0:0;
	if (nfcCurrent->IsHaveProperty(ACCUMULATOR))
	{
		m_Accum = (nfcCurrent->dev_settings.Options & DEVOPT_ACCUMULATOR) != 0;
	}
	UpdateData(0);
	m_WeekToScale.SetWindowText(S_O::FormatUINT(nfcCurrent->dev_settings.WeekToScale));
}

bool CLGMainDlg::CommandDone(CNFC_Command* c)
{
	if (!c->nRet)return 1;
	switch (c->nCommand)
	{
	case GetInfDeviceByte:
	{
		if (c->dev == nfcCurrent)InfToControls();
		main->PostMessage(WM_NFC_DEVICE_DETECTION_STATUS, (WPARAM)c->dev);
		if (!nfcCurrent->IsHaveProperty(INTERFACES))
		{
			c->dev->interfaces.Standard = RT_LORA;
			c->dev->interfaces.Module = RM_LORA_SX1272;
			c->dev->interfaces.Protocol = RP_LORA_IGM_PACK;
			c->dev->interfaces.bLoaded = 1;
		}
		break;
	}
	case GetSettingsByte:
	{
		if (c->dev == nfcCurrent)
		{
			SettingsToControls();
			ShowControls();
		}
		break;
	}
	}
	return 1;
}

void CLGMainDlg::ShowControls()
{
	UINT ids[] = { IDC_STATIC_WEEKTOSCALE ,IDC_WEEKTOSCALE ,IDC_SPIN_WEEKTOSCALE ,IDC_SETWEEKTOSCALE };
	for (int i = 0; i < sizeof(ids) / sizeof(UINT); i++)
	{
		GetDlgItem(ids[i])->ShowWindow(nfcCurrent->bManufact && nfcCurrent->IsHaveProperty(WEEKTOSCALE));
	}
	UINT ids2[] = { IDC_SETSERIALNO ,IDC_STATIC_IDSENSOR,IDC_EDIT_IDSENSOR ,IDC_STATIC_IDCPU,IDC_EDIT_IDCPU,IDC_CHECK_BEEP ,IDC_CHECK_ALARM };
	for (int i = 0; i < sizeof(ids2) / sizeof(UINT); i++)
		GetDlgItem(ids2[i])->ShowWindow(nfcCurrent->bManufact);
	UINT ids3[] = { IDC_SHORTLOG ,IDC_MIPEXLOG};
	for (int i = 0; i < sizeof(ids3) / sizeof(UINT); i++)
	{
		GetDlgItem(ids3[i])->ShowWindow(nfcCurrent->bManufact && nfcCurrent->IsHaveProperty(SHORTLOG));
		GetDlgItem(ids3[i])->EnableWindow(nfcCurrent->dev_settings.bLoaded);
	}
	GetDlgItem(IDC_SETLOGTIMEOUT2)->EnableWindow(1);
	GetDlgItem(IDC_CHECK_SKIPSELFTEST)->EnableWindow(nfcCurrent->IsHaveProperty(SKIPSELFTEST));
	GetDlgItem(IDC_CHECK_SKIPSELFTESTTRANSPORT)->ShowWindow(nfcCurrent->bManufact && nfcCurrent->IsHaveProperty(SKIPSELFTTESTTRANSPORT));
	GetDlgItem(IDC_ACCUM)->ShowWindow(nfcCurrent->bManufact && nfcCurrent->IsHaveProperty(ACCUMULATOR));

}

void CLGMainDlg::OnLogSpin(NMHDR* pNMHDR, CEdit* edit)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	CString str;
	edit->GetWindowText(str);
	int n = atoi(str) - pNMUpDown->iDelta;
	if (n < 1)n = 1;
	if (n > 255)n = 255;
	edit->SetWindowText(S_O::FormatUINT(n));
}

void CLGMainDlg::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnLogSpin(pNMHDR, &m_LogTimeout);
	*pResult = 0;
}

void CLGMainDlg::OnTabSelected()
{
//		if(!nfc->dev_info.bLoaded)nfc->GetDeviceInfo();
	if (!nfcCurrent->dev_settings.bLoaded)
	{
		nfcCurrent->GetSettings();
	}
	InfToControls();
	SettingsToControls();
	ShowControls();
}

void CLGMainDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CLongGasBaseDlg::OnShowWindow(bShow, nStatus);
	if (bShow && IsWindowEnabled() && nfcCurrent)
	{
		if (!nfcCurrent->dev_settings.bLoaded)
		{
			GetDlgItem(IDC_SETLOGTIMEOUT2)->EnableWindow(0);
			GetDlgItem(IDC_CHECK_SKIPSELFTEST)->EnableWindow(0);
			GetDlgItem(IDC_ACCUM)->EnableWindow(0);
		}
	}
}

void CLGMainDlg::OnBnClickedSetlogtimeout2()
{
	CString str;
	m_LogAlarmTimeout.GetWindowText(str);
	nfcCurrent->dev_settings.LogAlarmTimeOut = atoi(str);
	nfcCurrent->SetSettings(1);
}


void CLGMainDlg::OnDeltaposSpin7(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnLogSpin(pNMHDR, &m_LogAlarmTimeout);
	*pResult = 0;
}


void CLGMainDlg::OnBnClickedCheckSkipselftest()
{
	UpdateData();
	if(m_bSkipSelfTest)nfcCurrent->dev_settings.SkipSelfTest |= SKIPSELFTEST_STANDART;
	else nfcCurrent->dev_settings.SkipSelfTest &= ~SKIPSELFTEST_STANDART;
	nfcCurrent->SetSettings(1);
}


void CLGMainDlg::OnBnClickedSetweektoscale()
{
	CString str;
	m_WeekToScale.GetWindowText(str);
	nfcCurrent->SetWeekToScale(atoi(str));
	nfcCurrent->GetSettings();
}

void CLGMainDlg::OnDeltaposSpinWeektoscale(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnLogSpin(pNMHDR, &m_WeekToScale);
	*pResult = 0;
}

void CLGMainDlg::SetLogType()
{
	if (m_ShortLog)nfcCurrent->dev_settings.Options |= DEVOPT_SHORTLOG;
	else nfcCurrent->dev_settings.Options &= ~DEVOPT_SHORTLOG;
	if (m_bMipexLog)nfcCurrent->dev_settings.Options |= DEVOPT_MIPEXLOG;
	else nfcCurrent->dev_settings.Options &= ~DEVOPT_MIPEXLOG;
	nfcCurrent->SetSettings(1);
}

void CLGMainDlg::OnBnClickedShortlog()
{
	if (!nfcCurrent->IsHaveProperty(SHORTLOG) || !nfcCurrent->dev_settings.bLoaded)return;
	UpdateData();
	if (m_ShortLog && m_bMipexLog)
	{
		m_bMipexLog = 0;
		UpdateData(0);
	}
	SetLogType();
}

void CLGMainDlg::OnBnClickedMipexlog()
{
	if (!nfcCurrent->IsHaveProperty(SHORTLOG) || !nfcCurrent->dev_settings.bLoaded)return;
	UpdateData();
	if (m_ShortLog && m_bMipexLog)
	{
		m_ShortLog = 0;
		UpdateData(0);
	}
	SetLogType();
}


void CLGMainDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 0)
	{
		CDateTime tm=CDateTime::GetCurrent();
		m_Date.SetTime(tm);
		m_Time.SetTime(tm);
	}
	CLongGasBaseDlg::OnTimer(nIDEvent);
}

void CLGMainDlg::OnBnClickedCheckSkipselftesttransport()
{
	UpdateData();
	if (m_bSkipSelfTestTransport)nfcCurrent->dev_settings.SkipSelfTest |= SKIPSELFTEST_TRANSPORT;
	else nfcCurrent->dev_settings.SkipSelfTest &= ~SKIPSELFTEST_TRANSPORT;
	nfcCurrent->SetSettings(1);
}


void CLGMainDlg::OnBnClickedAccum()
{
	UpdateData();
	if (m_Accum)nfcCurrent->dev_settings.Options |= DEVOPT_ACCUMULATOR;
	else nfcCurrent->dev_settings.Options &= ~DEVOPT_ACCUMULATOR;
	nfcCurrent->SetSettings(1);
}
