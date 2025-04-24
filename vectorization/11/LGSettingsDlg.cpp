// LGSettingsDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "LGSettingsDlg.h"
#include "afxdialogex.h"
#include "GridCtrl.h"
#include "NFC_Device.h"


// CLGSettingsDlg dialog

IMPLEMENT_DYNAMIC(CLGSettingsDlg, CDialogEx)

CLGSettingsDlg::CLGSettingsDlg(CWnd* pParent /*=nullptr*/)
	: CLongGasBaseDlg(IDD_LG_SETTINGS, pParent)
{

}

CLGSettingsDlg::~CLGSettingsDlg()
{
}

void CLGSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLGSettingsDlg, CLongGasBaseDlg)
	ON_BN_CLICKED(IDC_GET_SETTINGS, &CLGSettingsDlg::OnBnClickedGetSettings)
	ON_BN_CLICKED(IDC_SET_SETTINGS, &CLGSettingsDlg::OnBnClickedSetSettings)
	ON_BN_CLICKED(IDC_TESTALARM1, &CLGSettingsDlg::OnBnClickedTestalarm1)
	ON_BN_CLICKED(IDC_TESTALARM2, &CLGSettingsDlg::OnBnClickedTestalarm2)
	ON_WM_SHOWWINDOW()
	ON_MESSAGE(WM_CELLSELECTED, OnCellSelected)
	ON_MESSAGE(WM_CELLMODIFIED, OnCellModified)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CLGSettingsDlg message handlers


BOOL CLGSettingsDlg::OnInitDialog()
{
	CLongGasBaseDlg::OnInitDialog();
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_SETTINGS);
	grid->bFirstColNarrow = 0;
	grid->columnnames.Add(S_O::LoadString(IDS_PARAMETER));
	grid->columnnames.Add(S_O::LoadString(IDS_VALUE));
	grid->FillData();
	return TRUE;
}

void CLGSettingsDlg::GetSettingsAsArray(DEVICE_SETTINGS* s, CStringArray& arr)
{
	CString str;
	arr.RemoveAll();
	char* h[] = { "LED_Alarm_PWM","Vibro_PWM","Led_Alarm_Time","Led_AlarmSlow_Time","Alarm_BeepOff_TimeOut","Alarm_TimeOut","LedRedSCR_PWM",
	"FreezeDeltaTemper","FreezeDeltaTime","Vref_WarmUp_Time","LPS_WarmUP_Time" };
	WORD* w = (WORD*)&s->Led_PWM;
	for (int i = 0; i < 10; i++)
	{
		arr.Add(h[i]);
		arr.Add(S_O::FormatUINT(w[i]));
	}
	arr.Add("BattLow");
	arr.Add(S_O::FormatUINT(s->BattLow));
	arr.Add("Log_State");
	arr.Add(S_O::FormatUINT(s->Log_State));
	arr.Add("Pressure_State");
	arr.Add(S_O::FormatUINT(s->Pressure_State));
	arr.Add("Life_Time_Week");
	arr.Add(S_O::FormatUINT(s->Life_Time_W));
	if (nfcCurrent->IsHaveProperty(FREEZE_MASK))
	{
		arr.Add("FreezeStàtusMask");
		arr.Add(S_O::FormatUINT(s->FreezeStàtusMask));
		float dDev = nfcCurrent->GetGasDevider(GAS_CO, 0);
		float f = (float)s->FreezeLimit;
		arr.Add(S_O::Format("FreezeLimit,%s", nfcCurrent->GetUnitsString(GAS_CO)));
		arr.Add(S_O::FormatValue(f));
	}
	arr.Add("CoefVolToLEL");
	arr.Add(S_O::FormatUINT(s->base.CoefVolToLEL));
	arr.Add("LogTimeOut");
	arr.Add(S_O::FormatUINT(s->LogTimeOut));
	arr.Add("LogAlarmTimeOut");
	arr.Add(S_O::FormatUINT(s->LogAlarmTimeOut));
	if (nfcCurrent->dev_info.btVerControl == 1)
	{
		arr.Add("Flash_Write_Time");
		arr.Add(S_O::FormatUINT(s->Flash_WriteTime));
		arr.Add("Task_Latency");
		arr.Add(S_O::FormatUINT(s->Task_Latency));
		arr.Add("StopTime");
		arr.Add(S_O::FormatUINT(s->Stop_Time));
	}
	else
	{
		if (nfcCurrent->IsHaveProperty(CH4_BUFFER))
		{
			arr.Add("CH4_Buffer_Term");
			arr.Add(S_O::FormatUINT(s->CH4_Buffer_Term));
			arr.Add("CH4_Buffer_Time");
			arr.Add(S_O::FormatUINT(s->CH4_Buffer_Time));
		}
		arr.Add("CoefH2SppmToMg");
		arr.Add(S_O::FormatUINT(s->base.CoefH2SppmToMg));
		arr.Add("CoefCOppmToMg");
		arr.Add(S_O::FormatUINT(s->base.CoefCOppmToMg));
		arr.Add("CoefCHEMppmToMg");
		arr.Add(S_O::FormatUINT(s->base.CoefCHEMppmToMg));
		arr.Add("NFCTimeOutDetectSeconds");
		arr.Add(S_O::FormatUINT(s->NFCTimeOutDetectSeconds));
		arr.Add("NFCTimeOutWaitMinutes");
		arr.Add(S_O::FormatUINT(s->NFCTimeOutWaitMinutes));
		arr.Add("SensorsUnits");
		arr.Add(S_O::FormatUINT(s->SensorsUnits));
		arr.Add("O2Chem");
		arr.Add(S_O::FormatUINT(s->base.O2Chem));
		if (nfcCurrent->IsHaveProperty(PRECISION))
		{
			arr.Add("SensorsPrecisions");
			arr.Add(S_O::FormatUINT(s->SensorsPrecisions));
		}
		if (nfcCurrent->IsHaveProperty(SKIPSELFTEST))
		{
			arr.Add("SkipSelfTest");
			arr.Add(S_O::FormatUINT(s->SkipSelfTest));
		}
		if (nfcCurrent->IsHaveProperty(AUTOZERO))
		{
			arr.Add("SensorsAutoZero");
			arr.Add(S_O::FormatUINT(s->SensorsAutoZero));
		}
		if (nfcCurrent->IsHaveProperty(ALTSCREENTIME))
		{
			arr.Add("AltScreenTime, s");
			arr.Add(S_O::FormatUINT(s->AltScreenTime));
		}
		if (nfcCurrent->IsHaveProperty(LORALOW))
		{
			arr.Add("RssiLow");
			str.Format("%d", (int)s->RssiLow);
			arr.Add(str);
			arr.Add("SnrLow");
			str.Format("%d", (int)s->SnrLow);
			arr.Add(str);
			arr.Add("AlarmType");
			str.Format("%u", (UINT)s->AlarmType);
			arr.Add(str);
			arr.Add("LostSec");
			str.Format("%u", (UINT)s->LostSec);
			arr.Add(str);
			arr.Add("LostPackets");
			str.Format("%u", (UINT)s->LostPackets);
			arr.Add(str);
		}
		if (nfcCurrent->IsHaveProperty(GASSIM))
		{
			arr.Add("O2 sim");
			str.Format("%hu", s->O2Sim);
			arr.Add(str);
			arr.Add("CO sim");
			str.Format("%hu", s->COSim);
			arr.Add(str);
			arr.Add("H2S sim");
			str.Format("%hu", s->H2SSim);
			arr.Add(str);
			arr.Add("CH4 sim");
			str.Format("%hu", s->CH4Sim);
			arr.Add(str);
		}
		if (nfcCurrent->IsHaveProperty(O2CHEMPOS))
		{
			arr.Add("ScreenPosition");
			arr.Add(S_O::FormatUINT(s->O2ChemScrPos));
		}
		if (nfcCurrent->IsHaveProperty(SCALEPOINT))
		{
			arr.Add("O2 scalepoint");
			str.Format("%hu", s->ScalePoint.O2);
			arr.Add(str);
			arr.Add("CO scalepoint");
			str.Format("%hu", s->ScalePoint.CO);
			arr.Add(str);
			arr.Add("H2S scalepoint");
			str.Format("%hu", s->ScalePoint.H2S);
			arr.Add(str);
			arr.Add(nfcCurrent->GetGasString(GAS_MIPEX)+" scalepoint");
			str.Format("%hu", s->ScalePoint.CH4);
			arr.Add(str);
		}
		if (nfcCurrent->IsHaveProperty(CHPRESSURE))
		{
			arr.Add("Options");
			str.Format("%u", (UINT)s->Options);
			arr.Add(str);
		}
		if (nfcCurrent->IsHaveProperty(WEEKTOSCALE))
		{
			arr.Add("WeekToScale");
			str.Format("%u", (UINT)s->WeekToScale);
			arr.Add(str);
		}
		if (nfcCurrent->IsHaveProperty(TRANSPORTALARM))
		{
			arr.Add("TransportAlarmOffMin");
			str.Format("%u", (UINT)s->TransportAlarmOffMin);
			arr.Add(str);
			arr.Add("Unfreeze");
			str.Format("%u", (UINT)s->Unfreeze);
			arr.Add(str);
		}		
	}
}

bool CLGSettingsDlg::CommandDone(CNFC_Command* c)
{
	if (!c->nRet)return 1;
	switch (c->nCommand)
	{
	case GetSettingsByte:
	{
		if (c->dev == nfcCurrent)
		{
			CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_SETTINGS);
			GetSettingsAsArray(&nfcCurrent->dev_settings, grid->names);
			grid->FillData();
			OnCellSelected((WPARAM)grid, 0);
		}
		break;
	}
	}
	return 1;
}

void CLGSettingsDlg::OnBnClickedGetSettings()
{
	nfcCurrent->GetSettings();
}

void CLGSettingsDlg::OnBnClickedSetSettings()
{
	if (!nfcCurrent->dev_settings.bLoaded)return;
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_SETTINGS);
	grid->SaveModifyed();
	DEVICE_SETTINGS* s = &nfcCurrent->dev_settings;
	WORD* w = (WORD*)&s->Led_PWM;
	int n = 1;
	for (int i = 0; i < 10; i++)
	{
		w[i] = atoi(grid->names[n]);
		n += 2;
	}
	s->BattLow=atoi(grid->names[n]);n+=2;
	s->Log_State=atoi(grid->names[n]);n+=2;
	s->Pressure_State=atoi(grid->names[n]);n+=2;
	s->Life_Time_W=atoi(grid->names[n]);n+=2;
	if (nfcCurrent->IsHaveProperty(FREEZE_MASK))
	{
		s->FreezeStàtusMask = atoi(grid->names[n]); n += 2;
		float dDev = nfcCurrent->GetGasDevider(GAS_CO, 0);
		float f = S_O::ToDouble(grid->names[n]); n += 2;
		s->FreezeLimit = f;
	}
	s->base.CoefVolToLEL=atoi(grid->names[n]);n+=2;
	s->LogTimeOut=atoi(grid->names[n]);n+=2;
	s->LogAlarmTimeOut=atoi(grid->names[n]);n+=2;
	if (nfcCurrent->dev_info.btVerControl == 1)
	{
		s->Flash_WriteTime=atoi(grid->names[n]);n+=2;
		s->Task_Latency=atoi(grid->names[n]);n+=2;
		s->Stop_Time=atoi(grid->names[n]);n+=2;
	}
	else
	{
		if (nfcCurrent->IsHaveProperty(CH4_BUFFER))
		{
			s->CH4_Buffer_Term = atoi(grid->names[n]); n += 2;
			s->CH4_Buffer_Time = atoi(grid->names[n]); n += 2;
		}
		s->base.CoefH2SppmToMg=atoi(grid->names[n]);n+=2;
		s->base.CoefCOppmToMg=atoi(grid->names[n]);n+=2;
		s->base.CoefCHEMppmToMg=atoi(grid->names[n]);n+=2;
		s->NFCTimeOutDetectSeconds=atoi(grid->names[n]);n+=2;
		s->NFCTimeOutWaitMinutes=atoi(grid->names[n]);n+=2;
		s->SensorsUnits=atoi(grid->names[n]);n+=2;
		s->base.O2Chem=atoi(grid->names[n]);n+=2;
		if (nfcCurrent->IsHaveProperty(PRECISION))
		{
			s->SensorsPrecisions = atoi(grid->names[n]); n += 2;
		}
		if (nfcCurrent->IsHaveProperty(SKIPSELFTEST))
		{
			s->SkipSelfTest = atoi(grid->names[n]); n += 2;
		}
		if (nfcCurrent->IsHaveProperty(AUTOZERO))
		{
			s->SensorsAutoZero = atoi(grid->names[n]); n += 2;
		}
		if (nfcCurrent->IsHaveProperty(ALTSCREENTIME))
		{
			s->AltScreenTime = atoi(grid->names[n]); n += 2;
		}
		if (nfcCurrent->IsHaveProperty(LORALOW))
		{
			s->RssiLow = atoi(grid->names[n]); n += 2;
			s->SnrLow = atoi(grid->names[n]); n += 2;
			s->AlarmType = atoi(grid->names[n]); n += 2;
			s->LostSec = atoi(grid->names[n]); n += 2;
			s->LostPackets = atoi(grid->names[n]); n += 2;
		}
		if (nfcCurrent->IsHaveProperty(GASSIM))
		{
			s->O2Sim = atoi(grid->names[n]); n += 2;
			s->COSim = atoi(grid->names[n]); n += 2;
			s->H2SSim = atoi(grid->names[n]); n += 2;
			s->CH4Sim = atoi(grid->names[n]); n += 2;
		}
		if (nfcCurrent->IsHaveProperty(O2CHEMPOS))
		{
			s->O2ChemScrPos = atoi(grid->names[n]); n += 2;
		}
		if (nfcCurrent->IsHaveProperty(SCALEPOINT))
		{
			s->ScalePoint.O2 = atoi(grid->names[n]); n += 2;
			s->ScalePoint.CO = atoi(grid->names[n]); n += 2;
			s->ScalePoint.H2S = atoi(grid->names[n]); n += 2;
			s->ScalePoint.CH4 = atoi(grid->names[n]); n += 2;
		}
		if (nfcCurrent->IsHaveProperty(CHPRESSURE))
		{
			s->Options = atoi(grid->names[n]); n += 2;
		}
		if (nfcCurrent->IsHaveProperty(WEEKTOSCALE))
		{
			s->WeekToScale = atoi(grid->names[n]); n += 2;
		}
		if (nfcCurrent->IsHaveProperty(TRANSPORTALARM))
		{
			s->TransportAlarmOffMin = atoi(grid->names[n]); n += 2;
			s->Unfreeze = atoi(grid->names[n]); n += 2;
		}		
	}
	nfcCurrent->SetSettings(1);
}

void CLGSettingsDlg::OnBnClickedTestalarm1()
{
	nfcCurrent->TestAlarm(0);
}

void CLGSettingsDlg::OnBnClickedTestalarm2()
{
	nfcCurrent->TestAlarm(1);
}

void CLGSettingsDlg::OnTabSelected()
{
//	if(nfcCurrent->IsHaveProperty(CH4_LIMIT) && !nfcCurrent->sensors_info.bGasRangeLoaded)nfcCurrent->GetGasRange();
	if(!nfcCurrent->dev_settings.bLoaded)nfcCurrent->GetSettings();
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_SETTINGS);
	GetSettingsAsArray(&nfcCurrent->dev_settings, grid->names);
	grid->FillData();
	OnCellSelected((WPARAM)grid, 0);
}

void CLGSettingsDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CLongGasBaseDlg::OnShowWindow(bShow, nStatus);
	if (bShow && IsWindowEnabled() && nfcCurrent)
	{
	}
}

void CLGSettingsDlg::OnSize(UINT nType, int cx, int cy)
{
	CLongGasBaseDlg::OnSize(nType, cx, cy);
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_SETTINGS);
	if (grid)grid->FillData();
}


BOOL CLGSettingsDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_CHAR)
	{
		return !IsNumber(pMsg, 0);
	}
	return CLongGasBaseDlg::PreTranslateMessage(pMsg);
}

LRESULT CLGSettingsDlg::OnCellModified(WPARAM wp, LPARAM lp)
{
	CGridCtrl* grid = (CGridCtrl*)wp;
	CGridCtrl* gridSett = (CGridCtrl*)GetDlgItem(IDC_GRID_SETTINGS);
	int nRowMod = HIWORD(lp), nColMod = LOWORD(lp);
	if (grid == (CGridCtrl*)GetDlgItem(IDC_GRID_EXT))
	{
		int nr = gridSett->GetActiveRow(), n = nr * 2;
		if (nr < 0 || n >= gridSett->names.GetSize())return 1;
		if (gridSett->names[n] == "FreezeStàtusMask")
		{
			nRowMod *= 3;
			if (nRowMod >= grid->names.GetSize())return 1;
			int nBit = S_O::ToUINT(grid->names[nRowMod]);
			int nSet = S_O::ToUINT(grid->names[nRowMod + 1]);
			if (nSet)nfcCurrent->dev_settings.FreezeStàtusMask |= (1 << nBit);
			else nfcCurrent->dev_settings.FreezeStàtusMask &= ~(1 << nBit);
			gridSett->names[n + 1] = S_O::FormatUINT(nfcCurrent->dev_settings.FreezeStàtusMask);
			gridSett->InvalidateRow(nr);
		}
	}
	return 1;
}

LRESULT CLGSettingsDlg::OnCellSelected(WPARAM wp, LPARAM lp)
{
	CGridCtrl* grid = (CGridCtrl*)wp;
	CGridCtrl* gridExt = (CGridCtrl*)GetDlgItem(IDC_GRID_EXT);
	if (grid == (CGridCtrl*)GetDlgItem(IDC_GRID_SETTINGS))
	{
		int nr = grid->GetActiveRow(), n = nr * 2;
		if (nr < 0 || n >= grid->names.GetSize())return 1;
		bool bShow = 0;
		if (grid->names[n] == "FreezeStàtusMask")
		{
			gridExt->bChangeDataAllowed = 0;
			gridExt->bFirstColNarrow = 1;
			gridExt->columnnames.RemoveAll();
			gridExt->columnnames.Add("NBit");
			gridExt->columnnames.Add(S_O::LoadString(IDS_VALUE));
			gridExt->columnnames.Add(S_O::LoadString(IDS_PARAMETER));
			gridExt->names.RemoveAll();
			gridExt->nSelColumn = 1;
			bShow = 1;
			UINT ids[] = { IDS_FREEZEMASK0,IDS_FREEZEMASK1,IDS_FREEZEMASK2,0,
						IDS_FREEZEMASK4,IDS_FREEZEMASK5,IDS_FREEZEMASK6,IDS_FREEZEMASK7,
						IDS_FREEZEMASK8,IDS_FREEZEMASK9,0,0,
						0,0,0,IDS_FREEZEMASK15 };
			for (int i = 0; i < 16; i++)
			{
				if (ids[i] == 0)continue;
				gridExt->names.Add(S_O::FormatUINT(i));
				gridExt->names.Add(S_O::FormatUINT((nfcCurrent->dev_settings.FreezeStàtusMask & (1<<i))!=0));
				gridExt->names.Add(S_O::LoadString(ids[i]));
			}
			gridExt->FillData();
		}
		gridExt->ShowWindow(bShow);
	}
	return 1;
}