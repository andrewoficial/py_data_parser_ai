#pragma once

#include "CLongGasMainDlg.h"

// CLGWifiDlg dialog

class CLGWifiDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLGWifiDlg)

public:
	CLGWifiDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CLGWifiDlg();
	CNFC_Device* nfc;
	void UpdateControls();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LG_WIFI };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_Autoconnect;
	BOOL m_DHCP;
	CComboBox m_Channel;
	CComboBox m_IPProtocol;
	CComboBox m_Power;
	CComboBox m_Security;
	CString m_LocalIP;
	UINT m_LocalPort;
	CString m_RemoteIP;
	UINT m_RemotePort;
	CString m_SecurityKey;
	CString m_SSID;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedSet();
	afx_msg void OnBnClickedGet();
private:
	bool CheckIP(CString str);
public:
	UINT m_DataPeriod;
	BOOL m_bTCPServer;
	CString m_MAC;
	UINT m_nTCPServerPort;
};
