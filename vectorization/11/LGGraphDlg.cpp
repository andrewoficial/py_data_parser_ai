// LGGraphDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "LGGraphDlg.h"
#include "afxdialogex.h"
#include <DialogOperations.h>
#include "GridCtrl.h"
#include <LongNameFile.h>
#include "LGMipexDlg.h"


extern COLORREF g_Colors[];
extern int g_nColors;
extern CString g_szCurrentDir;
// CLGGraphDlg dialog

IMPLEMENT_DYNAMIC(CLGGraphDlg, CLongGasBaseDlg)

CLGGraphDlg::CLGGraphDlg(CWnd* pParent /*=nullptr*/)
	: CLongGasBaseDlg(IDD_LG_GRAPH, pParent)
	, m_PollTime(_T("5"))
	, m_Mean(_T(""))
	, m_Last(_T(""))
	, m_Command(_T(""))
{
	gr = 0;
	bCurveSelChanged = 0;
	strFolder = AfxGetApp()->GetProfileString("LGGraph", "SaveFolder", g_szCurrentDir);
	m_PollTime = AfxGetApp()->GetProfileString("LGGraph", "GraphPollTime", "5");
	m_Command = AfxGetApp()->GetProfileString("LGGraph", "FCommand", "");
	m_bSendF = AfxGetApp()->GetProfileInt("LGGraph", "SendF", 0);
	bInSetAll = 0;
	bSave = 0;
}

CLGGraphDlg::~CLGGraphDlg()
{
	if (gr)delete gr;
}

void CLGGraphDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_POLLTIME, m_PollTime);
	DDX_Control(pDX, IDC_LIST_CURVES, m_ListCurves);
	DDX_Text(pDX, IDC_EDIT_MEAN, m_Mean);
	DDX_Check(pDX, IDC_CHECK_CH4STATUS, m_CH4En);
	DDX_Check(pDX, IDC_CHECK_H2SSTATUS, m_H2SEn);
	DDX_Check(pDX, IDC_CHECK_O2STATUS, m_O2En);
	DDX_Check(pDX, IDC_CO_STATUS, m_COEn);
	DDX_Check(pDX, IDC_FCOMMAND, m_bSendF);
	DDX_Text(pDX, IDC_LAST, m_Last);
	DDX_Text(pDX, IDC_COMMAND, m_Command);
	DDX_Control(pDX, IDC_FILEPATH, m_FilePath);
}

BEGIN_MESSAGE_MAP(CLGGraphDlg, CLongGasBaseDlg)
	ON_BN_CLICKED(IDC_START, &CLGGraphDlg::OnBnClickedStart)
	ON_WM_TIMER()
	ON_LBN_SELCHANGE(IDC_LIST_CURVES, &CLGGraphDlg::OnLbnSelchangeListCurves)
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_RADIO_GRAPH, &CLGGraphDlg::OnBnClickedRadioGraph)
	ON_BN_CLICKED(IDC_RADIO_DATA, &CLGGraphDlg::OnBnClickedRadioData)
	ON_BN_CLICKED(IDC_START2, &CLGGraphDlg::OnBnClickedStart2)
	ON_BN_CLICKED(IDC_GET, &CLGGraphDlg::OnBnClickedGet)
	ON_BN_CLICKED(IDC_GETALL, &CLGGraphDlg::OnBnClickedGetall)
	ON_BN_CLICKED(IDC_FCOMMAND, &CLGGraphDlg::OnBnClickedFcommand)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_CLEAR, &CLGGraphDlg::OnBnClickedClear)
	ON_BN_CLICKED(IDC_OPENFOLDER, &CLGGraphDlg::OnBnClickedOpenfolder)
END_MESSAGE_MAP()

enum
{
	CURVE_O2=0,
	CURVE_CO,
	CURVE_H2S,
	CURVE_CH4LEL,
	CURVE_TEMP,
	CURVE_O2ADC,
	CURVE_O2VOLT,
	CURVE_COADC,
	CURVE_COVOLT,
	CURVE_H2SADC,
	CURVE_H2SVOLT,
	CURVE_BATTVOLT,
	CURVE_PRESSURE,
	CURVE_CPUTEMP,
	CURVE_SIZE
};

CString CLGGraphDlg::GetCurveName(UINT nId, bool bUnicode)
{
	CNFC_Device* nfc = nfcCurrent;
	CString strG = "%s,%s";
	CString h[] = { S_O::Format(strG,nfc->GetGasString(GAS_O2,1,bUnicode),nfc->GetUnitsString(GAS_O2,bUnicode)),
		S_O::Format(strG,nfc->GetGasString(GAS_CO,1,bUnicode),nfc->GetUnitsString(GAS_CO,bUnicode)),
		S_O::Format(strG,nfc->GetGasString(GAS_H2S,1,bUnicode),nfc->GetUnitsString(GAS_H2S,bUnicode)),
		S_O::Format(strG,nfc->GetGasString(GAS_MIPEX,1,bUnicode),nfc->GetUnitsString(GAS_MIPEX,bUnicode)),
		"Temp",nfc->GetGasString(GAS_O2,1,bUnicode)+" (ADC)",nfc->GetGasString(GAS_O2,1,bUnicode)+" (mV)",
		nfc->GetGasString(GAS_CO,1,bUnicode)+" (ADC)",nfc->GetGasString(GAS_CO,1,bUnicode)+" (mV)",
		nfc->GetGasString(GAS_H2S,1,bUnicode)+" (ADC)",nfc->GetGasString(GAS_H2S,1,bUnicode)+" (mV)",
		"Battery(V)","Pressure","CPU Temp" };
	return h[nId];
}

BOOL CLGGraphDlg::OnInitDialog()
{
	CLongGasBaseDlg::OnInitDialog();
	gr = CGraph::CreateOnPlace(GetDlgItem(IDC_STATIC_GRAPH));
	gr->curve.clear();
	gr->bGlobalMarkerOnMouseMove = gr->bGlobalMarkerOnCurvePointsOnly = gr->bGlobalMarkerOnEveryCurve = 1;
	gr->bNoRButtonMenu = 1;
	gr->bDrawAdditionalAxis = gr->bEveryCurveHasOwnY = 1;
	gr->bRescaleToMaxFromZero = 1;
	gr->net.nXAxisMode = AXISMODE_CTIME;
	layout.AddControl(gr);
	m_ListCurves.nParams|= MLBS_UNICODE;
	UINT nSel = AfxGetApp()->GetProfileInt("LGGraph", "CurveSel", (1 << CURVE_O2) | (1 << CURVE_CO) | (1 << CURVE_H2S) | (1 << CURVE_CH4LEL) | (1 << CURVE_TEMP));
	for (int i = CURVE_O2; i < CURVE_SIZE; i++)
		D_O::AddStringWithId(&m_ListCurves, GetCurveName(i,1),i,(nSel & (1<<i))!=0);
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID);
	grid->nGridId = GRIDID_GRAPH;
	grid->bChangeDataAllowed = 0;
	grid->FillData(nfcCurrent);
	CheckRadioButton(IDC_RADIO_GRAPH, IDC_RADIO_DATA, IDC_RADIO_GRAPH);
	return TRUE; 
}

void CLGGraphDlg::UpdateCurveName(CUIntArray& arr,int nCurve)
{
	int n = D_O::GetListItemNum(m_ListCurves, nCurve);
	CString str = GetCurveName(nCurve, 1);
	if (n != -1)
	{
		m_ListCurves.DeleteString(n);
		m_ListCurves.InsertString(n, str);
		m_ListCurves.SetItemData(n, nCurve);
		if (S_O::IsAlreadyInArray(n, arr))m_ListCurves.SetSel(n);
	}
	if (gr->curve.size() > nCurve)gr->curve[nCurve].strCurveInfo = GetCurveName(nCurve);
}

void CLGGraphDlg::SensStatusToControls()
{
	UpdateData();
	BYTE* en[] = { &nfcCurrent->sensors_info.bO2En,&nfcCurrent->sensors_info.bCOEn,&nfcCurrent->sensors_info.bH2SEn,&nfcCurrent->sensors_info.bCH4En };
	BOOL* c[] = { &m_O2En,&m_COEn,&m_H2SEn ,&m_CH4En };
	for (int i = 0; i < 4; i++)
	{
		*(c[i]) = *(en[i]) != 0;
	}
	UpdateData(0);
	GetDlgItem(IDC_CHECK_CH4STATUS)->SetWindowText(nfcCurrent->GetGasString(GAS_MIPEX));
	GetDlgItem(IDC_CHECK_O2STATUS)->SetWindowText(nfcCurrent->GetGasString(GAS_O2));
	GetDlgItem(IDC_CO_STATUS)->SetWindowText(nfcCurrent->GetGasString(GAS_CO));
	GetDlgItem(IDC_CHECK_H2SSTATUS)->SetWindowText(nfcCurrent->GetGasString(GAS_H2S));
	CUIntArray arr;
	D_O::GetListBoxSel(&m_ListCurves, arr);
	UINT nCurves[] = { CURVE_CH4LEL, CURVE_O2,CURVE_O2ADC ,CURVE_O2VOLT,CURVE_CO,CURVE_COADC,	CURVE_COVOLT,CURVE_H2S,CURVE_H2SADC,CURVE_H2SVOLT };
	for (int i = 0; i < sizeof(nCurves) / sizeof(UINT); i++)
	{
		UpdateCurveName(arr, nCurves[i]);
	}
}

void CLGGraphDlg::UpdateMean()
{
	UpdateData();
	m_Mean = m_Last = "";
	if (nfcCurrent->monitoring.GetSize())
	{
		int nAverage = 5;
		int nS = nfcCurrent->monitoring.GetSize() - nAverage;
		if (nS < 0)nS = 0;
		CString str;
		for (int i = CURVE_O2; i < CURVE_SIZE; i++)
		{
			if (i == CURVE_O2VOLT || i == CURVE_COVOLT || i == CURVE_H2SVOLT || i == CURVE_CPUTEMP)continue;
			double d = 0;
			int nC = 0, nCount = gr->curve[i].vData.size();
			for (UINT k = nS; k < nCount; k++)
			{
				d += gr->curve[i].vData[k].y;
				nC++;
			}
			str.Format("%s=%.2f", GetCurveName(i), d / nC);
			if (m_Mean != "")m_Mean += "   ";
			m_Mean += str;
			if (nCount)
			{
				str.Format("%s=%.2f", GetCurveName(i), gr->curve[i].vData[nCount - 1].y);
				if (m_Last != "")m_Last += "   ";
				m_Last += str;
			}
		}
	}
	UpdateData(0);
}

bool CLGGraphDlg::CommandDone(CNFC_Command* c)
{
	CString str;
	switch (c->nCommand)
	{
	case GetSettingsByte:
	{
		if (c->dev == nfcCurrent)
		{
			CUIntArray arr;
			D_O::GetListBoxSel(&m_ListCurves, arr);
			UINT nCurves[] = { CURVE_O2,CURVE_CO,CURVE_H2S,CURVE_CH4LEL };
			for (int i = 0; i < sizeof(nCurves) / sizeof(UINT); i++)
			{
				UpdateCurveName(arr, nCurves[i]);
			}
		}
		break;
	}
	case GetSensStatusByte:
	{
		if (!c->nRet || c->dev!=nfcCurrent)break;
		SensStatusToControls();		
		break;
	}
	case SetCommandMIPEXByte:
	{
		if (!c->nRet)break;
		str = m_Command;
		if (str == "")str = "F";
		if (c->dev->bInMonitoring && c->dev->additional_pars.strMIPEXCommand==str)
		{
			c->dev->lastF = c->dev->GetMIPEXAnswer(c->get);
			c->dev->lastF.Replace(";", "");
		}
		break;
	}
	case GetDataSensorsByte:
	{
		if (c->dev->bStopMonitoring)
		{
			c->dev->bInMonitoring = c->dev->bStopMonitoring =0;
			c->dev->tmMonitoringStart.SetNull();
			KillTimer(c->dev->nDeviceNum);
			if (c->dev == nfcCurrent)
			{
				GetDlgItem(IDC_START)->SetWindowText(S_O::LoadString(IDS_MONITORING));				
			}
			return 1;
		}
		if (c->nRet)
		{
			if (c->dev->bInMonitoring)
			{
				c->dev->sensors_data.strCH4F = c->dev->lastF;
				c->dev->lastF = "";
				c->dev->monitoring.Add(c->dev->sensors_data);
				if (c->dev == nfcCurrent)
				{
					CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID);
					grid->FillData(nfcCurrent);
					FillGraph(0);
					UpdateMean();
				}
				if (bSave)
				{					
					if (!c->dev->sensors_info.bStatusLoaded)c->dev->GetSensorStatus();
					else
					{
						str.Format("%s\\%s", strFolder, GetSaveFileName(c->dev,1));
						SaveCSV(c->dev, str, 1, 1);
					}
				}
			}
		}
		if (c->dev->bInMonitoring)
		{
			if (c->dev->bMonitorOnce)
			{
				c->dev->bMonitorOnce = c->dev->bInMonitoring = 0;
			}
			else
			{				
				UpdateData();
				double dS = S_O::ToDouble(m_PollTime);
				if (dS < 0)dS = 0;
				if (c->dev->IsVirtualDevice())dS = 1;
				if (dS)
				{
					SetTimer(c->dev->nDeviceNum, (UINT)(dS * 1000), 0);
				}
				else
				{
					if (!c->dev->bPauseMonitoring)c->dev->UpdateSensorsData(m_bSendF,m_Command);
				}
			}
		}
		break;
	}
	}
	return 1;
}

CString CLGGraphDlg::GetSaveFileName(CNFC_Device* nfc,bool bAuto)
{
	CString out, s;
	BYTE* en[] = { &nfc->sensors_info.bO2En,&nfc->sensors_info.bCOEn,&nfc->sensors_info.bH2SEn,&nfc->sensors_info.bCH4En };
	//BOOL* c[] = { &m_O2En,&m_COEn,&m_H2SEn ,&m_CH4En };	
	CString g[] = { nfc->GetGasString(GAS_O2),nfc->GetGasString(GAS_CO),nfc->GetGasString(GAS_H2S),nfc->GetGasString(GAS_MIPEX)};	
	for (int i = 0; i < 4; i++)
	{
		if (!*(en[i]))continue;
		if (s != "")s += ";";
		s += g[i];
	}
	out.Format("%s_%s_%s.csv", nfc->GetSerialNumber(), s, nfc->tmMonitoringStart.Format("%d.%m.%y_%H-%M"));
	S_O::CorrectFileName(out);
	return out;
}

void CLGGraphDlg::OnBnClickedStart()
{
	if (nfcCurrent->bInMonitoring)
	{
		main->BreakAll(nfcCurrent,STARTALL_MONITORING);
		return;
	}
	GetFolderName(nfcCurrent,0);
	StartMonitoring(nfcCurrent);
}

void CLGGraphDlg::OnTimer(UINT_PTR nIDEvent)
{	
	CNFC_Device* dev = main->GetDeviceByNum(nIDEvent);
	if (dev)
	{
		if (dev->bPauseMonitoring)return;
		dev->UpdateSensorsData(m_bSendF, m_Command);
	}
	KillTimer(nIDEvent);
	CLongGasBaseDlg::OnTimer(nIDEvent);
}

void CLGGraphDlg::FillGraph(bool bFromStart)
{	
	if (!gr->curve.size())
	{
		CCurve cc;
		for (int i = CURVE_O2; i < CURVE_SIZE; i++)
		{			
			cc.Pen.lopnColor = g_Colors[i % g_nColors];
			cc.strCurveInfo = GetCurveName(i);
			cc.bOwnLimits = 1;
			gr->curve.push_back(cc);
		}
	}
	if (nfcCurrent->monitoring.GetSize())
	{
		int nS = bFromStart ? 0 : nfcCurrent->monitoring.GetSize() - 1;
		CMyPoint pt;
		SYSTEMTIME st;
		for (int i = nS; i < nfcCurrent->monitoring.GetSize(); i++)
		{
			SENSORS_DATA* d = &nfcCurrent->monitoring[i];
			d->time.GetAsSystemTime(st);
			pt.x = (double)CTime(st).GetTime();
			pt.y = (double)d->O2 / nfcCurrent->GetGasDevider(GAS_O2, 1);
			gr->curve[CURVE_O2].vData.push_back(pt);
			pt.y = (double)d->CO / nfcCurrent->GetGasDevider(GAS_CO, 1);
			gr->curve[CURVE_CO].vData.push_back(pt);
			pt.y = (double)d->H2S / nfcCurrent->GetGasDevider(GAS_H2S, 1);
			gr->curve[CURVE_H2S].vData.push_back(pt);
			pt.y = (double)(nfcCurrent->GetUnits(GAS_MIPEX) ? d->CH4LEL : d->CH4VOL) / nfcCurrent->GetGasDevider(GAS_MIPEX, 1);
			gr->curve[CURVE_CH4LEL].vData.push_back(pt);
			pt.y = d->SensorTemp;
			pt.y /= 100;
			gr->curve[CURVE_TEMP].vData.push_back(pt);
			pt.y = d->O2Volt;
			gr->curve[CURVE_O2ADC].vData.push_back(pt);
			pt.y = d->GetVolt(d->O2Volt);
			gr->curve[CURVE_O2VOLT].vData.push_back(pt);
			pt.y = d->COVolt;
			gr->curve[CURVE_COADC].vData.push_back(pt);
			pt.y = d->GetVolt(d->COVolt);
			gr->curve[CURVE_COVOLT].vData.push_back(pt);
			pt.y = d->H2SVolt;
			gr->curve[CURVE_H2SADC].vData.push_back(pt);
			pt.y = d->GetVolt(d->H2SVolt);
			gr->curve[CURVE_H2SVOLT].vData.push_back(pt);
			pt.y = (double)d->Batt / 100;
			gr->curve[CURVE_BATTVOLT].vData.push_back(pt);
			pt.y = d->Press;
			gr->curve[CURVE_PRESSURE].vData.push_back(pt);
			pt.y = d->CPUTemp;
			gr->curve[CURVE_CPUTEMP].vData.push_back(pt);
		}
	}
	ShowCurves();
}

void CLGGraphDlg::ShowCurves()
{
	if (!gr->curve.size())return;
	CUIntArray arr;
	D_O::GetListBoxSel(&m_ListCurves, arr);
	if(!arr.GetSize())arr.Add(CURVE_O2);
	for (int i = CURVE_O2; i < CURVE_SIZE; i++)
		gr->curve[i].bShow = S_O::IsAlreadyInArray(i, arr);
	gr->RescaleToMax();
}

void CLGGraphDlg::OnLbnSelchangeListCurves()
{	
	bCurveSelChanged = 1;
	ShowCurves();
}


void CLGGraphDlg::OnDestroy()
{	
	UpdateData();
	if (bCurveSelChanged)
	{
		int nSel = m_ListCurves.GetSelCount();
		if (nSel)
		{
			CUIntArray arr;
			arr.SetSize(nSel);
			m_ListCurves.GetSelItems(nSel, (LPINT)&arr[0]);
			nSel = 0;
			for (int i = 0; i < arr.GetSize(); i++)nSel |= (1 << arr[i]);
		}
		AfxGetApp()->WriteProfileInt("LGGraph", "CurveSel", nSel);
	}
	AfxGetApp()->WriteProfileString("LGGraph", "GraphPollTime", m_PollTime);
	CLongGasBaseDlg::OnDestroy();
}

bool CLGGraphDlg::SaveCSV(CNFC_Device* nfc, CString strPath,bool bAll,bool bLast)
{
	CString str, buf;
	CLongNameFile f;
	UINT nMode = CFile::modeCreate | CFile::modeReadWrite;
	if (bLast)nMode |= CFile::modeNoTruncate;
	if (!f.Open(strPath, nMode))return 0;
	if (bLast)f.SeekToEnd();
	str = "Time";
	if (bAll)
	{
		CString strO2 = nfc->GetGasString(GAS_O2), strMipex = nfc->GetGasString(GAS_MIPEX),
			strCO= nfc->GetGasString(GAS_CO), strH2S = nfc->GetGasString(GAS_H2S);
		CString hdr[] = { strH2S +"- ADC",strH2S+" - mV",strH2S,strH2S+" - Unit",strH2S+" - AlarmState",strH2S+" - CoeffppmToMg",
			strCO+"-ADC",strCO+"-mV",strCO,strCO+"-Unit",strCO+"-AlarmState",strCO+"-CoeffppmtoMg",
			strO2 +"-ADC",strO2 + "-mV",strO2,strO2 + "-Unit",strO2 + "-AlarmState",strO2 + "-EChemFlag",
			strMipex,strMipex +"-Unit",strMipex +"-AlarmState",strMipex +"-CoeffppmToMg",strMipex +"-F",
			"Battery (V)","Pressure","CPU Temp","Temp °C"};
		int nC = sizeof(hdr) / sizeof(CString);
		for (int i = 0; i < nC; i++)
		{
			if (str != "")str += ";";
			str += hdr[i];
		}
		str += "\r\n";
		if(!bLast || !f.GetLength())f.Write(str, str.GetLength());
		int nFrom = bLast ? (nfc->monitoring.GetSize() - 1):0;
		for (int k = nFrom; k < nfc->monitoring.GetSize(); k++)
		{
			SENSORS_DATA& d = nfc->monitoring[k];
			str = d.time.FormatDB();

			buf.Format(";%u", (UINT)d.H2SVolt);
			str += buf;
			buf.Format(";%.0f", d.GetVolt(d.H2SVolt));
			str += buf;
			buf.Format(";%.2f", (double)d.H2S / nfc->GetGasDevider(GAS_H2S,1));
			str += buf;
			str += ";" + nfc->GetUnitsString(GAS_H2S);
			str += ";" + nfc->GetAlarmString(GAS_H2S, d);
			str += ";" + S_O::FormatUINT(nfc->dev_settings.base.CoefH2SppmToMg);

			buf.Format(";%u", (UINT)d.COVolt);
			str += buf;
			buf.Format(";%.0f", d.GetVolt(d.COVolt));
			str += buf;
			buf.Format(";%.2f", (double)d.CO / nfc->GetGasDevider(GAS_CO,1));
			str += buf;			
			str += ";" + nfc->GetUnitsString(GAS_CO);
			str += ";" + nfc->GetAlarmString(GAS_CO, d);
			str += ";" + S_O::FormatUINT(nfc->dev_settings.base.CoefCOppmToMg);

			buf.Format(";%u", (UINT)d.O2Volt);
			str += buf;
			buf.Format(";%.0f", d.GetVolt(d.O2Volt));
			str += buf;
			buf.Format(";%.2f", (double)d.O2 / nfc->GetGasDevider(GAS_O2,1));
			str += buf;			
			str += ";" + nfc->GetUnitsString(GAS_O2);
			str += ";" + nfc->GetAlarmString(GAS_O2, d);
			str += ";" + S_O::FormatUINT(nfc->dev_settings.base.O2Chem);

			buf.Format(S_O::Format(";%%.%df",nfc->GetPrecision(GAS_MIPEX)), (double)(nfc->GetUnits(GAS_MIPEX) ? d.CH4LEL : d.CH4VOL) / nfc->GetGasDevider(GAS_MIPEX,1));
			str += buf;
			str += ";" + nfc->GetUnitsString(GAS_MIPEX);
			str += ";" + nfc->GetAlarmString(GAS_MIPEX, d);
			str += ";" + S_O::FormatUINT(nfc->dev_settings.base.CoefCHEMppmToMg);
			buf = d.strCH4F;
			buf.Replace("\n", "");
			buf.Replace("\r", "");
			str += ";" + buf;

			buf.Format(";%.2f", (double)d.Batt/100);
			str += buf;
			buf.Format(";%.0f", (double)d.Press);
			str += buf;
			buf.Format(";%.0f", (double)d.CPUTemp);
			str += buf;
			buf.Format(";%.1f", (double)d.SensorTemp/100);
			str += buf;

			str += "\r\n";
			f.Write(str, str.GetLength());
		}
	}
	else
	{
		for (UINT i = 0; i < gr->curve.size(); i++)
		{
			if (!gr->curve[i].bShow)continue;
			if (str != "")str += ";";
			str += gr->curve[i].strCurveInfo;
		}
		str += "\r\n";
		if (!bLast || !f.GetLength())f.Write(str, str.GetLength());
		int nFrom = bLast ? (nfc->monitoring.GetSize() - 1) : 0;
		for (int k = nFrom; k < nfc->monitoring.GetSize(); k++)
		{
			str = nfc->monitoring[k].time.FormatDB();
			for (UINT i = 0; i < gr->curve.size(); i++)
			{
				if (!gr->curve[i].bShow)continue;
				buf.Format("%g", gr->curve[i].vData[k].y);
				if (str != "")str += ";";
				str += buf;
			}
			str += "\r\n";
			f.Write(str, str.GetLength());
		}
	}
	return 1;
}

void CLGGraphDlg::OnTabSelected()
{
	if (!nfcCurrent->sensors_info.bStatusLoaded)nfcCurrent->GetSensorStatus();
	SensStatusToControls();
	gr->curve.clear();
	FillGraph(1);
	SensStatusToControls();
	UpdateMean();
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID);
	grid->FillData(nfcCurrent);
	m_FilePath.SetWindowText(bSave ? S_O::Format("%s\\%s", strFolder, GetSaveFileName(nfcCurrent, 1)) : "");
	GetDlgItem(IDC_START)->SetWindowText(S_O::LoadString((nfcCurrent->bInMonitoring && !nfcCurrent->bStopMonitoring) ? IDS_STOP : IDS_MONITORING));
}

void CLGGraphDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CLongGasBaseDlg::OnShowWindow(bShow, nStatus);
	if (bShow && IsWindowEnabled() && nfcCurrent)
	{
		
	}
}


void CLGGraphDlg::ClickedRadioGraph()
{
	UINT nId = GetCheckedRadioButton(IDC_RADIO_GRAPH, IDC_RADIO_DATA);
	bool bShow = nId == IDC_RADIO_GRAPH;
	gr->ShowWindow(bShow);
	GetDlgItem(IDC_LIST_CURVES)->ShowWindow(bShow);
	GetDlgItem(IDC_GRID)->ShowWindow(!bShow);
}

void CLGGraphDlg::OnBnClickedRadioGraph()
{	
	ClickedRadioGraph();	
}


void CLGGraphDlg::OnBnClickedRadioData()
{
	OnBnClickedRadioGraph();
}

void CLGGraphDlg::OnBnClickedStart2()
{
	GetFolderName(nfcCurrent,1);
	main->StartAll(STARTALL_MONITORING, nfcCurrent);
}

bool CLGGraphDlg::StartMonitoring(CNFC_Device* nfc)
{
	if (nfc->bInMonitoring)
	{
		return 1;
	}		
	UpdateData();
	nfc->lastF = "";	
	if (nfc == nfcCurrent)
	{
		gr->curve.clear();
		gr->RescaleToMax();
	}
	nfc->tmMonitoringStart = CDateTime::GetCurrentTime();
	if (!nfc->StartMonitoring(m_bSendF,m_Command))return 0;
	if (nfc == nfcCurrent)
	{
		m_FilePath.SetWindowText(bSave ? S_O::Format("%s\\%s", strFolder, GetSaveFileName(nfc, 1)) : "");
		GetDlgItem(IDC_START)->SetWindowText(S_O::LoadString(IDS_STOP));
		m_Mean = m_Last = "";
		UpdateData(0);
	}
	return 1;
}

void CLGGraphDlg::StopMonitoring(CNFC_Device* nfc)
{
	if (!nfc->bInMonitoring || nfc->bStopMonitoring)return;
	if (!nfc->IsInCommand())
	{
		if(nfc==nfcCurrent)GetDlgItem(IDC_START)->SetWindowText(S_O::LoadString(IDS_MONITORING));
		nfc->bInMonitoring = 0;
		nfc->tmMonitoringStart.SetNull();
	}
	else nfc->bStopMonitoring = 1;
	KillTimer(nfc->nDeviceNum);
}

void CLGGraphDlg::ChangeAll(CNFC_Device* dev, UINT nMode, CHANGEALL* c)
{
	switch (nMode)
	{
	case CHANGEALL_MONITORINGONCE:
		OnBnClickedGet();
		break;
	}
}

void CLGGraphDlg::OnBnClickedGet()
{
	if (nfcCurrent->bInMonitoring)return;
	GetFolderName(nfcCurrent,0);
	nfcCurrent->bMonitorOnce = nfcCurrent->bInMonitoring = 1;
	if (!nfcCurrent->sensors_info.bStatusLoaded)nfcCurrent->GetSensorStatus();
	if (!nfcCurrent->sensors_info.alarms.bLoaded)nfcCurrent->GetAlarms();
	if (!nfcCurrent->dev_settings.bLoaded)nfcCurrent->GetSettings();
	if (nfcCurrent->tmMonitoringStart.IsNull())nfcCurrent->tmMonitoringStart = CDateTime::GetCurrentTime();
	UpdateData();
	nfcCurrent->UpdateSensorsData(m_bSendF,m_Command);
}

void CLGGraphDlg::GetFolderName(CNFC_Device* dev, bool bAll)
{
	if (dev == nfcCurrent)
	{
		bSave = D_O::SelectFolder(strFolder, m_hWnd, S_O::LoadString(IDS_SELECTSAVEPATH), strFolder);
		m_FilePath.SetWindowText(bSave ? S_O::Format("%s\\%s", strFolder, GetSaveFileName(dev, 1)) : "");
		if (bSave)AfxGetApp()->WriteProfileString("LGGraph", "SaveFolder", strFolder);
		if (m_bSendF)
		{
			CLongGasBaseDlg* base = main->GetByClass(dev, "CLGMipexDlg");
			if (base)base->UpdateSaveFolder(strFolder, bSave);
		}
	}
}

void CLGGraphDlg::OnBnClickedGetall()
{
	GetFolderName(nfcCurrent,1);
	OnBnClickedGet();
	main->ChangeAll(this, CHANGEALL_MONITORINGONCE, 0);
}

void CLGGraphDlg::OnBnClickedFcommand()
{
	UpdateData();
	AfxGetApp()->WriteProfileInt("LGGraph", "SendF", m_bSendF);
	AfxGetApp()->WriteProfileString("LGGraph", "FCommand", m_Command);	
}

void CLGGraphDlg::OnSize(UINT nType, int cx, int cy)
{
	CLongGasBaseDlg::OnSize(nType, cx, cy);
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID);
	if(grid)grid->FillData(nfcCurrent);
}

bool CLGGraphDlg::OperationEnded(CNFC_Device* dev, UINT nType)
{
	if (nType == STARTALL_MONITORING && dev==nfcCurrent)GetDlgItem(IDC_START)->SetWindowText(S_O::LoadString(IDS_MONITORING));
	else __super::OperationEnded(dev,nType);
	return 1;
}

//void CLGGraphDlg::ConcChanged(UINT n, UINT nId)
//{
//	if (bInSetAll)return;
//	bChanged[n] = 1;	
//	CHANGEALL c;
//	c.n = n;
//	c.str = D_O::GetWindowText(GetDlgItem(nId));
//	main->ChangeAll(this, CHANGEALL_GRAPHCONC, &c);
//}
//
//void CLGGraphDlg::OnEnChangeConcO2()
//{
//	ConcChanged(0, IDC_CONC_O2);
//}
//
//void CLGGraphDlg::OnEnChangeConcCo()
//{
//	ConcChanged(1, IDC_CONC_CO);
//}
//
//void CLGGraphDlg::OnEnChangeConcH2s()
//{
//	ConcChanged(2, IDC_CONC_H2S);
//}
//
//void CLGGraphDlg::OnEnChangeConcCh4()
//{
//	ConcChanged(4, IDC_CONC_CH4);
//}


void CLGGraphDlg::OnBnClickedClear()
{
	gr->curve.clear();
	gr->RescaleToMax();
	nfcCurrent->monitoring.RemoveAll();
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID);
	grid->FillData(nfcCurrent);
	UpdateData();
	m_Mean = m_Last = "";
	UpdateData(0);
}


void CLGGraphDlg::OnBnClickedOpenfolder()
{
	if (!bSave)return;
	D_O::OpenFolderInExplorer(strFolder, m_hWnd);
}
