// CLGSaveDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "CLGSaveDlg.h"
#include "afxdialogex.h"
#include "NFC_Device.h"
#include "GridCtrl.h"
#include <DialogOperations.h>
#include <StatusDialog.h>
#include "LGSensorsDlg.h"
#include "LGSettingsDlg.h"
#include <CRC.h>


// CLGSaveDlg dialog

IMPLEMENT_DYNAMIC(CLGSaveDlg, CDialogEx)

enum
{
	RESTORE_ALL=0,
	RESTORE_ALLCOEFF,
	RESTORE_BASESETTINGS,
	RESTORE_SETTINGS,
	RESTORE_LORA,
	RESTORE_GASSELECTION,
	RESTORE_O2COEFF,
	RESTORE_COCOEFF,
	RESTORE_H2SCOEFF,
	RESTORE_CH4COEFF,
	RESTORE_CH4_MULT,
	RESTORE_ACCEL,
	RESTORE_DIAP,
	RESTORE_VOLTDIAP,
	RESTORE_COEFF,
	RESTORE_ALARMS,
	RESTORE_SERIALNO,
	RESTORE_USER_ALL,
	RESTORE_USER_LOGPERIOD,
	RESTORE_USER_UNITS,
	RESTORE_USER_VIBRO,
	RESTORE_SIZE

};

CLGSaveDlg::CLGSaveDlg(CWnd* pParent /*=nullptr*/)
	: CLongGasBaseDlg(IDD_LG_SAVE, pParent)
{
	settings = new DEVICE_SAVED_SETTINGS;
}

CLGSaveDlg::~CLGSaveDlg()
{
	delete settings;
}

void CLGSaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INFO, m_Info);
	DDX_Control(pDX, IDC_RICHEDIT21, m_Rich);
	DDX_Control(pDX, IDC_BASE, m_Base);
	DDX_Control(pDX, IDC_SELINFO, m_SelInfo);
}


BEGIN_MESSAGE_MAP(CLGSaveDlg, CLongGasBaseDlg)
	ON_BN_CLICKED(IDC_SAVE, OnBnClickedSave)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedRestore)
	ON_MESSAGE(WM_CELLSELECTED, OnWmGridCellSelected)
	ON_MESSAGE(WM_HEADCLICKED, OnWmGridHeadClicked)
	ON_BN_CLICKED(IDC_SAVE2, &CLGSaveDlg::OnBnClickedSave2)
END_MESSAGE_MAP()

bool CLGSaveDlg::CommandDone(CNFC_Command* c)
{
	if (!c->nRet)return 1;
/*	switch (c->nCommand)
	{
	default:
		break;
	}*/
	return 1;
}

extern CString g_szCurrentDir;

BOOL CLGSaveDlg::OnInitDialog()
{
	CLongGasBaseDlg::OnInitDialog();
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_SETTINGS);
	grid->bChangeDataAllowed = 0;
	grid->bFirstColNarrow = 1;
	grid->columnnames.Add("#");
	grid->columnnames.Add(S_O::LoadString(IDS_SETTINGS));
	CString strFile;
	strFile.Format("%s\\save\\*.lgs", g_szCurrentDir);
	CFileFind ff;
	bool bf = ff.FindFile(strFile);
	CStringArray arr;
	while (bf)
	{
		bf = ff.FindNextFile();
		arr.Add(ff.GetFileTitle());
	}
	S_O::Sort(arr);
	for (int i = 0; i < arr.GetSize(); i++)
	{
		grid->names.Add(S_O::FormatUINT(i+1));
		grid->names.Add(arr[i]);
	}
	grid->FillData();
	grid->SetActiveRow(0);
	PostMessage(WM_CELLSELECTED,(WPARAM)grid, 0);
	return TRUE;
}

void CLGSaveDlg::FillSettingsInfo()
{
	UINT nBase[] = { RESTORE_GASSELECTION ,RESTORE_DIAP,RESTORE_SETTINGS };
	for(int i=0;i<sizeof(nBase)/sizeof(UINT *);i++)
		FillRestoredSettingsInfo(nBase[i], m_Base, 1);
	UINT nCoeff[] = {RESTORE_ALLCOEFF, RESTORE_COEFF, RESTORE_ACCEL };
	for (int i = 0; i < sizeof(nBase) / sizeof(UINT*); i++)
		FillRestoredSettingsInfo(nCoeff[i], m_Rich, 1);
}

LRESULT CLGSaveDlg::OnWmGridHeadClicked(WPARAM wp, LPARAM lp)
{
	CGridCtrl* from = (CGridCtrl*)wp;
	if (from == (CGridCtrl*)GetDlgItem(IDC_GRID_TYPES))
	{
		UINT nId = from->GetCellCtrlID(lp, 0);
		if (nId == RESTORE_SEL)
		{
			int nSel = -1;
			for (int i = 0; i < rarr.arr.GetSize(); i++)
			{
				RESTORE_TYPE& t = rarr.arr[i];
				if (t.nId == -1)continue;
				if (nSel == -1)nSel = !t.bSelect;
				t.bSelect = nSel;
			}
			FillTypesGrid();
		}
	}
	return 1;
}

LRESULT CLGSaveDlg::OnWmGridCellSelected(WPARAM wp, LPARAM lp)
{
	CGridCtrl* from = (CGridCtrl*)wp;
	if (from == (CGridCtrl*)GetDlgItem(IDC_GRID_TYPES))
	{
		m_SelInfo.SetWindowText("");
		int nR = from->GetActiveRow();
		if (nR<0 || nR>=rarr.arr.GetSize())return 1;
		RESTORE_TYPE& t = rarr.arr[nR];
		if (t.nId == -1)return 1;
		FillRestoredSettingsInfo(t.nId, m_SelInfo, 0);
	}
	else if (from == (CGridCtrl*)GetDlgItem(IDC_GRID_SETTINGS))
	{
		CUIntArray last;
		for (int i = 0; i < rarr.arr.GetSize(); i++)
		{
			if (rarr.arr[i].bSelect)last.Add(rarr.arr[i].nId);
		}
		m_Info.SetWindowText("");
		m_Rich.SetWindowText("");
		m_Base.SetWindowText("");
		m_SelInfo.SetWindowText("");
		settings->bLoaded = 0;
		int nR = from->GetActiveRow();
		int nPos = nR * from->columnnames.GetSize() + 1;
		if (nR < 0 || nPos >= from->names.GetSize())return 0;
		CString strPath;
		strPath.Format("%s\\save\\%s.lgs", g_szCurrentDir, from->names[nPos]);
		CFile f;
		if (!f.Open(strPath, CFile::modeRead))return 0;
		char buf[4];
		f.Read(buf, 4);
		int nVer = 0;
		if (memcmp(buf, "#LG0",4) == 0)nVer = 0;
		else if (memcmp(buf, "#LG1", 4) == 0)nVer = 1;
		else return 0;
		if (nVer > 0)
		{
			CMemFile mf;
			UINT nLen = (UINT)f.GetLength() - 8;
			BYTE* bt = new BYTE[nLen];
			f.Read(bt, nLen);
			ULONG nCRC = 0, nCRCCalc = 0;
			CRC_32 crc;
			crc.Calculate(bt, nLen, nCRCCalc);
			f.Read(&nCRC, sizeof(ULONG));
			int nRet = 0;
			mf.Attach(bt, nLen);
			if (nCRC == nCRCCalc && settings->Load(&mf))nRet = 1;
			delete[] bt;
			if(!nRet)return 0;
		}
		else
		{
			if (!settings->Load(&f))
			{
				return 0;
			}
		}
		CFileStatus fs;
		f.GetStatus(fs);
		settings->tmCreate = fs.m_mtime;
		FillList();
		if (last.GetSize())
		{
			for (int i = 0; i < rarr.arr.GetSize(); i++)
			{
				if (S_O::IsAlreadyInArray(rarr.arr[i].nId, last))rarr.arr[i].bSelect = 1;
			}
			FillTypesGrid();
		}
		m_Info.SetWindowText(settings->GetDescr());
		FillSettingsInfo();
	}
	return 1;
}

void CLGSaveDlg::FillTypesGrid()
{
	CGridCtrl* gridType = (CGridCtrl*)GetDlgItem(IDC_GRID_TYPES);
	gridType->nGridId = GRIDID_RESTORE;
	gridType->FillData(&rarr);
}

bool CLGSaveDlg::IsUserSetting(UINT nId)
{
	UINT ids[] = {RESTORE_USER_LOGPERIOD,RESTORE_LORA,RESTORE_ALARMS,
					RESTORE_USER_UNITS,RESTORE_USER_VIBRO };
	for (int i = 0; i < sizeof(ids) / sizeof(UINT); i++)
	{
		if (nId == ids[i])return 1;
	}
	return 0;
}

void CLGSaveDlg::FillList()
{
	rarr.arr.RemoveAll();
	FillTypesGrid();
	if (!settings->bLoaded)return;
	RESTORE_TYPE t;
	CString strUser[] = { S_O::LoadString(IDS_USERSETTINGS),S_O::LoadString(IDS_ALLUSERSETTINGS), S_O::LoadString(IDS_LOGPERIOD),S_O::LoadString(IDS_LG_RADIO), S_O::LoadString(IDS_LG_ALARMS) ,
						S_O::LoadString(IDS_UNITS),S_O::LoadString(IDS_VIBROPOWER) };
	UINT ids[] = { (UINT)-1,RESTORE_USER_ALL,RESTORE_USER_LOGPERIOD,RESTORE_LORA,RESTORE_ALARMS,
					RESTORE_USER_UNITS,RESTORE_USER_VIBRO };
	for (int i = 0; i < sizeof(strUser) / sizeof(CString*); i++)
	{
		t.nId = ids[i];
		t.strDescr = strUser[i];
		t.bSelect = i == 1;
		rarr.arr.Add(t);
	}
	if (!settings->bUserOnly)
	{
		CString str[] = { S_O::LoadString(IDS_SERVICESETTINGS),S_O::LoadString(IDS_ALLSETTINGS) ,S_O::LoadString(IDS_ALLCOEFF) ,S_O::LoadString(IDS_BASESETTINGS),
				S_O::LoadString(IDS_LG_SETTINGS),S_O::LoadString(IDS_LG_RADIO),S_O::LoadString(IDS_SENSORSTATUS),S_O::LoadString(IDS_COEFFS) + " O2",
				S_O::LoadString(IDS_COEFFS) + " CO",S_O::LoadString(IDS_COEFFS) + " H2S",S_O::LoadString(IDS_COEFFS) + " MIPEX",S_O::LoadString(IDS_COEFFS) + " CH4_MULT",
				S_O::LoadString(IDS_SENSORACC),S_O::LoadString(IDS_MEASUREDIAP),S_O::LoadString(IDS_VOLTDIAP),S_O::LoadString(IDS_COEFFS),
				S_O::LoadString(IDS_LG_ALARMS),S_O::LoadString(IDS_SERIALNO) };
		for (int i = 0; i < sizeof(str) / sizeof(CString*); i++)
		{
			if (!nfcCurrent->bManufact && i==2)break;
			t.nId = i-1;
			if (t.nId == RESTORE_LORA && settings->nSWVer < 80)continue;
			if (t.nId == RESTORE_CH4_MULT && (!nfcCurrent->IsHaveProperty(CH4_MULT) || !nfcCurrent->IsHaveProperty(CH4_MULT, settings->nSWVer)))continue;
			t.strDescr = str[i];
			t.bSelect = 0;
			rarr.arr.Add(t);
		}
	}
	FillTypesGrid();
}

void CLGSaveDlg::Save(bool bUser)
{
	if (nfcCurrent->IsVirtualDevice())return;
	CGridCtrl* from = (CGridCtrl*)GetDlgItem(IDC_GRID_SETTINGS);
	CInputNameDlg ind;
	int nR = from->GetActiveRow();
	int nPos = nR * from->columnnames.GetSize() + 1;
	if (nR >= 0 && nPos < from->names.GetSize())
	{
		ind.m_Name = from->names[nPos];
	}
	ind.m_Title = S_O::LoadStringA(IDS_SETSAVENAME);
	int nRowReplace = -1;
	CFile f;
	CString str, strPath;
	while (1)
	{
		if (ind.DoModal() == IDCANCEL)return;
		S_O::CorrectFileName(ind.m_Name);
		S_O::Trim(ind.m_Name);
		if (ind.m_Name == "")continue;
		bool bBreak = 1;
		for (int i = 0; i < from->names.GetSize() / 2; i++)
		{
			if (S_O::CompareNoCase(ind.m_Name, from->names[i * 2 + 1]) == 0)
			{
				if (AfxMessageBox(S_O::LoadString(IDS_REPLACESETTINGS_Q), MB_Q) == IDYES)nRowReplace = i;
				else bBreak = 0;
				break;
			}
		}
		strPath.Format("%s\\save\\%s.lgs", g_szCurrentDir, ind.m_Name);
		CFileException ex;
		if (!f.Open(strPath, CFile::modeCreate | CFile::modeReadWrite, &ex))
		{
			char buf[256];
			ex.GetErrorMessage(buf, 256);
			AfxMessageBox(buf, MB_ICONERROR);
			bBreak = 0;
		}
		if (bBreak)break;
	}
	main->EnableWindow(0);
	CStatusThread* thread = new CStatusThread(this, S_O::LoadString(IDS_SAVEINGSETTINGS));
	thread->bShowCancel = 0;
	thread->SetMessage(S_O::LoadString(IDS_WAIT));
	nfcCurrent->TestIfDeviceChanged();
	int nRet = 1;
	try
	{
		//		if (!nfc->dev_settings.bLoaded)
		{
			nfcCurrent->GetSettings();
			if (!nfcCurrent->WaitComplete(1))throw 0;
		}
		//		if (!nfc->sensors_info.bLoaded)
		if(!bUser)
		{
			nfcCurrent->GetAllCoeffs();
			if (!nfcCurrent->WaitComplete(1))throw 0;
		}
		else
		{
			nfcCurrent->GetSensorStatus();
			if (!nfcCurrent->WaitComplete(1))throw 0;
		}
		//		if (!nfc->sensors_info.alarms.bLoaded)
		{
			nfcCurrent->GetAlarms();
			if (!nfcCurrent->WaitComplete(1))throw 0;
		}
		//		if (!nfc->lora_settings.bLoaded)
		if (nfcCurrent->IsHaveProperty(INTERFACES))
		{
			nfcCurrent->GetInterfaces();
			if (!nfcCurrent->WaitComplete(1))throw 0;
		}
		else
		{
			nfcCurrent->interfaces.bLoaded = 0;
			nfcCurrent->interfaces.Standard = RT_LORA;
		}
		if (nfcCurrent->interfaces.Standard == RT_LORA)
		{
			nfcCurrent->GetLoraSettings();
			if (!nfcCurrent->WaitComplete(1))throw 0;
		}
		else if (nfcCurrent->interfaces.Standard == RT_WIFI)
		{
			nfcCurrent->GetWifiSettings();
			if (!nfcCurrent->WaitComplete(1))throw 0;
		}
	}
	catch (int)
	{
		nRet = 0;
	}
	main->EnableWindow(1);
	thread->Stop();
	if (!nRet)
	{
		AfxMessageBox(S_O::LoadString(IDS_QUERYERROR), MB_ICONERROR);
		return;
	}
	f.Write("#LG1", 4);
	settings->bUserOnly = bUser;
	settings->strDevNoFrom = nfcCurrent->GetSerialNumber();
	settings->strDevVersion = nfcCurrent->dev_info.strSoftwareVer;
	settings->base = nfcCurrent->dev_info.base;
	settings->dev_settings = nfcCurrent->dev_settings;
	settings->sensors = nfcCurrent->sensors_info;
	settings->lora = nfcCurrent->lora_settings;
	settings->mipex = nfcCurrent->mipex;
	settings->interfaces = nfcCurrent->interfaces;
	settings->wifi = nfcCurrent->wifi;
	settings->tmCreate = CDateTime::GetCurrent();
	CMemFile mf;
	settings->Save(&mf);
	UINT nLen = (UINT)mf.GetLength();
	BYTE* data = mf.Detach();
	CRC_32 crc;
	ULONG nCrc = 0;
	crc.Calculate(data, nLen, nCrc);
	f.Write(data, nLen);
	f.Write(&nCrc, sizeof(ULONG));
	f.Close();
	delete[] data;
	if (nRowReplace == -1)
	{
		int nR = from->names.GetSize() / from->columnnames.GetSize();
		from->names.Add(S_O::FormatUINT(nR + 1));
		from->names.Add(ind.m_Name);
		from->FillData();
		from->SetActiveRow(nR);
		FillList();
	}
	else from->SetActiveRow(nRowReplace);
	OnWmGridCellSelected((WPARAM)from, 0);
	strPath.Format("%s\\save\\%s.txt", g_szCurrentDir, ind.m_Name);
	CFileStatus fs;
	if (CFile::GetStatus(strPath, fs))
	{
		fs.m_attribute = FILE_ATTRIBUTE_NORMAL;
		f.SetStatus(strPath, fs);
	}
	::DeleteFile(strPath);
	if (!f.Open(strPath, CFile::modeCreate | CFile::modeReadWrite))return;
	m_Base.GetWindowText(str);
	str += "\r\n";
	f.Write(str, str.GetLength());
	str = "Coefficients\r\n\r\n";
	f.Write(str, str.GetLength());
	m_Rich.GetWindowText(str);
	str += "\r\n";
	f.Write(str, str.GetLength());

	f.GetStatus(fs);
	f.Close();
	fs.m_attribute = FILE_ATTRIBUTE_READONLY;
	f.SetStatus(strPath, fs);
}

void CLGSaveDlg::OnBnClickedSave()
{
	Save(0);
}

void CLGSaveDlg::WriteLogInfo(CRichEditCtrl& rich, CString strDescr, CString strInfo,bool bAddCR)
{
	D_O::WriteLog(rich, strDescr + "   ", 0xff0000);
	D_O::WriteLog(rich, strInfo + (bAddCR?"\r\n":""), 0, 0);
}

void CLGSaveDlg::FillRestoredSettingsInfo(UINT nId, CRichEditCtrl& rich,bool bAddDescr)
{
	CUIntArray arr;
	GetRestoredSettings(nId, arr);
	CString str, buf;
	CLGSensorsDlg* s = (CLGSensorsDlg*)main->GetByClass(nfcCurrent, "CLGSensorsDlg");
	if (!s)return;
	UINT g[] = { GAS_O2,GAS_CO,GAS_H2S };
	float* f = 0;
	for(int k=0;k<arr.GetSize();k++)
	{
		nId = arr[k];
		str = "";
		switch (nId)
		{
		case RESTORE_COEFF:
		{
			if (!nfcCurrent->bManufact)break;
			if (bAddDescr)D_O::WriteLog(rich, "Coeffs\r\n", 0xff, 1);
			BASE_COEFF* c = &settings->dev_settings.base;
			WriteLogInfo(rich, "CoefH2SppmToMg", S_O::FormatUINT(c->CoefH2SppmToMg));
			WriteLogInfo(rich, "CoefCOppmToMg", S_O::FormatUINT(c->CoefCOppmToMg));
			WriteLogInfo(rich, "CoefVolToLEL", S_O::FormatUINT(c->CoefVolToLEL));
			WriteLogInfo(rich, "CoefCHEMppmToMg", S_O::FormatUINT(c->CoefCHEMppmToMg));
			WriteLogInfo(rich, "O2Chem", S_O::FormatUINT(c->O2Chem));
			break;
		}
		case RESTORE_DIAP:
		{
			if (bAddDescr)D_O::WriteLog(rich, "Diap\r\n", 0xff, 1);
			GAS_RANGE* r[] = { &settings->sensors.ch4Range,&settings->sensors.coRange,&settings->sensors.h2sRange,&settings->sensors.o2Range };
			UINT gas[] = { GAS_MIPEX,GAS_CO,GAS_H2S,GAS_O2 };
			BYTE en[] = { settings->sensors.bCH4En,settings->sensors.bCOEn,settings->sensors.bH2SEn,settings->sensors.bO2En };
			for (int i = 0; i < 4; i++)
			{
				float dDev = nfcCurrent->GetGasDevider(gas[i],1);
				double f = (float)r[i]->wFrom / dDev;
				double f1 = (float)r[i]->wTo / dDev;
				str.Format("%s - %s",S_O::FormatValue(f), S_O::FormatValue(f1));
				WriteLogInfo(rich, nfcCurrent->GetGasStringByVal(gas[i],en[i]), str);
			}
			break;
		}
		case RESTORE_BASESETTINGS:
			if (!nfcCurrent->bManufact)break;
			if (bAddDescr)D_O::WriteLog(rich, "BaseSettings\r\n", 0xff, 1);
			WriteLogInfo(rich, "Beep", S_O::FormatUINT(settings->base.bBeep));
			WriteLogInfo(rich, "Vibro", S_O::FormatUINT(settings->base.bVibro));
			WriteLogInfo(rich, "Alarm", S_O::FormatUINT(settings->base.bAlarm));
			WriteLogInfo(rich, "LogTimeout", S_O::FormatUINT(settings->base.nLogTimeout));
			WriteLogInfo(rich, "VibroPower", S_O::FormatUINT(settings->base.btVibroPower));
			break;
		case RESTORE_SERIALNO:
		{
			if (!nfcCurrent->bManufact)break;
			WriteLogInfo(rich, "SerialNo", settings->strDevNoFrom);
			break;
		}
		case RESTORE_VOLTDIAP:
		{
			if (!nfcCurrent->bManufact)break;
			if (bAddDescr)D_O::WriteLog(rich, "VoltDiap\r\n", 0xff, 1);
			GAS_RANGE* r[] = { &settings->sensors.o2VRange,&settings->sensors.coVRange,&settings->sensors.h2sVRange };
			BYTE val[] = { settings->sensors.bO2En,settings->sensors.bCOEn,settings->sensors.bH2SEn };
			for (int i = 0; i < 3; i++)
			{
				WriteLogInfo(rich, nfcCurrent->GetGasStringByVal(g[i],val[i]), S_O::FormatUINT(r[i]->wFrom) + " - " + S_O::FormatUINT(r[i]->wTo));
			}
			break;
		}
		case RESTORE_LORA:
		{
			if (bAddDescr)D_O::WriteLog(rich, "Radio\r\n", 0xff, 1);
			if (!settings->interfaces.bLoaded || settings->interfaces.Standard == RT_LORA)
			{
				WriteLogInfo(rich, "Interface", "LORA");
				LORA_SETTINGS* s = &settings->lora;
				LORA_FREQ* lf[] = { &s->f1,&s->f2,&s->f3 };
				str.Format("%08x", s->nAddress);
				WriteLogInfo(rich, "Address", str);
				WriteLogInfo(rich, "Period", S_O::FormatUINT(s->nDataPeriod));
				for (int i = 0; i < 3; i++)
				{
					LORA_FREQ* f = lf[i];
					if (!f->bEn)continue;
					WriteLogInfo(rich, "Freq", S_O::FormatUINT(f->nFreq), 0);
					WriteLogInfo(rich, " Scatter", S_O::FormatUINT(f->nSF), 0);
					WriteLogInfo(rich, " Power", S_O::FormatUINT(f->btPower));
				}
			}
			else if (settings->interfaces.Standard == RT_WIFI)
			{
				WriteLogInfo(rich, "Interface", "WiFi");
				WIFI_SETTINGS* wifi = &settings->wifi;
				WriteLogInfo(rich, "Channel", wifi->RF.Channel==0?"Auto":S_O::FormatUINT(wifi->RF.Channel));
				WriteLogInfo(rich, "Power", S_O::FormatAllDigits((double)wifi->RF.OutputPower / 10, 1));
				WriteLogInfo(rich, "Period", S_O::FormatUINT(wifi->RF.nSendDataPeriod));
				WriteLogInfo(rich, "SSID", CString(wifi->AccPoint.SSID));
				//WriteLogInfo(rich, "SecurityKey", CString(wifi->AccPoint.Password));
				WriteLogInfo(rich, "Local Port", S_O::FormatUINT(wifi->IP.LPort));
				WriteLogInfo(rich, "Remote IP", CString(wifi->IP.Remote));
				WriteLogInfo(rich, "Remote Port", S_O::FormatUINT(wifi->IP.RPort));
				WriteLogInfo(rich, "TCP Server", wifi->AccPoint.Options & WIFI_TCP_SERVER?"On":"Off");
				WriteLogInfo(rich, "TCP Server Port", S_O::FormatUINT(wifi->TCPServerLPort));
			}
			break;
		}
		case RESTORE_O2COEFF:
		case RESTORE_COCOEFF:
		case RESTORE_H2SCOEFF:
		{
			if (!nfcCurrent->bManufact)break;
			int nCount[] = { 19,14,14 };
			UINT nIds[] = { RESTORE_O2COEFF,RESTORE_COCOEFF,RESTORE_H2SCOEFF };
			BYTE enabled[] = { settings->sensors.bO2En,settings->sensors.bCOEn,settings->sensors.bH2SEn };
			int nC = 0;
			f = &settings->sensors.fO2Coeff[0];
			for (int i = 0; i < 3; i++)
			{
				if (enabled[i] && nIds[i] == nId)
				{
					D_O::WriteLog(rich, nfcCurrent->GetGasStringByVal(g[i],enabled[i]) + "\r\n", 0xff, 1);
					for (int k = 0; k < nCount[i]; k++)
					{
						CWnd* w = s->GetDlgItem(IDC_STATIC_1 + (k + nC) * 2);
						if (!w)break;
						WriteLogInfo(rich, D_O::GetWindowText(w), S_O::FormatAllDigits(f[k + nC]));
					}
				}
				nC += nCount[i];
			}
			break;
		}
		case RESTORE_CH4COEFF:
			if (!nfcCurrent->bManufact)break;
			if (settings->sensors.bCH4En)
			{
				if (bAddDescr)D_O::WriteLog(rich, "MIPEX\r\n", 0xff, 1);
				f = (float*)&settings->sensors.fCH4Threshold;
				for (int i = 0; i < 7; i++)
				{
					CWnd* w = s->GetDlgItem(IDC_STATIC_52 + i * 2);
					if (!w)break;
					if (i)str = S_O::FormatAllDigits(f[i]);// str.Format("%f", f[i]);
					else str = S_O::FormatUINT(settings->sensors.fCH4Threshold);
					WriteLogInfo(rich, D_O::GetWindowText(w), str);
				}
				if (settings->mipex.IsValid())
				{
					if (bAddDescr)D_O::WriteLog(rich, "MIPEX coeff\r\n", 0xff, 1);
					WriteLogInfo(rich, "Serial", settings->mipex.GetSerial());
					WriteLogInfo(rich, "RX", S_O::FormatUINT(settings->mipex.RX));
					WriteLogInfo(rich, "RT", S_O::FormatUINT(settings->mipex.RT));
					WriteLogInfo(rich, "T0", S_O::FormatUINT(settings->mipex.T[0]));
					WriteLogInfo(rich, "T1", S_O::FormatUINT(settings->mipex.T[1]));
					WriteLogInfo(rich, "Smin0", S_O::FormatUINT(settings->mipex.Smin[0]));
					WriteLogInfo(rich, "Smin1", S_O::FormatUINT(settings->mipex.Smin[1]));
					WriteLogInfo(rich, "Kscale0", S_O::FormatUINT(settings->mipex.Kscale[0]));
					WriteLogInfo(rich, "Kscale1", S_O::FormatUINT(settings->mipex.Kscale[1]));
					WriteLogInfo(rich, "Ktgd0", S_O::FormatValue(settings->mipex.Ktgd[0]));
					WriteLogInfo(rich, "Ktgd1", S_O::FormatValue(settings->mipex.Ktgd[1]));
					for (int i = 0; i < 6; i++)WriteLogInfo(rich, S_O::Format("GPoly0_%d", i), S_O::FormatValue(settings->mipex.GPoly0[i]));
					for (int i = 0; i < 6; i++)WriteLogInfo(rich, S_O::Format("GPoly1_%d", i), S_O::FormatValue(settings->mipex.GPoly1[i]));
					for (int i = 0; i < 6; i++)WriteLogInfo(rich, S_O::Format("TPoly%d", i), S_O::FormatValue(settings->mipex.TPoly[i]));
					for (int i = 0; i < 20; i++)
					{
						WriteLogInfo(rich, S_O::Format("ZeroO%d", i), S_O::Format("%u %u", settings->mipex.TABZ[i][0], settings->mipex.TABZ[i][1]));
					}
				}
			}
			break;
		case RESTORE_CH4_MULT:
			if (!nfcCurrent->bManufact)break;
			if (settings->sensors.bCH4En)
			{
				if (bAddDescr)D_O::WriteLog(rich, "CH4_Mult\r\n", 0xff, 1);
				WriteLogInfo(rich, "Conc", S_O::FormatUINT(settings->sensors.nSwitchConc));
				for (int i = 0; i < 6; i++)
				{
					WriteLogInfo(rich, S_O::Format("M%d",i+1), S_O::FormatAllDigits(settings->sensors.fCoefCH4_Mult[i]));
				}
				for (int i = 0; i < 6; i++)
				{
					WriteLogInfo(rich, S_O::Format("P%d", i + 1), S_O::FormatAllDigits(settings->sensors.fCoefCH4_Mult2[i]));
				}
			}
			break;
		case RESTORE_ACCEL:
		{
			if (!nfcCurrent->bManufact)break;
			if (bAddDescr)D_O::WriteLog(rich, "Accelerometer\r\n", 0xff, 1);
			f = &settings->sensors.fAccO2;
			for (int k = 0; k < 4; k++)
			{
				CWnd* w = s->GetDlgItem(IDC_STATIC_48 + k * 2);
				if (!w)break;
				WriteLogInfo(rich, D_O::GetWindowText(w), S_O::FormatAllDigits(f[k]));
			}
			break;
		}
		case RESTORE_GASSELECTION:
		{
			if (!nfcCurrent->bManufact)break;
			if (bAddDescr)D_O::WriteLog(rich, "Sensors\r\n", 0xff, 1);
			BYTE en[] = { settings->sensors.bCOEn,settings->sensors.bH2SEn,settings->sensors.bCH4En ,settings->sensors.bO2En };
			for (int i = 0; i < 4; i++)
			{
				if (en[i])
				{
					if (str != "")str += ", ";
					str += nfcCurrent->GetGasStringByVal(1 << i,en[i]);
				}
			}
			WriteLogInfo(rich, "Selected", str);
			break;
		}
		case RESTORE_ALARMS:
		{
			if (bAddDescr)D_O::WriteLog(rich, "Limits\r\n", 0xff, 1);
			BYTE en[] = { settings->sensors.bCOEn,settings->sensors.bH2SEn,settings->sensors.bCH4En ,settings->sensors.bO2En };
			GAS_RANGE* r[] = { &settings->sensors.alarms.coAlarm,&settings->sensors.alarms.h2sAlarm,&settings->sensors.alarms.ch4Alarm,&settings->sensors.alarms.o2Alarm };
			for (int i = 0; i < 4; i++)
			{
				if (!en[i])continue;
				float dDev = nfcCurrent->GetGasDevider(1 << i,1);
				double f = (float)r[i]->wFrom / dDev;
				double f1 = (float)r[i]->wTo / dDev;
				str.Format("%s - %s", S_O::FormatValue(f), S_O::FormatValue(f1));
				WriteLogInfo(rich, nfcCurrent->GetGasStringByVal(1 << i,en[i]), str);
			}
			break;
		}
		case RESTORE_SETTINGS:
		{
			if (!nfcCurrent->bManufact)break;
			CLGSettingsDlg* sett = (CLGSettingsDlg*)main->GetByClass(nfcCurrent, "CLGSettingsDlg");
			if (!sett)return;
			if (bAddDescr)D_O::WriteLog(rich, "PowerSettings\r\n", 0xff, 1);
			CStringArray a;
			sett->GetSettingsAsArray(&settings->dev_settings, a);
			for (int i = 0; i < a.GetSize(); i += 2)
			{
				WriteLogInfo(rich, a[i], a[i + 1]);
			}
			break;
		}
		case RESTORE_USER_LOGPERIOD:
		{
			WriteLogInfo(rich, S_O::LoadString(IDS_LOGPERIODWORK), S_O::FormatUINT(settings->base.nLogTimeout));
			WriteLogInfo(rich, S_O::LoadString(IDS_LOGPERIODALARM), S_O::FormatUINT(settings->dev_settings.LogAlarmTimeOut));
			break;
		}
		case RESTORE_USER_UNITS:
		{
			if (bAddDescr)D_O::WriteLog(rich, S_O::LoadString(IDS_UNITS)+"\r\n", 0xff, 1);
			BYTE en[] = { settings->sensors.bCOEn,settings->sensors.bH2SEn,settings->sensors.bCH4En ,settings->sensors.bO2En };
			for (int i = 0; i < 4; i++)
			{
				if (en[i])
				{
					RM_DATABASEIDLIST l;
					UINT nGas = 1 << i;
					if (nGas == GAS_O2 && !settings->dev_settings.base.O2Chem)l.Add("%VOL");
					else nfcCurrent->GetAvailableUnits(nGas, l);
					WriteLogInfo(rich, nfcCurrent->GetGasStringByVal(nGas,en[i]), l.Get(nfcCurrent->GetUnits(nGas)));
				}
			}
			break;
		}
		case RESTORE_USER_VIBRO:
		{
			WriteLogInfo(rich, S_O::LoadString(IDS_VIBROPOWER), S_O::FormatUINT(settings->base.btVibroPower));
			break;
		}
		}
	}
}

void CLGSaveDlg::GetRestoredSettings(UINT nId, CUIntArray& arr)
{
	if (nId == RESTORE_ALL)
	{

		UINT Ids[] = { RESTORE_SERIALNO,RESTORE_BASESETTINGS,RESTORE_SETTINGS ,RESTORE_LORA ,RESTORE_O2COEFF ,RESTORE_COCOEFF,RESTORE_H2SCOEFF,
					RESTORE_CH4COEFF ,RESTORE_CH4_MULT,RESTORE_ACCEL ,RESTORE_DIAP,RESTORE_VOLTDIAP ,RESTORE_ALARMS,RESTORE_GASSELECTION };
		for (int i = 0; i < sizeof(Ids) / sizeof(UINT*); i++)
		{
			if (Ids[i] == RESTORE_LORA && settings->nSWVer < 80)continue;
			if(Ids[i] == RESTORE_CH4_MULT && (!nfcCurrent->IsHaveProperty(CH4_MULT, settings->nSWVer) ||
				!nfcCurrent->IsHaveProperty(CH4_MULT)))continue;
			arr.Add(Ids[i]);
		}
	}
	else if (nId == RESTORE_ALLCOEFF)
	{
		UINT Ids[] = { RESTORE_O2COEFF ,RESTORE_COCOEFF,RESTORE_H2SCOEFF,
					RESTORE_CH4COEFF ,RESTORE_COEFF,RESTORE_GASSELECTION,RESTORE_CH4_MULT };
		for (int i = 0; i < sizeof(Ids) / sizeof(UINT*); i++)
		{
			if (Ids[i] == RESTORE_CH4_MULT && (!nfcCurrent->IsHaveProperty(CH4_MULT, settings->nSWVer) ||
				!nfcCurrent->IsHaveProperty(CH4_MULT)))continue;
			arr.Add(Ids[i]);
		}
	}
	else if (nId == RESTORE_USER_ALL)
	{
		UINT Ids[] = { RESTORE_USER_LOGPERIOD,RESTORE_LORA,RESTORE_ALARMS,RESTORE_USER_UNITS,RESTORE_USER_VIBRO };
		for (int i = 0; i < sizeof(Ids) / sizeof(UINT*); i++)arr.Add(Ids[i]);
	}
	else arr.Add(nId);
}

void CLGSaveDlg::OnBnClickedRestore()
{
	if (!settings->bLoaded)
	{
		AfxMessageBox(S_O::LoadString(IDS_SETTINGSNOTLOADED));
		return;
	}
	if (nfcCurrent->IsVirtualDevice())return;
	CGridCtrl* gridType = (CGridCtrl*)GetDlgItem(IDC_GRID_TYPES);
	gridType->SaveModifyed();
	bool bGetCoeff = 0, bGetInfo = 0, bGetSettings = 0, bSetSettings = 0;
	CString str, buf;
	bool bAsked = 0,bDifferentSerial= settings->strDevNoFrom != nfcCurrent->GetSerialNumber();
	for (int i = 0; i < rarr.arr.GetSize(); i++)
	{
		RESTORE_TYPE& t = rarr.arr[i];
		if (t.nId == -1 || !t.bSelect)continue;
		if (!nfcCurrent->bManufact)
		{
			if (t.nId == RESTORE_ALL && bDifferentSerial)
			{
				AfxMessageBox(S_O::LoadString(IDS_CANTRESTORESETTINGS));
				return;
			}
		}
	}
	for (int i = 0; i < rarr.arr.GetSize(); i++)
	{
		RESTORE_TYPE& t = rarr.arr[i];
		if (t.nId == -1 || !t.bSelect)continue;
		if (!nfcCurrent->bManufact)
		{
			if (bDifferentSerial)
			{
				if (!bAsked)
				{
					if (AfxMessageBox(S_O::LoadString(IDS_SETTINGSNOTFORTHISDEVICE), MB_YESNO) == IDNO)return;
					bAsked = 1;
				}
			}
			else if (t.nId == RESTORE_ALL)
			{
				if (AfxMessageBox(S_O::LoadString(IDS_RESTOREALERT), MB_YESNO) == IDNO)return;
			}
		}
		else
		{
			if (bDifferentSerial)
			{
				if (!bAsked)
				{
					if (AfxMessageBox(S_O::LoadString(IDS_SETTINGSNOTFORTHISDEVICE), MB_YESNO) == IDNO)return;
					bAsked = 1;
				}
			}
		}
		if (buf != "")buf += "\n";
		buf += t.strDescr;
	}
	if (buf == "")return;
	str.Format("%s\n%s?", S_O::LoadString(IDS_RESTORE), buf);
	if (AfxMessageBox(str, MB_Q) == IDNO)return;
	main->EnableWindow(0);
	CStatusThread* thread = new CStatusThread(this, S_O::LoadString(IDS_RESTORING));
	thread->bShowCancel = 0;
	thread->SetMessage(S_O::LoadString(IDS_WAIT));
	CUIntArray arrDone;
	int i = 0;
	for (int k = 0; k < rarr.arr.GetSize(); k++)
	{
		RESTORE_TYPE& t = rarr.arr[k];
		if (t.nId == -1 || !t.bSelect)continue;
		CUIntArray arr;
		GetRestoredSettings(t.nId, arr);
		for (i = 0; i < arr.GetSize(); i++)
		{
			if (!nfcCurrent->bManufact && bDifferentSerial && !IsUserSetting(arr[i]))break;
			if (S_O::IsAlreadyInArray(arr[i], arrDone))continue;
			arrDone.Add(arr[i]);
			switch (arr[i])
			{
			case RESTORE_USER_LOGPERIOD:
				nfcCurrent->SetLogTimeout(settings->base.nLogTimeout);
				nfcCurrent->dev_settings.LogAlarmTimeOut = settings->dev_settings.LogAlarmTimeOut;
				bGetInfo = bGetSettings = bSetSettings = 1;
				break;
			case RESTORE_USER_VIBRO:
				nfcCurrent->dev_info.base.btVibroPower = settings->base.btVibroPower;
				nfcCurrent->SetVibroPower();
				bGetInfo = 1;
				break;
			case RESTORE_USER_UNITS:
				nfcCurrent->dev_settings.SensorsUnits = settings->dev_settings.SensorsUnits;
				bGetSettings = bSetSettings = 1;
				break;
			case RESTORE_BASESETTINGS:
				nfcCurrent->EnableBeep(settings->base.bBeep);
				nfcCurrent->EnableVibro(settings->base.bVibro);
				nfcCurrent->EnableAlarm(settings->base.bAlarm);
				nfcCurrent->SetLogTimeout(settings->base.nLogTimeout);
				nfcCurrent->dev_info.base.btVibroPower = settings->base.btVibroPower;
				nfcCurrent->SetVibroPower();
				bGetInfo = 1;
				break;
			case RESTORE_SERIALNO:
				nfcCurrent->SetSerialNumber(atoi(settings->strDevNoFrom));
				bGetInfo = 1;
				break;
			case RESTORE_GASSELECTION:
				nfcCurrent->sensors_info.bO2En = settings->sensors.bO2En;
				nfcCurrent->sensors_info.bCOEn = settings->sensors.bCOEn;
				nfcCurrent->sensors_info.bH2SEn = settings->sensors.bH2SEn;
				nfcCurrent->sensors_info.bCH4En = settings->sensors.bCH4En;
				nfcCurrent->SetSensorStatus();
				nfcCurrent->GetSensorStatus();
				break;
			case RESTORE_SETTINGS:
				nfcCurrent->dev_settings = settings->dev_settings;
				if (nfcCurrent->IsHaveProperty(GASPOS) && !nfcCurrent->IsHaveProperty(GASPOS, settings->nSWVer))
				{
					BYTE nPos[] = { 2,3,0,1 };
					nfcCurrent->dev_settings.O2ChemScrPos = 0;
					for (int i = 0; i < 4; i++)nfcCurrent->dev_settings.O2ChemScrPos |= (nPos[i] << (i * 2));
				}
				bGetSettings = bSetSettings = 1;
				break;
			case RESTORE_LORA:
				if (nfcCurrent->IsHaveProperty(INTERFACES) && settings->interfaces.bLoaded)
				{
					nfcCurrent->interfaces = settings->interfaces;
					nfcCurrent->SetInterfaces();
				}
				if (!settings->interfaces.bLoaded || settings->interfaces.Standard == RT_LORA)
				{
					nfcCurrent->lora_settings = settings->lora;
					nfcCurrent->SetLoraSettings();
					nfcCurrent->SetLoraKey();
					if (nfcCurrent->IsHaveProperty(LORAOTAA))nfcCurrent->SetLoraOTAASettings();
					nfcCurrent->GetLoraSettings();
					if (nfcCurrent->IsHaveProperty(LORAOTAA))nfcCurrent->GetLoraOTAASettings();
				}
				else if (settings->interfaces.Standard == RT_WIFI)
				{
					nfcCurrent->wifi = settings->wifi;
					nfcCurrent->SetWifiSettings();
				}
				break;
			case RESTORE_O2COEFF:
				memcpy(nfcCurrent->sensors_info.fO2Coeff, settings->sensors.fO2Coeff, sizeof(nfcCurrent->sensors_info.fO2Coeff));
				nfcCurrent->SetO2Coeff();
				bGetCoeff = 1;
				break;
			case RESTORE_COCOEFF:
				memcpy(nfcCurrent->sensors_info.fCOCoeff, settings->sensors.fCOCoeff, sizeof(nfcCurrent->sensors_info.fCOCoeff));
				nfcCurrent->SetCOCoeff();
				bGetCoeff = 1;
				break;
			case RESTORE_H2SCOEFF:
				memcpy(nfcCurrent->sensors_info.fH2SCoeff, settings->sensors.fH2SCoeff, sizeof(nfcCurrent->sensors_info.fH2SCoeff));
				nfcCurrent->SetH2SCoeff();
				bGetCoeff = 1;
				break;
			case RESTORE_CH4COEFF:
				nfcCurrent->sensors_info.fCH4Threshold = settings->sensors.fCH4Threshold;
				memcpy(&nfcCurrent->sensors_info.fCH4K1, &settings->sensors.fCH4K1, 6 * sizeof(float));
				nfcCurrent->SetCH4Coeff();
				if (nfcCurrent->IsHaveProperty(MIPEXCOEFF) && settings->mipex.IsValid())nfcCurrent->MipexSetBackup(&settings->mipex);
				bGetCoeff = 1;
				break;
			case RESTORE_CH4_MULT:
				if (!nfcCurrent->IsHaveProperty(CH4_MULT) || !nfcCurrent->IsHaveProperty(CH4_MULT,settings->nSWVer))continue;
				memcpy(&nfcCurrent->sensors_info.fCoefCH4_Mult, &settings->sensors.fCoefCH4_Mult, 6 * sizeof(float));
				memcpy(&nfcCurrent->sensors_info.fCoefCH4_Mult2, &settings->sensors.fCoefCH4_Mult2, 6 * sizeof(float));
				nfcCurrent->sensors_info.nSwitchConc = settings->sensors.nSwitchConc;
				nfcCurrent->SetCH4CoeffMult();
				bGetCoeff = 1;
				break;
			case RESTORE_ACCEL:
				memcpy(&nfcCurrent->sensors_info.fAccO2, &settings->sensors.fAccO2, 4 * sizeof(float));
				nfcCurrent->SetSensorAccel();
				bGetCoeff = 1;
				break;
			case RESTORE_DIAP:
				nfcCurrent->sensors_info.ch4Range = settings->sensors.ch4Range;
				nfcCurrent->sensors_info.coRange = settings->sensors.coRange;
				nfcCurrent->sensors_info.h2sRange = settings->sensors.h2sRange;
				nfcCurrent->sensors_info.o2Range = settings->sensors.o2Range;
				nfcCurrent->SetGasRange();
				nfcCurrent->GetGasRange();
				break;
			case RESTORE_VOLTDIAP:
				nfcCurrent->sensors_info.o2VRange = settings->sensors.o2VRange;
				nfcCurrent->sensors_info.coVRange = settings->sensors.coVRange;
				nfcCurrent->sensors_info.h2sVRange = settings->sensors.h2sVRange;
				nfcCurrent->SetSensorVRange();
				nfcCurrent->GetSensorVoltRange();
				break;
			case RESTORE_COEFF:
				nfcCurrent->dev_settings.base = settings->dev_settings.base;
				bGetSettings = bSetSettings = 1;
				break;
			case RESTORE_ALARMS:
			{
				bool bRestoreUnits = 0;
				BYTE btUnits = 0;
				if (!nfcCurrent->dev_settings.bLoaded)
				{
					nfcCurrent->GetSettings();
					nfcCurrent->WaitComplete(1);
					if (nfcCurrent->dev_settings.SensorsUnits != settings->dev_settings.SensorsUnits)
					{
						btUnits = nfcCurrent->dev_settings.SensorsUnits;
						nfcCurrent->dev_settings.SensorsUnits = settings->dev_settings.SensorsUnits;
						nfcCurrent->SetSettings();
						nfcCurrent->WaitComplete(1);
						bRestoreUnits=1;
					}
				}
				nfcCurrent->sensors_info.alarms.ch4Alarm = settings->sensors.alarms.ch4Alarm;
				nfcCurrent->sensors_info.alarms.o2Alarm = settings->sensors.alarms.o2Alarm;
				nfcCurrent->sensors_info.alarms.coAlarm = settings->sensors.alarms.coAlarm;
				nfcCurrent->sensors_info.alarms.h2sAlarm = settings->sensors.alarms.h2sAlarm;
				nfcCurrent->SetAlarms();
				nfcCurrent->GetAlarms();
				if (bRestoreUnits)
				{
					nfcCurrent->dev_settings.SensorsUnits = btUnits;
					bGetSettings = bSetSettings = 1;
				}
				break;
			}
			}
		}
		if (i != arr.GetSize())break;
	}
	nfcCurrent->WaitComplete(1);
	if (bSetSettings)
	{
		nfcCurrent->SetSettings();
		nfcCurrent->WaitComplete(1);
	}
	if (bGetInfo)nfcCurrent->GetDeviceInfo();
	if (bGetSettings)nfcCurrent->GetSettings();
	if (bGetCoeff)nfcCurrent->GetCoeffsOnly();
	nfcCurrent->WaitComplete(1);
	main->EnableWindow(1);
	thread->Stop();
}

void CLGSaveDlg::OnTabSelected()
{

}

void CLGSaveDlg::OnBnClickedSave2()
{
	Save(1);
}
