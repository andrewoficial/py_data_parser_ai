// CLGWifiDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "CLGWifiDlg.h"
#include "afxdialogex.h"
#include "NFC_Device.h"
#include <DialogOperations.h>
#include "GridCtrl.h"


// CLGWifiDlg dialog

IMPLEMENT_DYNAMIC(CLGWifiDlg, CDialogEx)

CLGWifiDlg::CLGWifiDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LG_WIFI, pParent)
	, m_Autoconnect(1)
	, m_DHCP(1)
	, m_LocalIP(_T(""))
	, m_LocalPort(0)
	, m_RemoteIP(_T(""))
	, m_RemotePort(0)
	, m_SecurityKey(_T(""))
	, m_SSID(_T(""))
	, m_DataPeriod(0)
	, m_bTCPServer(FALSE)
	, m_MAC(_T(""))
	, m_nTCPServerPort(0)
{
	nfc = 0;
}

CLGWifiDlg::~CLGWifiDlg()
{
}

void CLGWifiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_AUTOCONNECT, m_Autoconnect);
	DDX_Check(pDX, IDC_CHECK_DHCP, m_DHCP);
	DDX_Control(pDX, IDC_COMBO_CHANNEL, m_Channel);
	DDX_Control(pDX, IDC_COMBO_IPPROTOCOL, m_IPProtocol);
	DDX_Control(pDX, IDC_COMBO_POWER, m_Power);
	DDX_Control(pDX, IDC_COMBO_SECURITY, m_Security);
	DDX_Text(pDX, IDC_LOCALIP, m_LocalIP);
	DDX_Text(pDX, IDC_LOCALPORT, m_LocalPort);
	DDX_Text(pDX, IDC_REMOTEIP, m_RemoteIP);
	DDX_Text(pDX, IDC_REMOTEPORT, m_RemotePort);
	DDX_Text(pDX, IDC_SECURITYKEY, m_SecurityKey);
	DDX_Text(pDX, IDC_SSID, m_SSID);
	DDX_Text(pDX, IDC_DATAPERIOD, m_DataPeriod);
	DDX_Check(pDX, IDC_CHECK_TCPSERVER, m_bTCPServer);
	DDX_Text(pDX, IDC_MAC, m_MAC);
	DDX_Text(pDX, IDC_TCPSERVERPORT, m_nTCPServerPort);
}

BEGIN_MESSAGE_MAP(CLGWifiDlg, CDialogEx)
	ON_BN_CLICKED(IDC_SET, &CLGWifiDlg::OnBnClickedSet)
	ON_BN_CLICKED(IDC_GET, &CLGWifiDlg::OnBnClickedGet)
END_MESSAGE_MAP()

BOOL CLGWifiDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	D_O::AddStringWithId(&m_Channel, "Auto", 0);
	for (int i = 1; i < 14; i++)D_O::AddStringWithId(&m_Channel, S_O::FormatUINT(i), i);
	for(int i=100;i<=195;i+=5)D_O::AddStringWithId(&m_Power, S_O::FormatAllDigits((double)i/10,1), i);
	D_O::AddStringWithId(&m_Security, "Нет", 0,1);
	D_O::AddStringWithId(&m_IPProtocol, "UDP", 0,1);
	return TRUE;
}

bool CLGWifiDlg::CheckIP(CString str)
{
	S_O::Trim(str);
	if (str == "")
	{
		AfxMessageBox("IP address not set");
		return 0;
	}
	int n1, n2, n3, n4;
	if (sscanf(str, "%d.%d.%d.%d", &n1, &n2, &n3, &n4) != 4)
	{
		AfxMessageBox("IP address must be in xxx.xxx.xxx.xxx format");
		return 0;
	}
	return 1;
}

void CLGWifiDlg::OnBnClickedSet()
{
	UpdateData();	
	S_O::Trim(m_SSID);
	if (m_SSID == "")
	{
		AfxMessageBox("SSID not set");
		return;
	}
	if (!CheckIP(m_RemoteIP))return;
	nfc->wifi.RF.Channel=D_O::GetSelectedItemData(m_Channel);
	nfc->wifi.RF.OutputPower = D_O::GetSelectedItemData(m_Power);
	nfc->wifi.RF.nSendDataPeriod = m_DataPeriod;
	nfc->wifi.IP.LPort = m_LocalPort;
	nfc->wifi.IP.RPort = m_RemotePort;
	nfc->wifi.TCPServerLPort = m_nTCPServerPort;
	strncpy((char*)nfc->wifi.IP.Remote, (LPCSTR)m_RemoteIP, 15);
	strncpy((char*)nfc->wifi.AccPoint.SSID, (LPCSTR)m_SSID, 15);	
	nfc->wifi.AccPoint.Options = 0;
	if (!m_DHCP)nfc->wifi.AccPoint.Options |= WIFI_STATIC_IP;
	if (m_Autoconnect)nfc->wifi.AccPoint.Options |= WIFI_AUTO_CONNECT;
	if (m_bTCPServer)nfc->wifi.AccPoint.Options |= WIFI_TCP_SERVER;	
	if (nfc->IsHaveProperty(RSSI_PROPERTY))
	{
		CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_RSSI);
		grid->SaveModifyed();
	}
	nfc->SetWifiSettings();
}

void CLGWifiDlg::OnBnClickedGet()
{
	nfc->GetWifiSettings();
}

void CLGWifiDlg::UpdateControls()
{
	D_O::SelectIDInCombo(m_Channel, nfc->wifi.RF.Channel);
	D_O::SelectIDInCombo(m_Power, nfc->wifi.RF.OutputPower);
	m_DataPeriod = nfc->wifi.RF.nSendDataPeriod;
	m_SSID = nfc->wifi.AccPoint.SSID;
	m_SecurityKey = nfc->wifi.AccPoint.Password;
	m_LocalIP = nfc->wifi.IP.Local;
	m_LocalPort = nfc->wifi.IP.LPort;
	m_RemoteIP = nfc->wifi.IP.Remote;
	m_RemotePort = nfc->wifi.IP.RPort;	
	m_bTCPServer = (nfc->wifi.AccPoint.Options & WIFI_TCP_SERVER) != 0;
	m_MAC = nfc->wifi.LocalMAC;
	m_nTCPServerPort = nfc->wifi.TCPServerLPort;
	UpdateData(0);
	bool bRSSI = nfc->IsHaveProperty(RSSI_PROPERTY);
	GetDlgItem(IDC_GROUP_RSSI)->ShowWindow(bRSSI);
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_RSSI);
	grid->ShowWindow(bRSSI);
	if (bRSSI)
	{
		grid->nGridId = GRIDID_RSSI;
		grid->FillDataRSSI(nfc);
	}
}