// CLGDelayedCommandDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "CLGDelayedCommandDlg.h"
#include "GridCtrl.h"
#include "afxdialogex.h"
#include "NFC_Device.h"
#include <DialogOperations.h>
#include <LongNameFile.h>
#include <LogFile.h>

extern CString g_szCurrentDir;
extern VEGA_PREFERENCES g_pref;

TERMOCAMERA_PROC CLGDelayedCommandDlg::t_proc;
CWnd* TERMOCAMERA_PROC::wNotify = 0;

// CLGDelayedCommandDlg dialog

IMPLEMENT_DYNAMIC(CLGDelayedCommandDlg, CDialogEx)

CLGDelayedCommandDlg::CLGDelayedCommandDlg(CWnd* pParent /*=nullptr*/)
	: CLongGasBaseDlg(IDD_LG_DELAYED, pParent)
	, m_Command(_T(""))
{
	strFolder = AfxGetApp()->GetProfileString("DelayedCommandDlg", "SaveFolder", g_szCurrentDir);
	m_Command = AfxGetApp()->GetProfileString("DelayedCommandDlg", "LastCommand", "");
	m_RepeatTime = AfxGetApp()->GetProfileString("DelayedCommandDlg", "RepeatTime", "5");
	m_BkGas = AfxGetApp()->GetProfileInt("DelayedCommandDlg", "BkGas", 0);
	m_BkOffAfter = AfxGetApp()->GetProfileInt("DelayedCommandDlg", "BkOffAfter", 0);
	m_BkGasConc = theApp.GetProfileDouble("DelayedCommandDlg", "BkGasConc", 10);
	m_StopAfter = AfxGetApp()->GetProfileInt("DelayedCommandDlg", "StopAfter", 1);
	bSave = 0;
	bShowStatus = 0;
	bProfileSelected = 0;
}

CLGDelayedCommandDlg::~CLGDelayedCommandDlg()
{

}

void CLGDelayedCommandDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_COMMAND, m_Command);
	DDX_Control(pDX, IDC_FILEPATH, m_FilePath);
	DDX_Control(pDX, IDC_TIMEELAPSED, m_Elapsed);
	DDX_Control(pDX, IDC_PROFILE, m_Profile);
	DDX_Text(pDX, IDC_REPEATTIME, m_RepeatTime);

	DDX_Control(pDX, IDC_STATIC_T, m_T);
	DDX_Control(pDX, IDC_STATIC_TSET, m_TSet);
	DDX_Control(pDX, IDC_STATIC_TSTATUS, m_TStatus);
	DDX_Text(pDX, IDC_BKCONC, m_BkGasConc);
	DDX_Control(pDX, IDC_BKVALVE, m_BkValve);
	DDX_Check(pDX, IDC_BKGAS, m_BkGas);
	DDX_Check(pDX, IDC_OFFAFTER, m_BkOffAfter);
	DDX_Check(pDX, IDC_STOPALLAFTER, m_StopAfter);
	DDX_Control(pDX, IDC_STATIC_VALVE, m_CurrentValve);
	DDX_Control(pDX, IDC_STATIC_CONC, m_CurrentConc);

}


BEGIN_MESSAGE_MAP(CLGDelayedCommandDlg, CLongGasBaseDlg)
	ON_BN_CLICKED(IDC_ADD, &CLGDelayedCommandDlg::OnBnClickedAdd)
	ON_BN_CLICKED(IDC_MOD, &CLGDelayedCommandDlg::OnBnClickedMod)
	ON_BN_CLICKED(IDC_DEL, &CLGDelayedCommandDlg::OnBnClickedDel)
	ON_BN_CLICKED(IDC_SEND, &CLGDelayedCommandDlg::OnBnClickedSend)
	ON_BN_CLICKED(IDC_SENDALL, &CLGDelayedCommandDlg::OnBnClickedSendall)
	ON_BN_CLICKED(IDC_CLEAR, &CLGDelayedCommandDlg::OnBnClickedClear)
	ON_BN_CLICKED(IDC_OPENFOLDER, &CLGDelayedCommandDlg::OnBnClickedOpenfolder)
	ON_BN_CLICKED(IDC_MOD_PROFILE, &CLGDelayedCommandDlg::OnBnClickedModProfile)
	ON_BN_CLICKED(IDC_DEL_PROFILE, &CLGDelayedCommandDlg::OnBnClickedDelProfile)
	ON_CBN_SELCHANGE(IDC_PROFILE, &CLGDelayedCommandDlg::OnCbnSelchangeProfile)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_MENU, &CLGDelayedCommandDlg::OnBnClickedMenu)
	ON_BN_CLICKED(IDC_CAMERASETTINGS, &CLGDelayedCommandDlg::OnBnClickedCamerasettings)
	ON_MESSAGE(WM_CAMERASTATUSCHANGED, OnWmCameraStatusChanged)
	ON_MESSAGE(WM_CAMERALOG, OnWmCameraLog)
	ON_MESSAGE(WM_CAMERAERROR, OnWmCameraError)

	ON_BN_CLICKED(IDC_OFFAFTER, &CLGDelayedCommandDlg::OnBnClickedOffafter)
	ON_BN_CLICKED(IDC_STOPALLAFTER, &CLGDelayedCommandDlg::OnBnClickedStopallafter)
END_MESSAGE_MAP()


// CLGDelayedCommandDlg message handlers
void CLGDelayedCommandDlg::LoadProfiles(CString strSelect)
{
	CString strPrev;
	if (strSelect != "")strPrev = strSelect;
	else m_Profile.GetWindowText(strPrev);
	m_Profile.ResetContent();
	CFileFind ff;
	bool bf = ff.FindFile(S_O::Format("%s\\*.dca", dca.GetProfileFolder()));
	D_O::AddStringWithId(&m_Profile, S_O::LoadString(IDS_NEW), -1, 1);
	while (bf)
	{
		bf = ff.FindNextFile();
		D_O::AddStringWithId(&m_Profile, ff.GetFileTitle(), 0);
	}
	if (strPrev != "")m_Profile.SelectString(-1, strPrev);
	ProfileChanged(0);
}

BOOL CLGDelayedCommandDlg::OnInitDialog()
{
	CLongGasBaseDlg::OnInitDialog();
	D_O::AdjustControlFont(&m_T, &fT);
	D_O::AdjustControlFont(&m_TSet, &fTSet);
	D_O::AdjustControlFont(&m_TStatus, &fTStatus);
	D_O::AdjustControlFont(&m_CurrentValve, &fValve);
	D_O::AdjustControlFont(&m_CurrentConc, &fConc);
	UINT nValve = AfxGetApp()->GetProfileInt("DelayedCommandDlg", "BkValve", 4);
	for (int i = 1; i <= (g_pref.b10ChanBKM?10:4); i++)
		D_O::AddStringWithId(&m_BkValve, S_O::FormatUINT(i), i, i == nValve);

	CheckRadioButton(IDC_RADIO_MIPEX, IDC_RADIO_SPI, AfxGetApp()->GetProfileInt("DelayedCommandDlg", "MIPEXDest", IDC_RADIO_MIPEX));
	CheckRadioButton(IDC_RADIO_D, IDC_RADIO_DA, AfxGetApp()->GetProfileInt("DelayedCommandDlg", "MIPEXEnd", IDC_RADIO_D));
	LoadProfiles(AfxGetApp()->GetProfileString("DelayedCommandDlg", "LastProfile", S_O::LoadString(IDS_NEW)));
	FillResultGrid();
	return TRUE;
}

CString CLGDelayedCommandDlg::GetSaveFileName(CNFC_Device* dev, bool bAuto)
{
	CString str, out;
	if (dtStart.IsNull())dtStart = CDateTime::GetCurrent();
	out.Format("%s_%s_DM.csv", dev->GetSerialNumber(), dtStart.Format("%d.%m.%y_%H-%M"));
	return out;
}

void CLGDelayedCommandDlg::GetFolderName(bool bAll)
{
	bSave = D_O::SelectFolder(strFolder, m_hWnd, S_O::LoadString(IDS_SELECTSAVEPATH), strFolder);
	m_FilePath.SetWindowText(bSave ? S_O::Format("%s\\%s", strFolder, GetSaveFileName(nfcCurrent,1)) : "");
	if (bSave)AfxGetApp()->WriteProfileString("DelayedCommandDlg", "SaveFolder", strFolder);
}

void CLGDelayedCommandDlg::FillGrid()
{
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_ARRAY);
	grid->nGridId = GRIDID_DCA;
	grid->FillData(nfcCurrent->dca.arr.GetSize() ? &nfcCurrent->dca : &dca, bShowStatus);
}

void CLGDelayedCommandDlg::FillResultGrid()
{
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_RESULT);
	grid->nGridId = GRIDID_DCAR;
	grid->FillData(&nfcCurrent->dcar);
}

void CLGDelayedCommandDlg::SaveToFile(CString& strPath,DELAYED_COMMAND_RESULT& r)
{
	CString str;
	CLongNameFile f;
	if (!f.Open(strPath, CFile::modeCreate | CFile::modeReadWrite | CFile::modeNoTruncate))return;
	f.SeekToEnd();
	str.Format("%s;%s;%s\r\n", r.tm.FormatDB(), r.strCommand, r.strResult);
	f.Write(str, str.GetLength());
}

void CLGDelayedCommandDlg::LogAnswer(CNFC_Command* c, bool bShowIfError)
{
	if (c->dev->additional_pars.nMIPEXCommandId == -1)return;
	DELAYED_COMMAND_RESULT r;
	r.tm = CDateTime::GetCurrent();
	r.strCommand = c->dev->additional_pars.strMIPEXCommand;
	r.strDeviceId = c->dev->GetSerialNumber();
	if(c->nRet)r.strResult = c->dev->GetMIPEXAnswer(c->get);
	bool bBad = 0;
	if (!c->nRet || r.strResult.Find("TIMEOUT")!=-1)
	{
		c->dev->additional_pars.nSendMIPEXCommandTry++;
		if (c->dev->additional_pars.nSendMIPEXCommandTry < 10)
		{
			c->dev->dtLastRepeat = CDateTime::GetCurrent();
			c->dev->SendCommandMIPEX();
			return;
		}
		bBad = 1;
	}
	if (c->dev->additional_pars.nMIPEXCommandId > 0)
	{
		for (int i = 0; i < c->dev->dca.arr.GetSize(); i++)
		{
			DELAYED_COMMAND& cc = c->dev->dca.arr[i];
			if (cc.nCommandId == c->dev->additional_pars.nMIPEXCommandId)
			{
				if (bBad)cc.nStatus = DCAS_NOTDONE;
				else cc.nStatus = DCAS_DONE;
				t_proc.SetCommandStatus(c->dev, 1);
				if (i == c->dev->dca.arr.GetSize() - 1 || bBad)
				{
					c->dev->nCurrentCommandId = -1;
					if (bBad || m_StopAfter || m_Command == "")
					{
						c->dev->additional_pars.nDelayedMIPEXResult = bBad ? RESULT_ERROR : RESULT_GOOD;
						c->dev->OperationStatusChange(STARTALL_DCA, 1);
					}
				}
				else
				{

				}
				if (c->dev == nfcCurrent)
				{
					CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_ARRAY);
					grid->InvalidateRow(i);
				}
				break;
			}
		}
	}
	else if(bBad)
	{
		c->dev->additional_pars.nDelayedMIPEXResult = RESULT_ERROR;
		c->dev->OperationStatusChange(STARTALL_DCA, 1);
		t_proc.SetCommandStatus(c->dev, 1);
	}
	CString strPath;
	strPath.Format("%s\\%s", strFolder, GetSaveFileName(c->dev,1));
	if (c->nRet)
	{
		if (dtStart.IsNull())dtStart = CDateTime::GetCurrentTime();
		CStringArray arr;
		S_O::ParseString(r.strResult, arr, "\r");
		for (int i = 0; i < arr.GetSize(); i++)
		{
			if (arr[i].GetLength() == 0)continue;
			r.strResult = arr[i];
			c->dev->dcar.arr.Add(r);
			if (bSave)SaveToFile(strPath, r);
		}
	}
	else if (bShowIfError)
	{
		c->dev->dcar.arr.Add(r);
	}
	if (c->dev == nfcCurrent)FillResultGrid();
}

bool CLGDelayedCommandDlg::CommandDone(CNFC_Command* c)
{
	switch (c->nCommand)
	{
	case GetInfDeviceByte:
		break;
	case SetCommandMIPEXByte:
	case SetCommandUARTByte:
	case SetCommandSPIByte:
	{
		LogAnswer(c, 1);
		break;
	}
	}
	return 1;
}

LRESULT CLGDelayedCommandDlg::OnWmCameraError(WPARAM wp, LPARAM lp)
{
	D_O::SetWindowTextW(&m_TStatus,S_O::LoadStringW(IDS_CAMERAERROR));
	main->BreakAll(0, STARTALL_DCA);
	return 1;
}

LRESULT CLGDelayedCommandDlg::OnWmCameraStatusChanged(WPARAM wp, LPARAM lp)
{
	m_TSet.SetWindowText(S_O::FormatValue(t_proc.c->dT));
	if (t_proc.nStatus == TC_UNKNOWN)
	{
		m_T.SetWindowText("???");
		D_O::SetWindowTextW(&m_TStatus, S_O::LoadStringW(IDS_CONNECTIONERROR));
	}
	else
	{
		if (t_proc.dT != -1000)m_T.SetWindowText(S_O::FormatValue(t_proc.dT, 1));
		CString str;
		switch (t_proc.nStatus)
		{
		case TC_SLOPE:
		{
			COleDateTimeSpan ts = CDateTime::GetCurrent() - t_proc.c->tmStarted;
			str.Format("%s %s", S_O::LoadStringW(IDS_WAIT),CRealtimeObject::FormatTime(ts.GetTotalSeconds()));
			break;
		}
		case TC_WAIT:
		{
			COleDateTimeSpan ts = CDateTime::GetCurrent() - t_proc.c->tmPlateStarted;
			str.Format("%s %s", S_O::LoadStringW(IDS_TWAIT), CRealtimeObject::FormatTime(ts.GetTotalSeconds()));
			break;
		}
		case TC_GASSTARTED:
		{
			COleDateTimeSpan ts = CDateTime::GetCurrent() - t_proc.c->tmGasStarted;
			str.Format("%s %s", S_O::LoadStringW(IDS_GASPUFFING), CRealtimeObject::FormatTime(ts.GetTotalSeconds()));
			break;
		}
		case TC_DONE:
			str = S_O::LoadStringW(IDS_TOVER);
			break;
		}
		D_O::SetWindowTextW(&m_TStatus, str);
	}
	if (t_proc.nCurrentValve == 0)
	{
		D_O::SetWindowTextW(&m_CurrentValve, S_O::LoadStringW(IDS_OFF));
		m_CurrentConc.SetWindowText("");
	}
	else
	{
		m_CurrentValve.SetWindowText(S_O::FormatUINT(t_proc.nCurrentValve));
		m_CurrentConc.SetWindowText(S_O::FormatValue(t_proc.dCurrentConc));
	}
	t_proc.graph->FillExpectedCurve(&dca);
	return 1;
}

void CLGDelayedCommandDlg::ChangeAll(CNFC_Device* dev, UINT nMode, CHANGEALL* c)
{

}

void CLGDelayedCommandDlg::OnTabSelected()
{
	m_FilePath.SetWindowText(bSave ? S_O::Format("%s\\%s", strFolder, GetSaveFileName(nfcCurrent, 1)) : "");
	FillGrid();
	FillResultGrid();
}

bool CLGDelayedCommandDlg::UpdateMIPEXCommand(CNFC_Device* dev, CString str)
{
	if (!dev->UpdateMIPEXCommand(str))return 0;
	dev->additional_pars.nMipexDest = GetCheckedRadioButton(IDC_RADIO_MIPEX, IDC_RADIO_SPI) - IDC_RADIO_MIPEX;
	if (GetCheckedRadioButton(IDC_RADIO_D, IDC_RADIO_DA) == IDC_RADIO_D)dev->additional_pars.strMipexEnd = "\r";
	else dev->additional_pars.strMipexEnd = "\r\n";
	return 1;
}

void CLGDelayedCommandDlg::SaveDCA()
{
	bool bAdded = 0;
	if (dca.strName == "")
	{
		CInputNameDlg ind;
		ind.m_Title = S_O::LoadString(IDS_PROFILENAME);
		while (1)
		{
			if (ind.DoModal() == IDCANCEL)break;
			S_O::CorrectFileName(ind.m_Name);
			S_O::Trim(ind.m_Name);
			if (ind.m_Name == "")continue;
			if (D_O::FindStringNoCase(&m_Profile, ind.m_Name) != -1)
			{
				AfxMessageBox(S_O::LoadString(IDS_NAMEALREADYEXISTS));
				continue;
			}
			bAdded = 1;
			break;
		}
		if (bAdded)
		{
			dca.strName = ind.m_Name;
			D_O::AddStringWithId(&m_Profile, dca.strName, 0, 1);
		}
	}
	dca.RecalcTime();
	dca.Save();
	FillGrid();
}

bool CLGDelayedCommandDlg::GetCommandParam(DELAYED_COMMAND& c)
{
	CDelayedCommandEditor ed;
	ed.c = c;
	if (ed.DoModal() == IDCANCEL)return 0;
	c = ed.c;
	return 1;
}

void CLGDelayedCommandDlg::ClearIfWasMeasure()
{
	if (!nfcCurrent->dca.arr.GetSize())return;
	nfcCurrent->dca.arr.RemoveAll();
	FillGrid();
}

void CLGDelayedCommandDlg::OnBnClickedAdd()
{
	if (main->IsInDelayedMIPEX())return;
	ClearIfWasMeasure();
	DELAYED_COMMAND c;
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_ARRAY);
	int nR = grid->GetActiveRow();
	if (nR >= 0 && nR < dca.arr.GetSize())c = dca.arr[nR];
	if (!GetCommandParam(c))return;
	c.nCommandId = dca.nMaxId++;
	dca.arr.Add(c);
	SaveDCA();
}

void CLGDelayedCommandDlg::OnBnClickedMod()
{
	if (main->IsInDelayedMIPEX())return;
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_ARRAY);
	int nR = grid->GetActiveRow();
	if (nR < 0 || nR >= dca.arr.GetSize())return;
	ClearIfWasMeasure();
	DELAYED_COMMAND c = dca.arr[nR];
	if (!GetCommandParam(c))return;
	dca.arr[nR] = c;
	SaveDCA();
}

void CLGDelayedCommandDlg::OnBnClickedDel()
{
	if (main->IsInDelayedMIPEX())return;
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_ARRAY);
	int nR = grid->GetActiveRow();
	if (nR < 0 || nR >= dca.arr.GetSize())return;
	ClearIfWasMeasure();
	DELAYED_COMMAND& c = dca.arr[nR];
	CString str;
	str.Format("%s %s (%g)?", S_O::LoadString(IDS_DELETE), c.strCommand, c.dTimeDelay);
	if (AfxMessageBox(str, MB_Q) == IDNO)return;
	dca.arr.RemoveAt(nR);
	SaveDCA();
}


void CLGDelayedCommandDlg::OnBnClickedSend()
{
}


void CLGDelayedCommandDlg::OnBnClickedSendall()
{
	if (!t_proc.IsAllDone())
	{
		main->BreakAll(nfcCurrent, STARTALL_DCA);
		return;
	}
	if (!UpdateData())return;
	AfxGetApp()->WriteProfileString("DelayedCommandDlg", "LastCommand", m_Command);
	AfxGetApp()->WriteProfileString("DelayedCommandDlg", "RepeatTime", m_RepeatTime);
	AfxGetApp()->WriteProfileInt("DelayedCommandDlg", "MIPEXDest", GetCheckedRadioButton(IDC_RADIO_MIPEX, IDC_RADIO_SPI));
	AfxGetApp()->WriteProfileInt("DelayedCommandDlg", "MIPEXEnd", GetCheckedRadioButton(IDC_RADIO_D, IDC_RADIO_DA));
	S_O::Trim(m_Command);
	if (!dca.arr.GetSize() && (m_Command == "" || S_O::ToDouble(m_RepeatTime) <= 0))return;
	if (m_BkGas && m_BkGasConc <= 0)return;
	if (!t_proc.graph)
	{
		t_proc.graph = new CGraphDlg;
		t_proc.graph->Create(IDD_GRAPH, main);
		t_proc.graph->CenterWindow();
		t_proc.graph->gr->curve.clear();
		CCurve cc;
		cc.strCurveInfo = "Реальная температура";
		t_proc.graph->gr->curve.push_back(cc);
	}
	t_proc.graph->m_Rich.SetWindowText("");
	t_proc.graph->gr->curve[0].Clear();
	t_proc.StopCamera();
	t_proc.bBkGas = m_BkGas;
	t_proc.nBkValve = D_O::GetSelectedItemData(m_BkValve);
	t_proc.dBkConc = m_BkGasConc;
	t_proc.wNotify = this;
	t_proc.bInBk = 0;
	t_proc.tmAllStarted.SetToCurrent();
	AfxGetApp()->WriteProfileInt("DelayedCommandDlg", "BkGas", m_BkGas);
	theApp.WriteProfileDouble("DelayedCommandDlg", "BkGasConc", m_BkGasConc);
	AfxGetApp()->WriteProfileInt("DelayedCommandDlg", "BkValve", t_proc.nBkValve);
	GetFolderName(1);
	if (dca.arr.GetSize())
	{
		bool bTFound = 0, bGasFound = 0;
		for (int i = 0; i < dca.arr.GetSize(); i++)
		{
			DELAYED_COMMAND& c = dca.arr[i];
			c.tmPlateStarted.SetNull();
			c.tmStarted.SetNull();
			c.tmDone.SetNull();
			c.tmGasStarted.SetNull();
			c.nStatus = TC_UNKNOWN;
			if (c.bT)
			{
				if (g_pref.strCameraAddr == "")
				{
					AfxMessageBox("Надо задать адрес камеры");
					return;
				}
				bTFound = 1;
			}
			if (c.bGas)
			{
				if (!g_pref.b10ChanBKM && c.nValve > 4)
				{
					AfxMessageBox(S_O::Format("Номер клапана должен быть < 5 для позиции %d", i + 1));
					return;
				}
				if (g_pref.strGasAddr == "")
				{
					AfxMessageBox("Надо задать адрес БКМ");
					return;
				}
				bGasFound = 1;
			}
		}
		if (bTFound || bGasFound)
		{
			if (!t_proc.Open())return;
		}
		StartCamera(dca.arr[0]);
	}
	main->StartAll(STARTALL_DCA, nfcCurrent);
	dca.tmStarted.SetToCurrent();
	t_proc.graph->FillExpectedCurve(&dca);
}

void CLGDelayedCommandDlg::OnBnClickedClear()
{
	nfcCurrent->dcar.arr.RemoveAll();
	FillResultGrid();
}

void CLGDelayedCommandDlg::OnBnClickedOpenfolder()
{
	if (!bSave)return;
	D_O::OpenFolderInExplorer(strFolder, m_hWnd);
}


void CLGDelayedCommandDlg::CheckDelayedExecution(CNFC_Device* dev)
{
	if (t_proc.bStopping)return;
	COleDateTimeSpan s = CDateTime::GetCurrent() - dtStart;
	int n = s.GetTotalSeconds();
	m_Elapsed.SetWindowText(CRealtimeObject::FormatTime(n));
	if (dev->additional_pars.bInMIPEXSend)return;
	double dEvery = S_O::ToDouble(m_RepeatTime);
	DELAYED_COMMAND_ARRAY& dca = dev->dca;
	if (dev->nCurrentCommandId != -1)
	{
		for (int i = 0; i < dca.arr.GetSize(); i++)
		{
			DELAYED_COMMAND& cc = dca.arr[i];
			if (cc.nCommandId == dev->nCurrentCommandId)
			{
				if (cc.nStatus != DCAS_WAIT && dev->additional_pars.nMIPEXCommandId == dev->nCurrentCommandId)return;
				bool bAllow = 1;
				if (cc.bT || cc.bGas)
				{
					if (!cc.bTSet)
					{
						if (t_proc.nStatus == TC_DONE)
						{
							cc.bTSet = 1;
							cc.dTimeDelayFromStart = n;
							double d = cc.dTimeDelayFromStart;
							for (int k = i + 1; k < dca.arr.GetSize(); k++)
							{
								d += dca.arr[k].dTimeDelay;
								dca.arr[k].dTimeDelayFromStart = d;
							}
							if (cc.strCommand == "")
							{
								cc.nStatus = DCAS_DONE;
								t_proc.SetCommandStatus(dev, 1);
								if (i == (dca.arr.GetSize() - 1))
								{
									dev->nCurrentCommandId = -1;
									if (m_StopAfter || m_Command == "")
									{
										dev->additional_pars.nDelayedMIPEXResult = RESULT_GOOD;
										dev->OperationStatusChange(STARTALL_DCA, 1);
									}
								}

							}
							FillGrid();
						}
						else bAllow = 0;
					}
				}
				if (bAllow && cc.dTimeDelayFromStart <= n && cc.strCommand!="")
				{
					UpdateMIPEXCommand(dev,cc.strCommand);
					dev->additional_pars.nMIPEXCommandId = dev->nCurrentCommandId;
					dev->additional_pars.nSendMIPEXCommandTry = 0;
					dev->SendCommandMIPEX();
					cc.nStatus = DCAS_EXECUTE;
					if (dev == nfcCurrent)
					{
						CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_ARRAY);
						grid->InvalidateRow(i);
					}
					return;
				}
				break;
			}
		}
	}
	if (dEvery > 0)
	{
		if (dev->dtLastRepeat.IsNotNull())
		{
			s = CDateTime::GetCurrent() - dev->dtLastRepeat;
			n = s.GetTotalSeconds();
		}
		else n = dEvery + 1;
		if (n >= dEvery)
		{
			dev->dtLastRepeat = CDateTime::GetCurrent();
			if (!UpdateMIPEXCommand(dev,m_Command))return;
			dev->additional_pars.nMIPEXCommandId = 0;
			dev->additional_pars.nSendMIPEXCommandTry = 0;
			dev->SendCommandMIPEX();
		}

	}
}

void CLGDelayedCommandDlg::SelectNextCommand(CNFC_Device* dev,bool bStartCamera)
{
	DELAYED_COMMAND_ARRAY& dca = dev->dca;
	if (dev->nCurrentCommandId == -1 && !m_StopAfter && m_Command!="")
	{
		t_proc.SetCommandStatus(dev, 0);
		return;
	}
	for (int i = 0; i < dca.arr.GetSize(); i++)
	{
		DELAYED_COMMAND& cc = dca.arr[i];
		if (cc.nCommandId == dev->nCurrentCommandId)
		{
			COleDateTimeSpan s = CDateTime::GetCurrent() - dtStart;
			cc.dTimeDelayFromStart = s.GetTotalSeconds();
			double d = cc.dTimeDelayFromStart;
			for (int k = i + 1; k < dca.arr.GetSize(); k++)
			{
				d += dca.arr[k].dTimeDelay;
				dca.arr[k].dTimeDelayFromStart = d;
			}
			dev->nCurrentCommandId = dca.arr[i + 1].nCommandId;
			t_proc.SetCommandStatus(dev, 0);
			if (bStartCamera)
			{
				t_proc.c->tmDone.SetToCurrent();
				for (int k = 0; k < this->dca.arr.GetSize(); k++)
				{
					DELAYED_COMMAND& c = this->dca.arr[k];
					if (c.nCommandId == dev->nCurrentCommandId)
					{
						StartCamera(c);
						break;
					}
				}
			}
			break;
		}
	}
}

void CLGDelayedCommandDlg::StartCamera(DELAYED_COMMAND& c)
{
	t_proc.nStatus = TC_UNKNOWN;
	t_proc.StopCamera();
	if (!c.bT && !c.bGas && !m_BkGas)
	{
		c.tmStarted.SetToCurrent();
		return;
	}
	t_proc.c = &c;
	t_proc.nStatus = TC_UNKNOWN;
	t_proc.dT = -1000;
	t_proc.StartCamera();
}

void CLGDelayedCommandDlg::Start(CNFC_Device* dev)
{
	dev->dca = this->dca;
	DELAYED_COMMAND_ARRAY& dca = dev->dca;
	for (int i = 0; i < dca.arr.GetSize(); i++)
	{
		DELAYED_COMMAND& c = dca.arr[i];
		c.nStatus = DCAS_WAIT;
		c.bTSet = 0;
	}
	bShowStatus = dev->additional_pars.bInDelayedMIPEX = 1;
	dev->additional_pars.nDelayedMIPEXResult = 0;
	dev->dcar.arr.RemoveAll();
	DELAYED_COMMAND_RESULT r;
	r.tm = dtStart = CDateTime::GetCurrent();
	r.strDeviceId = dev->GetSerialNumber();
	r.strCommand = "Start";
	dev->dcar.arr.Add(r);
	if (bSave)
	{
		CString strPath;
		strPath.Format("%s\\%s", strFolder, GetSaveFileName(dev, 1));
		//m_FilePath.SetWindowText(strPath);
		SaveToFile(strPath, r);
	}
	dca.RecalcTime();
	if (dca.arr.GetSize())
	{
		dev->nCurrentCommandId = dca.arr[0].nCommandId;
	}
	FillGrid();
	FillResultGrid();
	dev->dtLastRepeat.SetNull();
	m_Elapsed.SetWindowText("");
	D_O::SetWindowTextW(GetDlgItem(IDC_SENDALL), S_O::LoadStringW(IDS_STOP));
	m_Profile.EnableWindow(0);
}

void CLGDelayedCommandDlg::Stop(CNFC_Device* dev,bool bStopCamera)
{
	if (bStopCamera)
	{
		t_proc.c->tmDone.SetToCurrent();
		t_proc.StopCamera();
		if (m_BkOffAfter)t_proc.OffGas();
		if (t_proc.IsCameraOpened() && g_pref.dRoomTemp!=-100)t_proc.SetTemp(g_pref.dRoomTemp);
	}
	dev->additional_pars.bInDelayedMIPEX = 0;
	t_proc.SetCommandStatus(dev, 1);
	if (dev == nfcCurrent)
	{
		D_O::SetWindowTextW(GetDlgItem(IDC_SENDALL), S_O::LoadStringW(IDS_STARTALL));
		D_O::SetWindowTextW(&m_TStatus,S_O::LoadStringW(IDS_STOPPED));
		m_Profile.EnableWindow(1);
	}
}

void CLGDelayedCommandDlg::OnBnClickedModProfile()
{
	if (main->IsInDelayedMIPEX())return;
	int ns = m_Profile.GetCurSel();
	UINT nd = m_Profile.GetItemData(ns);
	if (nd == -1)return;
	CString str = D_O::GetWindowText(&m_Profile);
	CInputNameDlg ind;
	ind.m_Title = S_O::LoadString(IDS_PROFILENAME);
	ind.m_Name = str;
	while (1)
	{
		if (ind.DoModal() == IDCANCEL)return;
		S_O::CorrectFileName(ind.m_Name);
		S_O::Trim(ind.m_Name);
		if (ind.m_Name == "" || ind.m_Name==str)continue;
		if (D_O::FindStringNoCase(&m_Profile, ind.m_Name) != -1)
		{
			AfxMessageBox(S_O::LoadString(IDS_NAMEALREADYEXISTS));
			continue;
		}
		break;
	}
	CString strOld, strNew;
	strOld.Format("%s\\%s.dca", dca.GetProfileFolder(), str);
	strNew.Format("%s\\%s.dca", dca.GetProfileFolder(), ind.m_Name);
	if (!CopyFile(strOld, strNew, 1))return;
	DeleteFile(strOld);
	m_Profile.DeleteString(ns);
	D_O::AddStringWithId(&m_Profile, ind.m_Name, 0, 1);
	ProfileChanged(1);
}

void CLGDelayedCommandDlg::OnBnClickedDelProfile()
{
	if (main->IsInDelayedMIPEX())return;
	int ns = m_Profile.GetCurSel();
	UINT nd = m_Profile.GetItemData(ns);
	if (nd == -1)return;
	CString str = D_O::GetWindowText(&m_Profile);
	if (AfxMessageBox(S_O::Format("%s %s?", S_O::LoadString(IDS_DELETE), str), MB_Q) == IDNO)return;
	::DeleteFile(S_O::Format("%s\\%s.dca", dca.GetProfileFolder(), str));
	m_Profile.DeleteString(ns);
	m_Profile.SetCurSel(0);
	ProfileChanged(1);
}

void CLGDelayedCommandDlg::ProfileChanged(bool bSendAll)
{
	int ns = m_Profile.GetCurSel();
	UINT nd = m_Profile.GetItemData(ns);
	if (nd == -1)
	{
		dca.strName = "";
	}
	else
	{
		dca.strName = D_O::GetWindowText(&m_Profile);
		dca.Load();
	}
	FillGrid();
}

void CLGDelayedCommandDlg::OnCbnSelchangeProfile()
{
	bProfileSelected = 1;
	ProfileChanged(1);
}

void CLGDelayedCommandDlg::OnDestroy()
{
	if(bProfileSelected)AfxGetApp()->WriteProfileString("DelayedCommandDlg", "LastProfile", D_O::GetWindowText(&m_Profile));
	CLongGasBaseDlg::OnDestroy();
}

void CLGDelayedCommandDlg::OnBnClickedMenu()
{
	if (main->IsInDelayedMIPEX())return;
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_ARRAY);
	int nR = grid->GetActiveRow();
	CMenu m;
	m.CreatePopupMenu();
	if (nR >= 0 && nR < dca.arr.GetSize())
	{
		m.AppendMenuA(MF_STRING, 1, S_O::LoadString(IDS_UP));
		m.AppendMenuA(MF_STRING, 2, S_O::LoadString(IDS_DOWN));
		m.AppendMenu(MF_SEPARATOR, 0);
		m.AppendMenuA(MF_STRING, 3, S_O::LoadString(IDS_DUBLICATE));
		m.AppendMenu(MF_SEPARATOR, 0);
	}
	m.AppendMenuA(MF_STRING, 4, S_O::LoadString(IDS_REMOVEALL));
	CPoint pt;
	GetCursorPos(&pt);
	int res = m.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this, 0);
	if (!res)return;
	ClearIfWasMeasure();
	DELAYED_COMMAND c;
	int nSel = -1;
	if (res == 1)
	{
		if (nR == 0)return;
		c = dca.arr[nR - 1];
		dca.arr[nR - 1] = dca.arr[nR];
		dca.arr[nR] = c;
		nSel = nR - 1;
	}
	else if (res == 2)
	{
		if (nR == dca.arr.GetSize() - 1)return;
		c = dca.arr[nR + 1];
		dca.arr[nR + 1] = dca.arr[nR];
		dca.arr[nR] = c;
		nSel = nR + 1;
	}
	else if (res == 3)
	{
		c = dca.arr[nR];
		c.nCommandId = dca.nMaxId++;
		dca.arr.InsertAt(nR + 1, c);
		nSel = nR + 1;
	}
	else if (res == 4)
	{
		if (AfxMessageBox(S_O::Format("%s?", S_O::LoadString(IDS_REMOVEALL)), MB_Q) == IDNO)return;
		dca.arr.RemoveAll();
	}
	SaveDCA();
	if (nSel != -1)grid->SetActiveRow(nSel);
}

bool CLGDelayedCommandDlg::OperationEnded(CNFC_Device* nfc, UINT nType)
{
	if (nType == STARTALL_DCA)Stop(nfc, 0);
	return 1;
}

IMPLEMENT_DYNAMIC(CDelayedCommandEditor, CDialogEx)

CDelayedCommandEditor::CDelayedCommandEditor(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DELAYEDCOMMANDEDITOR, pParent)
	, m_Command(_T(""))
	, m_FromPrev(0)
	, m_T(0)
	, m_bT(FALSE)
	, m_TTime(0)
{
}

CDelayedCommandEditor::~CDelayedCommandEditor()
{
}

void CDelayedCommandEditor::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_COMMAND, m_Command);
	DDX_Text(pDX, IDC_FROMPREV, m_FromPrev);
	DDX_Text(pDX, IDC_T, m_T);
	DDX_Check(pDX, IDC_CHECK1, m_bT);
	DDX_Text(pDX, IDC_TTIME, m_TTime);
	DDX_Check(pDX, IDC_GAS, m_bGas);
	DDX_Text(pDX, IDC_CONC, m_GasConc);
	DDX_Text(pDX, IDC_GASTIME, m_GasTime);
	DDX_Control(pDX, IDC_VALVE, m_Valve);
}


BEGIN_MESSAGE_MAP(CDelayedCommandEditor, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDelayedCommandEditor::OnBnClickedOk)
END_MESSAGE_MAP()


// CDelayedCommandEditor message handlers


BOOL CDelayedCommandEditor::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	TRANSLATION::TranslateWindow(this);
	for (int i = 1; i <= (g_pref.b10ChanBKM ? 10 : 4); i++)
		D_O::AddStringWithId(&m_Valve, S_O::FormatUINT(i), i,i==c.nValve);
	m_Command = c.strCommand;
	m_FromPrev = c.dTimeDelay;
	m_bT = c.bT;
	m_T = c.dT;
	m_TTime = c.dTTime;
	m_bGas = c.bGas;
	m_GasConc = c.dGasConc;
	m_GasTime = c.dGasTime;
	UpdateData(0);
	return TRUE;
}


void CDelayedCommandEditor::OnBnClickedOk()
{
	UpdateData();
	S_O::Trim(m_Command);
	if (m_FromPrev < 0 || m_TTime < 0)return;
	c.strCommand = m_Command;
	c.dTimeDelay = m_FromPrev;
	c.bT = m_bT;
	c.dT = m_T;
	c.dTTime = m_TTime;
	c.bGas = m_bGas;
	c.dGasConc = m_GasConc;
	c.dGasTime = m_GasTime;
	c.nValve = D_O::GetSelectedItemData(m_Valve);
	CDialogEx::OnOK();
}

bool TERMOCAMERA_PROC::Open()
{
	gas.Close();
	if (camera)
	{
		camera->Close();
		delete camera;
		camera = 0;
	}
	if (g_pref.strGasAddr != "")
	{
		if (!gas.Open(g_pref.strGasAddr))
		{
			AfxMessageBox("Ошибка соединения с БКМ");
			return 0;
		}
		if (!SendGasCommand("&A1", 1))
		//if (!SendGasCommand("at+send=lorap2p:015C1904200110010000005D1422\r\n", 1))
		{
			AfxMessageBox("Ошибка инициализации БКМ");
			return 0;
		}
	}
	if (g_pref.strCameraAddr != "")
	{
		if (g_pref.nCameraType == CAMERA_BINDER)camera=new CBinderTCamera;
		else if (g_pref.nCameraType == CAMERA_TESTA)camera = new CTestaTCamera;
		else if(g_pref.nCameraType== CAMERA_BINDERMODBUS)camera = new CModbusTCamera;
		if (!camera->Open(g_pref.strCameraAddr))
		{
			AfxMessageBox("Ошибка соединения с термокамерой");
			return 0;
		}
	}
	return 1;
}

bool TERMOCAMERA_PROC::StartCamera()
{
	StopCamera();
	bRunning = 1;
	DWORD id;
	evBreak.ResetEvent();
	CreateThread(NULL, 0, CameraThread, this, 0, &id);
	return 1;
}

void TERMOCAMERA_PROC::StopCamera()
{
	if (!bRunning)return;
	bStopping = 1;
	bool bWaitNotRunning = 0;
	if (!bWaitingForGasOff)evBreak.SetEvent();
	else
	{
		bGasOffAllowed = 1;
		bWaitNotRunning = 1;
	}
	wNotify->EnableWindow(0);
	while (1)
	{
		if (bWaitNotRunning)
		{
			if (!bRunning)break;
		}
		else if (WaitForSingleObject(evBreak.m_hObject, 0) != WAIT_OBJECT_0)break;
		if (!CMainAppTemplate::MessageQueue())Sleep(100);
	}
	bStopping = 0;
	wNotify->EnableWindow(1);
	bRunning = 0;
}

DWORD WINAPI TERMOCAMERA_PROC::CameraThread(void* ptr)
{
	TERMOCAMERA_PROC* dev = (TERMOCAMERA_PROC*)ptr;
	dev->c->tmStarted = CDateTime::GetCurrent();
	bool bSet = 0, bInInterval = 0;
	double d = 0, dTWait = dev->c->dTTime * 60,dPrev=-1000;
	if (dev->bBkGas && !dev->bInBk && (dev->c->bT || !dev->c->bGas))
	{
		dev->StartGas(dev->nBkValve, dev->dBkConc, 9000);
		dev->bInBk = 1;
	}
	int nMode = AfxGetApp()->GetProfileInt("DelayedCommandDlg", "CameraMode", 0);
	dev->bWaitingForGasOff = 0;
	if (dev->c->bT)
	{
		while (WaitForSingleObject(dev->evBreak.m_hObject, 0) != WAIT_OBJECT_0)
		{
			if (!bSet)
			{
				if (dev->SetTemp(dev->c->dT))
				{
					bSet = 1;
					if (dev->nStatus != TC_WAIT)dev->nStatus = TC_SLOPE;
				}
				else
				{
					dev->nStatus = TC_UNKNOWN;
				}
			}
			else
			{
				if (dev->GetTemp(d))
				{
					bSet = !dev->camera->IsNeedSetTemp();
					dev->dT = d;
					bool bIn = 0;
					if (nMode == 0)
					{
						bIn = d >= (dev->c->dT - g_pref.dCameraInterval) && d <= (dev->c->dT + g_pref.dCameraInterval);
					}
					else
					{
						if (bInInterval)bIn = 1;
						else
						{
							if (dPrev != -1000)
							{
								if ((dPrev <= dev->c->dT && d >= dev->c->dT) || (dPrev >= dev->c->dT && d <= dev->c->dT))
								{
									bIn = 1;
								}
							}
							dPrev = d;
						}
					}
					if (!bInInterval && g_pref.dCameraMaxTime>0)
					{
						COleDateTimeSpan ts = CDateTime::GetCurrent() - dev->c->tmStarted;
						if (ts.GetTotalSeconds() > g_pref.dCameraMaxTime*60)
						{
							WriteLog(S_O::Format("Камера не выходит в интервал в течении %s", CRealtimeObject::FormatTime(ts.GetTotalSeconds())), 1);
							dev->SetTemp(20);
							dev->OffGas();
							dev->bRunning = 0;
							dev->wNotify->PostMessage(WM_CAMERAERROR, (WPARAM)dev);
							return 0;
						}
					}
					if (bIn)
					{
						if (!bInInterval)bInInterval = 1;
						dev->nStatus = TC_WAIT;
						if (dev->c->tmPlateStarted.IsNull())
						{
							dev->c->tmPlateStarted.SetToCurrent();
						}
						else
						{
							COleDateTimeSpan ts = CDateTime::GetCurrent() - dev->c->tmPlateStarted;
							if (ts.GetTotalSeconds() >= dTWait)
							{
								if (nMode == 1)
								{
									bIn = d >= (dev->c->dT - g_pref.dCameraInterval) && d <= (dev->c->dT + g_pref.dCameraInterval);
									if (!bIn && g_pref.dCameraAddTime>0)
									{
										dTWait += g_pref.dCameraAddTime * 60;
										WriteLog(S_O::Format("Выдержка продлена на %s (T=%g)", CRealtimeObject::FormatTime(g_pref.dCameraAddTime * 60),d), 1);
										dev->wNotify->PostMessage(WM_CAMERASTATUSCHANGED, (WPARAM)dev);
										//bSet = 0;
										continue;
									}
								}
								if (dev->c->bGas)
								{
									dev->nStatus = TC_GASSTARTED;
									dev->c->tmGasStarted.SetToCurrent();
								}
								else
								{
									dev->nStatus = TC_DONE;
									dev->c->tmDone.SetToCurrent();
								}
								dev->wNotify->PostMessage(WM_CAMERASTATUSCHANGED, (WPARAM)dev);
								break;
							}
						}
					}
					else
					{
						if (nMode == 0)
						{
							dev->c->tmPlateStarted.SetNull();
							dev->nStatus = TC_SLOPE;
						}
					}
				}
				else dev->nStatus = TC_UNKNOWN;
			}
			if (dev->wNotify)dev->wNotify->PostMessage(WM_CAMERASTATUSCHANGED, (WPARAM)dev);
			Sleep(1000);
		}
	}
	if (dev->c->bGas)
	{
		bool bStarted = 0;
		dev->bGasOffAllowed = dev->c->strCommand == "";
		CDateTime tmGasAdded;
		UINT nGasTime = dev->c->dGasTime + ((dev->c->strCommand != "") ? 5 * 60 : 0);
		while (WaitForSingleObject(dev->evBreak.m_hObject, 0) != WAIT_OBJECT_0)
		{
			if (!bStarted)
			{
				dev->StartGas(dev->c->nValve, dev->c->dGasConc, nGasTime);
				tmGasAdded = dev->c->tmGasStarted = CDateTime::GetCurrent();
				bStarted = 1;
				dev->bInBk = 0;
			}
			COleDateTimeSpan ts = CDateTime::GetCurrent() - dev->c->tmGasStarted;
			int n = ts.GetTotalSeconds();
			if (dev->GetTemp(d))
			{
				if (n < nGasTime)dev->nStatus = TC_GASSTARTED;
			}
			else dev->nStatus = TC_UNKNOWN;
			if (n >= dev->c->dGasTime)
			{
				dev->nStatus = TC_DONE;
				dev->bWaitingForGasOff = 1;
				if (dev->bGasOffAllowed)
				{
					if (dev->bBkGas)
					{
						dev->StartGas(dev->nBkValve, dev->dBkConc, 9000);
						dev->bInBk = 1;
					}
					else
					{
						dev->CloseValve();
					}
					if (dev->wNotify)dev->wNotify->PostMessage(WM_CAMERASTATUSCHANGED, (WPARAM)dev);
					break;
				}
				else
				{
					ts = CDateTime::GetCurrent() - tmGasAdded;
					if (ts.GetTotalSeconds() > (nGasTime - 60))
					{
						nGasTime = 5 * 60;
						dev->StartGas(dev->c->nValve, dev->c->dGasConc, nGasTime);
						tmGasAdded.SetToCurrent();
					}
				}
			}
			if (dev->wNotify)dev->wNotify->PostMessage(WM_CAMERASTATUSCHANGED, (WPARAM)dev);
			Sleep(1000);
		}
	}

	dev->c->tmDone.SetToCurrent();
	if (WaitForSingleObject(dev->evBreak.m_hObject, 0) == WAIT_OBJECT_0)
	{
		dev->OffGas();
	}
	dev->bWaitingForGasOff = 0;
	dev->bRunning = 0;
	dev->evBreak.ResetEvent();
	return 0;
}

bool TERMOCAMERA_PROC::SendGasCommand(CString str, bool bInitialization)
{
	CString strRead;
	str += "\r";
	while (1)
	{
		gas.Write(str);
		if (!bInitialization)
		{
			Sleep(350);
			break;
		}
#ifdef _DEBUG
		return 1;
#endif

		if (!gas.Read(strRead))return 0;
		if (strRead.Find("@A1") != -1)break;
		if (strRead.Find("@ERROR") != -1)continue;
		return 0;
	}
	return 1;
}

bool TERMOCAMERA_PROC::OffGas()
{
	if (!gas.IsOpened())return 1;
	SendGasCommand("&G0");
	SendGasCommand("&V0");
	WriteLog(CString("Отключение подачи газа"));
	nCurrentValve = 0;
	return 1;
}

bool TERMOCAMERA_PROC::CloseValve()
{
	if (!gas.IsOpened())return 1;
	SendGasCommand("&V0");
	nCurrentValve = 0;
	WriteLog(CString("Закрытие клапана"));
	return 1;
}

bool TERMOCAMERA_PROC::StartGas(UINT nValve, UINT nConc, UINT nTime)
{
	SendGasCommand("&G0");
	SendGasCommand("&V0");
	SendGasCommand(S_O::Format("&T%u", nTime));
	SendGasCommand(S_O::Format("&V%u", nValve));
	SendGasCommand(S_O::Format("&S%u", nConc));
	SendGasCommand("&G1");
	TERMOCAMERA_PROC::WriteLog(S_O::Format("Открытие клапана %u,Расход %u, Время %s", nValve, nConc, CRealtimeObject::FormatTime(nTime)));
	dCurrentConc = nConc;
	nCurrentValve = nValve;
	return 1;
}

#include "crctables.h"
#include "CCameraSettingsDlg.h"

WORD  TERMOCAMERA_PROC::CalcCheckSum(CString& str)
{
	if (g_pref.nCameraType == CAMERA_2500)
	{
		BYTE sum = 0;
		for (int i = 1; i < str.GetLength(); i++)
		{
			sum += str.GetAt(i);
		}
		return sum;
	}
	return 0;
}

bool TERMOCAMERA_PROC::CheckChecksum(CString& str)
{
	if (g_pref.nCameraType == CAMERA_2500)
	{
		if (str.GetLength() < 4)return 0;
		CString s = str.Left(str.GetLength() - 4), crc = str.Mid(str.GetLength() - 4, 2);
		BYTE bt = CalcCheckSum(s);
		if (S_O::Format("%02X", (UINT)bt) != crc)
		{
			s.Format("Wrong checksum: %s", str);
			CLogFile::Write(s);
			return 0;
		}
	}
	return 1;
}

bool TERMOCAMERA_PROC::SetTemp(double dTemp)
{
	WriteLog(S_O::Format("Установка температуры %g", dTemp));
	CLogFile::Write(S_O::Format("SetT=%g", dTemp));

#ifdef _DEBUG
	return 1;
#endif


	/*CString str, read;
	if (nCameraType == CAMERA_2500)
	{
		short v = dTemp * 10;
		read.Format("%04X", (UINT)v);
		str.Format("\x02%02XWRD,01,0104,%s", (UINT)btSlaveId, read.Right(4));
		BYTE btCheckSum = CalcCheckSum(str);
		str += S_O::Format("%02X\r\n", (UINT)btCheckSum);
		if (!camera.Write(str))return 0;
		if (!camera.Read(read))return 0;
		if (!CheckChecksum(read))return 0;
	}*/
	if (!camera->SetTemp(dTemp))
	{
		WriteLog(S_O::Format("Ошибка установки температуры %g", dTemp), 1);
		return 0;
	}
	return 1;
}

unsigned char reverse(unsigned char b) {
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

bool TERMOCAMERA_PROC::GetTemp(double& dTemp)
{
#ifdef _DEBUG
	dTemp = c->dT + ((double)rand() / 5 / RAND_MAX);
//	if (!camera->GetTemp(dTemp))return 0;
#else
/*	CString str, read;
	if (nCameraType == CAMERA_2500)
	{
		str.Format("\x02%02XRSD,03,0001", (UINT)btSlaveId);
		BYTE btCheckSum = CalcCheckSum(str);
		str += S_O::Format("%02X\r\n", (UINT)btCheckSum);
		if (!camera.Write(str))return 0;
		if (!camera.Read(read))return 0;
		if (!CheckChecksum(read))return 0;
		CStringArray a;
		S_O::ParseString(read, a, ",");
		if (a.GetSize() < 3 || a[2].GetLength()!=4)return 0;
		BYTE b[2];
		S_O::ConvertFromHex(a[2], b);
		short v = (b[0] << 8) | b[1];
		dTemp = v;
		dTemp /= 10;
	}*/
	if (!camera->GetTemp(dTemp))return 0;
#endif
	CLogFile::Write(S_O::Format("T=%g", dTemp));
	CMyPoint pt;
	pt.x = CDateTime::GetCurrent().GetUINT();
	pt.y = dTemp;
	graph->gr->curve[0].vData.push_back(pt);
	//graph->gr->AddPointToEndOfCurve(graph->gr->curve[0], pt, 1, 300);
	return 1;
}

void TERMOCAMERA_PROC::SetCommandStatus(CNFC_Device* nfc, UINT nStatus)
{
	for (int i = 0; i < done.GetSize(); i++)
	{
		if (done[i].nfc == nfc)
		{
			done[i].nStatus = nStatus;
			return;
		}
	}
}

bool TERMOCAMERA_PROC::IsAllDone()
{
	for (int i = 0; i < done.GetSize(); i++)
	{
		if (!done[i].nfc->IsDeviceInstalled())continue;
		if (!done[i].nStatus)return 0;
	}
	return 1;
}

bool TERMOCAMERA_PROC::IsDone(CNFC_Device* nfc)
{
	for (int i = 0; i < done.GetSize(); i++)
	{
		if (done[i].nfc != nfc)continue;
		return done[i].nStatus;
	}
	return 1;
}

TERMOCAMERA_PROC::TERMOCAMERA_PROC() :evBreak(0, 1)
{
	nStatus = TC_UNKNOWN;
	bRunning = 0;
	wNotify = 0;
	bGasOffAllowed = 0;
	bBkGas = 0;
	dBkConc = 10;
	nBkValve = 4;
	bInBk = 0;
	bWaitingForGasOff = 0;
	graph = 0;
	camera = 0;
	dT = -1000;
	c = 0;
	nCurrentValve = 0;
	dCurrentConc = 0;
	bStopping = 0;
}

BOOL CLGDelayedCommandDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_LBUTTONDOWN)
	{
		if (D_O::IsCursorOnWindow(&m_T))
		{
			if (t_proc.graph)t_proc.graph->ShowWindow(SW_SHOW);
			return 1;
		}
	}
	return CLongGasBaseDlg::PreTranslateMessage(pMsg);
}

bool CBinderTCamera::Open(CString strCameraAddr)
{
	bool bUDP = 0, bBind = 1;
	if (strCameraAddr.Find(":") == -1)strCameraAddr += ":10001";
	camera.bTryReadAll = 1;
	return camera.Open(strCameraAddr);
}

bool CBinderTCamera::SetTemp(double dTemp)
{
	CString str,read;
	for (int nRep = 0; nRep < 3; nRep++)
	{
		BYTE n[11] = { (BYTE)g_pref.nCameraSlaveId ,0x10 ,0x15 ,0x81 ,0x00 ,0x02,0x04,0,0,0,0 };
		float t = dTemp;
		BYTE* bt = (BYTE*)&t;
		n[7] = bt[1];
		n[8] = bt[0];
		n[9] = bt[3];
		n[10] = bt[2];
		str = S_O::MakeString(n, sizeof(n) / sizeof(BYTE));
		if (!WriteRead(str, read, 8, "SetT"))continue;
		if (read.GetLength() != 8 || read.GetAt(1) != 0x10 || read.GetAt(2) != 0x15 || (BYTE)read.GetAt(3) != 0x81)
		{
			str.Format("Binder: SetT: Wrong Reply: %s", S_O::ConvertToHex(read));
			CLogFile::Write(str);
			continue;
		}
		return 1;
	}
	return 0;
}

void CBinderTCamera::CheckHumidityControl(double dTemp)
{
	int needToTurnOnHumidityControl = nHumidityControlIsOn;
	if (dTemp > -6) {
		needToTurnOnHumidityControl = true;
	}
	else if (dTemp < -12) {
		needToTurnOnHumidityControl = false;
	}
	if (nHumidityControlIsOn != needToTurnOnHumidityControl) {
		CLogFile::Write(S_O::Format("Binder: Set HC %d", needToTurnOnHumidityControl));
		BYTE n[] = { (BYTE)g_pref.nCameraSlaveId ,0x06 ,0x15 ,0x8b ,0x00 ,(BYTE)(needToTurnOnHumidityControl?0x81:0)};
		CString str = S_O::MakeString(n, sizeof(n) / sizeof(BYTE)),read;
		if (!WriteRead(str, read, 8, "SetHC"))return;
		if (read.GetLength() != 8 || read.GetAt(1) != 0x06 || read.GetAt(2) != 0x15 || (BYTE)read.GetAt(3) != 0x8b)
		{
			str.Format("Binder: SetHC: Wrong Reply: %s", S_O::ConvertToHex(read));
			CLogFile::Write(str);
			return;
		}
		nHumidityControlIsOn = needToTurnOnHumidityControl;
	}

}

bool CBinderTCamera::WriteRead(CString& str, CString& read, int nSize, CString strDescr)
{
	WORD w = CalcCheckSum(str);
	BYTE* b = (BYTE*)read.GetBufferSetLength(2);
	b[0] = HIBYTE(w);
	b[1] = LOBYTE(w);
	read.ReleaseBuffer(2);
	S_O::AppendBinary(str, read);
	if (!camera.Write(str))
	{
		str.Format("Binder: %s: Error write", strDescr);
		CLogFile::Write(str);
		return 0;
	}
	if (!camera.Read(read, nSize))
	{
		str.Format("Binder: %s: Error read", strDescr);
		CLogFile::Write(str);
		return 0;
	}
	if (!CheckChecksum(read))
	{
		str.Format("Binder: %s: Wrong checksum: %s", strDescr, S_O::ConvertToHex(read));
		CLogFile::Write(str);
		return 0;
	}
	return 1;
}

bool CBinderTCamera::GetTemp(double& dTemp)
{
	dTemp = 0;
	CString str, read;
	for (int nRep = 0; nRep < 3; nRep++)
	{
		if (nRep)Sleep(500);
		BYTE n[] = { (BYTE)g_pref.nCameraSlaveId ,0x03 ,0x11 ,0xA9 ,0x00 ,0x02 };
		str = S_O::MakeString(n, sizeof(n) / sizeof(BYTE));
		if (!WriteRead(str, read, 9, "GetT"))continue;
		if (read.GetLength() > 9)read.Delete(9, read.GetLength() - 9);
		if (read.GetLength() != 9 || read.GetAt(1) != 0x03)
		{
			str.Format("Binder: GetT: Wrong reply: %s", S_O::ConvertToHex(read));
			CLogFile::Write(str);
			continue;
		}
		float number;
		BYTE* ch = (BYTE*)&number;
		ch[0] = read.GetAt(4);
		ch[1] = read.GetAt(3);
		ch[2] = read.GetAt(6);
		ch[3] = read.GetAt(5);
		dTemp = number;
		CheckHumidityControl(dTemp);
		return 1;
	}
	return 0;
}

WORD  CBinderTCamera::CalcCheckSum(CString& str)
{
	BYTE highByte = 0xFF;
	BYTE lowByte = 0xFF;
	unsigned int tablePosition = 0;

	for (auto i = 0; i < str.GetLength(); ++i) {
		tablePosition = highByte ^ (BYTE)str.GetAt(i);
		highByte = lowByte ^ crcTableHigh[tablePosition];
		lowByte = crcTableLow[tablePosition];
	}
	return (highByte << 8) | lowByte;
}

bool CBinderTCamera::CheckChecksum(CString& str)
{
	if (str.GetLength() < 2)return 0;
	CString s = str.Left(str.GetLength() - 2), crc = str.Mid(str.GetLength() - 2, 2);
	WORD w = CalcCheckSum(s), c = ((BYTE)crc.GetAt(0) << 8) | (BYTE)crc.GetAt(1);
	return w==c;
}

bool CTestaTCamera::Open(CString strCameraAddr)
{
	if (strCameraAddr.Find(":") == -1)strCameraAddr += ":1300";
	if (!camera.Open(strCameraAddr, 1, 0))return 0;
	CStringArray a;
	S_O::ParseString(strCameraAddr, a, ":");
	cameraUDPRcv.bTryReadAll = 1;
	return cameraUDPRcv.Open(a[0] + ":1200", 1);
}

bool CTestaTCamera::SetTemp(double dTemp)
{
	BYTE btSend[32] = { 0x02,0x33,0x88,0x66};
	for (int i = 4; i < 32; i++)btSend[i] = 0;
	float f = dTemp;
	memcpy(&btSend[12], &f, 4);
	CString str = S_O::MakeString(btSend, 32);
	camera.Write(str);
	dTSet = dTemp;
	return 1;
}

bool CTestaTCamera::IsNeedSetTemp()
{
	if (dTSet == -1000)return 1;
	bool b = dTSet != dTSetRead;
	if (b)CLogFile::Write(S_O::Format("TSet!=TRead (%g!=%g)", dTSet , dTSetRead));
	return b;
}

bool CTestaTCamera::GetTemp(double& dTemp)
{
	dTemp = 0;
	CString str, read;
	for (int i = 0; i < 3; i++)
	{
		cameraUDPRcv.Clear();
		if (!cameraUDPRcv.Read(read,40))
		{
			str.Format("Testa: GetT: Error read");
			CLogFile::Write(str);
			continue;
		}
		if (read.GetLength() != 40)
		{
			str.Format("Testa: GetT: Wrong datagram size: %d, %s", read.GetLength(), S_O::ConvertToHex(read));
			CLogFile::Write(str);
			continue;
		}
		if (read.GetAt(0) != 0x11 || read.GetAt(1) != 0x22 || read.GetAt(2) != 0 || read.GetAt(3) != 0)
		{
			str.Format("Testa: GetT: Wrong datagram: %s", S_O::ConvertToHex(read));
			CLogFile::Write(str);
			continue;
		}
		WORD w = (BYTE)read.GetAt(4) | ((BYTE)read.GetAt(5) << 8);
		short t = w;
		dTemp = t;
		dTemp /= 100;
		w = (BYTE)read.GetAt(6) | ((BYTE)read.GetAt(7) << 8);
		t = w;
		dTSetRead = t;
		dTSetRead /= 100;
		return 1;
	}
	return 0;
}

void CLGDelayedCommandDlg::OnBnClickedCamerasettings()
{
	CCameraSettingsDlg cs;
	if (cs.DoModal() == IDCANCEL)return;
	UINT nId = D_O::GetSelectedItemData(m_BkValve);
	m_BkValve.ResetContent();
	for (int i = 1; i <= (g_pref.b10ChanBKM ? 10 : 4); i++)
		D_O::AddStringWithId(&m_BkValve, S_O::FormatUINT(i), i, i == nId);
	if (D_O::GetSelectedItemData(m_BkValve) == -1)m_BkValve.SetCurSel(0);

}


void CLGDelayedCommandDlg::OnBnClickedOffafter()
{
	UpdateData();
	AfxGetApp()->WriteProfileInt("DelayedCommandDlg", "BkOffAfter", m_BkOffAfter);
}

bool TERMOCAMERA_PROC::WriteLog(CString& str, bool bError)
{
	if (!wNotify)return 0;
	CString* s = new CString;
	*s = str;
	wNotify->PostMessage(WM_CAMERALOG, (WPARAM)s, bError);
	return 1;
}

LRESULT CLGDelayedCommandDlg::OnWmCameraLog(WPARAM wp, LPARAM lp)
{
	CString* str = (CString*)wp;
	*str += "\n";
	D_O::WriteLog(t_proc.graph->m_Rich, S_O::Format("%s - ",CDateTime::GetCurrent().FormatStandard(1,0)));
	D_O::WriteLog(t_proc.graph->m_Rich, *str, lp?0xff:0xff0000, 0, 1);
	delete str;
	return 1;
}

void CLGDelayedCommandDlg::OnBnClickedStopallafter()
{
	UpdateData();
	AfxGetApp()->WriteProfileInt("DelayedCommandDlg", "StopAfter", m_StopAfter);
}

CModbusTCamera::CModbusTCamera()
{

}

bool CModbusTCamera::Open(CString strAddr)
{
	strAddr.Replace("COM", "");
	if (!CModbusDevice::OpenPort(S_O::ToUINT(strAddr), 9600, g_pref.nCameraSlaveId))return 0;
	TurnOn(1);
	return 1;
}

void CModbusTCamera::LogError()
{
	CString err = modbus_strerror(errno);
	CLogFile::Write(S_O::Format("Modbus error: %s",err));
}

#define REG_TEMP        12//10
#define REG_SET_TEMP    100//60
#define REG_SET_MOD     105//63


bool CModbusTCamera::SetTemp(double dTemp)
{
	if (!ctx)return 0;
	int16_t u = dTemp * 100, u1 = 0;
	try
	{
		if (modbus_write_registers(ctx, nSlaveAddress, REG_SET_TEMP, 1, (uint16_t*)&u) == -1)throw 0;
		if (modbus_read_registers(ctx, nSlaveAddress, REG_SET_TEMP, 1, (uint16_t*)&u1) == -1)throw 0;
		if (u != u1)
		{
			CLogFile::Write(S_O::Format("Modbus: Set!=Get (%d!=%d)", (int)u, (int)u1));
		}
		return 1;
	}
	catch (int)
	{
		LogError();
	}
	return 0;
}

bool CModbusTCamera::GetTemp(double& dTemp)
{
	if (!ctx)return 0;
	int16_t u = 0;
	if (modbus_read_registers(ctx, nSlaveAddress, REG_TEMP, 1, (uint16_t*)&u) == -1)
	{
		LogError();
		return 0;
	}
	dTemp = u;
	dTemp /= 100;
	return 1;
}

bool CModbusTCamera::TurnOn(bool bOn)
{
	if (!ctx)return 0;
	uint16_t u = bOn;
	if (modbus_write_registers(ctx, nSlaveAddress, REG_SET_MOD, 1, &u) == -1)
	{
		LogError();
		return 0;
	}
	return 1;
}