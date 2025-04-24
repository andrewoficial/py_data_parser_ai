// LGFirmwareDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "LGFirmwareDlg.h"
#include "afxdialogex.h"
#include "NFC_Device.h"


// CLGFirmwareDlg dialog

IMPLEMENT_DYNAMIC(CLGFirmwareDlg, CLongGasBaseDlg)

CLGFirmwareDlg::CLGFirmwareDlg(CWnd* pParent /*=nullptr*/)
	: CLongGasBaseDlg(IDD_LG_FIRMWARE, pParent)
{

}

CLGFirmwareDlg::~CLGFirmwareDlg()
{
}

void CLGFirmwareDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
}

BEGIN_MESSAGE_MAP(CLGFirmwareDlg, CLongGasBaseDlg)
	ON_BN_CLICKED(IDC_SENDFIRMWARE, &CLGFirmwareDlg::OnBnClickedSendfirmware)
	ON_BN_CLICKED(IDC_SENDFIRMWAREALL, &CLGFirmwareDlg::OnBnClickedSendfirmwareall)
END_MESSAGE_MAP()


BOOL CLGFirmwareDlg::OnInitDialog()
{
	CLongGasBaseDlg::OnInitDialog();
	return TRUE;
}

bool CLGFirmwareDlg::CommandDone(CNFC_Command* c)
{
	if (!c->nRet)return 1;
	if (c->nCommand == FirmwareStatus)
	{
		if (c->dev == nfcCurrent)
		{
			int nBlock = c->nParam & 0xffff, nTotal = (c->nParam >> 16) & 0xffff;
			if (nTotal)
			{
				if (nBlock == 0)m_Progress.SetRange32(0, nTotal);
				m_Progress.SetPos(nBlock);
			}
			if (IsWindowVisible())
			{
				ITaskbarList3* l = main->GetTaskbarList();
				if (l)
				{
					if (nTotal == 0)l->SetProgressState(main->GetSafeHwnd(), nBlock ? TBPF_NOPROGRESS : TBPF_ERROR);
					else
					{
						if (nBlock == 0)l->SetProgressState(main->GetSafeHwnd(), TBPF_NORMAL);
						l->SetProgressValue(main->GetSafeHwnd(), nBlock, nTotal);
					}
				}
			}
		}
	}
	return 1;
}

void CLGFirmwareDlg::OnBnClickedSendfirmware()
{
	StartFirmware(0);
}

void CLGFirmwareDlg::OnBnClickedSendfirmwareall()
{
	StartFirmware(1);
}

void CLGFirmwareDlg::StartFirmware(bool bAll)
{
	CString str, strLastSave = CMainAppTemplate::GetLastSavePath();
	str.Format("%s\\*.bin", strLastSave);
	CFileDialog fd(1, "*.bin", str, OFN_EXPLORER | OFN_HIDEREADONLY, "Firmware Files (*.bin)|*.bin||", NULL);
	if (fd.DoModal() == IDCANCEL)return;
	nfcCurrent->firmware_settings.strPath = fd.GetPathName();
	if (bAll)main->StartAll(STARTALL_FIRMWARE, nfcCurrent);
	else nfcCurrent->StartFirmware();
	CMainAppTemplate::SetLastSavePath(fd.GetPathName());
}

void CLGFirmwareDlg::OnTabSelected()
{
	ITaskbarList3* l = main->GetTaskbarList();
	if (nfcCurrent->nCurrentOperation == STARTALL_FIRMWARE)
	{
		int nBlock = nfcCurrent->nFirmwareProgress & 0xffff, nTotal = (nfcCurrent->nFirmwareProgress >> 16) & 0xffff;
		if (nTotal)
		{
			if (nBlock == 0)m_Progress.SetRange32(0, nTotal);
			m_Progress.SetPos(nBlock);
		}
		if (l)
		{
			if (nTotal == 0)l->SetProgressState(main->GetSafeHwnd(), nBlock ? TBPF_NOPROGRESS : TBPF_ERROR);
			else
			{
				if (nBlock == 0)l->SetProgressState(main->GetSafeHwnd(), TBPF_NORMAL);
				l->SetProgressValue(main->GetSafeHwnd(), nBlock, nTotal);
			}
		}
	}
	else
	{
		m_Progress.SetPos(0);
		if (l)l->SetProgressState(main->GetSafeHwnd(), TBPF_NOPROGRESS);
	}


}
