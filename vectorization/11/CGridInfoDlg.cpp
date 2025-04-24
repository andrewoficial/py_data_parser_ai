// CGridInfoDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "CGridInfoDlg.h"
#include "afxdialogex.h"
#include "NFC_Device.h"
#include "GridCtrl.h"


// CGridInfoDlg dialog

IMPLEMENT_DYNAMIC(CGridInfoDlg, CDialogEx)

CGridInfoDlg::CGridInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GRIDINFO, pParent)
{
	nfc = 0;
}

CGridInfoDlg::~CGridInfoDlg()
{
}

void CGridInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGridInfoDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CGridInfoDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CGridInfoDlg message handlers


BOOL CGridInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID);		
	switch (mode)
	{
	case MIPEX:
	{
		grid->bFirstColNarrow = 0;
		grid->bChangeDataAllowed = 1;
		grid->columnnames.Add(S_O::LoadString(IDS_PARAMETER));
		grid->columnnames.Add(S_O::LoadString(IDS_VALUE));
		grid->columnnames.Add(S_O::LoadString(IDS_VALUE)+"2");
		grid->AddRow("Serial", nfc->mipex.GetSerial());
		grid->AddRow("RX", S_O::FormatUINT(nfc->mipex.RX));
		grid->AddRow("RT", S_O::FormatUINT(nfc->mipex.RT));
		grid->AddRow("T0", S_O::FormatUINT(nfc->mipex.T[0]));
		grid->AddRow("T1", S_O::FormatUINT(nfc->mipex.T[1]));
		grid->AddRow("Smin0", S_O::FormatUINT(nfc->mipex.Smin[0]));
		grid->AddRow("Smin1", S_O::FormatUINT(nfc->mipex.Smin[1]));
		grid->AddRow("Kscale0", S_O::FormatUINT(nfc->mipex.Kscale[0]));
		grid->AddRow("Kscale1", S_O::FormatUINT(nfc->mipex.Kscale[1]));
		grid->AddRow("Ktgd0", S_O::FormatValue(nfc->mipex.Ktgd[0]));
		grid->AddRow("Ktgd1", S_O::FormatValue(nfc->mipex.Ktgd[1]));
		for (int i = 0; i < 6; i++)grid->AddRow(S_O::Format("GPoly0_%d", i), S_O::FormatValue(nfc->mipex.GPoly0[i]));
		for (int i = 0; i < 6; i++)grid->AddRow(S_O::Format("GPoly1_%d", i), S_O::FormatValue(nfc->mipex.GPoly1[i]));
		for (int i = 0; i < 6; i++)grid->AddRow(S_O::Format("TPoly%d", i), S_O::FormatValue(nfc->mipex.TPoly[i]));
		for (int i = 0; i < 20; i++)
		{
			grid->AddRow(S_O::Format("ZeroO%d", i), S_O::FormatValue(nfc->mipex.TABZ[i][0]), S_O::FormatValue(nfc->mipex.TABZ[i][1]));			
		}			
		grid->changed.SetSize(grid->names.GetSize() / 2);
		for (int i = 0; i < grid->changed.GetSize(); i++)grid->changed[i] = 0;
		GetDlgItem(IDOK)->SetWindowText(S_O::LoadString(IDS_SAVEINGSETTINGS));
		SetWindowText(S_O::LoadString(IDS_COEFF));
		break;
	}
	default:
		EndDialog(IDCANCEL);
		return 0;		
	}
	grid->FillData();
	return TRUE;
}


void CGridInfoDlg::OnBnClickedOk()
{
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID);
	grid->SaveModifyed();
	switch (mode)
	{
	case MIPEX:
	{
		bool bm = 0;
		MIPEX_COEFF c;
		c = nfc->mipex;
		CString str,str2;
		for (int i = 1; i < grid->changed.GetSize(); i++)
		{
			if (!grid->changed[i])continue;
			str = grid->names[i * 3 + 1];
			str2 = grid->names[i * 3 + 2];
			if (i == 1)c.RX = S_O::ToUINT(str);
			if (i == 2)c.RT = S_O::ToUINT(str);
			if (i == 3)c.T[0] = S_O::ToUINT(str);
			if (i == 4)c.T[1] = S_O::ToUINT(str);
			if (i == 5)c.Smin[0] = S_O::ToUINT(str);
			if (i == 6)c.Smin[1] = S_O::ToUINT(str);
			if (i == 7)c.Kscale[0] = S_O::ToUINT(str);
			if (i == 8)c.Kscale[1] = S_O::ToUINT(str);
			if (i == 9)c.Ktgd[0] = S_O::ToDouble(str);
			if (i == 10)c.Ktgd[1] = S_O::ToDouble(str);
			if (i >= 11 && i <= 16)c.GPoly0[i - 11] = S_O::ToDouble(str);
			if (i >= 17 && i <= 22)c.GPoly1[i - 17] = S_O::ToDouble(str);
			if (i >= 23 && i <= 28)c.TPoly[i - 23] = S_O::ToDouble(str);
			if (i >= 29 && i <= 48)
			{
				int n = (i - 29);
				c.TABZ[n][0] = S_O::ToUINT(str);
				c.TABZ[n][1] = S_O::ToUINT(str2);
			}
			bm = 1;
		}
		if (!bm)return;
		nfc->MipexSetBackup(&c);
		if (!nfc->WaitComplete(1))return;
		break;
	}
	}
	CDialogEx::OnOK();
}
