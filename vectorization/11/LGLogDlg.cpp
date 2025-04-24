// LGLogDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "LGLogDlg.h"
#include "afxdialogex.h"
#include "NFC_Device.h"
#include "GridCtrl.h"
#include <DialogOperations.h>
#include <LongNameFile.h>


// CLGLogDlg dialog
extern COLORREF g_Colors[];
extern int g_nColors;
extern CString g_szCurrentDir;

IMPLEMENT_DYNAMIC(CLGLogDlg, CLongGasBaseDlg)

CLGLogDlg::CLGLogDlg(CWnd* pParent /*=nullptr*/)
	: CLongGasBaseDlg(IDD_LG__LOG, pParent)
	, m_bShowBadData(FALSE)
{
	gr = 0;
	bCurveSelChanged = 0;
	strFolder = AfxGetApp()->GetProfileString("LGLog", "SaveFolder", g_szCurrentDir);
	bSave = bPrevShort2 = bPrevMIPEX = 0;
}

CLGLogDlg::~CLGLogDlg()
{
	if (gr)delete gr;
}

void CLGLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CLongGasBaseDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CURVES, m_Curves);
	DDX_Control(pDX, IDC_FILEPATH, m_FilePath);
	DDX_Check(pDX, IDC_SHOWBADDATA, m_bShowBadData);
}


BEGIN_MESSAGE_MAP(CLGLogDlg, CLongGasBaseDlg)
	ON_BN_CLICKED(IDC_GET, &CLGLogDlg::OnBnClickedGet)
	ON_BN_CLICKED(IDC_STOP, &CLGLogDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_RADIO_LOG, &CLGLogDlg::OnBnClickedRadioLog)
	ON_BN_CLICKED(IDC_RADIO_GRAPH, &CLGLogDlg::OnBnClickedRadioLog)
	ON_LBN_SELCHANGE(IDC_LIST_CURVES, &CLGLogDlg::OnLbnSelchangeListCurves)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_FORWARD, &CLGLogDlg::OnBnClickedForward)
	ON_BN_CLICKED(IDC_BACKWARD, &CLGLogDlg::OnBnClickedForward)
	ON_BN_CLICKED(IDC_RESET, &CLGLogDlg::OnBnClickedReset)
	ON_BN_CLICKED(IDC_GET_ALL, &CLGLogDlg::OnBnClickedGetAll)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_OPENFOLDER, &CLGLogDlg::OnBnClickedOpenfolder)
END_MESSAGE_MAP()

#define NCURVE_STANDARD 5
#define NCURVE_MANUFAC 11

int CLGLogDlg::GetCurveCount()
{
	return nfcCurrent->bManufact? (NCURVE_MANUFAC + (nfcCurrent->log.bMIPEXLog ? 2:0)):NCURVE_STANDARD;
}

CString CLGLogDlg::GetCurveName(int n)
{
	CString h[] = {"Temp" ,
		nfcCurrent->GetGasString(GAS_O2),nfcCurrent->GetGasString(GAS_CO),nfcCurrent->GetGasString(GAS_H2S),nfcCurrent->GetGasString(GAS_MIPEX),
		nfcCurrent->GetGasString(GAS_O2) + " (ADC)",nfcCurrent->GetGasString(GAS_O2) + " (mV)",
		nfcCurrent->GetGasString(GAS_CO) + " (ADC)",nfcCurrent->GetGasString(GAS_CO) + " (mV)",
		nfcCurrent->GetGasString(GAS_H2S) + " (ADC)",nfcCurrent->GetGasString(GAS_H2S) + "(mV)","",""};
	CString hM[] = { "Temp" ,
		"TS","Skz1","Sm","Conc1",
		"Sc","",
		"Skz2","",
		"Skz3","","Sktg","C0" };
	return nfcCurrent->log.bMIPEXLog?hM[n]:h[n];
}

void CLGLogDlg::FillGraph(bool bFromStart)
{
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_LOG);
	if (gr->curve.size() < GetCurveCount())
	{
		CCurve cc;
		int nfrom = gr->curve.size();
		for (int i = nfrom; i < GetCurveCount(); i++)
		{
			cc.strCurveInfo = GetCurveName(i);
			cc.Pen.lopnColor = g_Colors[i % g_nColors];
			cc.bOwnLimits = 1;
			gr->curve.push_back(cc);
		}
		gr->net.nXAxisMode = AXISMODE_CTIME;
		grid->FillData(&nfcCurrent->log);
	}
	int nLast = bFromStart ? 0 : gr->curve[0].vData.size(), nWrongCount = 0;
	UINT prev = 0;
	for (int j = nLast; j < nfcCurrent->log.records.GetSize(); j++)
	{
		DEVICE_LOG_RECORD* r = &nfcCurrent->log.records[j];
		if (prev)
		{
			double dif = (double)r->nDateTime - (double)prev;
			if (fabs(dif) > 31.5e7)//1 year
			{
				nWrongCount++;
				if (nWrongCount < 3)continue;
			}
		}
		nWrongCount = 0;
		prev = r->nDateTime;
		double* u = &r->nTemp;
		CMyPoint pt;
		pt.x = r->nDateTime;
		for (int i = 0; i < 5; i++)
		{
			if (i >= gr->curve.size())break;
			pt.y = u[i];
			gr->curve[i].vData.push_back(pt);
		}
		int nC = 5;
		for (int i = 0; i < 3; i++)
		{
			if (nC >= gr->curve.size())break;
			pt.y = u[i + 5];
			gr->curve[nC++].vData.push_back(pt);
			pt.y = SENSORS_DATA::GetVolt((WORD)pt.y);
			gr->curve[nC++].vData.push_back(pt);
		}
		if (nfcCurrent->log.bMIPEXLog)
		{
			if (nC < gr->curve.size())
			{
				pt.y = r->nBattVolt;
				gr->curve[nC++].vData.push_back(pt);
			}
			if (nC < gr->curve.size())
			{
				pt.y = (int)r->nRSSI;
				gr->curve[nC++].vData.push_back(pt);
			}
		}
	}
	if (!nfcCurrent->log.bFromPast)
	{
		for (int i = 0; i < gr->curve.size(); i++)gr->curve[i].Sort();
	}
	ShowCurves();
}

bool CLGLogDlg::CommandDone(CNFC_Command* c)
{
	if (!c->nRet)return 1;
	switch (c->nCommand)
	{
	case GetSensStatusByte:
	{
		CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_LOG);
		if (c->dev == nfcCurrent)
		{
			grid->FillData(&nfcCurrent->log);
			CUIntArray arr;
			CString str;
			D_O::GetListBoxSel(&m_Curves, arr);
			//		UINT nO2Curves[] = { 1, 2,5,6,7,8 };
			for (int i = 1; i < GetCurveCount(); i++)
			{
				int nCurve = i;
				int n = D_O::GetListItemNum(m_Curves, nCurve);
				str = GetCurveName(nCurve);
				if (n != -1)
				{
					m_Curves.DeleteString(n);
					m_Curves.InsertString(n, str);
					m_Curves.SetItemData(n, nCurve);
					if (S_O::IsAlreadyInArray(n, arr))m_Curves.SetSel(n);
				}
				if (gr->curve.size() > nCurve)gr->curve[nCurve].strCurveInfo = str;
			}
		}
		break;
	}
	case ClearLogByte:
	{
		nfcCurrent->log.records.RemoveAll();
		if (c->dev == nfcCurrent)
		{
			gr->curve.clear();
			gr->RescaleToMax();
			CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_LOG);
			grid->FillData(&nfcCurrent->log);
		}
		break;
	}
	case GetLogByte:
	{
		if (c->dev == nfcCurrent)
		{
			CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_LOG);
			if (!nfcCurrent->log.records.GetSize())
			{
				if (nfcCurrent->log.bShort2 != bPrevShort2 || nfcCurrent->log.bMIPEXLog != bPrevMIPEX)
				{
					bPrevShort2 = nfcCurrent->log.bShort2;
					bPrevMIPEX = nfcCurrent->log.bMIPEXLog;
					FillCurveList(0);
				}
				grid->FillData(&nfcCurrent->log);
			}
			else
			{
				grid->SetGridRowCount(nfcCurrent->log.records.GetSize());
				grid->UpdateScrollSizes();
				grid->RedrawWindow();
				FillGraph(0);
			}
		}
		if (bSave)SaveToDir(c->dev,strFolder);
		break;
	}
	}
	return 1;
}

void CLGLogDlg::FillCurveList(bool bInit)
{
	m_Curves.ResetContent();
	if (bInit)
	{
		int nSel = AfxGetApp()->GetProfileInt("LGLogDlg", "CurveSel", 1 << 1);
		for (int i = 0; i < GetCurveCount(); i++)D_O::AddStringWithId(&m_Curves, GetCurveName(i), i, (nSel & (1 << i)) != 0);
	}
	else
	{
		CUIntArray arr;
		D_O::GetListBoxSel(&m_Curves, arr);
		UINT nG[] = { (UINT)-1 ,
		GAS_O2,GAS_CO,GAS_H2S,GAS_MIPEX,
		GAS_O2,GAS_O2,
		GAS_CO,GAS_CO,
		GAS_H2S,GAS_H2S,(UINT)-1,(UINT)-1 };
		for (int i = 0; i < GetCurveCount(); i++)
		{
			if (nfcCurrent->log.bShort2 && (i == 5 || i == 6 || i == 8 || i == 10))continue;
			if (nfcCurrent->log.bMIPEXLog && (i == 6 || i == 8 || i == 10))continue;
			if (!nfcCurrent->log.bMIPEXLog && !nfcCurrent->bManufact)
			{
				if (nG[i] != -1 && !nfcCurrent->IsGasSelected(nG[i]))continue;
			}
			D_O::AddStringWithId(&m_Curves, GetCurveName(i), i, S_O::IsAlreadyInArray(i, arr));
		}
	}
}

BOOL CLGLogDlg::OnInitDialog()
{
	CLongGasBaseDlg::OnInitDialog();
	gr = CGraph::CreateOnPlace(GetDlgItem(IDC_STATIC_GRAPH));
	gr->curve.clear();
	gr->bGlobalMarkerOnMouseMove = gr->bGlobalMarkerOnCurvePointsOnly = gr->bGlobalMarkerOnEveryCurve = 1;
	gr->bNoRButtonMenu = 1;
	gr->bDrawAdditionalAxis = gr->bEveryCurveHasOwnY = 1;
	gr->bRescaleToMaxFromZero = 1;
	layout.AddControl(gr);
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_LOG);
	grid->nGridId = GRIDID_LOG;
	grid->FillData(&nfcCurrent->log);
	FillCurveList(1);
	CheckRadioButton(IDC_RADIO_LOG, IDC_RADIO_GRAPH, AfxGetApp()->GetProfileInt("LGLogDlg","ViewMode", IDC_RADIO_LOG));
	CheckRadioButton(IDC_FORWARD, IDC_BACKWARD, AfxGetApp()->GetProfileInt("LGLogDlg", "LogMode", IDC_FORWARD));
	OnBnClickedRadioLog();
	return TRUE;
}

void CLGLogDlg::OnBnClickedGet()
{
	GetFolderName(0);
	UpdateData();
	nfcCurrent->log.bFromPast = GetCheckedRadioButton(IDC_FORWARD, IDC_BACKWARD) == IDC_FORWARD;
	nfcCurrent->log.bShowBadData = m_bShowBadData;
	nfcCurrent->nRecordsFrom = 0;
	nfcCurrent->GetLog();
}

void CLGLogDlg::OperationToControls(CNFC_Device* nfc, UINT nType, bool bOperationStarted)
{
	if (nfc != nfcCurrent)return;
	if (bOperationStarted)
	{
		if (nType != STARTALL_GETLOG)EnableWindow(0);
		else
		{
			EnableWindow(1);
			D_O::EnableAllChilds(this, 0, IDC_STOP);
			GetDlgItem(IDC_GRID_LOG)->EnableWindow(1);
			GetDlgItem(IDC_RADIO_LOG)->EnableWindow(1);
			GetDlgItem(IDC_RADIO_GRAPH)->EnableWindow(1);
			GetDlgItem(IDC_OPENFOLDER)->EnableWindow(1);
			gr->EnableWindow(1);
			m_Curves.EnableWindow(1);
			gr->curve.clear();
			gr->RescaleToMax();
		}
	}
	else
	{
		EnableWindow(1);
		D_O::EnableAllChilds(this, 1, IDC_STOP);
	}
}

bool CLGLogDlg::OperationStarted(CNFC_Device* nfc, UINT nType)
{
	OperationToControls(nfc, nType, 1);
	return 1;
}

bool CLGLogDlg::OperationEnded(CNFC_Device* nfc, UINT nType)
{
	OperationToControls(nfc, nType, 0);
	if (nType == STARTALL_GETLOG)
	{
		if (nfc->log.records.GetSize() > 1)
		{
			nCompareMode = nfc->log.bFromPast;
			qsort(&nfc->log.records[0], nfc->log.records.GetSize(), sizeof(DEVICE_LOG_RECORD), compare);
			if (bSave)SaveToDir(nfc, strFolder, 0);
			if (nfc == nfcCurrent)
			{
				gr->curve.clear();
				FillGraph(1);
			}
		}
	}
	return 1;
}

int CLGLogDlg::nCompareMode = 0;

int CLGLogDlg::compare(const void* arg1, const void* arg2)
{
	DEVICE_LOG_RECORD* r1 = (DEVICE_LOG_RECORD*)arg1;
	DEVICE_LOG_RECORD* r2 = (DEVICE_LOG_RECORD*)arg2;
	int nRet = -1;
	if (r1->nDateTime > r2->nDateTime)nRet = 1;
	else if (r1->nDateTime == r2->nDateTime)nRet = 0;
	if (!nCompareMode)nRet = -nRet;
	return nRet;
}



void CLGLogDlg::OnBnClickedStop()
{
	main->BreakAll(nfcCurrent);
}

void CLGLogDlg::ClickedRadioLog()
{
	UINT nSel = GetCheckedRadioButton(IDC_RADIO_LOG, IDC_RADIO_GRAPH);
	GetDlgItem(IDC_GRID_LOG)->ShowWindow(nSel == IDC_RADIO_LOG);
	GetDlgItem(IDC_LIST_CURVES)->ShowWindow(nSel == IDC_RADIO_GRAPH);
	gr->ShowWindow(nSel == IDC_RADIO_GRAPH);
}

void CLGLogDlg::OnBnClickedRadioLog()
{
	ClickedRadioLog();
}

void CLGLogDlg::OnLbnSelchangeListCurves()
{
	bCurveSelChanged = 1;
	ShowCurves();
}

void CLGLogDlg::ShowCurves()
{
	if (!gr->curve.size())return;
	CUIntArray arr;
	D_O::GetListBoxSel(&m_Curves, arr);
	if (!arr.GetSize())arr.Add(1);
	for (int i = 0; i < gr->curve.size(); i++)
		gr->curve[i].bShow = S_O::IsAlreadyInArray(i, arr);
	gr->RescaleToMax();
}

void CLGLogDlg::OnDestroy()
{
	if (bCurveSelChanged)
	{
		CUIntArray arr;
		D_O::GetListBoxSel(&m_Curves, arr);
		int nSelCurves = 0;
		if (arr.GetSize())
		{
			for (int i = 0; i < arr.GetSize(); i++)nSelCurves |= (1 << arr[i]);
		}
		AfxGetApp()->WriteProfileInt("LGLogDlg", "CurveSel", nSelCurves);
	}
	CLongGasBaseDlg::OnDestroy();
}


void CLGLogDlg::OnBnClickedForward()
{
	AfxGetApp()->WriteProfileInt("LGLogDlg", "LogMode", GetCheckedRadioButton(IDC_FORWARD, IDC_BACKWARD));
}

void CLGLogDlg::OnBnClickedReset()
{
	if (AfxMessageBox(S_O::LoadString(IDS_CLEARLOG_Q), MB_Q) == IDNO)return;
	nfcCurrent->ResetLog();
}

void CLGLogDlg::OnBnClickedGetAll()
{
	GetFolderName(1);
	UpdateData();
	nfcCurrent->log.bFromPast = GetCheckedRadioButton(IDC_FORWARD, IDC_BACKWARD) == IDC_FORWARD;
	nfcCurrent->log.bShowBadData = m_bShowBadData;
	main->StartAll(STARTALL_GETLOG, nfcCurrent);
}

CString CLGLogDlg::GetSaveFileName(CNFC_Device* nfc, CString strDir)
{
	CString str;
	if (tmStart.IsNull())tmStart = CDateTime::GetCurrent();
	str.Format("%s\\%s_%s_L.csv", strDir, nfc->GetSerialNumber(), tmStart.Format("%d.%m.%y_%H-%M"));
	return str;
}

bool CLGLogDlg::SaveToDir(CNFC_Device* nfc, CString strPath, bool bAppend)
{
	if (!nfc->log.records.GetSize())return 1;
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_LOG);
	CUIntArray ids;
	CStringArray names;
	grid->GetLogColumns(&nfc->log, ids, names);
	int nRowFrom = bAppend ? nfc->nRecordsFrom : 0;
	CLongNameFile f;
	UINT nMode = CFile::modeReadWrite | CFile::modeCreate;
	if (bAppend)nMode |= CFile::modeNoTruncate;
	if (!f.Open(GetSaveFileName(nfc, strPath), nMode))return 0;
	if (bAppend)f.SeekToEnd();
	CString strFeed = "\r\n", strTab = ";";
	int nc = ids.GetSize();
	if (!bAppend || f.GetLength() == 0)
	{
		for (int i = 0; i < nc; i++)
		{
			f.Write(names[i], names[i].GetLength());
			if (i < nc - 1)f.Write(strTab, strTab.GetLength());
		}
		f.Write(strFeed, strFeed.GetLength());
	}
	CString str;
	for (int i = nRowFrom; i < nfc->log.records.GetSize(); i++)
	{
		for (int k = 0; k < nc; k++)
		{
			str = grid->GetCellDataLog(&nfc->log,ids[k],i);
			if (str == "")str = " ";
			f.Write(str, str.GetLength());
			if (k < nc - 1)f.Write(strTab, strTab.GetLength());
		}
		f.Write(strFeed, strFeed.GetLength());
	}
	f.Close();
	//bool bRet = grid->SaveAsXLS(GetSaveFileName(nfc,strPath), 1, bAppend? nfc->nRecordsFrom:0, bAppend);
	nfc->nRecordsFrom = nfc->log.records.GetSize();
	return 1;
}

void CLGLogDlg::ChangeAll(CNFC_Device* nfc, UINT nMode, CHANGEALL* c)
{

}

void CLGLogDlg::OnSize(UINT nType, int cx, int cy)
{
	CLongGasBaseDlg::OnSize(nType, cx, cy);
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_LOG);
	if (grid)grid->FillData(&nfcCurrent->log);
}

void CLGLogDlg::OnTabSelected()
{
	bPrevShort2= nfcCurrent->log.bShort2;
	bPrevMIPEX = nfcCurrent->log.bMIPEXLog;
	FillCurveList(0);
	gr->curve.clear();
	GetDlgItem(IDC_SHOWBADDATA)->ShowWindow(nfcCurrent->bManufact);
	m_FilePath.SetWindowText(bSave ? GetSaveFileName(nfcCurrent, strFolder) : "");
	OperationToControls(nfcCurrent, nfcCurrent->nCurrentOperation, nfcCurrent->nCurrentOperation!=-1);
	FillGraph(1);
}

void CLGLogDlg::GetFolderName(bool bAll)
{
	tmStart.SetNull();
	bSave = D_O::SelectFolder(strFolder, m_hWnd, S_O::LoadString(IDS_SELECTSAVEPATH), strFolder);
	m_FilePath.SetWindowText(bSave ? GetSaveFileName(nfcCurrent,strFolder) : "");
	if (bSave)AfxGetApp()->WriteProfileString("LGLog", "SaveFolder", strFolder);
}

void CLGLogDlg::OnBnClickedOpenfolder()
{
	if (!bSave)return;
	D_O::OpenFolderInExplorer(strFolder, m_hWnd);
}
