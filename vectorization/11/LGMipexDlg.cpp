// LGMipexDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "LGMipexDlg.h"
#include "afxdialogex.h"
#include "NFC_Device.h"
#include <DialogOperations.h>
#include <LongNameFile.h>
#include "CGridInfoDlg.h"
#include "GridCtrl.h"


// CLGMipexDlg dialog
extern CString g_szCurrentDir;

IMPLEMENT_DYNAMIC(CLGMipexDlg, CDialogEx)

CLGMipexDlg::CLGMipexDlg(CWnd* pParent /*=nullptr*/)
	: CLongGasBaseDlg(IDD_LG_MIPEX, pParent)
	, m_RepeatTime(_T(""))
	, m_bMipex4(FALSE)
{
	m_Command = AfxGetApp()->GetProfileString("MIPEXDlg", "LastCommand", "");
	m_RepeatTime = AfxGetApp()->GetProfileString("MIPEXDlg", "RepeatTime", "5");
	m_bMipex4 = 0;// AfxGetApp()->GetProfileInt("MIPEXDlg", "Mipex4", 1);
	strFolder = AfxGetApp()->GetProfileString("MIPEXDlg", "SaveFolder", g_szCurrentDir);
	bSave = 0;
}

CLGMipexDlg::~CLGMipexDlg()
{
	
}

void CLGMipexDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_COMMAND, m_Command);
	//  DDX_Text(pDX, IDC_ANSWER, m_Answer);
	//  DDX_Text(pDX, IDC_REPEATTIME, m_nRepeatTime);
	DDX_Text(pDX, IDC_REPEATTIME, m_RepeatTime);
	//  DDX_Check(pDX, IDC_AUTOSAVE, m_bAutosave);
	DDX_Check(pDX, IDC_MIPEX4, m_bMipex4);
	DDX_Control(pDX, IDC_FILEPATH, m_FilePath);
}


BEGIN_MESSAGE_MAP(CLGMipexDlg, CLongGasBaseDlg)
	ON_BN_CLICKED(IDC_BUTTON1, &CLGMipexDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_SENDALL, &CLGMipexDlg::OnBnClickedSendall)
	ON_BN_CLICKED(IDC_REPEAT, &CLGMipexDlg::OnBnClickedRepeat)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CLEAR, &CLGMipexDlg::OnBnClickedClear)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_REPEATALL, &CLGMipexDlg::OnBnClickedRepeatall)
	ON_BN_CLICKED(IDC_MIPEX4, &CLGMipexDlg::OnBnClickedMipex4)
	ON_BN_CLICKED(IDC_FROMFILE, &CLGMipexDlg::OnBnClickedFromfile)
	ON_BN_CLICKED(IDC_OPENFOLDER, &CLGMipexDlg::OnBnClickedOpenfolder)
	ON_BN_CLICKED(IDC_BACKUP, &CLGMipexDlg::OnBnClickedBackup)
	ON_BN_CLICKED(IDC_RESTORE, &CLGMipexDlg::OnBnClickedRestore)
	ON_BN_CLICKED(IDC_COEFF, &CLGMipexDlg::OnBnClickedCoeff)
	ON_BN_CLICKED(IDC_LASTCOMMAND, &CLGMipexDlg::OnBnClickedLastcommand)
END_MESSAGE_MAP()


// CLGMipexDlg message handlers


BOOL CLGMipexDlg::OnInitDialog()
{
	CLongGasBaseDlg::OnInitDialog();
	CheckRadioButton(IDC_RADIO_MIPEX, IDC_RADIO_SPI, AfxGetApp()->GetProfileInt("MIPEXDlg", "MIPEXDest", IDC_RADIO_MIPEX));
	CheckRadioButton(IDC_RADIO_D, IDC_RADIO_DA, AfxGetApp()->GetProfileInt("MIPEXDlg", "MIPEXEnd", IDC_RADIO_D));
	nfcCurrent->additional_pars.bCheckMIPEXFAnswer = m_bMipex4;
	FillGrid();
	return TRUE;
}

void CLGMipexDlg::ShowControls()
{
	UINT ids[] = { IDC_BACKUP ,IDC_RESTORE ,IDC_COEFF };
	for (int i = 0; i < sizeof(ids) / sizeof(UINT); i++)
	{
		GetDlgItem(ids[i])->ShowWindow(nfcCurrent->IsHaveProperty(MIPEXCOEFF));
	}
	GetDlgItem(IDC_LASTCOMMAND)->ShowWindow(nfcCurrent->IsHaveProperty(MIPEX_LASTREAD));
}

void CLGMipexDlg::FillGrid()
{
	CGridCtrl* grid = (CGridCtrl*)GetDlgItem(IDC_GRID_RESULT);
	grid->nGridId = GRIDID_DCAR;
	grid->FillData(&nfcCurrent->dcarMIPEX);
}

void CLGMipexDlg::LogAnswer(CNFC_Command* c, DELAYED_COMMAND_RESULT& r,bool bShowIfError)
{
	r.tm = CDateTime::GetCurrent();
	r.strCommand = c->dev->additional_pars.strMIPEXCommand;
	r.strDeviceId = c->dev->GetSerialNumber();
	if (c->nRet)
	{
		if (dtStart.IsNull())dtStart = CDateTime::GetCurrentTime();		
		r.strResult = c->dev->GetMIPEXAnswer(c->get);
		CStringArray arr;
		S_O::ParseString(r.strResult, arr, "\r");
		for (int i = 0; i < arr.GetSize(); i++)
		{
			if (arr[i].GetLength() == 0)continue;
			r.strResult = arr[i];
			if(!bShowIfError)c->dev->dcarMIPEX.arr.Add(r);
			if (c->dev == nfcCurrent)
			{
				/*D_O::WriteLog(m_Answer, q.tm.Format("%H:%M:%S") + "   ", 0xff0000);
				D_O::WriteLog(m_Answer, q.strReply);
				D_O::WriteLog(m_Answer, "\r\n", 0, 0, 1);*/
				FillGrid();
			}
		}
	}
	else if (bShowIfError)
	{
		r.strResult.Format("MIPEX - %s", S_O::LoadString(IDS_NOANSWER));
		c->dev->dcarMIPEX.arr.Add(r);
		r.strResult = "";
		if (c->dev == nfcCurrent)FillGrid();
		/*D_O::WriteLog(m_Answer, q.tm.Format("%H:%M:%S") + "   ", 0xff0000);
		D_O::WriteLog(m_Answer, S_O::Format("MIPEX - %s", S_O::LoadString(IDS_NOANSWER)));
		D_O::WriteLog(m_Answer, "\r\n", 0, 0, 1);*/
	}
}

bool CLGMipexDlg::CommandDone(CNFC_Command* c)
{	
	switch (c->nCommand)
	{
	case GetLastMIPEXCommand:
	{
		DELAYED_COMMAND_RESULT r;
		if (c->dev->ParseLastMIPEXCommand(c->get, r))
		{
			c->dev->dcar.arr.Add(r);
			if (c->dev == nfcCurrent)FillGrid();
		}
		break;
	}
	case NFCMipexRestore:
	case NFCMipexBackup:
	{
		DELAYED_COMMAND_RESULT r;
		LogAnswer(c,r, 1);
		break;
	}
	case GetInfDeviceByte:
		ShowControls();
		break;	
	case SetCommandMIPEXByte:
	case SetCommandUARTByte:
	case SetCommandSPIByte:
	{
		if (c->dev->additional_pars.nMIPEXCommandId != -1)break;
		DELAYED_COMMAND_RESULT r;
		if (c->nRet)
		{
			if (dtStart.IsNull())dtStart = CDateTime::GetCurrentTime();
			LogAnswer(c,r, 0);
			if (bSave && (c->dev->bInMIPEXRepeat || c->dev->bInMonitoring))
			{				
				CString str;
				str.Format("%s\\%s", strFolder, GetSaveFileName(c->dev,1));
				if (c->dev == nfcCurrent)m_FilePath.SetWindowText(str);
				SaveData(c->dev,str, 1);
			}
		}
		bool bAllowRepeat = 1;
		if (c->dev->additional_pars.bRetryMIPEXCommand)
		{
			if (r.strResult!="" || c->dev->additional_pars.nSendMIPEXCommandTry)
			{
				c->dev->additional_pars.bRetryMIPEXCommand = 0;
				OPERATION_STATUS* s = new OPERATION_STATUS;
				s->nStatus = 1;
				s->nOperation = STARTALL_MIPEXCOMMAND;
				if (r.strResult != "")
				{
					s->strResult = r.strResult;
					c->dev->additional_pars.nSendMIPEXResult = RESULT_GOOD;
				}
				else
				{
					s->strResult.Format("MIPEX - %s", S_O::LoadString(IDS_NOANSWER));
					c->dev->additional_pars.nSendMIPEXResult = RESULT_ERROR;
				}
				main->PostMessage(WM_NFC_OPERATION_DONE, (WPARAM)c->dev, (LPARAM)s);
			}
			else
			{
				c->dev->additional_pars.nSendMIPEXCommandTry++;
				c->dev->SendCommandMIPEX();
				bAllowRepeat = 0;
			}
		}
		if (c->dev->bInMIPEXRepeat && bAllowRepeat)
		{
			UpdateData();
			int n = atoi(m_RepeatTime);
			if (n <= 0)n = 1;
			SetTimer(c->dev->nDeviceNum, n * 1000, 0);
		}
		break;
	}
	}
	return 1;
}

bool CLGMipexDlg::UpdateMIPEXCommand(CNFC_Device* dev)
{
	UpdateData();
	if (!dev->UpdateMIPEXCommand(m_Command))return 0;
	dev->additional_pars.nMIPEXCommandId = -1;
	dev->additional_pars.nMipexDest = GetCheckedRadioButton(IDC_RADIO_MIPEX, IDC_RADIO_SPI) - IDC_RADIO_MIPEX;
	if (GetCheckedRadioButton(IDC_RADIO_D, IDC_RADIO_DA) == IDC_RADIO_D)dev->additional_pars.strMipexEnd = "\r";
	else dev->additional_pars.strMipexEnd = "\r\n";
	return 1;
}

bool CLGMipexDlg::SendCommand(CNFC_Device* dev,bool bAll)
{
	if (!UpdateMIPEXCommand(dev))return 0;
	if (bAll)
	{
		main->StartAll(STARTALL_MIPEXCOMMAND, dev);
	}
	else
	{
		dev->additional_pars.bRetryMIPEXCommand = 0;
		dev->SendCommandMIPEX();
	}
	return 1;
}

void CLGMipexDlg::OnBnClickedButton1()
{
	SendCommand(nfcCurrent,0);	
}

void CLGMipexDlg::OnBnClickedSendall()
{
	SendCommand(nfcCurrent,1);
}

void CLGMipexDlg::StartRepeat(CNFC_Device* dev)
{
	if (dev->bInMIPEXRepeat)return;
	else
	{
		dtStart = CDateTime::GetCurrentTime();
		if (!SendCommand(dev,0))return;
		dev->dcarMIPEX.arr.RemoveAll();		
		dev->bInMIPEXRepeat = 1;
		if (dev == nfcCurrent)
		{
			FillGrid();
			GetDlgItem(IDC_REPEAT)->SetWindowText("Stop");
		}
	}
}

void CLGMipexDlg::StopRepeat(CNFC_Device* dev)
{
	if (dev->bInMIPEXRepeat)
	{
		dev->bInMIPEXRepeat = 0;
		KillTimer(dev->nDeviceNum);
		if (dev == nfcCurrent)
		{
			GetDlgItem(IDC_REPEAT)->SetWindowText(S_O::LoadString(IDS_CYCLICALLY));
		}
	}
}

void CLGMipexDlg::OnBnClickedRepeat()
{
	if (nfcCurrent->bInMIPEXRepeat)
	{
		main->BreakAll(nfcCurrent, STARTALL_MIPEX);
		return;
	}
	GetFolderName(nfcCurrent,0);
	StartRepeat(nfcCurrent);
}

void CLGMipexDlg::OnTimer(UINT_PTR nIDEvent)
{
	CNFC_Device* dev = main->GetDeviceByNum(nIDEvent);
	if(dev)SendCommand(dev,0);
	KillTimer(nIDEvent);
	CLongGasBaseDlg::OnTimer(nIDEvent);
}

void CLGMipexDlg::OnBnClickedClear()
{
	nfcCurrent->dcarMIPEX.arr.RemoveAll();
	FillGrid();
}

void CLGMipexDlg::OnDestroy()
{
	AfxGetApp()->WriteProfileString("MIPEXDlg", "LastCommand", m_Command);
	AfxGetApp()->WriteProfileString("MIPEXDlg", "RepeatTime", m_RepeatTime);
	AfxGetApp()->WriteProfileInt("MIPEXDlg", "Mipex4", m_bMipex4);
	AfxGetApp()->WriteProfileInt("MIPEXDlg", "MIPEXDest", GetCheckedRadioButton(IDC_RADIO_MIPEX, IDC_RADIO_SPI));
	AfxGetApp()->WriteProfileInt("MIPEXDlg", "MIPEXEnd", GetCheckedRadioButton(IDC_RADIO_D, IDC_RADIO_DA));
	CLongGasBaseDlg::OnDestroy();
}

void CLGMipexDlg::ChangeAll(CNFC_Device* nfc, UINT nMode, CHANGEALL* c)
{
	
}

void CLGMipexDlg::OnBnClickedRepeatall()
{
	if (!UpdateMIPEXCommand(nfcCurrent))return;
	GetFolderName(nfcCurrent,1);
	main->StartAll(STARTALL_MIPEX, nfcCurrent);
}

CString CLGMipexDlg::GetSaveFileName(CNFC_Device* nfc, bool bAuto)
{
	CString str, out;	
	if (dtStart.IsNull())dtStart = CDateTime::GetCurrent();
	out.Format("%s_%s_M.csv", nfc->GetSerialNumber(), dtStart.Format("%d.%m.%y_%H-%M"));
	return out;
}

bool CLGMipexDlg::SaveData(CNFC_Device* nfc, CString strPath, bool bLast)
{
	if (!nfc->dcarMIPEX.arr.GetSize())return 0;
	CLongNameFile f;
	CString str, buf;
	int nMode = CFile::modeCreate | CFile::modeReadWrite;
	if (bLast)nMode |= CFile::modeNoTruncate;
	if (!f.Open(strPath, nMode))return 0;
	if(bLast)f.SeekToEnd();
	int nFrom = bLast ? (nfc->dcarMIPEX.arr.GetSize() - 1) : 0;
	for (int i = nFrom; i < nfc->dcarMIPEX.arr.GetSize(); i++)
	{
		buf.Format("%s;%s\r\n", nfc->dcarMIPEX.arr[i].tm.FormatDB(), nfc->dcarMIPEX.arr[i].strResult);
		str += buf;
	}
	f.Write(str, str.GetLength());
	return 1;
}

//void CLGMipexDlg::OnBnClickedSave()
//{
//	if (!nfc->MipexData.GetSize())return;
//	CString str;
//	str.Format("%s\\%s", CMainAppTemplate::GetLastSavePath(), GetSaveFileName(0));
//	CFileDialog fd(FALSE, "*.csv", str, OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Comma Separated Values (*.csv)|*.csv||", NULL);
//	if (fd.DoModal() == IDCANCEL)return;
//	CMainAppTemplate::SetLastSavePath(fd.GetPathName());
//	SaveData(fd.GetPathName(), 0);
//}

//void CLGMipexDlg::OnBnClickedSaveall()
//{
//	CString strPath;
//	if (!D_O::SelectFolder(strPath, m_hWnd, "Выберите папку для сохранения", CMainAppTemplate::GetLastSavePath()))return;
//	CString str;
//	str.Format("%s\\%s",strPath, GetSaveFileName(0));
//	SaveData(str, 0);
//	CHANGEALL c;
//	c.str = strPath;
//	main->ChangeAll(this, CHANGEALL_MIPEXSAVE, &c);
//}

void CLGMipexDlg::OnBnClickedMipex4()
{
	nfcCurrent->additional_pars.bCheckMIPEXFAnswer = m_bMipex4;
}

void CLGMipexDlg::OnTabSelected()
{
	ShowControls();
	FillGrid();
	m_FilePath.SetWindowText(bSave ? S_O::Format("%s\\%s", strFolder, GetSaveFileName(nfcCurrent, 1)) : "");
}

void CLGMipexDlg::OnBnClickedFromfile()
{
	CString str, strLastSave = CMainAppTemplate::GetLastSavePath(),buf;
	str.Format("%s\\*.bin", strLastSave);
	CFileDialog fd(1, "*.bin", str, OFN_EXPLORER | OFN_HIDEREADONLY|OFN_LONGNAMES, "Binary Files (*.bin)|*.bin||", NULL);
	if (fd.DoModal() == IDCANCEL)return;
	strLastSave = fd.GetPathName();
	CMainAppTemplate::SetLastSavePath(strLastSave);
	CLongNameFile f;
	if (!f.Open(strLastSave, CFile::modeRead))return;
	if (!S_O::ReadAllFile(f, buf))return;
	m_Command = "";
	for (int i = 0; i < buf.GetLength(); i++)
	{
		BYTE b = buf.GetAt(i);
		if (isalnum(b))m_Command += (char)b;
		else
		{
			str.Format("\\x%02x", (UINT)b);
			m_Command += str;
		}
	}
	UpdateData(0);
}

void CLGMipexDlg::GetFolderName(CNFC_Device* nfc, bool bAll)
{
	bSave = D_O::SelectFolder(strFolder, m_hWnd, S_O::LoadString(IDS_SELECTSAVEPATH), strFolder);
	if(nfc==nfcCurrent)m_FilePath.SetWindowText(bSave ? S_O::Format("%s\\%s", strFolder, GetSaveFileName(nfc,1)) : "");
	if (bSave)AfxGetApp()->WriteProfileString("MIPEXDlg", "SaveFolder", strFolder);
}

void CLGMipexDlg::OnBnClickedOpenfolder()
{
	if (!bSave)return;
	D_O::OpenFolderInExplorer(strFolder, m_hWnd);
}


void CLGMipexDlg::OnBnClickedBackup()
{
	nfcCurrent->MipexBackup();
}

void CLGMipexDlg::OnBnClickedRestore()
{
	nfcCurrent->MipexRestore();
}

void CLGMipexDlg::OnBnClickedCoeff()
{
	if (!nfcCurrent->mipex.bLoaded)
	{
		nfcCurrent->MipexGetBackup();
		nfcCurrent->WaitComplete();
	}
	if (!nfcCurrent->mipex.IsValid())
	{
		AfxMessageBox(S_O::LoadString(IDS_NOMIPEXBACKUP));
		return;
	}
	CGridInfoDlg gi;
	gi.nfc = nfcCurrent;
	gi.mode = GRIDINFO_MODE::MIPEX;
	gi.DoModal();
}


void CLGMipexDlg::OnBnClickedLastcommand()
{
	nfcCurrent->MipexGetLastCommand();
}
