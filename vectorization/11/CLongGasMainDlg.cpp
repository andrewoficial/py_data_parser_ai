// CLongGasMainDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "CLongGasMainDlg.h"
#include "GridCtrl.h"
#include "resource.h"
#include "LGMainDlg.h"
#include <DialogOperations.h>
#include <structwithdata.h>
#include "NFC_Device.h"
#include "LGSensorsDlg.h"
#include "LGAlarmsDlg.h"
#include "LGSettingsDlg.h"
#include "LGRadioDlg.h"
#include "LGWorkStatDlg.h"
#include "LGMipexDlg.h"
#include "LGGraphDlg.h"
#include "LGLogDlg.h"
#include "LGFirmwareDlg.h"
#include "CLGDelayedCommandDlg.h"
#include "CLGMultipleDlg.h"
#include "GroupInfoDlg.h"
#include "CLGSaveDlg.h"
#include "ProgressCtrlX.h"
#include "SetHotkeyDlg.h"
#include <Shobjidl.h>

// CLongGasMainDlg dialog

CLongGasMainDlg* mainPtr = 0;
extern TRANSLATION g_Translation;
extern TRANSLATION g_EngTranslation;
extern VEGA_PREFERENCES g_pref;
extern CString g_szCurrentDir;

HOTKEY_ARRAY g_hotkeys;

void LG_WND_ARRAY::Clear()
{
	if (dev)delete dev;

};

IMPLEMENT_DYNAMIC(CLongGasMainDlg, CDialog)

#define QUALITY_REFRSESH_TIMER 1
#define DELAYED_MIPEX_TIMER 2
#define MARK_ERROR_TIMER_START 100

CLongGasMainDlg::CLongGasMainDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_LONGGASDLG, pParent)
{
	api = 0;
	mainPtr = this;
	bInStartAll = bWasStartAll= bWasError= bWasTerminated= 0;
	group = 0;
	keybdhook = 0;
	bManufact = AfxGetApp()->GetProfileInt("System", "Manufacturer", 0)==1;
	nLastTabSel = -2;
	CMyComboBox::CreateFont(fRich, 14);
	m_Tab.EnableDraw(BTC_ALL);
	m_Tab.EnableCustomLook(1, ETC_FLAT | ETC_SELECTION);
	m_hIcon = AfxGetApp()->LoadIcon(IDI_MAIN);
	itl = 0;
	nLastDeviceNum = 0;
}

CLongGasMainDlg::~CLongGasMainDlg()
{
	if(keybdhook)UnhookWindowsHookEx(keybdhook);
	for (int i = 0; i < dlgs.GetSize(); i++)
	{
		LG_WND_ARRAY* a = &dlgs[i];
		a->Clear();
	}
	for (int k = 0; k < arrWnd.GetSize(); k++)
		delete arrWnd[k];
	for (int i = 0; i < logs.GetSize(); i++)delete logs[i];
	for (int i = 0; i < progress.GetSize(); i++)delete progress[i];
	for (int i = 0; i < statics.GetSize(); i++)
	{
		if (statics[i])delete statics[i];
	}
	if (api)delete api;
	if (group)delete group;
	if(itl)itl->Release();
}

void CLongGasMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_Tab);
	DDX_Control(pDX, IDC_COMBO_LANG, m_Lang);
	DDX_Control(pDX, IDC_QUALITY, m_Quality);
}


BEGIN_MESSAGE_MAP(CLongGasMainDlg, CDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CLongGasMainDlg::OnTcnSelchangeTab1)
	ON_MESSAGE(WM_NOTIFYRBUTTONUP, OnGridRButtonUp)
	ON_MESSAGE(WM_CELLSELECTED, OnCellSelected)
	ON_MESSAGE(WM_NFC_COMMAND_DONE,OnNFCCommandDone)
	ON_MESSAGE(WM_NFC_DEVICE_DETECTION_STATUS, OnNFCDeviceDetectionStatus)
	ON_MESSAGE(WM_NFC_DEVICE_LOG, OnNFCLog)
	ON_MESSAGE(WM_NFC_DEVICE_ADDED, OnNFCDeviceAdded)
	ON_MESSAGE(WM_NFC_OPERATION_DONE, OnNFCOperationDone)
	ON_MESSAGE(WM_NFC_MARKERROR, OnNFCMarkError)
	ON_MESSAGE(WM_SWITCHTOMANUFACT, OnWmSwitchToManufact)
	ON_MESSAGE(WM_SELECTNEXTTAB, OnWmSelectNextTab)
	ON_MESSAGE(WM_SELECTNEXTDEVICE, OnWmSelectNextDevice)
	ON_MESSAGE(WM_HOTKEYCLICKED, OnWmHotkey)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_COMBO_LANG, &CLongGasMainDlg::OnCbnSelchangeComboLang)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_SYSCOMMAND()
	ON_WM_CTLCOLOR()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_GETMINMAXINFO()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CLongGasMainDlg message handlers

enum
{
	LG_MAIN=0,
	LG_SENSORS,
	LG_ALARMS,
	LG_SETTINGS,
	LG_LOG,
	LG_GRAPH,
	LG_FIRMWARE,
	LG_MIPEX,
	LG_RADIO,
	LG_WORKSTAT,
	LG_MULTIPLE,
	LG_SAVE,
	LG_DELAYED
};

#define BAR_CLR RGB(78,162,60)

void CLongGasMainDlg::CreateWindows(LG_WND_ARRAY& a, HidDeviceDescr* descr, bool bVirtual)
{
	CLayoutInfo infoLG, infoLog, infoProg;
	infoLG.SetReferenceRect(rtInitial);
	infoLog.SetReferenceRect(rtInitialLog);
	infoProg.SetReferenceRect(rtInitialProg);
	infoProg.AddOption(CLayoutInfo::OT_MIN_HEIGHT, 3);
	infoProg.AddOption(CLayoutInfo::OT_MAX_HEIGHT, 3);
	CRect rt, rtLog,rtProg;
	GetDlgItem(IDC_STATIC_CHILD)->GetWindowRect(rt);
	ScreenToClient(rt);
	GetDlgItem(IDC_STATIC_LOG)->GetWindowRect(rtLog);
	ScreenToClient(rtLog);
	GetDlgItem(IDC_PROGRESS1)->GetWindowRect(rtProg);
	ScreenToClient(rtProg);
	rtProg.bottom = rtProg.top + 3;
	CProgressCtrlX* p = new CProgressCtrlX;
	p->Create(WS_CHILD, rtProg, this, 0);
	p->SendMessage(PBM_SETBARCOLOR, 0, BAR_CLR);
	layout.AddControl(p, infoProg);
	a.dev = new CNFC_Device(descr, this, p);
	a.dev->bManufact = bManufact;
	a.dev->nDeviceNum = nLastDeviceNum++;
	if (bVirtual)a.dev->dev_info.nSerialNo = -1;
	if (!arrWnd.GetSize())
	{
		CLGMainDlg* main = new CLGMainDlg;
		arrWnd.Add(main);
		CLGSensorsDlg* sens = new CLGSensorsDlg;
		arrWnd.Add(sens);
		CLGAlarmsDlg* al = new CLGAlarmsDlg;
		arrWnd.Add(al);
		CLGSettingsDlg* s = new CLGSettingsDlg;
		arrWnd.Add(s);
		CLGLogDlg* l = new CLGLogDlg;
		arrWnd.Add(l);
		CLGGraphDlg* g = new CLGGraphDlg;
		arrWnd.Add(g);
		CLGFirmwareDlg* f = new CLGFirmwareDlg;
		arrWnd.Add(f);
		CLGMipexDlg* m = new CLGMipexDlg;
		arrWnd.Add(m);
		CLGRadioDlg* lora = new CLGRadioDlg;
		arrWnd.Add(lora);
		CLGWorkStatDlg* ws = new CLGWorkStatDlg;
		arrWnd.Add(ws);
		CLGMultipleDlg* mul = new CLGMultipleDlg;
		arrWnd.Add(mul);
		CLGSaveDlg* save = new CLGSaveDlg;
		arrWnd.Add(save);
		CLGDelayedCommandDlg* delayed = new CLGDelayedCommandDlg;
		arrWnd.Add(delayed);
		for (int k = 0; k < arrWnd.GetSize(); k++)
		{
			CLongGasBaseDlg* b = arrWnd[k];
			b->nTabId = k;
			b->main = this;
			b->nfcCurrent = a.dev;
			b->Create(b->GetId(), this);
			layout.AddControl(b, infoLG);
			b->SetWindowPos(&wndTop, rt.left, rt.top + 1, rt.Width(), rt.Height() - 1, 0);
		}
	}
	CRichEditChildDlg* r = new CRichEditChildDlg;
	r->Create(IDD_RICHEDIT, this);
	layout.AddControl(r,infoLog);
	r->SetWindowPos(0, rtLog.left, rtLog.top, rtLog.Width(), rtLog.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);
	r->m_Rich.SetFont(&fRich);
	logs.Add(r);
	progress.Add(p);
	CStatic* st = new CStatic;
	st->Create("",WS_CHILD|SS_SUNKEN|SS_LEFT, rt, this);
	layout.AddControl(st, infoLG);
	st->SetWindowPos(0, rt.left, rt.top + 1, rt.Width(), rt.Height() - 1, SWP_NOZORDER|SWP_SHOWWINDOW);
	st->SetFont(&fRich);
	D_O::SetWindowTextW(st, S_O::LoadStringW(IDS_PUSHBUTTONTOSTART));
	statics.Add(st);
	if (!bVirtual)a.dev->StartDeviceDetection();
	else a.dev->nDeviceStatus = NFC_STATUS_DEVICEINSTALLED;
}

#define ID_SWITCHTOMANUFACT 20000

BOOL CLongGasMainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	TRANSLATION::TranslateWindow(this);
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);
	D_O::GetControlRect(IDC_STATIC_CHILD,this,rtInitial);
	D_O::GetControlRect(IDC_STATIC_LOG, this, rtInitialLog);
	D_O::GetControlRect(IDC_PROGRESS1, this, rtInitialProg);
	layout.AttachBasic(this);
	layout.AddFixedHeight(IDC_TAB1);
	CRect rtMax;
	GetClientRect(rtMax);
	scroll.AttachWnd(this);
	scroll.SetDisplaySize(rtMax.Width(), rtMax.Height());
	D_O::RestoreWndSize(this, "LongGasMain");
	TCHAR	szLangCode[128];
	for (int i = 0; i < g_pref.avLang.GetSize(); i++)
	{
		::GetLocaleInfo(g_pref.avLang[i], LOCALE_SENGLANGUAGE, szLangCode, 128);
		D_O::AddStringWithId(&m_Lang, szLangCode, g_pref.avLang[i], g_pref.avLang[i]==g_pref.nLang);
	}
	g_hotkeys.Load();
	keybdhook = SetWindowsHookEx(WH_GETMESSAGE, KeyboardProc, 0, GetCurrentThreadId());
	group = new CGroupInfoDlg;
	group->Create(IDD_GROUPINFO, this);
	group->CenterWindow();
	api = new HidApi(0, 1155, 53456);
	api->registerDeviceAddCallback(&deviceAddedCb);
	api->registerDeviceRemoveCallback(&deviceRemovedCb);
	HidDeviceList devList = api->scanDevices();
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_DEVICES);
	grid->bChangeDataAllowed = 0;
	grid->columnnames.Add("N");
	grid->columnnames.Add(S_O::LoadString(IDS_DEVICE));
	CString str;
	for (UINT i = 0; i < devList.size(); i++)
	{
		str.Format("%u", i + 1);
		grid->names.Add(str);
		grid->names.Add(S_O::LoadString(IDS_UNKNOWNREADER));
		LG_WND_ARRAY a;
		CreateWindows(a, &devList[i]);
		dlgs.Add(a);
	}
	grid->FillData();
	str.Format("LongGas %s", CMainAppTemplate::GetAppVersionString(1));
	SetWindowText(str);
	theApp.FillSystemMenu(this);
	m_Quality.SetBkColor(0xffffff);
	m_Quality.SetTextFormat("%d%%", PBS_SHOW_POSITION);
	m_Quality.SetRange(0, 100);
	SetTimer(QUALITY_REFRSESH_TIMER, 1000, 0);

//	grid->SetActiveRow(0);
//	m_Tab.SetCurSel(0);
	return TRUE;
}

void CLongGasMainDlg::hidApiErrorCb(HidError err)
{

}

LRESULT CLongGasMainDlg::OnNFCDeviceAdded(WPARAM wp, LPARAM lp)
{
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_DEVICES);
	int nRow = 0;
	bool bSelect = 0;
	if (lp == 1)
	{
		HidDeviceDescr* dev = (HidDeviceDescr*)wp;
		CString str;
		str.Format("%d", grid->names.GetSize()/grid->columnnames.GetSize()+1);
		grid->names.Add(str);
		grid->names.Add(S_O::LoadString(IDS_UNKNOWNREADER));
		LG_WND_ARRAY a;
		CreateWindows(a, dev);
		dlgs.Add(a);
		delete dev;
	}
	else
	{
		std::string* path = (std::string*)wp;
		bool bf = 0;
		for (int i = 0; i < dlgs.GetSize(); i++)
		{
			LG_WND_ARRAY* a = &dlgs[i];
			if (a->dev->GetHidDevice()->descr.path == *path)
			{
				bf = 1;
				if (a->dev->IsDeviceInstalled())
				{
					a->dev->BreakExecution();
					a->dev->WaitComplete(1);
				}
//				for (int k = 0; k < a->arr.GetSize(); k++)layout.RemoveControl(a->arr[k]);
				a->Clear();
				layout.RemoveControl(logs[i]);
				delete logs[i];
				logs.RemoveAt(i);
				layout.RemoveControl(progress[i]);
				delete progress[i];
				progress.RemoveAt(i);
				layout.RemoveControl(statics[i]);
				delete statics[i];
				statics.RemoveAt(i);
				dlgs.RemoveAt(i);
				grid->names.RemoveAt(i * 2);
				grid->names.RemoveAt(i * 2);
				if (i == grid->GetActiveRow())
				{
					if (i)nRow = i - 1;
					else nRow = 0;
					bSelect = 1;
				}
				break;
			}
		}
		if (!dlgs.GetSize())nLastDeviceNum = 0;
		delete path;
	}
	for (int i = 0; i < dlgs.GetSize(); i++)
	{
		grid->names[i * grid->columnnames.GetSize()] = S_O::FormatUINT(i + 1);
	}
	grid->FillData();
	if (bSelect)
	{
		grid->SetActiveCell(1,nRow);
		OnCellSelected((WPARAM)grid, 0);
	}
	return 1;
}

void CLongGasMainDlg::deviceAddedCb(HidDeviceDescr& descr)
{
	if (!IsWindow(mainPtr->GetSafeHwnd()))return;
	HidDeviceDescr* d = new HidDeviceDescr;
	*d = descr;
	mainPtr->PostMessage(WM_NFC_DEVICE_ADDED, (WPARAM)d,1);

}
void CLongGasMainDlg::deviceRemovedCb(std::string& path)
{
	if (!IsWindow(mainPtr->GetSafeHwnd()))return;
	std::string* p = new std::string;
	*p = path;
	mainPtr->PostMessage(WM_NFC_DEVICE_ADDED, (WPARAM)p, 0);
}

void CLongGasMainDlg::SelectTab(int nTab,bool bSetFocus)
{
	CGridCtrl * grid=(CGridCtrl*)GetDlgItem(IDC_GRID_DEVICES);
	int nR = grid->GetActiveRow();
	if (nR < 0 || nR >= dlgs.GetSize())return;
//	GetDlgItem(IDC_STATIC_CHILD)->ShowWindow(nTab != -1);
	for (int i = 0; i < dlgs.GetSize(); i++)
	{
		LG_WND_ARRAY* a = &dlgs[i];
		if (i == nR)
		{
			for (int k = 0; k < arrWnd.GetSize(); k++)
			{
				CLongGasBaseDlg* base = arrWnd[k];
				base->nfcCurrent = a->dev;
				bool bShow = (nTab == -1) ? 0 : (k == nTab);
				if (k == nTab && a->dev && a->dev->IsDeviceInstalled())
				{
					base->PostMessage(WM_TABSELECTED);
				}
				base->ShowWindow(bShow);
				if (bSetFocus && bShow)base->SetFocus();
			}
		}
		logs[i]->ShowWindow((nTab == -1) ? 0 : (i == nR));
		progress[i]->ShowWindow((nTab == -1) ? 0 : (i == nR));
		statics[i]->ShowWindow(nTab == -1);
	}
}

void CLongGasMainDlg::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	SelectTab(D_O::GetTabData(m_Tab,m_Tab.GetCurSel()));
	*pResult = 0;
}

LRESULT CLongGasMainDlg::OnCellSelected(WPARAM wp, LPARAM lp)
{
	CGridCtrl* from = (CGridCtrl*)wp;
	if (from == (CGridCtrl*)GetDlgItem(IDC_GRID_DEVICES))
	{
		int nR = from->GetActiveRow();
		if (nR < 0 || nR >= dlgs.GetSize())return 1;
		LG_WND_ARRAY* a = &dlgs[nR];
		if (a->dev->IsDeviceInstalled())
		{
			UINT nS = D_O::GetTabData(m_Tab,m_Tab.GetCurSel());
			if (nLastTabSel != -2)
			{
				nS = nLastTabSel;
				nLastTabSel = -2;
			}
			if (!m_Tab.GetItemCount())
			{
				for (int i = 0; i < arrWnd.GetSize(); i++)
				{
					if (!bManufact)
					{
						if (i != LG_MAIN && i != LG_ALARMS && i != LG_LOG && i != LG_FIRMWARE && i != LG_RADIO && i!= LG_SAVE)continue;
						if (i == LG_RADIO && a->dev->interfaces.bLoaded && a->dev->interfaces.Standard == RT_LORA && !a->dev->IsHasLora())
						{
							if(!a->dev->lora_settings.bLoaded)a->dev->GetLoraSettings();
							continue;
						}
					}
					DIALOG_OPERATIONS::AddTabW(m_Tab, i, arrWnd[i]->GetName(), i);
				}
			}
			bool bF = 0;
			for (int i = 0; i < m_Tab.GetItemCount(); i++)
			{
				if (D_O::GetTabData(m_Tab, i) == nS)
				{
					bF = 1;
					m_Tab.SetCurSel(i);
					SelectTab(nS);
					break;
				}
			}
			if (!bF)
			{
				m_Tab.SetCurSel(0);
				SelectTab(0);
			}
		}
		else
		{
			m_Tab.DeleteAllItems();
			SelectTab(-1);
		}
		UpdateQualityBar();
	}
	return 1;
}

LRESULT CLongGasMainDlg::OnGridRButtonUp(WPARAM wp, LPARAM lp)
{
	CGridCtrl* grid = (CGridCtrl*)wp;
	if (grid == (CGridCtrl*)GetDlgItem(IDC_GRID_DEVICES))
	{
		CMenu m;
		m.CreatePopupMenu();
		m.AppendMenuA(MF_STRING, 1, "Add Virtual Device");
		CPoint pt;
		GetCursorPos(&pt);
		int res = m.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this, 0);
		if (!res)return 1;
		HidDeviceDescr* dev = new HidDeviceDescr;
		grid->names.Add(S_O::Format("%d", grid->names.GetSize() / grid->columnnames.GetSize() + 1));
		grid->names.Add("Virtual");
		LG_WND_ARRAY a;
		CreateWindows(a, dev, 1);
		dlgs.Add(a);
		for (int i = 0; i < dlgs.GetSize(); i++)
		{
			grid->names[i * grid->columnnames.GetSize()] = S_O::FormatUINT(i + 1);
		}
		grid->FillData();
	}
	return 1;
}

LRESULT CLongGasMainDlg::OnNFCCommandDone(WPARAM wp, LPARAM lp)
{
	CNFC_Command* c = (CNFC_Command*)wp;
	for (int i = 0; i < dlgs.GetSize(); i++)
	{
		LG_WND_ARRAY* a = &dlgs[i];
		if (a->dev != c->dev)continue;
		for (int k = 0; k < arrWnd.GetSize(); k++)
		{
			if (!arrWnd[k]->CommandDone(c))break;
		}
		if (!a->dev->bManufact && (c->nCommand == GetRadioSettingsByte || c->nCommand== GetLoraOTAA) && a->dev->IsHasLora())
		{
			CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_DEVICES);
			if (grid->GetActiveRow() == i)
			{
				bool bf = 0;
				for (int k = 0; k < m_Tab.GetItemCount(); k++)
				{
					if (D_O::GetTabData(m_Tab, k) == LG_RADIO)
					{
						bf = 1;
						break;
					}
				}
				if (!bf)
				{
					m_Tab.DeleteAllItems();
					OnCellSelected((WPARAM)grid, 0);
				}
			}
		}
		break;
	}
	if (c->bReportCommandStatus)
	{
		CString str,strCommand=c->strCommandDescription,strDate= CDateTime::GetCurrentTime().Format("%H:%M:%S    ");
		if (strCommand == "")strCommand = c->dev->GetCommandString(c->nCommand);
		str.Format("%s - %s - ",c->dev->GetSerialNumber(), strCommand);
		group->ShowWindow(1);
		D_O::WriteLog(group->m_Rich, strDate,::GetSysColor(COLOR_GRAYTEXT));
		D_O::WriteLog(group->m_Rich, c->dev->GetSerialNumber(), 0, 1);
		str.Format(" - %s - ",strCommand);
		D_O::WriteLog(group->m_Rich, str, 0);
		D_O::WriteLog(group->m_Rich, S_O::LoadString(c->nRet ? IDS_DONE : IDS_ERROR)+"\r\n", c->nRet ? RGB(78, 162, 60) :0xff, 0, 1);
		for (int i = 0; i < arrReportCommandDone.GetSize(); i++)
		{
			if (arrReportCommandDone[i].strCommand == strCommand)
			{
				arrReportCommandDone[i].nCount--;
				if (!arrReportCommandDone[i].nCount)
				{
					D_O::WriteLog(group->m_Rich,"\r\n"+ strDate, ::GetSysColor(COLOR_GRAYTEXT));
					str.Format("%s - %s\r\n\r\n",strCommand, S_O::LoadString(IDS_DONE));
					D_O::WriteLog(group->m_Rich, str,0xff0000, 0, 1);
					arrReportCommandDone.RemoveAt(i);
				}
				break;
			}
		}
	}
	delete c;
	return 1;
}

void CLongGasMainDlg::UpdateQualityBar()
{
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_DEVICES);
	int nR = grid->GetActiveRow();
	if (nR < 0 || nR >= dlgs.GetSize())nR = 0;
	if (nR >= dlgs.GetSize())return;
	CNFC_Device* dev = dlgs[nR].dev;
	m_Quality.ShowWindow(bManufact && dev && dev->dev_info.dExchangeQuality != -1);
	if (dev)
	{
		double d = dev->dev_info.dExchangeQuality;
		if (d > 0.8)m_Quality.SetGradientColors(RGB(128, 255, 128), RGB(128, 255, 128));
		else if (d > 0.6)m_Quality.SetGradientColors(0xff00, 0xff00);
		else if (d > 0.4)m_Quality.SetGradientColors(RGB(255, 128, 64), RGB(255, 128, 64));
		else m_Quality.SetGradientColors(0xff, 0xff);
		m_Quality.SetPos(dev->dev_info.dExchangeQuality * 100);
	}
}

void CLongGasMainDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == QUALITY_REFRSESH_TIMER)
	{
		UpdateQualityBar();
	}
	else if (nIDEvent == DELAYED_MIPEX_TIMER)
	{
		if (CLGDelayedCommandDlg::t_proc.IsAllDone())
		{
			if (CLGDelayedCommandDlg::t_proc.bWaitingForGasOff)CLGDelayedCommandDlg::t_proc.bGasOffAllowed = 1;
			int nc = 0;
			for (int i = 0; i < dlgs.GetSize(); i++)
			{
				LG_WND_ARRAY* a = &dlgs[i];
				if (!a->dev->IsDeviceInstalled())continue;
				if (a->dev->additional_pars.nDelayedMIPEXResult)continue;
				for (int k = 0; k < arrWnd.GetSize(); k++)
				{
					if (arrWnd[k]->IsKindOf(RUNTIME_CLASS(CLGDelayedCommandDlg)))
					{
						CLGDelayedCommandDlg* m = (CLGDelayedCommandDlg*)arrWnd[k];
						m->SelectNextCommand(a->dev,nc == 0);
						nc++;
						break;
					}
				}
			}
			if (!nc)
			{
				BreakAll(0, STARTALL_DCA, 1);
			}
		}
		for (int i = 0; i < dlgs.GetSize(); i++)
		{
			LG_WND_ARRAY* a = &dlgs[i];
			if (!a->dev->IsDeviceInstalled())continue;
			for (int k = 0; k < arrWnd.GetSize(); k++)
			{
				if (arrWnd[k]->IsKindOf(RUNTIME_CLASS(CLGDelayedCommandDlg)))
				{
					CLGDelayedCommandDlg* m = (CLGDelayedCommandDlg*)arrWnd[k];
					if (!m->t_proc.IsDone(a->dev))
					{
						m->CheckDelayedExecution(a->dev);
					}
					break;
				}
			}
		}
	}
	else if (nIDEvent >= MARK_ERROR_TIMER_START)
	{
		int n = nIDEvent - MARK_ERROR_TIMER_START;
		if (n >= dlgs.GetSize())return;
		progress[n]->SendMessage(PBM_SETBARCOLOR, 0, BAR_CLR);
		KillTimer(nIDEvent);
	}
	CDialog::OnTimer(nIDEvent);
}

LRESULT CLongGasMainDlg::OnNFCDeviceDetectionStatus(WPARAM wp, LPARAM lp)
{
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_DEVICES);
	CNFC_Device* from = (CNFC_Device*)wp;
	for (int i = 0; i < dlgs.GetSize(); i++)
	{
		if (dlgs[i].dev == from)
		{
			if (from->IsDeviceInstalled())
			{
				grid->names[i * 2 + 1] = "";
				if (from->dev_info.bLoaded)grid->names[i * 2 + 1] = from->GetSerialNumber();
				else
				{
					from->GetDeviceInfo();
					grid->names[i * 2 + 1] += S_O::LoadString(IDS_DEVICEINSTALLED);
				}
			}
			else if (from->IsHasReader())
			{
				grid->names[i * 2 + 1] = S_O::LoadString(IDS_READERINSTALLED);
			}
			else grid->names[i * 2 + 1] = S_O::LoadString(IDS_UNKNOWNREADER);
			grid->InvalidateRow(i);
			break;
		}
	}
	int nR = grid->GetActiveRow();
	if (nR < 0 || nR >= dlgs.GetSize())return 1;
	if (dlgs[nR].dev == from)
	{
		OnCellSelected((WPARAM)grid,0);
	}
	return 1;
}

LRESULT CLongGasMainDlg::OnNFCLog(WPARAM wp, LPARAM lp)
{
	NFC_LOG* l = (NFC_LOG*)wp;
	for (int i = 0; i < dlgs.GetSize(); i++)
	{
		LG_WND_ARRAY* a = &dlgs[i];
		if (a->dev != l->from)continue;
		D_O::WriteLog(logs[i]->m_Rich, l->strLog+"\n", l->clr, 0, 1);
	}
	delete l;
	return 1;
}



LRESULT CLongGasMainDlg::OnNFCMarkError(WPARAM wp, LPARAM lp)
{
	CNFC_Device* nfc = (CNFC_Device*)wp;
	for (int i = 0; i < dlgs.GetSize(); i++)
	{
		LG_WND_ARRAY* a = &dlgs[i];
		if (a->dev != nfc)continue;
		progress[i]->SendMessage(PBM_SETBARCOLOR, 0, 0xff);
		SetTimer(MARK_ERROR_TIMER_START+i, 200, 0);
	}
	return 1;
}

void CLongGasMainDlg::StartAll(UINT nMode, CNFC_Device* from, bool bSkipDeviceFrom)
{
	group->m_Rich.SetWindowText("");
	if (nMode == STARTALL_CALIBRATION || nMode == STARTALL_FIRMWARE || nMode == STARTALL_GETLOG || nMode == STARTALL_FACTORYRESET ||
		nMode == STARTALL_UNITS || nMode== STARTALL_MIPEXCOMMAND || nMode== STARTALL_DCA)
	{
		bWasStartAll = 1;
		bWasTerminated = bWasError = 0;
	}
	CString strTitle,strPath;
	int nc = 0;
	CString strCommandDescr;
	bool bReportCommandDone = 0;
	if(nMode==STARTALL_DCA)CLGDelayedCommandDlg::t_proc.done.RemoveAll();
	for (int i = 0; i < dlgs.GetSize(); i++)
	{
		LG_WND_ARRAY* a = &dlgs[i];
		if (!a->dev->IsDeviceInstalled())continue;
		if (bSkipDeviceFrom && a->dev == from)continue;
		switch (nMode)
		{
		case STARTALL_MIPEXCOMMAND:
		{
			if (a->dev != from)
			{
				a->dev->additional_pars.nMIPEXCommandId = -1;
				a->dev->additional_pars.nMipexDest = from->additional_pars.nMipexDest;
				a->dev->additional_pars.strMipexEnd = from->additional_pars.strMipexEnd;
				a->dev->additional_pars.strMIPEXCommand = from->additional_pars.strMIPEXCommand;
				a->dev->bInMIPEXRepeat = from->bInMIPEXRepeat;
			}
			a->dev->additional_pars.bRetryMIPEXCommand = 1;
			a->dev->additional_pars.nSendMIPEXCommandTry = 0;
			a->dev->SendCommandMIPEX();
			break;
		}
		case STARTALL_GETCOEFF:
			a->dev->GetCoeffsOnly();
			break;
		case STARTALL_CLEARLOADSTATE:
			a->dev->ClearLoadState();
			a->dev->GetDeviceInfo();
			break;
		case STARTALL_DISABLEALARM:
			bReportCommandDone = 1;
			a->dev->additional_pars.bReportNextCommandStatus = 1;
			a->dev->additional_pars.strCommandDescription = strCommandDescr = S_O::LoadString(IDS_DISABLEALARM);
			a->dev->EnableAlarm(0);
			a->dev->GetDeviceInfo(0);
			break;
		case STARTALL_LOGTIMEOUT:
			bReportCommandDone = 1;
			a->dev->additional_pars.bReportNextCommandStatus = 1;
			a->dev->additional_pars.strCommandDescription= strCommandDescr=S_O::LoadString(IDS_LOGTIMEOUT);
			a->dev->SetLogTimeout(from->dev_info.base.nLogTimeout);
			a->dev->GetDeviceInfo(0);
			break;
		case STARTALL_DEVICEMODE:
			bReportCommandDone = 1;
			a->dev->additional_pars.bReportNextCommandStatus = 1;
			a->dev->additional_pars.strCommandDescription = strCommandDescr = S_O::LoadString(IDS_DEVICEMODE);
			a->dev->SetDeviceMode(from->dev_info.btDeviceMode);
			break;
		case STARTALL_MULTIPLEUNITS:
			bReportCommandDone = 1;
			if (a->dev != from)a->dev->CopyUnits(from);
			a->dev->additional_pars.bReportNextCommandStatus = 1;
			a->dev->additional_pars.strCommandDescription = strCommandDescr = S_O::LoadString(IDS_UNITS);
			a->dev->SetSensorUnits();
//			a->dev->GetSettings();
			break;
		case STARTALL_ALARMS:
			bReportCommandDone = 1;
			if (a->dev != from)
			{
				a->dev->sensors_info.alarms.ch4Alarm = from->sensors_info.alarms.ch4Alarm;
				a->dev->sensors_info.alarms.o2Alarm= from->sensors_info.alarms.o2Alarm;
				a->dev->sensors_info.alarms.coAlarm= from->sensors_info.alarms.coAlarm;
				a->dev->sensors_info.alarms.h2sAlarm = from->sensors_info.alarms.h2sAlarm;

			};
			a->dev->additional_pars.bReportNextCommandStatus = 1;
			a->dev->additional_pars.strCommandDescription = strCommandDescr = S_O::LoadString(IDS_LG_ALARMS);
			a->dev->SetAlarms();
			a->dev->GetAlarms();
			break;
		case STARTALL_COEFF_O2:
			bReportCommandDone = 1;
			if (a->dev != from)memcpy(a->dev->sensors_info.fO2Coeff, from->sensors_info.fO2Coeff, sizeof(from->sensors_info.fO2Coeff));
			a->dev->additional_pars.bReportNextCommandStatus = 1;
			a->dev->additional_pars.strCommandDescription.Format("%s %s", S_O::LoadString(IDS_COEFFS), a->dev->GetGasString(GAS_O2));
			strCommandDescr = a->dev->additional_pars.strCommandDescription;
			a->dev->SetO2Coeff();
			break;
		case STARTALL_COEFF_CO:
			bReportCommandDone = 1;
			if (a->dev != from)memcpy(a->dev->sensors_info.fCOCoeff, from->sensors_info.fCOCoeff, sizeof(from->sensors_info.fCOCoeff));
			a->dev->additional_pars.bReportNextCommandStatus = 1;
			a->dev->additional_pars.strCommandDescription.Format("%s %s", S_O::LoadString(IDS_COEFFS), a->dev->GetGasString(GAS_CO));
			strCommandDescr = a->dev->additional_pars.strCommandDescription;
			a->dev->SetCOCoeff();
			break;
		case STARTALL_COEFF_CH4:
			bReportCommandDone = 1;
			if (a->dev != from)
			{
				a->dev->sensors_info.fCH4Threshold = from->sensors_info.fCH4Threshold;
				memcpy(&a->dev->sensors_info.fCH4K1, &from->sensors_info.fCH4K1, 6*sizeof(float));
			}
			a->dev->additional_pars.bReportNextCommandStatus = 1;
			a->dev->additional_pars.strCommandDescription.Format("%s %s", S_O::LoadString(IDS_COEFFS),a->dev->GetGasString(GAS_MIPEX));
			strCommandDescr = a->dev->additional_pars.strCommandDescription;
			a->dev->SetCH4Coeff();
			break;
		case STARTALL_COEFF_H2S:
			bReportCommandDone = 1;
			if (a->dev != from)memcpy(a->dev->sensors_info.fH2SCoeff, from->sensors_info.fH2SCoeff, sizeof(from->sensors_info.fH2SCoeff));
			a->dev->additional_pars.bReportNextCommandStatus = 1;
			a->dev->additional_pars.strCommandDescription.Format("%s %s", S_O::LoadString(IDS_COEFFS), a->dev->GetGasString(GAS_H2S));
			strCommandDescr = a->dev->additional_pars.strCommandDescription;
			a->dev->SetH2SCoeff();
			break;
		case STARTALL_COEFF:
			bReportCommandDone = 1;
			if (!a->dev->dev_settings.bLoaded)
			{
				a->dev->GetSettings();
				a->dev->WaitComplete(1);
			}
			if (a->dev != from)
			{
				DEVICE_SETTINGS* s = &a->dev->dev_settings,*f=&from->dev_settings;
				s->base = f->base;
			}
			a->dev->additional_pars.bReportNextCommandStatus = 1;
			a->dev->additional_pars.strCommandDescription.Format("%s", S_O::LoadString(IDS_COEFFS));
			strCommandDescr = a->dev->additional_pars.strCommandDescription;
			a->dev->SetSettings(1);
			break;
		case STARTALL_GASPOS:
			if (!a->dev->IsHaveProperty(GASPOS))continue;
			bReportCommandDone = 1;
			if (!a->dev->dev_settings.bLoaded)
			{
				a->dev->GetSettings();
				a->dev->WaitComplete(1);
			}
			if (a->dev != from)
			{
				DEVICE_SETTINGS* s = &a->dev->dev_settings, * f = &from->dev_settings;
				s->O2ChemScrPos = f->O2ChemScrPos;
			}
			a->dev->additional_pars.bReportNextCommandStatus = 1;
			a->dev->additional_pars.strCommandDescription.Format("%s", S_O::LoadString(IDS_GASPOS));
			strCommandDescr = a->dev->additional_pars.strCommandDescription;
			a->dev->SetSettings(1);
			break;
		case STARTALL_PRECISIONS:
			if (!a->dev->IsHaveProperty(PRECISION))continue;
			bReportCommandDone = 1;
			if (!a->dev->dev_settings.bLoaded)
			{
				a->dev->GetSettings();
				a->dev->WaitComplete(1);
			}
			if (a->dev != from)
			{
				DEVICE_SETTINGS* s = &a->dev->dev_settings, * f = &from->dev_settings;
				s->SensorsPrecisions = f->SensorsPrecisions;
			}
			a->dev->additional_pars.bReportNextCommandStatus = 1;
			a->dev->additional_pars.strCommandDescription.Format("%s", S_O::LoadString(IDS_PRECISIONS));
			strCommandDescr = a->dev->additional_pars.strCommandDescription;
			a->dev->SetSettings(1);
			break;
		case STARTALL_AUTOZERO:
			if (!a->dev->IsHaveProperty(AUTOZERO))continue;
			bReportCommandDone = 1;
			if (!a->dev->dev_settings.bLoaded)
			{
				a->dev->GetSettings();
				a->dev->WaitComplete(1);
			}
			if (a->dev != from)
			{
				DEVICE_SETTINGS* s = &a->dev->dev_settings, * f = &from->dev_settings;
				s->SensorsAutoZero = f->SensorsAutoZero;
			}
			a->dev->additional_pars.bReportNextCommandStatus = 1;
			a->dev->additional_pars.strCommandDescription.Format("%s", S_O::LoadString(IDS_AUTOZERO));
			strCommandDescr = a->dev->additional_pars.strCommandDescription;
			a->dev->SetSettings(1);
			break;
		case STARTALL_SENSOR_ACCEL:
			bReportCommandDone = 1;
			if (a->dev != from)memcpy(&a->dev->sensors_info.fAccO2, &from->sensors_info.fAccO2, 4 * sizeof(float));
			a->dev->additional_pars.bReportNextCommandStatus = 1;
			a->dev->additional_pars.strCommandDescription = strCommandDescr=S_O::LoadString(IDS_SENSORACC);
			a->dev->SetSensorAccel();
			break;
		case STARTALL_SENSOR_GASRANGE:
			bReportCommandDone = 1;
			if (a->dev != from)
			{
				a->dev->sensors_info.ch4Range = from->sensors_info.ch4Range;
				a->dev->sensors_info.coRange = from->sensors_info.coRange;
				a->dev->sensors_info.h2sRange = from->sensors_info.h2sRange;
				a->dev->sensors_info.o2Range = from->sensors_info.o2Range;
			}
			a->dev->additional_pars.bReportNextCommandStatus = 1;
			a->dev->additional_pars.strCommandDescription = strCommandDescr= S_O::LoadString(IDS_MEASUREDIAP);
			a->dev->SetGasRange();
			a->dev->GetGasRange();
			break;
		case STARTALL_SENSOR_VRANGE:
			bReportCommandDone = 1;
			if (a->dev != from)
			{
				a->dev->sensors_info.o2VRange = from->sensors_info.o2VRange;
				a->dev->sensors_info.coVRange = from->sensors_info.coVRange;
				a->dev->sensors_info.h2sVRange = from->sensors_info.h2sVRange;
			}
			a->dev->additional_pars.bReportNextCommandStatus = 1;
			a->dev->additional_pars.strCommandDescription = strCommandDescr=S_O::LoadString(IDS_VOLTDIAP);
			a->dev->SetSensorVRange();
			a->dev->GetSensorVoltRange();
			break;
		case STARTALL_SENSOR_STATUS:
			bReportCommandDone = 1;
			if (a->dev != from)
			{
				a->dev->sensors_info.bCH4En = from->sensors_info.bCH4En;
				a->dev->sensors_info.bO2En = from->sensors_info.bO2En;
				a->dev->sensors_info.bCOEn = from->sensors_info.bCOEn;
				a->dev->sensors_info.bH2SEn = from->sensors_info.bH2SEn;
			}
			a->dev->additional_pars.bReportNextCommandStatus = 1;
			a->dev->additional_pars.strCommandDescription = strCommandDescr=S_O::LoadString(IDS_SENSORSTATUS);
			a->dev->SetSensorStatus();
			a->dev->GetSensorStatus();
			break;
		case STARTALL_MONITORING:
		{
			for (int k = 0; k < arrWnd.GetSize(); k++)
			{
				if (arrWnd[k]->IsKindOf(RUNTIME_CLASS(CLGGraphDlg)))
				{
					((CLGGraphDlg*)arrWnd[k])->StartMonitoring(a->dev);
					break;
				}
			}
			break;
		}
		case STARTALL_CALIBRATION:
			if (a->dev != from)a->dev->calibration_settings = from->calibration_settings;
			a->dev->Calibrate();
			strTitle = S_O::LoadString(IDS_CALIBRATION);
			break;
		case STARTALL_FACTORYRESET:
			if (a->dev != from)a->dev->additional_pars = from->additional_pars;
			a->dev->FactoryReset();
			break;
		case STARTALL_GETLOG:
			if (a->dev != from)a->dev->log = from->log;
			a->dev->nRecordsFrom = 0;
			a->dev->GetLog();
			break;
		case STARTALL_FIRMWARE:
			if (a->dev != from)
			{
				a->dev->firmware_settings.strPath = from->firmware_settings.strPath;
				/*for (int k = 0; k < a->arr.GetSize(); k++)
				{
					if (a->arr[k]->IsKindOf(RUNTIME_CLASS(CLGFirmwareDlg)))
					{
						a->dev->firmware_settings.progress = (CProgressCtrl*)a->arr[k]->GetDlgItem(IDC_PROGRESS1);
						break;
					}
				}*/
			}
			a->dev->StartFirmware();
			break;
		case STARTALL_MIPEX:
			for (int k = 0; k < arrWnd.GetSize(); k++)
			{
				if (arrWnd[k]->IsKindOf(RUNTIME_CLASS(CLGMipexDlg)))
				{
					CLGMipexDlg* m = (CLGMipexDlg*)arrWnd[k];
					m->StartRepeat(a->dev);
					break;
				}
			}
			break;
		case STARTALL_DCA:
		{
			DELAYED_COMMAND_STATUS s;
			for (int k = 0; k < arrWnd.GetSize(); k++)
			{
				if (arrWnd[k]->IsKindOf(RUNTIME_CLASS(CLGDelayedCommandDlg)))
				{
					CLGDelayedCommandDlg* m = (CLGDelayedCommandDlg*)arrWnd[k];
					s.nfc = a->dev;
					CLGDelayedCommandDlg::t_proc.done.Add(s);
					m->Start(a->dev);
					break;
				}
			}
			break;
		}
		case STARTALL_UNITS:
			if (a->dev != from)a->dev->CopyUnits(from);
			a->dev->SetSensorUnitsAll();
			break;
		}
		nc++;
	}
	if (nMode == STARTALL_DCA)SetTimer(DELAYED_MIPEX_TIMER, 500, 0);
	if ((nMode == STARTALL_MONITORING || nMode== STARTALL_MIPEX || nMode == STARTALL_DCA) && nc > 1)bInStartAll = 1;
	if (strTitle != "")group->SetWindowText(strTitle);
	else
	{
		group->SetWindowText(S_O::LoadString(IDS_INFORMATION));
		group->ShowWindow(0);
	}
	if (bReportCommandDone)
	{
		REPORT_COMMAND_DONE r;
		r.strCommand = strCommandDescr;
		r.nCount = nc;
		bool bAdd = 1;
		for (int i = 0; i < arrReportCommandDone.GetSize(); i++)
		{
			if (arrReportCommandDone[i].strCommand == strCommandDescr)
			{
				arrReportCommandDone[i].nCount += nc;
				bAdd = 0;
				break;
			}
		}
		if(bAdd)arrReportCommandDone.Add(r);
	}
}

void CLongGasMainDlg::OnClose()
{
	bool bBreakAll = 0;
	if (IsInDelayedMIPEX())
	{
		if (AfxMessageBox(S_O::LoadString(IDS_BREAKALLANDCLOSE_Q), MB_YESNO) == IDNO)return;
		bBreakAll = 1;
	}
	for (int i = 0; i < dlgs.GetSize(); i++)
	{
		if (dlgs[i].dev->bInMonitoring)
		{
			if (!bBreakAll && AfxMessageBox(S_O::LoadString(IDS_BREAKMONITORING_Q), MB_YESNO) == IDNO)return;
			BreakAll(dlgs[i].dev, STARTALL_MONITORING, 1);
			bBreakAll = 1;
		}
		if ((dlgs[i].dev->IsInCommand() || dlgs[i].dev->IsInService()))
		{
			if (dlgs[i].dev->IsDeviceInstalled())
			{
				if (!bBreakAll && AfxMessageBox(S_O::LoadString(IDS_BREAKALLANDCLOSE_Q), MB_YESNO) == IDNO)return;
				bBreakAll = 1;
			}
			dlgs[i].dev->BreakExecution();
			dlgs[i].dev->WaitComplete(1);
		}
	}
	CDialog::OnClose();
}

LRESULT CLongGasMainDlg::OnNFCOperationDone(WPARAM wp, LPARAM lp)
{
	CNFC_Device* dev = (CNFC_Device*)wp;
	OPERATION_STATUS* s = (OPERATION_STATUS*)lp;
	for (int i = 0; i < dlgs.GetSize(); i++)
	{
		LG_WND_ARRAY* a = &dlgs[i];
		if (!a->dev->IsDeviceInstalled())continue;
		if (a->dev != dev)continue;
		for (int k = 0; k < arrWnd.GetSize(); k++)
		{
			if (s->nStatus)arrWnd[k]->OperationEnded(a->dev,s->nOperation);
			else arrWnd[k]->OperationStarted(a->dev,s->nOperation);
		}
	}
	int nC=0;
	for (int i = 0; i < dlgs.GetSize(); i++)
	{
		if (s->nStatus == 1 && dlgs[i].dev == dev)
		{
			if (s->nOperation != STARTALL_MONITORING && s->nOperation != STARTALL_DCA && s->nOperation != STARTALL_MIPEXCOMMAND)dev->evService.ResetEvent();
		}
		if (dlgs[i].dev->IsDeviceInstalled())
		{
			if (s->nOperation == STARTALL_DCA)nC += (s->nStatus == (UINT)dlgs[i].dev->additional_pars.bInDelayedMIPEX);
			else if (s->nOperation == STARTALL_MIPEXCOMMAND)nC += dlgs[i].dev->additional_pars.bRetryMIPEXCommand;
			else if(dlgs[i].dev->IsInService())nC++;
		}
	}
	CString str,strDate,strAlt;
	if (s->nStatus == 1 && bWasStartAll)
	{
		strDate = CDateTime::GetCurrentTime().Format("%H:%M:%S   ");
		UINT nSidGood = -1, nSidBad = -1, nSidTerminated = -1,nSidDone=-1,nSidDoneError=-1,nSidDoneTerminated=-1;
		UINT nResult = 0;
		if (s->nOperation == STARTALL_FIRMWARE)
		{
			nSidGood = IDS_FIRMWARECOMPLETE; nSidBad = nSidTerminated= IDS_FIRMWAREERROR;
			nResult = dev->firmware_settings.nResult;
		}
		else if (s->nOperation == STARTALL_GETLOG)
		{
			nSidGood = IDS_LOGCOMPLETE; nSidBad = IDS_LOGFAILED; nSidTerminated = IDS_LOGTERMINATED;
			nSidDoneError = IDS_LOGCOMPLETEWITHERROR;
			nResult = dev->log.nResult;
		}
		else if (s->nOperation == STARTALL_FACTORYRESET)
		{
			nSidGood = IDS_FACTORYRESETCOMPLETE; nSidBad = nSidTerminated  = IDS_FACTORYRESETCOMPLETEERROR;
			nResult = dev->additional_pars.nFactoryResetResult;
		}
		else if (s->nOperation == STARTALL_CALIBRATION)
		{
			nSidGood = IDS_DEVICECALIBRATIONCOMPLETE; nSidBad = IDS_DEVICECALIBRATIONFAILED; nSidTerminated = IDS_DEVICECALIBRATIONTERMINATED;
			nSidDone = IDS_CALIBRATIONCOMPLETE;
			nResult = dev->calibration_settings.nResult;
		}
		else if (s->nOperation == STARTALL_UNITS)
		{
			nSidGood = IDS_SETUNITSCOMPLETE; nSidBad = nSidTerminated = IDS_SETUNITSERROR;
			nResult = dev->dev_settings.bSensorUnitsResult;
		}
		else if (s->nOperation == STARTALL_MIPEXCOMMAND)
		{
			nResult = dev->additional_pars.nSendMIPEXResult;
			nSidGood = IDS_MIPEXDONE; nSidBad = nSidTerminated = IDS_MIPEXDONEWITHERROR;
			strAlt.Format(S_O::LoadString((nResult == RESULT_GOOD) ? IDS_MIPEXDONE2 : IDS_MIPEXDONEWITHERROR2), dev->additional_pars.strMIPEXCommand);
		}
		else if (s->nOperation == STARTALL_DCA)
		{
			nResult = dev->additional_pars.nDelayedMIPEXResult;
			nSidGood = IDS_DCADONE; nSidBad = nSidTerminated = IDS_DCADONEWITHERROR;
		}
		if (nSidGood != -1)
		{
			COLORREF clr = 0xff0000;
			UINT nSid = nSidGood;
			if (nSidDone == -1)nSidDone = nSidGood;
			if (nSidDoneError == -1)nSidDoneError = nSidBad;
			if (nSidDoneTerminated == -1)nSidDoneTerminated = nSidTerminated;
			if (nResult == RESULT_ERROR)
			{
				clr = 0xff;
				nSid = nSidBad;
				bWasError = 1;
			}
			else if (nResult == RESULT_TERMINATED)
			{
				clr = 0;
				nSid = nSidTerminated;
				bWasTerminated = 1;
			}
			str.Format(" - %s\r\n", (strAlt != "") ? strAlt : ((s->strResult != "") ? s->strResult : S_O::LoadString(nSid)));
			group->ShowWindow(1);
			D_O::WriteLog(group->m_Rich, strDate, ::GetSysColor(COLOR_GRAYTEXT));
			D_O::WriteLog(group->m_Rich, dev->GetSerialNumber(),0, 1);
			if (dev->strAttention != "")
			{
				D_O::WriteLog(group->m_Rich, " - " + dev->strAttention + " ", (nResult == RESULT_ATTENTION) ? RGB(255, 128, 64) : 0xff);
				dev->strAttention = "";
			}
			D_O::WriteLog(group->m_Rich, str, clr, 0, 1);
			if (!nC)
			{
				D_O::WriteLog(group->m_Rich, "\r\n" + strDate, ::GetSysColor(COLOR_GRAYTEXT));
				UINT nSid = nSidDone;
				if (bWasError)nSid = nSidDoneError;
				else if (bWasTerminated)nSid = nSidDoneTerminated;
				str = S_O::LoadString(nSid);
				D_O::WriteLog(group->m_Rich, str + "\r\n", 0xff0000, 0, 1);
				bWasError = bWasTerminated = 0;
			}
		}
	}
	bInStartAll = nC > 1;
	if (!nC)bWasStartAll = 0;
	delete s;
	return 1;
}


void CLongGasMainDlg::BreakAll(CNFC_Device * from, int nOperation,bool bForce)
{
	bool bStopAllMontitoring = 0;
	if (bInStartAll)
	{
		if (bForce || AfxMessageBox(S_O::LoadString(IDS_CANCELFORALL_Q), MB_YESNO) == IDYES)
		{
			if (nOperation == STARTALL_MONITORING || nOperation==STARTALL_MIPEX || nOperation== STARTALL_DCA)
			{
				bStopAllMontitoring = 1;
			}
			else
			{
				for (int i = 0; i < dlgs.GetSize(); i++)
				{
					CNFC_Device* dev = dlgs[i].dev;
					if (!dev->IsDeviceInstalled() || !dev->IsInService())continue;
					dev->BreakExecution();
				}
			}
		}
	}
	else if(nOperation!=STARTALL_MONITORING && nOperation != STARTALL_MIPEX && nOperation != STARTALL_DCA && from->IsInService())from->BreakExecution();
	if (nOperation == STARTALL_MIPEX)
	{
		for (int i = 0; i < dlgs.GetSize(); i++)
		{
			LG_WND_ARRAY* a = &dlgs[i];
			if (!a->dev->IsDeviceInstalled())continue;
			if (!bStopAllMontitoring && from != a->dev)continue;
			for (int k = 0; k < arrWnd.GetSize(); k++)
			{
				if (arrWnd[k]->IsKindOf(RUNTIME_CLASS(CLGMipexDlg)))
				{
					CLGMipexDlg* m = (CLGMipexDlg*)arrWnd[k];
					m->StopRepeat(a->dev);
					break;
				}
			}
		}
	}
	else if (nOperation == STARTALL_DCA)
	{
		KillTimer(DELAYED_MIPEX_TIMER);
		int nc = 0;
		for (int i = 0; i < dlgs.GetSize(); i++)
		{
			LG_WND_ARRAY* a = &dlgs[i];
			if (!a->dev->IsDeviceInstalled())continue;
			for (int k = 0; k < arrWnd.GetSize(); k++)
			{
				if (arrWnd[k]->IsKindOf(RUNTIME_CLASS(CLGDelayedCommandDlg)))
				{
					CLGDelayedCommandDlg* m = (CLGDelayedCommandDlg*)arrWnd[k];
					m->Stop(a->dev,nc==0);
					nc++;
					break;
				}
			}
		}
	}
	else if (nOperation == STARTALL_MONITORING)
	{
		for (int i = 0; i < dlgs.GetSize(); i++)
		{
			LG_WND_ARRAY* a = &dlgs[i];
			if (!a->dev->IsDeviceInstalled())continue;
			if (!bStopAllMontitoring && from != a->dev)continue;
			for (int k = 0; k < arrWnd.GetSize(); k++)
			{
				if (arrWnd[k]->IsKindOf(RUNTIME_CLASS(CLGGraphDlg)))
				{
					CLGGraphDlg* m = (CLGGraphDlg*)arrWnd[k];
					m->StopMonitoring(a->dev);
					break;
				}
			}
		}
		int nc = 0;
		for (int i = 0; i < dlgs.GetSize(); i++)
		{
			LG_WND_ARRAY* a = &dlgs[i];
			if (!a->dev->IsDeviceInstalled())continue;
			if (a->dev->IsInService() || (!a->dev->bStopMonitoring && a->dev->bInMonitoring))
			{
				nc++;
			}
		}
		if (nc<2)bInStartAll = 0;
	}
}

LRESULT __stdcall CLongGasMainDlg::KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (!mainPtr)return 1;
	if (code < 0)return CallNextHookEx(mainPtr->keybdhook, code, wParam, lParam);
	MSG* cwp = (MSG*)lParam;
	if (cwp->message == WM_KEYUP)
	{
		WORD wMod = 0;
		if (GetKeyState(VK_SHIFT) < 0)wMod |= HOTKEYF_SHIFT;
		if (GetKeyState(VK_MENU) < 0)wMod |= HOTKEYF_ALT;
		if (GetAsyncKeyState(VK_CONTROL) < 0)wMod |= HOTKEYF_CONTROL;
		for (int i = 0; i < g_hotkeys.arrSystem.GetSize(); i++)
		{
			HOTKEY& h = g_hotkeys.arrSystem[i];
			if (h.wVirtualCode == cwp->wParam && h.wModifiers == wMod)
			{
				mainPtr->PostMessage(h.nMessage, h.wParam, h.lParam);
				return 1;
			}
		}
		for (int i = 0; i<g_hotkeys.arr.GetSize(); i++)
		{
			HOTKEY& h = g_hotkeys.arr[i];
			if (h.wVirtualCode == cwp->wParam && h.wModifiers == wMod)
			{
				mainPtr->PostMessage(WM_HOTKEYCLICKED,i);
				return 1;
			}
		}

	}
	return CallNextHookEx(mainPtr->keybdhook, code, wParam, lParam);
}

LRESULT CLongGasMainDlg::OnWmSwitchToManufact(WPARAM wp, LPARAM lp)
{
	bManufact = !bManufact;
	AfxGetApp()->WriteProfileInt("System", "Manufacturer", bManufact);
	SelectManufact();
	return 1;
}

LRESULT CLongGasMainDlg::OnWmSelectNextTab(WPARAM wp, LPARAM lp)
{
	int nSz = m_Tab.GetItemCount(),nS=0;
	if (wp == 0)
	{
		if (nSz < 2)return 1;
		nS = m_Tab.GetCurSel();
		if (nS == nSz - 1)nS = 0;
		else nS++;
	}
	else if (wp == 1)
	{
		if (lp < 0 || lp >= nSz)return 1;
		nS = lp;
	}
	m_Tab.SetCurSel(nS);
	SelectTab(D_O::GetTabData(m_Tab, nS),1);
	return 1;
}

LRESULT CLongGasMainDlg::OnWmSelectNextDevice(WPARAM wp, LPARAM lp)
{
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_DEVICES);
	int nR = grid->GetActiveRow();
	if (nR < 0 || nR>=dlgs.GetSize())nR = 0;
	if (nR >= dlgs.GetSize())return 0;
	for (int j = 0; j < 2; j++)
	{
		for (int i = nR + 1; i < dlgs.GetSize(); i++)
		{
			LG_WND_ARRAY* a = &dlgs[i];
			if (a->dev->IsDeviceInstalled())
			{
				grid->SetActiveCell(1,i);
				OnCellSelected((WPARAM)grid, 0);
				return 1;
			}

		}
		nR = -1;
	}
	return 1;
}

void CLongGasMainDlg::SelectManufact()
{
	nLastTabSel = D_O::GetTabData(m_Tab, m_Tab.GetCurSel());
	for (int i = 0; i < dlgs.GetSize(); i++)
		dlgs[i].dev->bManufact = bManufact;
	m_Tab.DeleteAllItems();
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_DEVICES);
	OnCellSelected((WPARAM)grid, 0);
	m_Quality.ShowWindow(bManufact);
}

BOOL CLongGasBaseDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	TRANSLATION::TranslateWindow(this);
	layout.AttachBasic(this);
	HighlightHotkeys();
	return TRUE;
}


void CLongGasMainDlg::OnCbnSelchangeComboLang()
{
	UINT nId = D_O::GetSelectedItemData(m_Lang);
	if (nId == g_pref.nLang)return;
	theApp.WriteProfileInt("System", "LanguageId", nId);
	theApp.ReloadApplication();
}

void CLongGasMainDlg::ChangeAll(CLongGasBaseDlg* from, UINT nMode, CHANGEALL* c)
{
	for (int i = 0; i < dlgs.GetSize(); i++)
	{
		LG_WND_ARRAY* a = &dlgs[i];
		for (int k = 0; k < arrWnd.GetSize(); k++)
		{
			if (arrWnd[k] == from)continue;
			arrWnd[k]->ChangeAll(a->dev,nMode, c);
		}
	}
}

void CLongGasBaseDlg::OnOK()
{
}


void CLongGasBaseDlg::OnCancel()
{

}


void CLongGasMainDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	scroll.OnSize(nType, cx, cy);
	layout.OnSize(nType, cx, cy);
}

BEGIN_MESSAGE_MAP(CLongGasBaseDlg, CDialogEx)
	ON_WM_SIZE()
	ON_MESSAGE(WM_TABSELECTED,OnWmTabSelected)
	ON_WM_TIMER()
END_MESSAGE_MAP()


void CLongGasBaseDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	layout.OnSize(nType, cx, cy);
}


void CLongGasMainDlg::OnDestroy()
{
	D_O::SaveWndSize(this, "LongGasMain");
	CLGDelayedCommandDlg::t_proc.OffGas();
	CDialog::OnDestroy();
}

bool CLongGasBaseDlg::OperationStarted(CNFC_Device* dev, UINT nType)
{
	if(dev==nfcCurrent)EnableWindow(0);
	return 1;
};

bool CLongGasBaseDlg::OperationEnded(CNFC_Device* dev, UINT nType)
{
	if (nType == STARTALL_MONITORING)return 1;
	if (dev == nfcCurrent)EnableWindow(1);
	return 1;
}

CLongGasBaseDlg* CLongGasMainDlg::GetByClass(CNFC_Device * nfc, CString strClass)
{
	for (int i = 0; i < dlgs.GetSize(); i++)
	{
		LG_WND_ARRAY* a = &dlgs[i];
		if (a->dev != nfc)continue;
		for (int k = 0; k < arrWnd.GetSize(); k++)
		{
			if (strClass== arrWnd[k]->GetRuntimeClass()->m_lpszClassName)return arrWnd[k];
		}
	}
	return nullptr;
}


void CLongGasMainDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (!theApp.ProcessSysCommand(nID, lParam))
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

LRESULT CLongGasBaseDlg::OnWmTabSelected(WPARAM, LPARAM)
{
	EnableWindow(nfcCurrent->nCurrentOperation == -1);
	OnTabSelected();
	return 1;
}

BOOL CLongGasMainDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)return 1;
	return CDialog::OnCommand(wParam, lParam);
}


BOOL CLongGasBaseDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_RBUTTONUP)
	{
		char controlClassName[128];
		GetClassName(pMsg->hwnd, controlClassName, 128);
		CString str = controlClassName;
		if (str == "Button")
		{
			UINT nStyle = CWnd::FromHandle(pMsg->hwnd)->GetStyle();
			if ((nStyle & BS_AUTORADIOBUTTON)== BS_AUTORADIOBUTTON)return 1;
			CSetHotkeyDlg hd;
			hd.hk.nType = HKT_TAB;
			hd.hk.nTab = nTabId;
			hd.hk.nCtrlId = ::GetDlgCtrlID(pMsg->hwnd);
			UINT n=g_hotkeys.GetHotkeyIndex(hd.hk);
			if (n != -1)hd.hk = g_hotkeys.arr[n];
			CPoint pt(LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));
			::ClientToScreen(pMsg->hwnd, &pt);
			CMenu m;
			m.CreatePopupMenu();
			if(n==-1)m.AppendMenuA(MF_STRING, 1, S_O::LoadString(IDS_ADDHOTKEY));
			else
			{
				str.Format("%s (%s)", S_O::LoadString(IDS_MODHOTKEY), hd.hk.GetHotkeyName());
				m.AppendMenuA(MF_STRING, 1, str);
				m.AppendMenu(MF_SEPARATOR, 0);
				m.AppendMenuA(MF_STRING, 2, S_O::LoadString(IDS_DELHOTKEY));
			}
			int res = m.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this, 0);
			if (!res)return 1;
			if (res == 1)
			{
				if (hd.DoModal() == IDCANCEL)return 1;
			}
			else
			{
				CWnd* w = GetDlgItem(hd.hk.nCtrlId);
				if (main->fButtonDefault.m_hObject)
				{
					w->SetFont(&main->fButtonDefault);
				}
				m_wndToolTip.DelTool(w);
				g_hotkeys.arr.RemoveAt(n);
				g_hotkeys.Save();
			}
			HighlightHotkeys();
			return 1;

		}
	}
	if (pMsg->message >= WM_MOUSEFIRST &&
		pMsg->message <= WM_MOUSELAST && IsWindow(m_wndToolTip.GetSafeHwnd()))
	{
		MSG msg;
		::CopyMemory(&msg, pMsg, sizeof(MSG));
		HWND hWndParent = ::GetParent(msg.hwnd);
		while (hWndParent && hWndParent != m_hWnd)
		{
			msg.hwnd = hWndParent;
			hWndParent = ::GetParent(hWndParent);
		}
		if (msg.hwnd)
		{
			m_wndToolTip.RelayEvent(&msg);
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

HOTKEY_ARRAY::HOTKEY_ARRAY()
{
	HOTKEY h;
	h.nType = HKT_SYSTEM;
	h.nMessage = WM_SWITCHTOMANUFACT;
	h.wVirtualCode = 'M';
	h.wModifiers = HOTKEYF_CONTROL | HOTKEYF_SHIFT;
	arrSystem.Add(h);
	h.nMessage = WM_SELECTNEXTDEVICE;
	h.wVirtualCode = VK_TAB;
	h.wModifiers = HOTKEYF_SHIFT;
	arrSystem.Add(h);
	h.nMessage = WM_SELECTNEXTTAB;
	h.wVirtualCode = VK_TAB;
	h.wModifiers = HOTKEYF_CONTROL;
	arrSystem.Add(h);
	h.nMessage = WM_SELECTNEXTTAB;
	h.wParam = 1;
	h.wModifiers = 0;
	for (UINT i = VK_F1; i <= VK_F12; i++)
	{
		h.wVirtualCode = i;
		h.lParam = i - VK_F1;
		arrSystem.Add(h);
	}
}

bool HOTKEY_ARRAY::Save()
{
	CString str;
	str.Format("%s\\hotkeys.dat", g_szCurrentDir);
	CFile f;
	if (!f.Open(str, CFile::modeReadWrite|CFile::modeCreate))return 0;
	int nVer = 0,nC=0;
	WriteVersion(&f, nVer);
	nC = arr.GetSize();
	f.Write(&nC, sizeof(int));
	for (int i = 0; i < nC; i++)
	{
		HOTKEY& h = arr[i];
		f.Write(&h.nType, 1);
		f.Write(&h.nTab, 1);
		f.Write(&h.nCtrlId, sizeof(int));
		f.Write(&h.nMessage, sizeof(int));
		f.Write(&h.wParam, sizeof(int));
		f.Write(&h.lParam, sizeof(int));
		f.Write(&h.wVirtualCode, sizeof(WORD));
		f.Write(&h.wModifiers, sizeof(WORD));
	}
	return 1;
}

bool HOTKEY_ARRAY::Load()
{
	CString str;
	str.Format("%s\\hotkeys.dat", g_szCurrentDir);
	CFile f;
	if (!f.Open(str, CFile::modeRead))return 0;
	int nVer = 0, nC = 0;
	if (!ReadVersion(&f, nVer))return 0;
	if (!f.Read(&nC, sizeof(int)))return 0;
	arr.SetSize(nC);
	for (int i = 0; i < nC; i++)
	{
		HOTKEY& h = arr[i];
		if (!f.Read(&h.nType, 1))return 0;
		if (!f.Read(&h.nTab, 1))return 0;
		if (!f.Read(&h.nCtrlId, sizeof(int)))return 0;
		if (!f.Read(&h.nMessage, sizeof(int)))return 0;
		if (!f.Read(&h.wParam, sizeof(int)))return 0;
		if (!f.Read(&h.lParam, sizeof(int)))return 0;
		if (!f.Read(&h.wVirtualCode, sizeof(WORD)))return 0;
		if (!f.Read(&h.wModifiers, sizeof(WORD)))return 0;
	}
	return 1;
}

bool HOTKEY_ARRAY::IsCanAdd(HOTKEY& hk)
{
	for (int i = 0; i < arrSystem.GetSize(); i++)
	{
		HOTKEY& h = arrSystem[i];
		if (h.wVirtualCode == hk.wVirtualCode && h.wModifiers == hk.wModifiers)return 0;
	}
	for (int i = 0; i < arr.GetSize(); i++)
	{
		HOTKEY& h = arr[i];
		if (h.nType == hk.nType && h.nTab == hk.nTab && h.wVirtualCode == hk.wVirtualCode && h.wModifiers == hk.wModifiers)return 0;
	}
	return 1;
}

UINT HOTKEY_ARRAY::GetHotkeyIndex(HOTKEY& hk)
{
	for (int i = 0; i < arr.GetSize(); i++)
	{
		HOTKEY& h = arr[i];
		if (h.nType == hk.nType && h.nTab == hk.nTab && h.nCtrlId == hk.nCtrlId)
		{
			return i;
		}
	}
	return -1;
}

bool HOTKEY_ARRAY::AddHotkey(HOTKEY& hk)
{
	if (!IsCanAdd(hk))return 0;
	UINT n = GetHotkeyIndex(hk);
	if(n!=-1)
	{
		arr[n] = hk;
		return 1;
	}
	arr.Add(hk);
	return 1;
}

LRESULT CLongGasMainDlg::OnWmHotkey(WPARAM wp, LPARAM lp)
{
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_DEVICES);
	int nR = grid->GetActiveRow();
	if (nR < 0 || nR >= dlgs.GetSize())return 1;
	LG_WND_ARRAY& a = dlgs[nR];
	if (!a.dev->IsDeviceInstalled())return 1;
	int nS = m_Tab.GetCurSel();
	if (nS == -1)return 1;
	UINT nD = D_O::GetTabData(m_Tab, nS);
	if (nD < 0 || nD >= (UINT)arrWnd.GetSize())return 1;
	return arrWnd[nD]->HotkeyPressed(wp);
}

bool CLongGasBaseDlg::HotkeyPressed(UINT nHotkey)
{
	if (nHotkey < 0 || nHotkey >= (UINT)g_hotkeys.arr.GetSize())return 0;
	HOTKEY h = g_hotkeys.arr[nHotkey];
	if (h.nTab != nTabId)
	{
		h.nTab = nTabId;
		UINT n = -1;
		for (int i = 0; i < g_hotkeys.arr.GetSize(); i++)
		{
			HOTKEY& hk = g_hotkeys.arr[i];
			if (hk.nTab == h.nTab && hk.wVirtualCode == h.wVirtualCode && hk.wModifiers == h.wModifiers)
			{
				n = i;
				break;
			}
		}
		if (n == -1)return 1;
		h = g_hotkeys.arr[n];
	}
	CWnd* w = GetDlgItem(h.nCtrlId);
	if (!w || !w->IsWindowVisible() || !w->IsWindowEnabled())
	{
		return 1;
	}
	UINT nStyle = w->GetStyle();
	if ((nStyle & BS_CHECKBOX) == BS_CHECKBOX)
	{
		CheckDlgButton(h.nCtrlId, IsDlgButtonChecked(h.nCtrlId) != BST_CHECKED);
	}
	PostMessage(WM_COMMAND, h.nCtrlId);
	if(!hl.GetSize())SetTimer(TIMER_HOTKEYHIGHLIGHT, 100, 0);
	hl.Add(HOTKEY_HIGHLIGHT(h.nCtrlId));
	return 1;
}

void CLongGasBaseDlg::HighlightHotkeys()
{
	LOGFONT lf;
	for (int i = 0; i < g_hotkeys.arr.GetSize(); i++)
	{
		HOTKEY& h = g_hotkeys.arr[i];
		if (h.nTab != nTabId)continue;
		CWnd* w = GetDlgItem(h.nCtrlId);
		if (!w)continue;
		CFont* f = w->GetFont();
		if (f)
		{
			if (!main->fButton.m_hObject)
			{
				f->GetObject(sizeof(LOGFONT), &lf);
				main->fButtonDefault.CreateFontIndirect(&lf);
				lf.lfUnderline = 1;
				main->fButton.CreateFontIndirect(&lf);
				lf.lfHeight -= 2;
				main->fButtonBald.CreateFontIndirect(&lf);
			}
		}
		if (!main->fButton.m_hObject)
		{
			main->fButton.CreateStockObject(DEFAULT_GUI_FONT);
			main->fButton.GetObject(sizeof(LOGFONT), &lf);
			main->fButtonDefault.CreateFontIndirect(&lf);
			lf.lfItalic = 1;
			main->fButton.DeleteObject();
			main->fButton.CreateFontIndirect(&lf);
			lf.lfHeight -= 2;
			main->fButtonBald.CreateFontIndirect(&lf);
		}
		w->SetFont(&main->fButton);
		if (!IsWindow(m_wndToolTip.m_hWnd))
		{
			m_wndToolTip.Create(this);
			m_wndToolTip.Activate(1);
		}
		m_wndToolTip.AddTool(w, h.GetHotkeyName());
	}
}

CString HOTKEY::GetHotkeyName()
{
	CString str, buf = "Unknown";
	if (wModifiers & HOTKEYF_ALT)str += "Alt + ";
	if (wModifiers & HOTKEYF_CONTROL)str += "Ctrl + ";
	if (wModifiers & HOTKEYF_SHIFT)str += "Shift + ";
	if ((wVirtualCode >= '0' && wVirtualCode <= '0') ||
		(wVirtualCode >= 'A' && wVirtualCode <= 'Z'))
		buf.Format("%c", wVirtualCode);
	if (wVirtualCode >= VK_F1 && wVirtualCode <= VK_F24)
	{
		buf.Format("F%d", wVirtualCode - VK_F1 + 1);
	}
	if (wVirtualCode == VK_RETURN)buf = "Enter";
	if (wVirtualCode == VK_BACK)buf = "Backspace";
	if (wVirtualCode == VK_ESCAPE)buf = "Escape";
	if (wVirtualCode == VK_SPACE)buf = "Space";
	if (wVirtualCode == VK_END)buf = "End";
	if (wVirtualCode == VK_HOME)buf = "Home";
	if (wVirtualCode == VK_LEFT)buf = "Left";
	if (wVirtualCode == VK_UP)buf = "Up";
	if (wVirtualCode == VK_RIGHT)buf = "Right";
	if (wVirtualCode == VK_DOWN)buf = "Down";
	if (wVirtualCode == VK_INSERT)buf = "Insert";
	if (wVirtualCode == VK_DELETE)buf = "Delete";
	if (wVirtualCode == VK_LWIN)buf = "Win";
	if (wVirtualCode == VK_TAB)buf = "Tab";
	str += buf;
	return str;
}
#define STAGE_MAX 6

void CLongGasBaseDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_HOTKEYHIGHLIGHT)
	{
		for (int i = 0; i < hl.GetSize(); i++)
		{
			HOTKEY_HIGHLIGHT& h = hl[i];
			CWnd* w = GetDlgItem(h.nCtrlId);
			if (!w)h.nStage = STAGE_MAX;
			else
			{
				if (h.nStage % 2)w->SetFont(&main->fButtonDefault);
				else w->SetFont(&main->fButtonBald);
				h.nStage++;
			}
			if (h.nStage == STAGE_MAX)
			{
				w->SetFont(&main->fButton);
				hl.RemoveAt(i);
				i--;
			}
		}
		if (!hl.GetSize())KillTimer(TIMER_HOTKEYHIGHLIGHT);
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CLongGasMainDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	scroll.OnVScroll(nSBCode, nPos, pScrollBar);
	//CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CLongGasMainDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	scroll.OnHScroll(nSBCode, nPos, pScrollBar);
	//CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


BOOL CLongGasMainDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	BOOL wasScrolled = scroll.OnMouseWheel(nFlags, zDelta, pt);
	return wasScrolled;
	//return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}


void CLongGasMainDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 300;
	lpMMI->ptMinTrackSize.y = 300;
//	CDialog::OnGetMinMaxInfo(lpMMI);
}


void CLongGasMainDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM)dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

bool CLongGasBaseDlg::IsNumber(MSG* pMsg, bool bAllowFloat)
{
	if (pMsg->wParam == VK_DELETE || pMsg->wParam == VK_BACK)return 1;
	CString str = "0123456789", find;
	if (bAllowFloat)str += ",.";
	find += (char)pMsg->wParam;
	return find.FindOneOf(str) != -1;
}

ITaskbarList3* CLongGasMainDlg::GetTaskbarList()
{
	if (!itl && afxGlobalData.bIsWindows7)
	{
		ITaskbarList3* ptbl = NULL;
		HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&ptbl));
		if (SUCCEEDED(hr))
		{
			HRESULT hr2 = ptbl->HrInit();
			if (SUCCEEDED(hr2))
			{
				itl = ptbl;
			}
			else
			{
				ptbl->Release();
			}
		}
	}
	return itl;
}

bool CLongGasMainDlg::IsInDelayedMIPEX()
{
	for (int i = 0; i < dlgs.GetSize(); i++)
	{
		LG_WND_ARRAY* a = &dlgs[i];
		if (!a->dev->IsDeviceInstalled())continue;
		if (a->dev->additional_pars.bInDelayedMIPEX)return 1;
	}
	return 0;
}

CNFC_Device* CLongGasMainDlg::GetDeviceByNum(UINT nNum)
{
	for (int i = 0; i < dlgs.GetSize(); i++)
	{
		if (dlgs[i].dev->nDeviceNum == nNum)return dlgs[i].dev;
	}
	return 0;
}