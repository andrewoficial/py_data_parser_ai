// SetHotkeyDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "SetHotkeyDlg.h"
#include "afxdialogex.h"

extern HOTKEY_ARRAY g_hotkeys;
// CSetHotkeyDlg dialog

IMPLEMENT_DYNAMIC(CSetHotkeyDlg, CDialogEx)

CSetHotkeyDlg::CSetHotkeyDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SETHOTKEYDLG, pParent)
{

}

CSetHotkeyDlg::~CSetHotkeyDlg()
{
}

void CSetHotkeyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HOTKEY1, m_Hotkey);
}


BEGIN_MESSAGE_MAP(CSetHotkeyDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSetHotkeyDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSetHotkeyDlg message handlers


void CSetHotkeyDlg::OnBnClickedOk()
{
	m_Hotkey.GetHotKey(hk.wVirtualCode, hk.wModifiers);
	if (!g_hotkeys.IsCanAdd(hk))
	{
		AfxMessageBox(S_O::LoadString(IDS_CANNOTADDHOTKEY));
		return;
	}
	if (!g_hotkeys.AddHotkey(hk))return;
	g_hotkeys.Save();
	CDialogEx::OnOK();
}


BOOL CSetHotkeyDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_Hotkey.SetRules(HKCOMB_SCA, HOTKEYF_CONTROL);
	if (hk.wVirtualCode != 0)
	{
		m_Hotkey.SetHotKey(hk.wVirtualCode, hk.wModifiers);
	}
	m_Hotkey.SetFocus();
	return 0;
}
