// LGWorkStatDlg.cpp : implementation file
//

#include "pch.h"

#include "LongGas.h"
#include "LGWorkStatDlg.h"
#include "afxdialogex.h"
#include "NFC_Device.h"
#include "GridCtrl.h"


// CLGWorkStatDlg dialog

IMPLEMENT_DYNAMIC(CLGWorkStatDlg, CLongGasBaseDlg)

CLGWorkStatDlg::CLGWorkStatDlg(CWnd* pParent /*=nullptr*/)
	: CLongGasBaseDlg(IDD_LG_WORKSTAT, pParent)
{

}

CLGWorkStatDlg::~CLGWorkStatDlg()
{
}

void CLGWorkStatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLGWorkStatDlg, CLongGasBaseDlg)
	ON_BN_CLICKED(IDC_GET, &CLGWorkStatDlg::OnBnClickedGet)
	ON_BN_CLICKED(IDC_RESET, &CLGWorkStatDlg::OnBnClickedReset)
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CLGWorkStatDlg message handlers


BOOL CLGWorkStatDlg::OnInitDialog()
{
	CLongGasBaseDlg::OnInitDialog();
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_STAT);
	grid->bFirstColNarrow = 0;
	grid->columnnames.Add(S_O::LoadString(IDS_PARAMETER));
	grid->columnnames.Add(S_O::LoadString(IDS_VALUE));
	grid->FillData();
	return TRUE; 
}

void CLGWorkStatDlg::FillGrid()
{
	char* h[] = { "AlarmTime", "ScrLEDTime", "WorkTime", "StopTime", "SleepTime", "RunTime", "StandByTime", "TurnOffTime", "WorkFrozen20Time"
			, "WorkFrozen10Time", "WorkFrozen0Time", "WorkHeat10Time", "WorkHeat20Time", "WorkHeat30Time", "WorkHeat40Time", "WorkHeatTime", "CRC16" };
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_STAT);
	grid->names.RemoveAll();
	UINT* s = &nfcCurrent->work_stat.AlarmTime;
	for (int i = 0; i < sizeof(h) / sizeof(char*); i++)
	{
		grid->names.Add(h[i]);
		grid->names.Add(S_O::FormatUINT(s[i]));
	}
	grid->FillData();
}

bool CLGWorkStatDlg::CommandDone(CNFC_Command* c)
{
	if (!c->nRet)return 1;
	switch (c->nCommand)
	{
	case GetWorkStatByte:
	{
		if (c->dev == nfcCurrent)FillGrid();
		break;
	}
	}
	return 1;
}

void CLGWorkStatDlg::OnBnClickedGet()
{
	nfcCurrent->GetWorkStat();
}

void CLGWorkStatDlg::OnBnClickedReset()
{
	nfcCurrent->ResetWorkStat();
}

void CLGWorkStatDlg::OnTabSelected()
{
	if (!nfcCurrent->work_stat.bLoaded)nfcCurrent->GetWorkStat();
	FillGrid();
}

void CLGWorkStatDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CLongGasBaseDlg::OnShowWindow(bShow, nStatus);
	if (bShow && IsWindowEnabled() && nfcCurrent)
	{
	}
}


void CLGWorkStatDlg::OnSize(UINT nType, int cx, int cy)
{
	CLongGasBaseDlg::OnSize(nType, cx, cy);
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_STAT);
	if(grid)grid->FillData();
}
