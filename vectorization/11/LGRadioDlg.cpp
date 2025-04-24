// LGRadioDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "LGRadioDlg.h"
#include "afxdialogex.h"
#include "NFC_Device.h"
#include <DialogOperations.h>
#include "CLGWifiDlg.h"


// CLGRadioDlg dialog

IMPLEMENT_DYNAMIC(CLGRadioDlg, CDialogEx)

CLGRadioDlg::CLGRadioDlg(CWnd* pParent /*=nullptr*/)
	: CLongGasBaseDlg(IDD_LG_RADIO, pParent)
	, m_En1(FALSE)
	, m_En2(FALSE)
	, m_En3(FALSE)
	, m_Addr(_T(""))
	, m_AKey(_T(""))
	, m_Freq1(_T(""))
	, m_Freq2(_T(""))
	, m_Freq3(_T(""))
	, m_Period(_T("60"))
	, m_NKey(_T(""))
	, m_LT1(_T(""))
	, m_LT2(_T(""))
	, m_LT3(_T(""))
	, m_LTAll(_T(""))
	, m_bABP(FALSE)
	, m_bOTAA(FALSE)
	, m_DevEUI(_T(""))
	, m_JoinDelay1(0)
	, m_JoinDelay2(0)
	, m_JoinEUI(_T(""))
	, m_JoinNum(0)
	, m_RootKey(_T(""))
	, m_RX1Delay(0)
	, m_RX2Delay(0)
	, m_FreqRX(_T(""))
{
	wifi = new CLGWifiDlg;
}

CLGRadioDlg::~CLGRadioDlg()
{
	delete wifi;
}

void CLGRadioDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_EN1, m_En1);
	DDX_Check(pDX, IDC_CHECK_EN2, m_En2);
	DDX_Check(pDX, IDC_CHECK_EN3, m_En3);
	DDX_Control(pDX, IDC_SF1, m_SF1);
	DDX_Control(pDX, IDC_SF2, m_SF2);
	DDX_Control(pDX, IDC_SF3, m_SF3);
	DDX_Control(pDX, IDC_BW, m_BW);
	DDX_Control(pDX, IDC_CR, m_CR);
	DDX_Text(pDX, IDC_ADDR, m_Addr);
	DDX_Text(pDX, IDC_AKEY, m_AKey);
	DDX_Text(pDX, IDC_FREQ1, m_Freq1);
	DDX_Text(pDX, IDC_FREQ2, m_Freq2);
	DDX_Text(pDX, IDC_FREQ3, m_Freq3);
	DDX_Text(pDX, IDC_DATEPERIOD, m_Period);
	DDX_Text(pDX, IDC_NKEY, m_NKey);
	DDX_Text(pDX, IDC_LIFETIME1, m_LT1);
	DDX_Text(pDX, IDC_LIFETIME2, m_LT2);
	DDX_Text(pDX, IDC_LIFETIME3, m_LT3);
	DDX_Text(pDX, IDC_LIFETIME_ALL, m_LTAll);
	DDX_Control(pDX, IDC_POWER1, m_Power1);
	DDX_Control(pDX, IDC_POWER2, m_Power2);
	DDX_Control(pDX, IDC_POWER3, m_Power3);
	DDX_Check(pDX, IDC_CHECK_ABP, m_bABP);
	DDX_Check(pDX, IDC_CHECK_OTAA, m_bOTAA);
	DDX_Text(pDX, IDC_DEVEUI, m_DevEUI);
	DDX_Text(pDX, IDC_JOINDELAY1, m_JoinDelay1);
	DDX_Text(pDX, IDC_JOINDELAY2, m_JoinDelay2);
	DDX_Text(pDX, IDC_JOINEUI, m_JoinEUI);
	DDX_Text(pDX, IDC_JOINNUM, m_JoinNum);
	DDX_Text(pDX, IDC_ROOTKEY, m_RootKey);
	DDX_Text(pDX, IDC_RX1DELAY, m_RX1Delay);
	DDX_Text(pDX, IDC_RX2DELAY, m_RX2Delay);
	DDX_Control(pDX, IDC_WINDOW, m_WindowCtrl);
	DDX_Text(pDX, IDC_FREQ_RX, m_FreqRX);
	DDX_Control(pDX, IDC_SF_RX, m_SFRX);
	DDX_Control(pDX, IDC_LORASIGNALSTATUS, m_SignalStatus);
	DDX_Control(pDX, IDC_COMBO_MODULE, m_Module);
	DDX_Control(pDX, IDC_COMBO_PROTOCOL, m_Protocol);
	DDX_Control(pDX, IDC_COMBO_STANDARD, m_Standard);
	DDX_Control(pDX, IDC_REGION, m_Region);
}

BEGIN_MESSAGE_MAP(CLGRadioDlg, CLongGasBaseDlg)
	ON_BN_CLICKED(IDC_GET_RF, &CLGRadioDlg::OnBnClickedGetRf)
	ON_BN_CLICKED(IDC_GET_KEYS, &CLGRadioDlg::OnBnClickedGetKeys)
	ON_BN_CLICKED(IDC_SET_KEYS, &CLGRadioDlg::OnBnClickedSetKeys)
	ON_BN_CLICKED(IDC_SET_RF, &CLGRadioDlg::OnBnClickedSetRf)
	ON_BN_CLICKED(IDC_SET_RFDEF, &CLGRadioDlg::OnBnClickedSetRfdef)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &CLGRadioDlg::OnDeltaposSpin1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2, &CLGRadioDlg::OnDeltaposSpin2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN3, &CLGRadioDlg::OnDeltaposSpin3)
	ON_CBN_SELCHANGE(IDC_SF1, &CLGRadioDlg::OnCbnSelchangeSf1)
	ON_CBN_SELCHANGE(IDC_SF2, &CLGRadioDlg::OnCbnSelchangeSf1)
	ON_CBN_SELCHANGE(IDC_SF3, &CLGRadioDlg::OnCbnSelchangeSf1)
	ON_EN_CHANGE(IDC_POWER1, &CLGRadioDlg::OnEnChangePower1)
	ON_EN_CHANGE(IDC_POWER2, &CLGRadioDlg::OnEnChangePower1)
	ON_EN_CHANGE(IDC_POWER3, &CLGRadioDlg::OnEnChangePower1)
	ON_EN_CHANGE(IDC_DATEPERIOD, &CLGRadioDlg::OnEnChangePower1)
	ON_BN_CLICKED(IDC_SENDPACKET, &CLGRadioDlg::OnBnClickedSendpacket)
	ON_BN_CLICKED(IDC_CHECK_EN1, &CLGRadioDlg::OnBnClickedCheckEn1)
	ON_BN_CLICKED(IDC_CHECK_EN2, &CLGRadioDlg::OnBnClickedCheckEn2)
	ON_BN_CLICKED(IDC_CHECK_EN3, &CLGRadioDlg::OnBnClickedCheckEn3)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_GET_OTAA, &CLGRadioDlg::OnBnClickedGetOtaa)
	ON_BN_CLICKED(IDC_SET_OTAA, &CLGRadioDlg::OnBnClickedSetOtaa)
	ON_BN_CLICKED(IDC_REJOIN, &CLGRadioDlg::OnBnClickedRejoin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_RX1SPIN, &CLGRadioDlg::OnDeltaposRx1spin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_RX2SPIN, &CLGRadioDlg::OnDeltaposRx2spin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_JOINDELAYSPIN1, &CLGRadioDlg::OnDeltaposJoindelayspin1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_JOINDELAYSPIN2, &CLGRadioDlg::OnDeltaposJoindelayspin2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_JOINNUMSPIN, &CLGRadioDlg::OnDeltaposJoinnumspin)
	ON_BN_CLICKED(IDC_CHECK_ABP, &CLGRadioDlg::OnBnClickedCheckAbp)
	ON_BN_CLICKED(IDC_CHECK_OTAA, &CLGRadioDlg::OnBnClickedCheckOtaa)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_RX, &CLGRadioDlg::OnDeltaposSpinRx)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_WOCONFIRM, &CLGRadioDlg::OnBnClickedconfirm)
	ON_BN_CLICKED(IDC_WITHCONFIRM, &CLGRadioDlg::OnBnClickedconfirm)
	ON_CBN_SELCHANGE(IDC_POWER1, &CLGRadioDlg::OnCbnSelchangePower1)
	ON_CBN_SELCHANGE(IDC_POWER2, &CLGRadioDlg::OnCbnSelchangePower1)
	ON_CBN_SELCHANGE(IDC_POWER3, &CLGRadioDlg::OnCbnSelchangePower1)
	ON_BN_CLICKED(IDC_RADIO_WAN, &CLGRadioDlg::OnBnClickedRadioWan)
	ON_BN_CLICKED(IDC_RADIO_P2P, &CLGRadioDlg::OnBnClickedRadioWan)
	ON_CBN_SELCHANGE(IDC_COMBO_STANDARD, &CLGRadioDlg::OnCbnSelchangeComboStandard)
	ON_CBN_SELCHANGE(IDC_COMBO_MODULE, &CLGRadioDlg::OnCbnSelchangeComboModule)
	ON_BN_CLICKED(IDC_SET_INTERFACE, &CLGRadioDlg::OnBnClickedSetInterface)
	ON_BN_CLICKED(IDC_GET_INTERFACE, &CLGRadioDlg::OnBnClickedGetInterface)
	ON_CBN_SELCHANGE(IDC_REGION, &CLGRadioDlg::OnCbnSelchangeRegion)
END_MESSAGE_MAP()

BOOL CLGRadioDlg::OnInitDialog()
{
	CLongGasBaseDlg::OnInitDialog();
	D_O::AddStringWithId(&m_Standard, "None", RT_NONE);
	D_O::AddStringWithId(&m_Standard, "Lora", RT_LORA);
	D_O::AddStringWithId(&m_Standard, "Wifi", RT_WIFI);
	D_O::AddStringWithId(&m_Standard, "BLE", RT_BLE);
	D_O::AddStringWithId(&m_Standard, "ZigBee", RT_ZIGBEE);
	D_O::AddStringWithId(&m_WindowCtrl, "Disable", 0);
	D_O::AddStringWithId(&m_WindowCtrl, "RX1", 1);
	D_O::AddStringWithId(&m_WindowCtrl, "RX2", 2);
	D_O::AddStringWithId(&m_WindowCtrl, "RX1 & RX2", 3);
	for (int i = 12; i >= 7; i--)
	{
		D_O::AddStringWithId(&m_SF1, S_O::FormatUINT(i), i);
		D_O::AddStringWithId(&m_SF2, S_O::FormatUINT(i), i);
		D_O::AddStringWithId(&m_SF3, S_O::FormatUINT(i), i);
		D_O::AddStringWithId(&m_SFRX, S_O::FormatUINT(i), i);
	}
	CString str;
	for (int i = 0; i < 8; i++)
	{
		str.Format("%ddbm", (7 - i + 1) * 2);
		if (!i)str += " (Max)";
		D_O::AddStringWithId(&m_Power1, str, i);
		D_O::AddStringWithId(&m_Power2, str, i);
		D_O::AddStringWithId(&m_Power3, str, i);
	}
	D_O::AddStringWithId(&m_BW,"125", 0);
	D_O::AddStringWithId(&m_BW, "250", 1);
	D_O::AddStringWithId(&m_BW, "500", 2);
	D_O::AddStringWithId(&m_CR, "4/5", 0);
	D_O::AddStringWithId(&m_CR, "4/6", 1);
	D_O::AddStringWithId(&m_CR, "4/7", 2);
	D_O::AddStringWithId(&m_CR, "4/8", 3);
	D_O::AddStringWithId(&m_Region, "EU868", 0, 1);
	D_O::AddStringWithId(&m_Region, "RU864", 1);
	D_O::AddStringWithId(&m_Region, "US915", 4);
	D_O::AddStringWithId(&m_Region, "AU915", 5);
	D_O::AddStringWithId(&m_Region, "KR920", 6);
	D_O::AddStringWithId(&m_Region, "AS923", 7);
	OnBnClickedSetRfdef();
	CheckRadioButton(IDC_WOCONFIRM, IDC_WITHCONFIRM, IDC_WOCONFIRM);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CLGRadioDlg::SelectInterface()
{
	UINT nExcl[] = { IDC_STATIC_STANDARD ,IDC_COMBO_STANDARD ,IDC_STATIC_MODULE ,IDC_COMBO_MODULE , IDC_STATIC_PROTOCOL ,
		IDC_COMBO_PROTOCOL ,IDC_SET_INTERFACE,IDC_GET_INTERFACE };
	CWnd* wnd = GetWindow(GW_CHILD);
	bool bShow = nfcCurrent->interfaces.Standard == RT_LORA;
	while (wnd)
	{
		UINT nId = wnd->GetDlgCtrlID();
		bool bf = 0;
		for (int i = 0; i < sizeof(nExcl) / sizeof(UINT); i++)
		{
			if (nId == nExcl[i])
			{
				bf = 1;
				break;
			}
		}
		if(!bf)wnd->ShowWindow(bShow);
		wnd = wnd->GetNextWindow();
	}
	if (nfcCurrent->interfaces.Standard == RT_WIFI)
	{
		wifi->nfc = nfcCurrent;
		if (!IsWindow(wifi->GetSafeHwnd()))
		{
			wifi->Create(IDD_LG_WIFI, this);
			CRect rt;
			GetDlgItem(IDC_STATIC_RADIOCHAN)->GetWindowRect(rt);
			ScreenToClient(rt);
			wifi->SetWindowPos(0, rt.left, rt.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}
	}
	if (IsWindow(wifi->GetSafeHwnd()))wifi->ShowWindow(nfcCurrent->interfaces.Standard == RT_WIFI);
	EnableInterfaces();
	D_O::SelectIDInCombo(m_Standard, nfcCurrent->interfaces.Standard);
	OnCbnSelchangeComboStandard();
	D_O::SelectIDInCombo(m_Module, nfcCurrent->interfaces.Module);
	OnCbnSelchangeComboModule();
	D_O::SelectIDInCombo(m_Protocol, nfcCurrent->interfaces.Protocol);
}

void CLGRadioDlg::InfoToControls()
{
	bool bEn = nfcCurrent->IsHaveProperty(LORAOTAA) && nfcCurrent->interfaces.Standard == RT_LORA;
	UINT nOTAA[] = { IDC_STATIC_RXSETTINGS ,IDC_STATIC_WINDOW ,IDC_WINDOW,IDC_STATIC_RX1DELAY,IDC_RX1DELAY,IDC_RX1SPIN,IDC_STATIC_RX2DELAY,IDC_RX2DELAY,IDC_RX2SPIN,
				IDC_STATIC_OTAA ,IDC_STATIC_DEVEUI ,IDC_DEVEUI,IDC_STATIC_JOINEUI,IDC_JOINEUI,IDC_STATIC_ROOTKEY,IDC_ROOTKEY,
				IDC_STATIC_JOINDELAY1,IDC_JOINDELAY1,IDC_JOINDELAYSPIN1,IDC_STATIC_JOINDELAY2,IDC_JOINDELAY2,IDC_JOINDELAYSPIN2,
				IDC_STATIC_JOINNUM ,IDC_JOINNUM,IDC_JOINNUMSPIN,IDC_SET_OTAA,IDC_GET_OTAA,
				IDC_STATIC_ACTIVATION ,IDC_STATIC_LORASTATUS ,IDC_LORASTATUS,IDC_CHECK_ABP,IDC_CHECK_OTAA,IDC_REJOIN,
				IDC_RXWINDOW_GROUP,IDC_SF_RX,IDC_STATIC_RXFREQ ,IDC_FREQ_RX ,IDC_STATIC_SFRX ,IDC_SPIN_RX };
	for (int i = 0; i < sizeof(nOTAA) / sizeof(int); i++)
	{
		GetDlgItem(nOTAA[i])->ShowWindow(bEn);
	}
	RAK811ToControls();
	bEn = nfcCurrent->IsHaveProperty(LORAP2P);
	GetDlgItem(IDC_RADIO_WAN)->EnableWindow(bEn);
	GetDlgItem(IDC_RADIO_P2P)->EnableWindow(bEn);
	if (!bEn)CheckRadioButton(IDC_RADIO_WAN, IDC_RADIO_P2P, IDC_RADIO_WAN);
	SelectInterface();
}

void CLGRadioDlg::RadioToControls()
{
	if (nfcCurrent->interfaces.Standard == RT_LORA)
	{
		UpdateData();
		LORA_SETTINGS* s = &nfcCurrent->lora_settings;
		D_O::SelectIDInCombo(m_BW, s->btBW);
		D_O::SelectIDInCombo(m_CR, s->btCR);
		m_Addr.Format("%08x", s->nAddress);
		m_Period = S_O::FormatUINT(s->nDataPeriod);
		CComboBox* sf[] = { &m_SF1,&m_SF2,&m_SF3 };
		CString* f[] = { &m_Freq1,&m_Freq2,&m_Freq3 };
		CComboBox* p[] = { &m_Power1,&m_Power2,&m_Power3 };
		LORA_FREQ* lf[] = { &s->f1,&s->f2,&s->f3 };
		BOOL* en[] = { &m_En1,&m_En2,&m_En3 };
		for (int i = 0; i < 3; i++)
		{
			*en[i] = lf[i]->bEn;
			*f[i] = S_O::FormatUINT(lf[i]->nFreq);
			D_O::SelectIDInCombo(*p[i], lf[i]->btPower);
			D_O::SelectIDInCombo(*sf[i], lf[i]->nSF);
		}
		m_FreqRX = S_O::FormatUINT(nfcCurrent->lora_settings.rx.RX2Frequency);
		if (nfcCurrent->IsHaveProperty(LORAOTAA))
		{
			m_RX1Delay = nfcCurrent->lora_settings.rx.RX1Delay;
			m_RX2Delay = nfcCurrent->lora_settings.rx.RX2Delay;
			D_O::SelectIDInCombo(m_WindowCtrl, nfcCurrent->lora_settings.rx.Window);
			D_O::SelectIDInCombo(m_SFRX, nfcCurrent->lora_settings.rx.RX2SpreadingFactor);
			nfcCurrent->GetLoraOTAASettings();
		}
		UpdateData(0);
		UpdateLifeTimes();
	}
	else if (nfcCurrent->interfaces.Standard == RT_WIFI)wifi->UpdateControls();
}

void CLGRadioDlg::OTAAToControls2()
{
	bool bP2P = nfcCurrent->IsHaveProperty(LORAP2P) && nfcCurrent->interfaces.Standard == RT_LORA && (nfcCurrent->lora_settings.otaa.Options & LORA_P2P_ENABLED) != 0;
	CheckRadioButton(IDC_RADIO_WAN, IDC_RADIO_P2P, bP2P ? IDC_RADIO_P2P : IDC_RADIO_WAN);
	UINT ids[] = { IDC_STATIC_CH2,IDC_CHECK_EN2 ,IDC_STATIC_FREQ2, IDC_FREQ2 ,IDC_SPIN2,IDC_STATIC_SF2,IDC_SF2,IDC_STATIC_POWER2,IDC_POWER2,IDC_STATIC_LT2,IDC_LIFETIME2,
					IDC_STATIC_CH3,IDC_CHECK_EN3 ,IDC_STATIC_FREQ3, IDC_FREQ3 ,IDC_SPIN3,IDC_STATIC_SF3, IDC_SF3,IDC_STATIC_POWER3,IDC_POWER3,IDC_STATIC_LT3,IDC_LIFETIME3,
					IDC_RXWINDOW_GROUP ,IDC_STATIC_RXFREQ,IDC_FREQ_RX,IDC_STATIC_SFRX ,IDC_SPIN_RX,IDC_SF_RX,
					IDC_STATIC_RXSETTINGS,IDC_STATIC_WINDOW,IDC_WINDOW,
					IDC_STATIC_RX1DELAY ,IDC_RX1DELAY ,IDC_RX1SPIN ,
					IDC_STATIC_RX2DELAY ,IDC_RX2DELAY ,IDC_RX2SPIN,
					IDC_STATIC_OTAA ,IDC_STATIC_DEVEUI ,IDC_DEVEUI,IDC_STATIC_JOINEUI,IDC_JOINEUI,IDC_STATIC_ROOTKEY,IDC_ROOTKEY,
				IDC_STATIC_JOINDELAY1,IDC_JOINDELAY1,IDC_JOINDELAYSPIN1,IDC_STATIC_JOINDELAY2,IDC_JOINDELAY2,IDC_JOINDELAYSPIN2,
				IDC_STATIC_JOINNUM ,IDC_JOINNUM,IDC_JOINNUMSPIN,IDC_SET_OTAA,IDC_GET_OTAA,
				IDC_STATIC_ABP ,IDC_STATIC_NWSKEY ,IDC_NKEY ,IDC_STATIC_APPSKEY ,IDC_AKEY ,IDC_SET_KEYS ,IDC_GET_KEYS,
				IDC_CHECK_ABP,IDC_CHECK_OTAA };
	bool bShowAll = nfcCurrent->interfaces.Standard == RT_LORA && !bP2P;
	for (int i = 0; i < sizeof(ids) / sizeof(int); i++)GetDlgItem(ids[i])->ShowWindow(bShowAll);
	if (bShowAll && nfcCurrent->interfaces.Module == RM_LORA_RAK3172)
	{
		for (int i = 0; i < sizeof(ids) / sizeof(int); i++)
		{
			if (ids[i] == IDC_RXWINDOW_GROUP)break;
			GetDlgItem(ids[i])->ShowWindow(0);
		}
	}
	UINT ids2[] = { IDC_CHECK_EN1,IDC_BW ,IDC_CR };
	for (int i = 0; i < sizeof(ids2) / sizeof(int); i++)GetDlgItem(ids2[i])->EnableWindow(!bP2P);
	if (bP2P)GetDlgItem(IDC_ADDR)->EnableWindow(1);
}

void CLGRadioDlg::OTAAToControls()
{
	UpdateData();
	CString str;
	m_DevEUI = "";
	m_JoinEUI = "";
	m_RootKey = "";
	if (nfcCurrent->lora_settings.bOTAALoaded)
	{
		for (int i = 0; i < 8; i++)
		{
			str.Format("%02x", (UINT)nfcCurrent->lora_settings.otaa.DevEUI[i]);
			m_DevEUI += str;
		}
		for (int i = 0; i < 8; i++)
		{
			str.Format("%02x", (UINT)nfcCurrent->lora_settings.otaa.JoinEUI[i]);
			m_JoinEUI += str;
		}
		for (int i = 0; i < 16; i++)
		{
			str.Format("%02x", (UINT)nfcCurrent->lora_settings.otaa.Appkey[i]);
			m_RootKey += str;
		}
	}
	m_JoinDelay1 = nfcCurrent->lora_settings.otaa.Join.AcceptDelay1;
	m_JoinDelay2 = nfcCurrent->lora_settings.otaa.Join.AcceptDelay2;
	m_JoinNum = nfcCurrent->lora_settings.otaa.Join.NumberOfJoins;
	UpdateJoinStatus();
	m_bABP = (nfcCurrent->lora_settings.otaa.Options & OTAA_ENABLED) == 0;
	m_bOTAA = (nfcCurrent->lora_settings.otaa.Options & OTAA_ENABLED) != 0;
	bool bConfirm = nfcCurrent->IsHaveProperty(LORACONFIRM) && (nfcCurrent->lora_settings.otaa.Options & LORA_CONFIRM) != 0;
	CheckRadioButton(IDC_WOCONFIRM, IDC_WITHCONFIRM, bConfirm ? IDC_WITHCONFIRM : IDC_WOCONFIRM);
	UpdateData(0);
	UINT nId[] = { IDC_ADDR ,IDC_NKEY ,IDC_AKEY,IDC_SET_KEYS };
	for (int i = 0; i < sizeof(nId) / sizeof(int); i++)GetDlgItem(nId[i])->EnableWindow(m_bABP);
	UINT nOTAA[] = { IDC_DEVEUI,IDC_JOINEUI,IDC_ROOTKEY,IDC_JOINDELAY1,IDC_JOINDELAYSPIN1,IDC_JOINDELAY2,IDC_JOINDELAYSPIN2,IDC_JOINNUM,IDC_JOINNUMSPIN,IDC_SET_OTAA };
	for (int i = 0; i < sizeof(nOTAA) / sizeof(int); i++)GetDlgItem(nOTAA[i])->EnableWindow(m_bOTAA);
	UINT nConfirm[] = { IDC_WOCONFIRM,IDC_WITHCONFIRM };
	bool bP2P = nfcCurrent->IsHaveProperty(LORAP2P) && (nfcCurrent->lora_settings.otaa.Options & LORA_P2P_ENABLED) != 0;
	for (int i = 0; i < sizeof(nConfirm) / sizeof(int); i++)GetDlgItem(nConfirm[i])->EnableWindow(nfcCurrent->lora_settings.bOTAALoaded && nfcCurrent->IsHaveProperty(LORACONFIRM) && !bP2P);
	if (bConfirm && (nfcCurrent->lora_settings.otaa.Joined || !(nfcCurrent->lora_settings.otaa.Options & OTAA_ENABLED)))
	{
		SetTimer(1, nfcCurrent->lora_settings.rx.RX2Delay, 0);
	}
	else
	{
		m_SignalStatus.SetWindowText("");
		KillTimer(1);
	}
	RAK811ToControls();
	OTAAToControls2();
	if(nfcCurrent->interfaces.Standard == RT_LORA && nfcCurrent->interfaces.Module == RM_LORA_RAK3172)D_O::SelectIDInCombo(m_Region, nfcCurrent->lora_settings.otaa.Options >> 4);
}

void CLGRadioDlg::LoraKeyToControls()
{
	UpdateData();
	m_AKey = nfcCurrent->lora_settings.strAppKey;
	m_NKey = nfcCurrent->lora_settings.strNetworkKey;
	UpdateData(0);
}

void CLGRadioDlg::LoraInfoControls()
{
	CString str;
	LORA_Info* l = &nfcCurrent->loraInfo;
	str.Format("%s\r\nRSSI=%d\r\nSNR=%d\r\nCountUp=%u\r\nCountDown=%u", CDateTime::GetCurrent().FormatStandard(1, 0), (int)l->Rssi, (int)l->Snr, l->FCntUp, l->FCntDown);
	m_SignalStatus.SetWindowText(str);
	SetTimer(1, (nfcCurrent->lora_settings.nDataPeriod + 1) * 1000, 0);
}

bool CLGRadioDlg::CommandDone(CNFC_Command* c)
{
	if (!c->nRet)return 1;
	switch (c->nCommand)
	{
	case GetInterfacesByte:
	{
		if (c->dev->interfaces.Standard == RT_LORA)c->dev->GetLoraSettings();
		else if (c->dev->interfaces.Standard == RT_WIFI)c->dev->GetWifiSettings();
		if(c->dev==nfcCurrent)SelectInterface();
		break;
	}
	case OTAARejoin:
	{
		if (c->dev != nfcCurrent)break;
		if (nfcCurrent->lora_settings.otaa.Options & OTAA_ENABLED)
		{
			SetTimer(0, nfcCurrent->lora_settings.otaa.Join.AcceptDelay2 + 1000, 0);
		}
		break;
	}
	case GetInfDeviceByte:
	{
		if (c->dev == nfcCurrent)InfoToControls();
		break;
	}
	case GetLoraKeyByte:
	{
		if (c->dev == nfcCurrent)LoraKeyToControls();
		break;
	}
	case GetRadioSettingsByte:
	{
		if (c->dev == nfcCurrent)RadioToControls();
		break;
	}
	case GetLoraOTAA:
	{
		if (c->dev == nfcCurrent)OTAAToControls();
		break;
	}
	case GetLORAInfo:
	{
		if (c->dev == nfcCurrent) LoraInfoControls();
		break;
	}
	}
	return 1;
}

void CLGRadioDlg::OnBnClickedGetRf()
{
	nfcCurrent->GetLoraSettings();
}

void CLGRadioDlg::OnBnClickedGetKeys()
{
	nfcCurrent->GetLoraKey();
}

void CLGRadioDlg::OnBnClickedSetKeys()
{
	UpdateData();
	if (m_AKey.GetLength() != 32 || m_NKey.GetLength() != 32)
	{
		AfxMessageBox(IDS_WRONG_KEYLENGTH);
		return;
	}
	strcpy_s(nfcCurrent->lora_settings.strAppKey, m_AKey);
	strcpy_s(nfcCurrent->lora_settings.strNetworkKey, m_NKey);
	nfcCurrent->SetLoraKey();
}

void CLGRadioDlg::OnBnClickedSetRf()
{
	UpdateData();
	LORA_SETTINGS* s = &nfcCurrent->lora_settings;
	s->btBW=D_O::GetSelectedItemData(m_BW);
	s->btCR=D_O::GetSelectedItemData(m_CR);
	s->nAddress = strtol(m_Addr, 0, 16);
	s->nDataPeriod = atoi(m_Period);
	if (s->nDataPeriod <= 0)s->nDataPeriod = 1;
	CComboBox* sf[] = { &m_SF1,&m_SF2,&m_SF3 };
	CString* f[] = { &m_Freq1,&m_Freq2,&m_Freq3 };
	CComboBox* p[] = { &m_Power1,&m_Power2,&m_Power3 };
	LORA_FREQ* lf[] = { &s->f1,&s->f2,&s->f3 };
	BOOL* en[] = { &m_En1,&m_En2,&m_En3 };
	for (int i = 0; i < 3; i++)
	{
		lf[i]->bEn = *en[i];
		if (lf[i]->bEn)lf[i]->nFreq = atoi(*f[i]);
		else lf[i]->nFreq = 0;
		if (lf[i]->nFreq == 0)lf[i]->bEn = 0;
		lf[i]->btPower = D_O::GetSelectedItemData(*p[i]);
//		if (lf[i]->btPower == 0)lf[i]->btPower = 1;
		lf[i]->nSF=D_O::GetSelectedItemData(*sf[i]);
	}
	s->rx.RX1Delay= m_RX1Delay;
	s->rx.RX2Delay= m_RX2Delay;
	s->rx.Window=D_O::GetSelectedItemData(m_WindowCtrl);
	s->rx.RX2Frequency = atoi(m_FreqRX);
	s->rx.RX2SpreadingFactor = D_O::GetSelectedItemData(m_SFRX);
	s->otaa.Options &= 0xf;
	if (nfcCurrent->interfaces.Module == RM_LORA_RAK3172)
	{
		s->otaa.Options |= (D_O::GetSelectedItemData(m_Region) << 4);
	}
	nfcCurrent->SetLoraSettings();
}

void CLGRadioDlg::OnBnClickedSetRfdef()
{
	UpdateData();
	D_O::SelectIDInCombo(m_Power1, 6);
	D_O::SelectIDInCombo(m_Power2, 4);
	D_O::SelectIDInCombo(m_Power3, 2);
	m_Freq1 = "867700000";
	m_Freq2 = "868500000";
	m_Freq3 = "868300000";
	UpdateData(0);
	D_O::SelectIDInCombo(m_SF1, 9);
	D_O::SelectIDInCombo(m_SF2, 11);
	D_O::SelectIDInCombo(m_SF3, 11);
	D_O::SelectIDInCombo(m_BW, 0);
	D_O::SelectIDInCombo(m_CR, 0);
}

double CLGRadioDlg::CalcLifeTime(int sf, int pwr, int per)
{
	double d = (per * (4000 / (0.093 + (0.000536592 * exp(0.65689463 * sf) /
		(60 - 0.000536592 * exp(0.65689463 * sf)) * (145.29 * pwr * pwr - 719.22 *
			pwr + 32870)) / 1000)) / 24 / 365) / 60;
	return d;
}

void CLGRadioDlg::UpdateLifeTimes()
{
	UpdateData();
	double dAll = 0,d;
	int nc = 0;
	CComboBox* sf[] = { &m_SF1,&m_SF2,&m_SF3 };
	CString* f[] = { &m_Freq1,&m_Freq2,&m_Freq3 };
	CString* l[] = { &m_LT1,&m_LT2,&m_LT3 };
	CComboBox* p[] = { &m_Power1,&m_Power2,&m_Power3 };
	BOOL* en[] = { &m_En1,&m_En2,&m_En3 };
	int per = atoi(m_Period);
	for (int i = 0; i < 3; i++)
	{
		*l[i] = "0";
		if (per<=0 || !*en[i] || atoi(*f[i]) == 0)continue;
		d = CalcLifeTime(D_O::GetSelectedItemData(*sf[i]),D_O::GetSelectedItemData(*p[i]), per);
		if (d < 0)continue;
		l[i]->Format("%.2f years (%d days)", d, (int)(d * 365));
		nc++;
		dAll += d;
	}
	if (nc)
	{
		m_LTAll.Format("%.2f years", dAll / nc);
	}
	else m_LTAll = "0";
	UpdateData(0);
}

#define MIN_FREQ 863000000
#define MAX_FREQ 927500000

void CLGRadioDlg::OnSpinFreq(CString* freq, LPNMUPDOWN pNMUpDown)
{
	UpdateData();
	if (atoi(*freq)<=0)*freq = "868500000";
	else
	{
		double df = S_O::ToDouble(*freq);
		df -= pNMUpDown->iDelta * 100000;
		if (df < MIN_FREQ)return;
		if(df> MAX_FREQ)return;
		*freq = S_O::FormatUINT((UINT)df);
	}
	UpdateData(0);
}

void CLGRadioDlg::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	OnSpinFreq(&m_Freq1, pNMUpDown);
	*pResult = 0;
}

void CLGRadioDlg::OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	OnSpinFreq(&m_Freq2, pNMUpDown);
	*pResult = 0;
}

void CLGRadioDlg::OnDeltaposSpin3(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	OnSpinFreq(&m_Freq3, pNMUpDown);
	*pResult = 0;
}

void CLGRadioDlg::OnCbnSelchangeSf1()
{
	UpdateLifeTimes();
}

void CLGRadioDlg::OnEnChangePower1()
{
	UpdateLifeTimes();
}

void CLGRadioDlg::OnBnClickedSendpacket()
{
	nfcCurrent->SendLoraPacket();
}

void CLGRadioDlg::OnBnClickedCheckEn1()
{
	UpdateLifeTimes();
}

void CLGRadioDlg::OnBnClickedCheckEn2()
{
	UpdateLifeTimes();
}

void CLGRadioDlg::OnBnClickedCheckEn3()
{
	UpdateLifeTimes();
}

void CLGRadioDlg::RAK811ToControls()
{
	bool bEn = nfcCurrent->bManufact || !nfcCurrent->IsHaveProperty(RAK811);
	UINT ids[] = { IDC_CHECK_EN1 ,IDC_FREQ1 ,IDC_SPIN1 ,IDC_CHECK_EN2 ,IDC_FREQ2 ,IDC_SPIN2,
					IDC_CHECK_EN3 ,IDC_FREQ3 ,IDC_SPIN3};
	for (int i = 0; i < sizeof(ids) / sizeof(UINT*); i++)
		GetDlgItem(ids[i])->EnableWindow(bEn);
	bEn = bEn && nfcCurrent->IsHaveProperty(LORAOTAA) && m_bOTAA;
	UINT ids1[] = { IDC_FREQ_RX,IDC_SPIN_RX,IDC_SF_RX,
					IDC_WINDOW ,IDC_RX1DELAY ,IDC_RX1SPIN ,IDC_RX2DELAY ,IDC_RX2SPIN ,
					IDC_JOINDELAY1 ,IDC_JOINDELAYSPIN1,IDC_JOINDELAY2 ,IDC_JOINDELAYSPIN2,
					IDC_JOINNUM ,IDC_JOINNUMSPIN };
	for (int i = 0; i < sizeof(ids1) / sizeof(UINT*); i++)
		GetDlgItem(ids1[i])->EnableWindow(bEn);
	CString str = S_O::LoadString(IDS_RADIOCHANSETTINGS);
	if (nfcCurrent->IsHaveProperty(RAK811))str += " (EU863-870)";
	GetDlgItem(IDC_STATIC_RADIOCHAN)->SetWindowText(str);
}

void CLGRadioDlg::EnableInterfaces()
{
	UINT nExcl[] = { IDC_STATIC_STANDARD ,IDC_COMBO_STANDARD ,IDC_STATIC_MODULE ,IDC_COMBO_MODULE , IDC_STATIC_PROTOCOL ,
		IDC_COMBO_PROTOCOL ,IDC_SET_INTERFACE,IDC_GET_INTERFACE };
	for (int i = 0; i < sizeof(nExcl) / sizeof(UINT); i++)
	{
		GetDlgItem(nExcl[i])->EnableWindow(nfcCurrent->bManufact && nfcCurrent->IsHaveProperty(INTERFACES));
	}
}

void CLGRadioDlg::OnTabSelected()
{
	if (nfcCurrent->IsHaveProperty(INTERFACES) && !nfcCurrent->interfaces.bLoaded)nfcCurrent->GetInterfaces();
	if (nfcCurrent->interfaces.bLoaded)
	{
		if (nfcCurrent->interfaces.Standard == RT_LORA && !nfcCurrent->lora_settings.bLoaded)nfcCurrent->GetLoraSettings();
		if (nfcCurrent->interfaces.Standard == RT_WIFI && !nfcCurrent->wifi.bLoaded)nfcCurrent->GetWifiSettings();
	}
	RAK811ToControls();
	EnableInterfaces();
	InfoToControls();
	RadioToControls();
	OTAAToControls();
	LoraKeyToControls();
	LoraInfoControls();
}

void CLGRadioDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CLongGasBaseDlg::OnShowWindow(bShow, nStatus);
	if (bShow && IsWindowEnabled() && nfcCurrent)
	{
		UINT nConfirm[] = { IDC_WOCONFIRM,IDC_WITHCONFIRM };
		bool bConfirm = nfcCurrent->interfaces.Standard==RT_LORA && nfcCurrent->lora_settings.bOTAALoaded && nfcCurrent->IsHaveProperty(LORACONFIRM);
		for (int i = 0; i < sizeof(nConfirm) / sizeof(int); i++)GetDlgItem(nConfirm[i])->EnableWindow(bConfirm);
		if (bConfirm && nfcCurrent->lora_settings.bOTAALoaded && (nfcCurrent->lora_settings.otaa.Joined || !(nfcCurrent->lora_settings.otaa.Options & OTAA_ENABLED)))
		{
			SetTimer(1, (nfcCurrent->lora_settings.nDataPeriod + 1) * 1000, 0);
		}
	}
	/*if (bShow)
	{
		SelectInterface();
		wifi->UpdateControls();
	}*/
}

void CLGRadioDlg::OnBnClickedGetOtaa()
{
	nfcCurrent->GetLoraOTAASettings();
}

void CLGRadioDlg::OnBnClickedSetOtaa()
{
	UpdateData();
	if (m_DevEUI.GetLength() != 16 || m_JoinEUI.GetLength() != 16 || m_RootKey.GetLength() != 32)
	{
		AfxMessageBox(IDS_WRONG_KEYLENGTH);
		return;
	}
	nfcCurrent->lora_settings.otaa.Join.AcceptDelay1= m_JoinDelay1;
	nfcCurrent->lora_settings.otaa.Join.AcceptDelay2= m_JoinDelay2;
	nfcCurrent->lora_settings.otaa.Join.NumberOfJoins= m_JoinNum;
	bool bConfirm = GetCheckedRadioButton(IDC_WOCONFIRM, IDC_WITHCONFIRM) == IDC_WITHCONFIRM;
	if (!nfcCurrent->IsHaveProperty(LORACONFIRM) || !bConfirm)nfcCurrent->lora_settings.otaa.Options &= ~LORA_CONFIRM;
	else nfcCurrent->lora_settings.otaa.Options |= LORA_CONFIRM;
	int n = 0;
	for (int i = 0; i < 16; i += 2)
	{
		nfcCurrent->lora_settings.otaa.DevEUI[n++] = (BYTE)strtol(m_DevEUI.Mid(i, 2), 0, 16);
	}
	n = 0;
	for (int i = 0; i < 16; i += 2)
	{
		nfcCurrent->lora_settings.otaa.JoinEUI[n++] = (BYTE)strtol(m_JoinEUI.Mid(i, 2), 0, 16);
	}
	n = 0;
	for (int i = 0; i < 32; i += 2)
	{
		nfcCurrent->lora_settings.otaa.Appkey[n++] = (BYTE)strtol(m_RootKey.Mid(i, 2), 0, 16);
	}
	nfcCurrent->SetLoraOTAASettings();
}

void CLGRadioDlg::OnBnClickedRejoin()
{
	nfcCurrent->LoraOTAARejoin();
}

void CLGRadioDlg::OnDeltaposRx1spin(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	OnSpinDelay(&m_RX1Delay, pNMUpDown);
	*pResult = 0;
}

void CLGRadioDlg::OnDeltaposRx2spin(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	OnSpinDelay(&m_RX2Delay, pNMUpDown);
	*pResult = 0;
}

void CLGRadioDlg::OnDeltaposJoindelayspin1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	OnSpinDelay(&m_JoinDelay1, pNMUpDown);
	*pResult = 0;
}

void CLGRadioDlg::OnDeltaposJoindelayspin2(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	OnSpinDelay(&m_JoinDelay2, pNMUpDown);
	*pResult = 0;
}

void CLGRadioDlg::OnDeltaposJoinnumspin(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	UpdateData();
	UINT nP = m_JoinNum - pNMUpDown->iDelta;
	if (nP == 0 || nP > 1000)return;
	m_JoinNum = nP;
	UpdateData(0);
	*pResult = 0;
}

void CLGRadioDlg::OnSpinDelay(int* delay, LPNMUPDOWN pNMUpDown)
{
	UpdateData();
	int nP = *delay - pNMUpDown->iDelta*100;
	if (nP < 0 || nP > 65000)return;
	*delay = nP;
	UpdateData(0);
}

void CLGRadioDlg::OnBnClickedCheckAbp()
{
	UpdateData();
	m_bOTAA = !m_bABP;
	UpdateData(0);
	if(m_bOTAA)nfcCurrent->lora_settings.otaa.Options|= OTAA_ENABLED;
	else nfcCurrent->lora_settings.otaa.Options &= ~OTAA_ENABLED;
	nfcCurrent->SetLoraOTAASettings();
}

void CLGRadioDlg::OnBnClickedCheckOtaa()
{
	UpdateData();
	m_bABP = !m_bOTAA;
	UpdateData(0);
	if (m_bOTAA)nfcCurrent->lora_settings.otaa.Options |= OTAA_ENABLED;
	else nfcCurrent->lora_settings.otaa.Options &= ~OTAA_ENABLED;
	nfcCurrent->SetLoraOTAASettings();
}


void CLGRadioDlg::OnDeltaposSpinRx(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	OnSpinFreq(&m_FreqRX, pNMUpDown);
	*pResult = 0;
}


void CLGRadioDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 0)
	{
		nfcCurrent->GetLoraOTAASettings();
		KillTimer(0);
	}
	else if (nIDEvent == 1)
	{
		nfcCurrent->GetLoraInfo();
		KillTimer(1);
	}
	CLongGasBaseDlg::OnTimer(nIDEvent);
}


void CLGRadioDlg::UpdateJoinStatus()
{
	CString str;
	if (nfcCurrent->lora_settings.otaa.Joined || !(nfcCurrent->lora_settings.otaa.Options & OTAA_ENABLED))str = "Joined";
	else str = "Not joined";
	GetDlgItem(IDC_LORASTATUS)->SetWindowText(str);
}

void CLGRadioDlg::OnBnClickedconfirm()
{
	bool bConfirm = GetCheckedRadioButton(IDC_WOCONFIRM, IDC_WITHCONFIRM) == IDC_WITHCONFIRM;
	if (!nfcCurrent->IsHaveProperty(LORACONFIRM) || !bConfirm)nfcCurrent->lora_settings.otaa.Options &= ~LORA_CONFIRM;
	else nfcCurrent->lora_settings.otaa.Options |= LORA_CONFIRM;
	nfcCurrent->SetLoraOTAASettings();
}

void CLGRadioDlg::OnCbnSelchangePower1()
{
	UpdateLifeTimes();
}

void CLGRadioDlg::OnBnClickedRadioWan()
{
	UINT nMode = GetCheckedRadioButton(IDC_RADIO_WAN, IDC_RADIO_P2P) == IDC_RADIO_P2P;
	if (nMode)nfcCurrent->lora_settings.otaa.Options |= LORA_P2P_ENABLED;
	else nfcCurrent->lora_settings.otaa.Options &= ~LORA_P2P_ENABLED;
	nfcCurrent->SetLoraOTAASettings();
	nfcCurrent->GetLoraInfo();
}


void CLGRadioDlg::OnCbnSelchangeComboStandard()
{
	UINT nId = D_O::GetSelectedItemData(m_Standard);
	m_Module.ResetContent();
	char* lora[] = { "Internal","SX1272","RAK811","RAK3172"};
	char* wifi[] = { "Internal","ESP32WROOM","ESP8266"};
	char* ble[] = { "Internal","ESP32WROOM","CH592F"};
	char* zigbee[] = { "Internal","ETRX357" };
	char** sel = 0;
	int nC = 0;
	UINT nDef = 0;
	switch (nId)
	{
	case RT_LORA:
		sel = lora;
		nC = sizeof(lora) / sizeof(char*);
		nDef = RM_LORA_SX1272;
		break;
	case RT_WIFI:
		sel = wifi;
		nC = sizeof(wifi) / sizeof(char*);
		nDef = RM_WIFI_ESP32WROOM;
		break;
	case RT_BLE:
		sel = ble;
		nC = sizeof(ble) / sizeof(char*);
		nDef = RM_BLE_ESP32WROOM;
		break;
	case RT_ZIGBEE:
		sel = zigbee;
		nC = sizeof(zigbee) / sizeof(char*);
		nDef = RM_ZB_ETRX357;
		break;
	}
	if (nC)
	{
		for (int i = 0; i < nC; i++)
		{
			D_O::AddStringWithId(&m_Module, sel[i], i, i == nDef);
		}
	}
	OnCbnSelchangeComboModule();
}


void CLGRadioDlg::OnCbnSelchangeComboModule()
{
	UINT nId = D_O::GetSelectedItemData(m_Standard);
	UINT nModule = D_O::GetSelectedItemData(m_Module);
	m_Protocol.ResetContent();
	char* lora[] = { "IGM_PACK","IGM_ASCII"};
	char* wifi[] = { "IGM_PACK","IGM_ASCII","BEACON_SYSTEM"};
	char* ble[] = { "IGM_PACK","IGM_ASCII" };
	char* zigbee[] = { "IGM_PACK","IGM_ASCII","MODBUS_SPKKPU"};
	char** sel = 0;
	int nC = 0;
	UINT nDef = 0;
	switch (nId)
	{
	case RT_LORA:
	{
		sel = lora;
		nC = sizeof(lora) / sizeof(char*);
		nDef = RP_LORA_IGM_PACK;
		bool bShowRegion = nfcCurrent->interfaces.Standard == RT_LORA && nfcCurrent->interfaces.Module == RM_LORA_RAK3172;
		GetDlgItem(IDC_FREQ1)->ShowWindow(!bShowRegion);
		GetDlgItem(IDC_SPIN1)->ShowWindow(!bShowRegion);
		GetDlgItem(IDC_CHECK_EN1)->ShowWindow(!bShowRegion);
		m_Region.ShowWindow(bShowRegion);
		OTAAToControls2();
		break;
	}
	case RT_WIFI:
		sel = wifi;
		nC = sizeof(wifi) / sizeof(char*);
		nDef = RP_WIFI_BEACON_SYSTEM;
		break;
	case RT_BLE:
		sel = ble;
		nC = sizeof(ble) / sizeof(char*);
		nDef = RP_BLE_IGM_PACK;
		break;
	case RT_ZIGBEE:
		sel = zigbee;
		nC = sizeof(zigbee) / sizeof(char*);
		nDef = RP_ZB_MODBUS_SPKKPU;
		break;
	}
	if (nC)
	{
		for (int i = 0; i < nC; i++)
		{
			D_O::AddStringWithId(&m_Protocol, sel[i], i, i == nDef);
		}
	}
}


void CLGRadioDlg::OnBnClickedSetInterface()
{
	nfcCurrent->interfacesToSet.Standard= D_O::GetSelectedItemData(m_Standard);
	nfcCurrent->interfacesToSet.Module = D_O::GetSelectedItemData(m_Module);
	nfcCurrent->interfacesToSet.Protocol = D_O::GetSelectedItemData(m_Protocol);
	nfcCurrent->SetInterfaces();
}


void CLGRadioDlg::OnBnClickedGetInterface()
{
	nfcCurrent->GetInterfaces();
}


void CLGRadioDlg::OnCbnSelchangeRegion()
{
}
