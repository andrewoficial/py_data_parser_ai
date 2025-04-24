// CUserDlg.cpp : implementation file
//

#include "pch.h"
#include "VegaConnect.h"
#include "CUserDlg.h"
#include "afxdialogex.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>

extern VEGA_PREFERENCES g_pref;
extern VEGA_CONNECTION g_conn;

// CUserDlg dialog

IMPLEMENT_DYNAMIC(CUserDlg, CDialogEx)

CUserDlg::CUserDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_USERDLG, pParent)
	, m_Name(_T(""))
{
	bChangeUser = 0;
}

CUserDlg::~CUserDlg()
{
}

void CUserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NAME, m_Name);
	DDX_Control(pDX, IDC_PWD, m_Pwd);
	DDX_Control(pDX, IDC_PWD2, m_Pwd2);
}


BEGIN_MESSAGE_MAP(CUserDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CUserDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CUserDlg message handlers


BOOL CUserDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	if (bChangeUser)
	{
		UpdateData(0);
		GetDlgItem(IDC_NAME)->EnableWindow(0);
		SetWindowText(S_O::LoadString(IDS_CHANGEUSERPASSWORD));
		GetDlgItem(IDC_PWD)->SetFocus();
	}
	else GetDlgItem(IDC_NAME)->SetFocus();
	return 0; 
}


void CUserDlg::OnBnClickedOk()
{
	UpdateData();
	if (m_Name == "")
	{
		AfxMessageBox(S_O::LoadString(IDS_NEEDUSERNAME));
		return;
	}
	CString strPwd, strPwd1;
	m_Pwd.GetWindowText(strPwd);
	m_Pwd2.GetWindowText(strPwd1);
	if (strPwd == "")
	{
		AfxMessageBox(S_O::LoadString(IDS_NEEDPASSWORD));
		return;
	}
	if (strPwd != strPwd1)
	{
		AfxMessageBox(S_O::LoadString(IDS_PASSWNOTMATCH));
		return;
	}
	JSON cmd;
	cmd.AddCommand("manage_users");
	cmd.AddOptions("user_list", 1);
	cmd.AddProperty("login", RM_STRINGOPERATIONS::ConvertToUnicode(m_Name));
	cmd.AddProperty("password", RM_STRINGOPERATIONS::ConvertToUnicode(strPwd),!bChangeUser);	
	if (!bChangeUser)
	{
		cmd.AddProperty("device_access", "FULL");
		cmd.AddOptions("command_list", 1, 0);
		CString str = "get_users,get_device_appdata,get_data,send_data,manage_device_appdata,delete_device_appdata,get_gateways,manage_gateways,delete_gateways,get_devices,manage_devices,delete_devices,get_coverage_map,get_device_downlink_queue,manage_device_downlink_queue,server_info";
		CStringArray a;
		RM_STRINGOPERATIONS::ParseString(str, a);
		for (int i = 0; i < a.GetSize(); i++)
		{
			cmd.AddProperty("", a[i], i != a.GetSize() - 1);
		}
		cmd.CloseArray();
	}
	cmd.CloseBracket(0);
	cmd.CloseArray();
	cmd.CloseBracket(1);
	CString s;
	try
	{
		if (g_conn.Query(cmd, s, this))
		{
			if (s.Find("add_user_list") == -1)
			{
				AfxMessageBox(S_O::LoadString(IDS_INCORRECTRESPONSE), MB_ICONERROR);
				return;
			}
			boost::property_tree::ptree root;
			std::istringstream is((LPCTSTR)s);
			boost::property_tree::read_json(is, root);
			namespace pt = boost::property_tree;
			for (pt::ptree::value_type& row : root.get_child("add_user_list"))
			{
				for (pt::ptree::value_type& cell : row.second)
				{
					if (cell.first == "status")
					{
						s = cell.second.get_value<std::string>().c_str();
						if (s != "true")
						{
							AfxMessageBox(S_O::LoadString(IDS_USERADDERROR) + s, MB_ICONERROR);
							return;
						}
						break;
					}
				}
			}
		}
	}
	catch (std::exception e)
	{
		s.Format("%s: %s", S_O::LoadString(IDS_ERROR), e.what());
		AfxMessageBox(s, MB_ICONERROR);
		return;
	}
	CDialogEx::OnOK();
}

void CUserDlg::StartWaitQuery(CString strInfo)
{
	EnableWindow(0);
}

void CUserDlg::StopWaitQuery()
{
	EnableWindow(1);
}