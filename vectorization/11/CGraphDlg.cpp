// CGraphDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "CGraphDlg.h"
#include "afxdialogex.h"


// CGraphDlg dialog

IMPLEMENT_DYNAMIC(CGraphDlg, CDialogEx)

CGraphDlg::CGraphDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GRAPH, pParent)
{
	gr = 0;
}

CGraphDlg::~CGraphDlg()
{
	if (gr)delete gr;
}

void CGraphDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT21, m_Rich);
}


BEGIN_MESSAGE_MAP(CGraphDlg, CDialogEx)
	ON_BN_CLICKED(IDC_EXPORT, &CGraphDlg::OnBnClickedExport)
END_MESSAGE_MAP()


// CGraphDlg message handlers


BOOL CGraphDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	gr = CGraph::CreateOnPlace(GetDlgItem(IDC_STATIC_GRAPH));
	gr->bGlobalMarkerOnMouseMove = gr->bGlobalMarkerOnEveryCurve = 1;
	gr->bRescaleToMaxFromZero = 1;
	gr->net.nXAxisMode = AXISMODE_CTIME;
	return TRUE;
}

void CGraphDlg::FillExpectedCurve(DELAYED_COMMAND_ARRAY* dca)
{
	if (gr->curve.size() < 2)
	{
		CCurve cc;
		cc.strCurveInfo = "Ожидаемая температура";
		cc.Pen.lopnColor = 0xff;
		gr->curve.push_back(cc);
	}
	CCurve& cc = gr->curve[1];
	cc.Clear();
	double dTPrev = -1000;
	CMyPoint pt;
	for (int i = 0; i < dca->arr.GetSize(); i++)
	{
		DELAYED_COMMAND& c = dca->arr[i];		
		if (!c.bT && dTPrev == -1000)continue;
		if (c.bT)
		{
			if (dTPrev == -1000)
			{
				if (i)
				{
					if (c.tmStarted.IsNotNull())
					{
						pt.x = c.tmStarted.GetUINT();
					}
					else
					{						
						pt.x = dca->tmStarted.GetUINT()+ c.dTimeDelayFromStart;
					}
				}
				else pt.x = dca->tmStarted.GetUINT();
			}
			pt.y = dTPrev = c.dT;
			cc.vData.push_back(pt);			 
		}
		pt.y = dTPrev;
		bool bAddGas = 0, bAddT = 0, bAddC = 1, bAddTWait = 0;
		if (c.tmDone.IsNotNull())
		{
			pt.x = c.tmDone.GetUINT();
			bAddC = 0;
		}
		else if (c.tmGasStarted.IsNotNull())
		{
			pt.x = c.tmGasStarted.GetUINT();
			bAddGas = 1;
		}
		else if (c.tmPlateStarted.IsNotNull())
		{
			pt.x = c.tmPlateStarted.GetUINT();
			bAddGas = bAddT = 1;
		}
		else if (c.tmStarted.IsNotNull())
		{
			pt.x = CDateTime::GetCurrent().GetUINT();
			bAddGas = bAddT = bAddTWait = 1;
		}	
		else
		{
			bAddGas = bAddT = bAddTWait = 1;
		}
		if (c.bT)
		{
			if (bAddT)pt.x += c.dTTime * 60;
			if (bAddTWait)pt.x += 10 * 60;
		}
		if (bAddGas && c.bGas)pt.x += c.dGasTime;
		if (bAddC && c.strCommand!="")pt.x += 60;
		cc.vData.push_back(pt);
	}
	gr->RescaleToMax();
}


void CGraphDlg::OnBnClickedExport()
{
	if (!gr->curve.size())return;
	CString str, strLastSave = CMainAppTemplate::GetLastSavePath();
	str.Format("%s\\*.csv", strLastSave);
	CFileDialog fd(0, "*.csv", str, OFN_EXPLORER | OFN_HIDEREADONLY | OFN_LONGNAMES, "CSV Files (*.csv)|*.csv||", NULL);
	if (fd.DoModal() == IDCANCEL)return;
	strLastSave = fd.GetPathName();
	CFile f;
	if (!f.Open(strLastSave, CFile::modeCreate | CFile::modeReadWrite))return;
	CCurve& cc = gr->curve[0];
	for (int i = 0; i < cc.vData.size(); i++)
	{
		CDateTime tm((UINT64)cc.vData[i].x);
		str.Format("%s;%g\r\n", tm.FormatDB(), cc.vData[i].y);
		f.Write(str, str.GetLength());
	}
}
