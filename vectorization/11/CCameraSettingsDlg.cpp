// CCameraSettingsDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "CCameraSettingsDlg.h"
#include "afxdialogex.h"
#include "CLGDelayedCommandDlg.h"
#include <DialogOperations.h>

extern VEGA_PREFERENCES g_pref;
// CCameraSettingsDlg dialog

IMPLEMENT_DYNAMIC(CCameraSettingsDlg, CDialogEx)

CCameraSettingsDlg::CCameraSettingsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CAMERASETTINGS, pParent)
	, m_CameraAddTime(g_pref.dCameraAddTime)
	, m_CameraMaxTime(g_pref.dCameraMaxTime)
	, m_Interval(g_pref.dCameraInterval)
	, m_RoomTemp(g_pref.dRoomTemp)
{
	m_nSlaveId = g_pref.nCameraSlaveId;
	m_b10Chan = g_pref.b10ChanBKM;

}

CCameraSettingsDlg::~CCameraSettingsDlg()
{
}

void CCameraSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CAMERATYPE, m_CameraType);
	DDX_Control(pDX, IDC_CAMERAPORT, m_CameraPort);
	DDX_Control(pDX, IDC_GASPORT, m_GasPort);
	DDX_Text(pDX, IDC_SLAVEID, m_nSlaveId);
	DDX_Check(pDX, IDC_10CHAN, m_b10Chan);
	DDX_Text(pDX, IDC_CAMERAADDTIME, m_CameraAddTime);
	DDX_Text(pDX, IDC_CAMERAMAXTIME, m_CameraMaxTime);
	DDX_Text(pDX, IDC_CAMERAINTERVAL, m_Interval);
	DDX_Text(pDX, IDC_ROOMTEMP, m_RoomTemp);
}


BEGIN_MESSAGE_MAP(CCameraSettingsDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CCameraSettingsDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CCameraSettingsDlg message handlers


BOOL CCameraSettingsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	TRANSLATION::TranslateWindow(this);
	m_CameraPort.SetWindowText(g_pref.strCameraAddr);
	m_GasPort.SetWindowText(g_pref.strGasAddr);
	//D_O::AddStringWithId(&m_CameraType, "Temi2500", CAMERA_2500);
	D_O::AddStringWithId(&m_CameraType, "Testa", CAMERA_TESTA);
	D_O::AddStringWithId(&m_CameraType, "Binder", CAMERA_BINDER);
	D_O::AddStringWithId(&m_CameraType, "China Modbus", CAMERA_BINDERMODBUS);
	D_O::SelectIDInCombo(m_CameraType, g_pref.nCameraType);
	return TRUE;
}


void CCameraSettingsDlg::OnBnClickedOk()
{
	UpdateData();
	g_pref.nCameraSlaveId = m_nSlaveId;
	g_pref.strCameraAddr = D_O::GetWindowText(&m_CameraPort);
	g_pref.strGasAddr = D_O::GetWindowText(&m_GasPort);
	g_pref.nCameraType = D_O::GetSelectedItemData(m_CameraType);
	g_pref.b10ChanBKM = m_b10Chan;
	g_pref.dCameraAddTime = m_CameraAddTime;
	g_pref.dCameraMaxTime = m_CameraMaxTime;
	g_pref.dCameraInterval = m_Interval;
	g_pref.dRoomTemp = m_RoomTemp;
	AfxGetApp()->WriteProfileInt("DelayedCommandDlg", "SlaveId", g_pref.nCameraSlaveId);
	AfxGetApp()->WriteProfileString("DelayedCommandDlg", "CameraPort", g_pref.strCameraAddr);
	AfxGetApp()->WriteProfileString("DelayedCommandDlg", "GasPort", g_pref.strGasAddr);
	AfxGetApp()->WriteProfileInt("DelayedCommandDlg", "CameraType", g_pref.nCameraType);
	AfxGetApp()->WriteProfileInt("DelayedCommandDlg", "b10Chan", g_pref.b10ChanBKM);
	theApp.WriteProfileDouble("DelayedCommandDlg", "CameraAddTime", g_pref.dCameraAddTime);
	theApp.WriteProfileDouble("DelayedCommandDlg", "CameraMaxTime", g_pref.dCameraMaxTime);
	theApp.WriteProfileDouble("DelayedCommandDlg", "CameraInterval", g_pref.dCameraInterval);
	theApp.WriteProfileDouble("DelayedCommandDlg", "RoomTemp", g_pref.dRoomTemp);
	CDialogEx::OnOK();
}
