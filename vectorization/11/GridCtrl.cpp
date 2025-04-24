// GridCtrl.cpp: implementation of the CGridCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "GridCtrl.h"
#include <StructWithData.h>
#include "resource.h"
#include "NFC_Device.h"
#include "CLGSaveDlg.h"
#include "CLGDelayedCommandDlg.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CUIntArray CGridCtrl::staticNumeric;
CStringArray CGridCtrl::staticDate;
CArray<GRID_SORT, GRID_SORT&> CGridCtrl::staticSort;

IMPLEMENT_DYNCREATE(CGridCtrl, CALXGridCtrl)
IMPLEMENT_REGISTER(CGridCtrl,CS_DBLCLKS)

BEGIN_MESSAGE_MAP(CGridCtrl, CALXGridCtrl)
	//{{AFX_MSG_MAP(CGridCtrl)
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGridCtrl::CGridCtrl()
{
	AddCol(100,"Text", DT_RIGHT | DT_VCENTER, DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);//,GA_EDITCTRL,WS_CHILD | ES_LEFT | ES_AUTOHSCROLL);
	AddCol(100,"Text", DT_CENTER | DT_VCENTER, DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);
//	DefineImage(1,7,7);
	AddCol(100,"Text", DT_LEFT | DT_VCENTER, DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);//,GA_EDITCTRL,WS_CHILD | ES_LEFT | ES_AUTOHSCROLL);
	AddCol(100,"Text", DT_LEFT | DT_VCENTER, DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);
	bChangeDataAllowed=TRUE;
	m_Normal = m_Error = m_Attention  = m_AttentionData= m_ErrorData= m_DeviceError=m_MetricsCells;
	m_Error.m_DefaultCellColor.m_crText = RGB(255, 0, 0);
	m_Attention.m_DefaultCellColor.m_crText = RGB(255, 153, 0);
	m_Error.m_ActiveCellColor.m_crText = RGB(255, 0, 0);
	m_Attention.m_ActiveCellColor.m_crText = RGB(255, 153, 0);
	m_Error.m_SelectCellColor.m_crText = RGB(255, 0, 0);
	m_Attention.m_SelectCellColor.m_crText = RGB(255, 153, 0);

	m_ErrorData.m_DefaultCellColor.m_crText = 0xffffff;
	m_ErrorData.m_ActiveCellColor.m_crText = 0xffffff;
	m_ErrorData.m_ActiveCellColor.m_crBackgrnd = RGB(255, 0, 0);
	m_ErrorData.m_SelectCellColor.m_crText = 0xffffff;
	m_ErrorData.m_SelectCellColor.m_crBackgrnd = RGB(255, 0, 0);
	m_ErrorData.m_DefaultCellColor.m_crBackgrnd = RGB(255, 0, 0);

	m_AttentionData.m_DefaultCellColor.m_crText = 0xffffff;
	m_AttentionData.m_ActiveCellColor.m_crText = 0xffffff;
	m_AttentionData.m_ActiveCellColor.m_crBackgrnd = RGB(255, 153, 0);
	m_AttentionData.m_SelectCellColor.m_crText = 0xffffff;
	m_AttentionData.m_SelectCellColor.m_crBackgrnd = RGB(255, 153, 0);
	m_AttentionData.m_DefaultCellColor.m_crBackgrnd = RGB(255, 153, 0);

	m_DeviceError.m_DefaultCellColor.m_crText = 0xffffff;
	m_DeviceError.m_ActiveCellColor.m_crText = 0xffffff;
	m_DeviceError.m_ActiveCellColor.m_crBackgrnd = RGB(255, 128, 128);
	m_DeviceError.m_SelectCellColor.m_crText = 0xffffff;
	m_DeviceError.m_SelectCellColor.m_crBackgrnd = RGB(255, 128, 128);
	m_DeviceError.m_DefaultCellColor.m_crBackgrnd = RGB(255, 128, 128);

	SetShowTip(1);
	nGridId = GRIDID_DEFAULT;
	bForceGetData = 0;
	bFirstColNarrow = 1;
	bWidthDependsOnColumnCount = 0;
	bSaveColWidth = 1;
	bCellWidthLoaded = 0;
	check = 0;
	nSelColumn = -1;
	bSortEnabled = 0;
}


CGridCtrl::~CGridCtrl()
{
//	delete val;
}

BOOL CGridCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;
	cs.style &= ~AGS_RANGESELECT;
	cs.style |= AGS_CUSTOMFROZEN_WIDTH|AGS_ROWSELECT;
	return CALXGridCtrl::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CRegGridCtrl diagnostics

#ifdef _DEBUG
void CGridCtrl::AssertValid() const
{
	CALXGridCtrl::AssertValid();
}

void CGridCtrl::Dump(CDumpContext& dc) const
{
	CALXGridCtrl::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CRegGridCtrl message handlers


HBITMAP CGridCtrl::GetBitmap(int nCol, int nRow, BOOL bSelected)
{
	return m_BMP;
}

CELL_DATA CGridCtrl::GetCellData(int nCol, int nRow)
{
	CELL_DATA CellData;
	if (!bForceGetData && gridrows.GetSize())
	{
		if (nRow >= gridrows.GetSize() || gridrows[nRow].group)return CellData;
		nRow = gridrows[nRow].nRealRow;
	}
	if (ptrData)
	{
		if (nGridId == GRIDID_LOG)return GetCellDataLog(nCol, nRow);
		if (nGridId == GRIDID_GRAPH)return GetCellDataGraph(nCol, nRow);
		if (nGridId == GRIDID_LIMITS)return GetCellDataLimits(nCol, nRow);
		if (nGridId == GRIDID_RESTORE)return GetCellDataRestore(nCol, nRow);
		if (nGridId == GRIDID_SCALEPOINT)return GetCellDataScalePoints(nCol, nRow);
		if (nGridId == GRIDID_RSSI)return GetCellDataRSSI(nCol, nRow);
		if (nGridId == GRIDID_DCA)return GetCellDataDCA(nCol, nRow);
		if (nGridId == GRIDID_DCAR)return GetCellDataDCAR(nCol, nRow);
	}
	if(names.GetSize()==0)return CellData;
	int nPos = nRow * columnnames.GetSize() + nCol;
	if (nPos >= names.GetSize())return CellData;
	if (nSelColumn == nCol)CellData.m_dwTag = S_O::ToUINT(names[nPos]);
	else CellData.m_strText=names[nPos];
	return CellData;
}

BOOL CGridCtrl::OnSaveCellData(int nCol, int nRow)
{
	static bool bInSave = 0;
	if (bInSave)return 1;
	bInSave = 1;
	CALXCellCtrl* pCellCtrl = GetCellCtrl(nCol, nRow);
	CELL_DATA Data = pCellCtrl->GetCellData();
	try
	{
		if (gridrows.GetSize())
		{
			if (nRow >= gridrows.GetSize() || gridrows[nRow].group)throw 0;
			nRow = gridrows[nRow].nRealRow;
		}
		if (ptrData)
		{
			if (nGridId == GRIDID_LOG)throw SaveCellDataLog(Data, nCol, nRow);
			if (nGridId == GRIDID_LIMITS)throw SaveCellDataLimits(Data,nCol, nRow);
			if (nGridId == GRIDID_RESTORE)throw SaveCellDataRestore(Data,nCol, nRow);
			if (nGridId == GRIDID_SCALEPOINT)throw SaveCellDataScalePoints(Data,nCol, nRow);
			if (nGridId == GRIDID_RSSI)throw SaveCellDataRSSI(Data, nCol, nRow);
		}
		CALXCellCtrl* pCellCtrl = GetCellCtrl(nCol, nRow);
		CELL_DATA Data = pCellCtrl->GetCellData();
		if (bChangeDataAllowed || nSelColumn == nCol)
		{
			int nPos = nRow * columnnames.GetSize() + nCol;
			if (nSelColumn == nCol)names[nPos] = S_O::FormatUINT(Data.m_dwTag);
			else names[nPos] = Data.m_strText;
			if (changed.GetSize())changed[nRow] = 1;
			GetParent()->PostMessage(WM_CELLMODIFIED, (WPARAM)this,MAKELONG(nCol,nRow));
		}
		throw 1;
	}
	catch (int n)
	{
		bInSave = 0;
//		if (n && bNotifyCellModified)GetParent()->PostMessage(WM_CELLMODIFY, (WPARAM)this, MAKELONG(nCol, nRow));
		return n;
	}
	return 0;
}
void CGridCtrl::FillData()
{
	if(columnnames.GetSize()==0)return;
	nNumeric.RemoveAll();
	if (bFirstColNarrow)nNumeric.Add(1);
	SetAdjustHeight(1);
	RemoveAllCol();
	CRect rt;
	GetClientRect(&rt);
	int nWidth=(rt.Width()-(bFirstColNarrow?60:30)-((nSelColumn!=-1)?20:0))/(columnnames.GetSize()-(bFirstColNarrow?1:0) - ((nSelColumn != -1) ? 1 : 0));
	if(nWidth<40)nWidth=40;
	DefineColCtrl(AddCol(bFirstColNarrow?40:nWidth, columnnames[0], ACFF_CENTER, AHFF_CENTER, 0, 0, 1), GA_CHECKCTRL, WS_CHILD |ES_CENTER);
	for (int i = 1; i < columnnames.GetSize(); i++)
	{
		if(nSelColumn==i)DefineColCtrl(AddCol(20, "", ACFF_CENTER, AHFF_CENTER, DFC_BUTTON, DFCS_BUTTONCHECK | DFCS_FLAT, i+2), GA_BUTTONCTRL, WS_CHILD | BS_AUTOCHECKBOX | BS_FLAT | ES_CENTER, i+2);
		else DefineColCtrl(AddCol(nWidth, columnnames[i], ACFF_CENTER, AHFF_CENTER, 0, 0, i + 2), bChangeDataAllowed ? GA_EDITCTRL : GA_CHECKCTRL, WS_CHILD | ES_CENTER);
	}
	SetGridRowCount(names.GetSize()/columnnames.GetSize());
	UpdateScrollSizes();
	RedrawWindow();
}

bool CGridCtrl::IsUnicode(int nCol, int nRow)
{
	if (nGridId == GRIDID_SCALEPOINT)
	{
		if (GetCellCtrlID(nCol, nRow) == SCALE_UNITS)return 1;
	}
	return 0;
}

bool CGridCtrl::OnDrawCellContent(CDC *pDC,int nCol,int nRow,CRect& rectCell)
{
	return 0;
}


int CGridCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CALXGridCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;
}

void CGridCtrl::OnHeadButtonDown(int nHeadCol,int nHeadRow)
{
	CALXGrid::OnHeadButtonDown(nHeadCol,nHeadRow);
	SortByColumn(nHeadCol, 1, 0, GetKeyState(VK_CONTROL) < 0,1);
	GetParent()->PostMessage(WM_HEADCLICKED,(WPARAM)this,nHeadCol);
}

void CGridCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CALXGrid::OnLButtonDown(nFlags,point);
	if (point.y < m_CountHeaderRows * m_nHeadRowHeight)return;
	int nCol=GetActiveCol(),nRowSave,nRow=nRowSave=GetActiveRow();
	if (gridrows.GetSize() && nRow<gridrows.GetSize() && point.y > m_CountHeaderRows* m_nHeadRowHeight)
	{
		if (gridrows[nRow].group && nCol >= 1)
		{
			return;
		}
		else nRow = gridrows[nRow].nRealRow;
	}
	GetParent()->PostMessage(WM_CELLSELECTED,(WPARAM)this);
}

void CGridCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CALXGrid::OnKeyDown(nChar,nRepCnt,nFlags);
	int nCol=GetActiveCol(),nRow=GetActiveRow();
	GetParent()->PostMessage(WM_CELLSELECTED, (WPARAM)this);
}

BOOL CGridCtrl::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message==WM_KEYDOWN)
	{
		if (LOWORD(pMsg->wParam) != VK_CONTROL && GetKeyState(VK_CONTROL) < 0)
		{
			return CALXGridCtrl::PreTranslateMessage(pMsg);
		}
		if(pMsg->hwnd == m_hWnd || LOWORD(pMsg->wParam) == VK_RETURN)
		{
			CGridCtrl::OnKeyDown(LOWORD(pMsg->wParam),LOWORD(pMsg->lParam),HIWORD(pMsg->lParam));
				return TRUE;
		}
		return CALXGridCtrl::PreTranslateMessage(pMsg);
	}
	else
		return CALXGridCtrl::PreTranslateMessage(pMsg);
}

CALXCellMetrics* CGridCtrl::GetCellMetrics(int nCol, int nRow)
{
	UINT id = GetCellCtrlID(nCol, nRow);
	if (nGridId == GRIDID_RESTORE)
	{
		if (nRow >= 0 && nRow < rarr->arr.GetSize())
		{
			RESTORE_TYPE* r = &rarr->arr[nRow];
			if (r->nId == -1)return &m_Attention;
		}
	}
	int nPos=nRow*columnnames.GetSize()+nCol;
	if((nPos<0 || nPos>=nStatus.GetSize()))return &m_MetricsCells;
	if(!nStatus[nPos])return &m_Error;
	return &m_MetricsCells;
}

void CGridCtrl::OnEndResizeCol(int nCol, int nNewWidth)
{
	if (nGridId != GRIDID_DEFAULT)SaveCellWidths();
}

void CGridCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (point.y > m_CountHeaderRows* m_nHeadRowHeight)
	{
			CALXGridCore::SetActiveCell(point);
			GetParent()->SendMessage(WM_NOTIFYRBUTTONUP, (WPARAM)this, MAKELONG(point.x, point.y));
	}
}

void CGridCtrl::SortByColumn(int nCol, bool bAsc, bool bForceDest, bool bControlPressed, bool bRedraw)
{
	if (!bSortEnabled)return;
	bool bf = 0, bdel = !bControlPressed;
	for (int i = 0; i < sortcols.GetSize(); i++)
	{
		if (sortcols[i].nCol == nCol)
		{
			if (bForceDest)sortcols[i].bAsc = bAsc;
			else sortcols[i].bAsc = !sortcols[i].bAsc;
			bf = 1;
			if (bdel)
			{
				GRID_SORT gs;
				gs = sortcols[i];
				sortcols.RemoveAll();
				sortcols.Add(gs);
			}
			break;
		}
	}
	if (!bf)
	{
		if (bdel)sortcols.RemoveAll();
		GRID_SORT gs;
		gs.nCol = nCol;
		gs.bAsc = bAsc;
		sortcols.Add(gs);
	}
	int nReal = GetActiveRow();
	SortGroups(bRedraw);
	SetActiveRow(nReal);
}

struct GRID_COMPARE
{
	CStringArray strCompare;
	int nr;
};

void CGridCtrl::SortGroups(bool bRedraw)
{
	CUIntArray bChecks;
	staticNumeric.RemoveAll();
	staticDate.RemoveAll();
	for (int i = 0; i < sortcols.GetSize(); i++)
	{
		if (sortcols[i].nCol >= GetHeadColCount())
		{
			sortcols.RemoveAt(i);
			i--;
			continue;
		}
		staticNumeric.Add(IsColumnNumeric(sortcols[i].nCol));
		staticDate.Add(IsColumnDate(GetCellCtrlID(sortcols[i].nCol, 0)));
		CELL_INFO CellInfo = GetCellInfo(sortcols[i].nCol, 0);
		bChecks.Add(CellInfo.m_nTypeCtrl == DFC_BUTTON);
	}
	if (!sortcols.GetSize())return;
	ALXSortOrder nSortOrder;
	if (sortcols[0].bAsc)nSortOrder = Ascending;
	else nSortOrder = Descending;
	CALXGrid::SetSortCol(sortcols[0].nCol, 0, nSortOrder);
	staticSort.Copy(sortcols);
	bForceGetData = 1;

	if (!gridrows.GetSize())
	{
		gridrows.SetSize(GetGridRowCount());
		for (int i = 0; i < GetGridRowCount(); i++)
			gridrows[i].nRealRow = i;
	}
	int nr = gridrows.GetSize();
	if (!nr)return;
	GRID_COMPARE* gcarr = new GRID_COMPARE[nr];
	CELL_DATA data;
	for (int k = 0; k < nr; k++)
	{
		for (int j = 0; j < sortcols.GetSize(); j++)
		{
			data = GetCellData(sortcols[j].nCol, gridrows[k].nRealRow);
			gcarr[k].nr = gridrows[k].nRealRow;
			if (bChecks[j])
			{
				if (data.m_dwTag)gcarr[k].strCompare.Add("Выбрано");
				else gcarr[k].strCompare.Add("Не выбрано");
			}
			else gcarr[k].strCompare.Add(data.m_strText);
		}
	}
	qsort(gcarr, nr, sizeof(GRID_COMPARE), compare);
	for (int i = 0; i < nr; i++)
		gridrows[i].nRealRow = gcarr[i].nr;
	delete[] gcarr;
	bForceGetData = 0;
	if (bRedraw)RedrawWindow();
}

int CGridCtrl::compare(const void* arg1, const void* arg2)
{
	GRID_COMPARE* g1 = ((GRID_COMPARE*)arg1);
	GRID_COMPARE* g2 = ((GRID_COMPARE*)arg2);
	int nRet = -1;
	for (int i = 0; i < g1->strCompare.GetSize(); i++)
	{
		if (i < staticDate.GetSize() && staticDate[i] != "")
		{
			int d1, m1, y1,h,m,s;
			CTime tm1 = 0, tm2 = 0;
			if (g1->strCompare[i] != "")
			{
				sscanf_s(g1->strCompare[i], "%d.%d.%d %d:%d:%d", &d1, &m1, &y1, &h, &m, &s);
				tm1 = CTime(y1 + 2000, m1, d1, h, m, s);
			}
			if (g2->strCompare[i] != "")
			{
				sscanf_s(g2->strCompare[i], "%d.%d.%d %d:%d:%d", &d1, &m1, &y1, &h, &m, &s);
				tm2 = CTime(y1 + 2000, m1, d1, h, m, s);
			}
			if (tm1 > tm2)nRet = 1;
			else if (tm1 == tm2)nRet = 0;
			else nRet = -1;
		}
		else if (!staticNumeric[i])nRet = g1->strCompare[i].CompareNoCase(g2->strCompare[i]);
		else
		{
			double d1 = S_O::ToDouble(g1->strCompare[i]);
			double d2 = S_O::ToDouble(g2->strCompare[i]);
			if (d1 > d2)nRet = 1;
			else if (d1 == d2)nRet = 0;
			else nRet = -1;
		}
		if (!staticSort[i].bAsc)nRet = -nRet;
		if (nRet != 0)break;

	}
	return nRet;
}

CString CGridCtrl::IsColumnDate(int nCol)
{
	for (int i = 0; i < strDateFormat.GetSize(); i++)
	{
		if (strDateFormat[i].nId == nCol)return strDateFormat[i].str;
	}
	return "";
}

bool CGridCtrl::IsColumnNumeric(int nCol)
{
	int id = GetCellCtrlID(nCol, 0);
	for (int i = 0; i < nNumeric.GetSize(); i++)
	{
		if (nNumeric[i] == id)
		{
			return 1;
		}
	}
	return 0;

}

int CGridCtrl::GetActiveRow()
{
	int nr = CALXGridCtrl::GetActiveRow();
	if (!gridrows.GetSize())return nr;
	if (nr >= gridrows.GetSize() || gridrows[nr].group)return 0;
	return gridrows[nr].nRealRow;
}

int CGridCtrl::GetRealActiveRow()
{
	return CALXGridCtrl::GetActiveRow();
}

void CGridCtrl::GetLogColumns(DEVICE_LOG* log, CUIntArray& ids, CStringArray& names)
{
	int nCols = 0;
	if (!log->dev->bManufact)nCols = 8;
	else if (log->bShort2)nCols = 14;
	else if (log->bShort)nCols = 8;
	else if (log->nVersion == 3 || log->bMIPEXLog)nCols = LOG_SIZE - LOG_N;
	else if (log->nVersion > 1)nCols = LOG_BATTLOG - LOG_N;
	else nCols = LOG_PRESSURE - LOG_N;
	if (!log->bShort && !log->dev->IsHaveProperty(VBATINLOG))nCols--;
	int nG = 0;
	CString str, strG = "%s,%s";
	CString h[] = { "#","TimeLog", "PacketFlag", "AirTemp",
		S_O::Format(strG,log->dev->GetGasString(GAS_O2),log->dev->GetUnitsString(GAS_O2)),
		S_O::Format(strG,log->dev->GetGasString(GAS_CO),log->dev->GetUnitsString(GAS_CO)),
		S_O::Format(strG,log->dev->GetGasString(GAS_H2S),log->dev->GetUnitsString(GAS_H2S)),
		S_O::Format(strG,log->dev->GetGasString(GAS_MIPEX),log->dev->GetUnitsString(GAS_MIPEX)),
		log->dev->GetGasString(GAS_O2) + "-ADC", log->dev->GetGasString(GAS_O2) + ",mV",
		log->dev->GetGasString(GAS_CO) + "-ADC",log->dev->GetGasString(GAS_CO) + ",mV",
		log->dev->GetGasString(GAS_H2S) + "-ADC", log->dev->GetGasString(GAS_H2S) + ",mV",
		"Pressure","Battery,V","RSSI,dB","SNR,dB","Ext","Freeze"};
	CString hM[] = { "#","TimeLog", "PacketFlag", "AirTemp",
		"TS",
		"Skz1",
		"Sm",
		"Conc1",
		"Sc", "",
		"Skz2","",
		"Skz3", "",
		"","Sktg","C0","Status",""};
	UINT nGases[] = { GAS_O2 ,GAS_CO,GAS_H2S,GAS_MIPEX };
	for (int i = LOG_N; i < LOG_N + nCols; i++)
	{
		if (!log->dev->bManufact)
		{
			if (i >= LOG_O2ADC)break;
			if (i >= LOG_O2 && i <= LOG_CH4 && !log->dev->IsGasSelected(nGases[i - LOG_O2]))continue;
		}
		if (i == LOG_FLAG)continue;
		if (i == LOG_PRESSURE && log->nVersion < 2)continue;
		if (i == LOG_EXT && (!log->dev->IsHaveProperty(SKIPSELFTTESTTRANSPORT) || log->nVersion < 2 || log->bShort2))continue;
		if (i == LOG_FREEZE && (!log->dev->IsHaveProperty(FREEZE_IN_LOG) || log->nVersion < 2 || log->bShort2))continue;
		if (log->bShort2 && (i == LOG_O2ADC || i == LOG_O2VOLT || i== LOG_COVOLT || i== LOG_H2SVOLT))continue;
		if (log->bMIPEXLog)
		{
			if (i == LOG_O2VOLT || i == LOG_COVOLT || i == LOG_H2SVOLT || i == LOG_PRESSURE)continue;
			str = hM[i - LOG_N];
		}
		else str = h[i - LOG_N];
		ids.Add(i);
		names.Add(str);
	}
}

void CGridCtrl::FillData(DEVICE_LOG* log)
{
	bSaveColWidth = 0;
	this->log = log;
	if (!nNumeric.GetSize())nNumeric.Add(LOG_N);
	RemoveAllCol();
	CRect rt;
	GetClientRect(&rt);
	CUIntArray ids;
	CStringArray names;
	GetLogColumns(log, ids,  names);
	int nWidth = (rt.Width() - 50) / (ids.GetSize()-1);
	for (int i = 0; i < ids.GetSize(); i++)
	{
		if (ids[i] == LOG_N)DefineColCtrl(AddCol(40, names[i], ACFF_CENTER, AHFF_LEFT, 0, 0, ids[i]), GA_CHECKCTRL, WS_CHILD | ES_LEFT | ES_AUTOHSCROLL, ids[i]);
		else DefineColCtrl(AddCol(nWidth, names[i], ACFF_LEFT, AHFF_LEFT, 0, 0, ids[i]), GA_CHECKCTRL, WS_CHILD | ES_LEFT | ES_AUTOHSCROLL, ids[i]);
	}
	LoadCellWidths();
	SetGridRowCount(log->records.GetSize());
	UpdateScrollSizes();
	RedrawWindow();
}

BOOL CGridCtrl::SaveCellDataLog(CELL_DATA& Data, int nCol, int nRow)
{
	return 1;
}

CString CGridCtrl::GetCellDataLog(DEVICE_LOG* log,int nColId, int nRow)
{
	CString str;
	if (nRow<0 || nRow >= log->records.GetSize())return str;
	DEVICE_LOG_RECORD* r = &log->records[nRow];
	if (nColId != LOG_N && nColId != LOG_DATE && r->nH2SVolt == 0xffff && r->nCOVolt == 0xffff && r->nO2Volt == 0xffff)str="——";
	else
	{
		switch (nColId)
		{
		case LOG_N:
			str.Format("%d", nRow + 1);
			break;
		case LOG_DATE:
		{
			if (CNFC_Device::IsCorrectDate(r->nDateTime))
			{
				SYSTEMTIME st;
				CNFC_Device::TimeToSystemTIME(r->nDateTime, st);
				CTime tm(st);
				str = tm.Format("%d.%m.%y %H:%M:%S");
			}
			break;
		}
		case LOG_FLAG:
		{
			str.Format("%u", (UINT)r->nFlag);
			break;
		}
		case LOG_TEMP:
		{
			str.Format("%g", r->nTemp);
			break;
		}
		case LOG_O2:
			if (log->bMIPEXLog)str.Format("%.0f", r->nO2);
			else str.Format("%g", r->nO2);
			break;
		case LOG_CO:
			if (log->bMIPEXLog)str.Format("%.0f", r->nCO);
			else str.Format("%g", r->nCO);
			break;
		case LOG_CH4:
			if (log->bMIPEXLog)str.Format("%.0f", r->nCH4);
			else str.Format("%.3f", r->nCH4);
			break;
		case LOG_H2S:
			if (log->bMIPEXLog)str.Format("%.0f", r->nH2S);
			else str.Format("%g", r->nH2S);
			break;
		case LOG_H2SVOLT:
			str.Format("%.0f", SENSORS_DATA::GetVolt((WORD)r->nH2SVolt));
			break;
		case LOG_O2VOLT:
			str.Format("%.0f", SENSORS_DATA::GetVolt((WORD)r->nO2Volt));
			break;
		case LOG_COVOLT:
			str.Format("%.0f", SENSORS_DATA::GetVolt((WORD)r->nCOVolt));
			break;
		case LOG_H2SADC:
			str.Format("%.0f", r->nH2SVolt);
			break;
		case LOG_O2ADC:
			str.Format("%.0f", r->nO2Volt);
			break;
		case LOG_COADC:
			str.Format("%.0f", r->nCOVolt);
			break;
		case LOG_PRESSURE:
			str.Format("%.0f", r->nPressure);
			break;
		case LOG_BATTLOG:
			if (log->bMIPEXLog)str.Format("%.0f", r->nBattVolt);
			else if (log->dev->IsHaveProperty(VBATINLOG))str.Format("%.2f", r->nBattVolt / 100);
			break;
		case LOG_RSSI:
			if (!log->bMIPEXLog && r->nRSSI == -1000)str = "-";
			else str.Format("%d", (int)r->nRSSI);
			break;
		case LOG_SNR:
			if (!log->bMIPEXLog && r->nSNR == -100)str = "-";
			else str.Format("%d", (int)r->nSNR);
			break;
		case LOG_EXT:
			str = S_O::ToBinary(r->nExt);
			break;
		case LOG_FREEZE:
			if (log->dev->IsHaveProperty(FREEZE_IN_LOG))str = r->GetFreezeStatus();
			break;

		}
	}
	return str;
}

CELL_DATA CGridCtrl::GetCellDataLog(int nCol, int nRow)
{
	CELL_DATA CellData;
	CellData.m_strText = GetCellDataLog(log, GetCellCtrlID(nCol, nRow), nRow);
	return CellData;
}

void CGridCtrl::FillData(CNFC_Device* nfc)
{
	this->nfc = nfc;
	if (!nNumeric.GetSize())nNumeric.Add(GRAPH_N);
	RemoveAllCol();
	CRect rt;
	GetClientRect(&rt);
	int nG = 0;
	int nWidth = (rt.Width() - 50) / (GRAPH_SIZE- GRAPH_N - 1);
	CString str;
	CString h[] = { "#","Time", "Temp", nfc->GetGasString(GAS_O2), nfc->GetGasString(GAS_CO), nfc->GetGasString(GAS_H2S), nfc->GetGasString(GAS_MIPEX),
		nfc->GetGasString(GAS_O2)+"-ADC",nfc->GetGasString(GAS_O2)+ "-mV",
		nfc->GetGasString(GAS_CO)+"-ADC",nfc->GetGasString(GAS_CO)+ "-mV",
		nfc->GetGasString(GAS_H2S) + "-ADC", nfc->GetGasString(GAS_H2S) + "-mV","Battery-V","Pressure","CPU Temp"};
	for (int i = GRAPH_N; i < GRAPH_SIZE; i++)
	{
		if (i == GRAPH_N)DefineColCtrl(AddCol(40, h[i - GRAPH_N], ACFF_CENTER, AHFF_LEFT, 0, 0, i), GA_CHECKCTRL, WS_CHILD | ES_LEFT | ES_AUTOHSCROLL, i);
		else DefineColCtrl(AddCol(nWidth, h[i - GRAPH_N], ACFF_LEFT, AHFF_LEFT, 0, 0, i), GA_CHECKCTRL, WS_CHILD | ES_LEFT | ES_AUTOHSCROLL, i);
	}
	LoadCellWidths();
	if(nfc)SetGridRowCount(nfc->monitoring.GetSize());
	UpdateScrollSizes();
	RedrawWindow();
}

CELL_DATA CGridCtrl::GetCellDataGraph(int nCol, int nRow)
{
	CELL_DATA CellData;
	int id = GetCellCtrlID(nCol, nRow);
	if (nRow >= nfc->monitoring.GetSize())return CellData;
	SENSORS_DATA* r = &nfc->monitoring[nRow];
	switch (id)
	{
	case GRAPH_N:
		CellData.m_strText.Format("%d", nRow + 1);
		break;
	case GRAPH_DATE:
	{
		CellData.m_strText = r->time.Format("%H:%M:%S");
		break;
	}
	case GRAPH_TEMP:
	{
		CellData.m_strText.Format("%.1f", (double)r->SensorTemp/100);
		break;
	}
	case GRAPH_O2:
		CellData.m_strText.Format("%g", (double)r->O2/ nfc->GetGasDevider(GAS_O2,1));
		break;
	case GRAPH_CO:
		CellData.m_strText.Format("%g", (double)r->CO/nfc->GetGasDevider(GAS_CO,1));
		break;
	case GRAPH_CH4:
	{
		CellData.m_strText.Format(S_O::Format("%%.%df",nfc->GetPrecision(GAS_MIPEX)), (double)(nfc->GetUnits(GAS_MIPEX) ? r->CH4LEL : r->CH4VOL) / nfc->GetGasDevider(GAS_MIPEX,1));
		break;
	}
	case GRAPH_H2S:
		CellData.m_strText.Format("%g", (double)r->H2S/ nfc->GetGasDevider(GAS_H2S,1));
		break;
	case GRAPH_H2SADC:
		CellData.m_strText.Format("%u", (UINT)r->H2SVolt);
		break;
	case GRAPH_O2ADC:
		CellData.m_strText.Format("%u", (UINT)r->O2Volt);
		break;
	case GRAPH_COADC:
		CellData.m_strText.Format("%u", (UINT)r->COVolt);
		break;
	case GRAPH_H2SVOLT:
		CellData.m_strText.Format("%.0f", r->GetVolt(r->H2SVolt));
		break;
	case GRAPH_O2VOLT:
		CellData.m_strText.Format("%.0f", r->GetVolt(r->O2Volt));
		break;
	case GRAPH_COVOLT:
		CellData.m_strText.Format("%.0f", r->GetVolt(r->COVolt));
		break;
	case GRAPH_PRESSURE:
		CellData.m_strText.Format("%.0f", (double)r->Press);
		break;
	case GRAPH_BATTERY:
		CellData.m_strText.Format("%.2f", (double)r->Batt/100);
		break;
	case GRAPH_CPU:
		CellData.m_strText.Format("%.0f", (double)r->CPUTemp);
		break;
	}
	return CellData;
}

void CGridCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (point.y > m_CountHeaderRows * m_nHeadRowHeight)
	{
		GetParent()->SendMessage(WM_NOTIFYLDBLCLK, (WPARAM)this, MAKELONG(point.x, point.y));
	}
}

void CGridCtrl::FillDataLimits(CNFC_Device* nfc)
{
	bWidthDependsOnColumnCount = 1;
	this->nfc = nfc;
	RemoveAllCol();
	CRect rt;
	GetClientRect(&rt);
	int nWidth = (rt.Width()-10) / (LIM_SIZE-LIM_GAS);
	CString str,strl= S_O::LoadString(IDS_LIMIT),strt= S_O::LoadString(IDS_TEST);
	CString h[] = { S_O::LoadString(IDS_GAS),strl +" 1", strl + " 2", strl + " 1 "+ strt, strl + " 2 " + strt};
	for (int i = LIM_GAS; i < LIM_SIZE; i++)
	{
		if (!nfc->bManufact && i >= LIM_LIM1TEST)break;
		if (i == LIM_GAS)DefineColCtrl(AddCol(nWidth, h[i - LIM_GAS], ACFF_CENTER, AHFF_LEFT, 0, 0, i), GA_CHECKCTRL, WS_CHILD | ES_LEFT | ES_AUTOHSCROLL, i);
		else if(i== LIM_LIM1 || i== LIM_LIM2)DefineColCtrl(AddCol(nWidth, h[i - LIM_GAS], ACFF_LEFT, AHFF_LEFT, 0, 0, i), GA_EDITCTRL, WS_CHILD | ES_LEFT | ES_AUTOHSCROLL, i);
		else DefineColCtrl(AddCol(nWidth, h[i - LIM_GAS], ACFF_CENTER, AHFF_LEFT, 0, 0, i), GA_CHECKCTRL, WS_CHILD | ES_LEFT | ES_AUTOHSCROLL, i);
	}
	LoadCellWidths();
	nData.RemoveAll();
	if (nfc)
	{
		if (!nfc->bManufact)
		{
			BYTE en[] = { nfc->sensors_info.bCH4En,nfc->sensors_info.bO2En, nfc->sensors_info.bCOEn,nfc->sensors_info.bH2SEn };
			for (int i = 0; i < 4; i++)
			{
				if (en[i])nData.Add(i);
			}
		}
		else
		{
			for (int i = 0; i < 4; i++)nData.Add(i);
		}
		SetGridRowCount(nData.GetSize());
	}
	UpdateScrollSizes();
	RedrawWindow();
}

CELL_DATA CGridCtrl::GetCellDataLimits(int nCol, int nRow)
{
	CELL_DATA CellData;
	int id = GetCellCtrlID(nCol, nRow);
	if (nRow >= nData.GetSize())return CellData;
	GAS_LIMITS* r = &nfc->limits[nData[nRow]];
	switch (id)
	{
	case LIM_GAS:
		CellData.m_strText = nfc->GetGasString(r->nGas);
		break;
	case LIM_LIM1:
		CellData.m_strText = r->strLimit1;
		break;
	case LIM_LIM2:
		CellData.m_strText = r->strLimit2;
		break;
	case LIM_LIM1TEST:
	case LIM_LIM2TEST:
		if(nfc->bManufact && nfc->sensors_info.alarms.bLoaded && r->nGas!=GAS_MIPEX)CellData.m_strText = S_O::LoadString(IDS_TEST);
		break;
	}
	return CellData;
}

BOOL CGridCtrl::SaveCellDataLimits(CELL_DATA& Data, int nCol, int nRow)
{
	int id = GetCellCtrlID(nCol, nRow);
	if (nRow >= nfc->limits.GetSize())return 1;
	GAS_LIMITS* r = &nfc->limits[nData[nRow]];
	switch (id)
	{
	case LIM_LIM1:
		r->strLimit1= Data.m_strText;
		break;
	case LIM_LIM2:
		r->strLimit2= Data.m_strText;
		break;
	}
	return 1;
}

int CGridCtrl::GetSaveColumnsCount()
{
	int nC = GetHeadColCount();
	if (nC > GetHeadColCount())nC = GetHeadColCount();
	return nC;
}

void CGridCtrl::SaveCellWidths()
{
	int nCol = GetSaveColumnsCount();
	if (nCol == 1 || !bSaveColWidth)return;
	CString buf;
	for (int i = 0; i < nCol; i++)
	{
		if (bWidthDependsOnColumnCount)buf.Format("Grid%d_%d_%d", nGridId, nCol, i);
		else buf.Format("Grid%d_%d", nGridId, i);
		int nW = -1;
		UINT nId = GetCellCtrlID(i, 0);
		if (IsShowColumn(nId))nW = GetHeadColWidth(i);
		AfxGetApp()->WriteProfileInt("Grid", buf, nW);
	}
	CRect rt;
	GetClientRect(&rt);
	buf.Format("Grid%d_Width", nGridId);
	AfxGetApp()->WriteProfileInt("Grid", buf, rt.Width());
	if (m_bFrozenWithText)
	{
		buf.Format("Grid%d_FrozenWidth", nGridId);
		AfxGetApp()->WriteProfileInt("Grid", buf, GetFrozenColWidth());
	}
}

void CGridCtrl::ClearCellWidths()
{
	if (nGridId == GRIDID_DEFAULT)return;
	int nCol = GetHeadColCount();
	if (nCol == 1)return;
	CString buf;
	for (int i = 0; i < nCol; i++)
	{
		if (bWidthDependsOnColumnCount)buf.Format("Grid%d_%d_%d", nGridId, nCol, i);
		else buf.Format("Grid%d_%d", nGridId, i);
		AfxGetApp()->WriteProfileInt("Grid", buf, -1);
	}
}

void CGridCtrl::LoadCellWidths()
{
	int nCol = GetSaveColumnsCount();
	if (nCol == 1 || !bSaveColWidth)return;
	bCellWidthLoaded = 1;
	if (!columnOrderDefault.order.list.GetSize())
	{
		for (int i = 0; i < nCol; i++)
		{
			RM_STRINGWITHID sid;
			sid.nId = GetCellCtrlID(i, 0);
			HEADCELL_INFO inf = GetHeadCellInfo(i, 0);
			sid.str = inf.m_strText;
			columnOrderDefault.order.list.Add(sid);
		}
	}
	CString strSel = AfxGetApp()->GetProfileString("Grid", columnOrderDefault.GetAsString(), ""), buf;
	if (strSel != "")
	{
		CUIntArray arr;
		columnOrderSel.order.list.RemoveAll();
		RM_STRINGOPERATIONS::ParseString(strSel, arr);
		for (int i = 0; i < arr.GetSize(); i++)
		{
			for (int k = 0; k < columnOrderDefault.order.list.GetSize(); k++)
			{
				if (columnOrderDefault.order.list[k].nId == arr[i])
				{
					columnOrderSel.order.list.Add(columnOrderDefault.order.list[k]);
					break;
				}
			}
		}
		ApplyColumnOrder();
	}
	else columnOrderSel = columnOrderDefault;
	bool bHasCol = 0;
	for (int i = 0; i < nCol; i++)
	{
		if (bWidthDependsOnColumnCount)buf.Format("Grid%d_%d_%d", nGridId, nCol, i);
		else buf.Format("Grid%d_%d", nGridId, i);
		UINT nId = GetCellCtrlID(i, 0);
		int nW = -1;
		if (!IsShowColumn(nId))nW = 0;
		else nW = AfxGetApp()->GetProfileInt("Grid", buf, -1);
		if (nW != -1)
		{
			SetHeadColWidth(i, 0, nW);
			if (nW > 0)bHasCol = 1;
		}
		else bHasCol = 1;
	}
	if (!bHasCol)EvenColumnWidth();
	if (m_bFrozenWithText)
	{
		buf.Format("Grid%d_FrozenWidth", nGridId);
		int nW = AfxGetApp()->GetProfileInt("Grid", buf, 0);
		if (nW)SetFrozenColWidth(nW);
	}
}

bool CGridCtrl::IsShowColumn(UINT nColId)
{
	if (!av_columns.GetSize())return 1;
	CString str;
	str.Format("Col%u", nColId);
	UINT nShow = AfxGetApp()->GetProfileInt("GridShow", str, 1);
	return nShow != 0;
}

void CGridCtrl::FillData(RESTORE_TYPE_ARRAY* arr)
{
	this->rarr = arr;
	RemoveAllCol();
	CRect rt;
	GetClientRect(&rt);
	int nWidth = (rt.Width() - 50);
	DefineColCtrl(AddCol(20, "", ACFF_CENTER, AHFF_LEFT, 0, 0, RESTORE_SEL), GA_CHECKCTRL, WS_CHILD | ES_LEFT | ES_AUTOHSCROLL, RESTORE_SEL);
	DefineColCtrl(AddCol(nWidth, S_O::LoadString(IDS_RESTORETYPE), ACFF_LEFT, AHFF_LEFT, 0, 0, RESTORE_DESCR), GA_CHECKCTRL, WS_CHILD | ES_LEFT | ES_AUTOHSCROLL, RESTORE_DESCR);
	LoadCellWidths();
	if (rarr)SetGridRowCount(rarr->arr.GetSize());
	UpdateScrollSizes();
	RedrawWindow();
}

BOOL CGridCtrl::SaveCellDataRestore(CELL_DATA& Data, int nCol, int nRow)
{
	int id = GetCellCtrlID(nCol, nRow);
	if (nRow >= rarr->arr.GetSize())return 0;
	RESTORE_TYPE* r = &rarr->arr[nRow];
	switch (id)
	{
	case RESTORE_SEL:
		r->bSelect = Data.m_dwTag;
		break;
	}
	return 1;
}

CELL_DATA CGridCtrl::GetCellDataRestore(int nCol, int nRow)
{
	CELL_DATA CellData;
	int id = GetCellCtrlID(nCol, nRow);
	if (nRow >= rarr->arr.GetSize())return CellData;
	RESTORE_TYPE* r = &rarr->arr[nRow];
	switch (id)
	{
	case RESTORE_SEL:
		CellData.m_dwTag = r->bSelect;
		break;
	case RESTORE_DESCR:
		CellData.m_strText = r->strDescr;
		break;
	}
	return CellData;
}

CALXCellCtrl* CGridCtrl::GetCellCtrl(int nCol, int nRow)
{
	if (nCol >= GetHeadColCount() || nCol < 0)return 0;
	UINT nId = GetCellCtrlID(nCol, nRow);
	if (nGridId == GRIDID_RESTORE && nId == RESTORE_SEL)
	{
		if (nRow >= 0 && nRow < rarr->arr.GetSize())
		{
			RESTORE_TYPE* r = &rarr->arr[nRow];
			if (r->nId != -1)return GetCheckButton();
		}
	}
	return CALXGridCtrl::GetCellCtrl(nCol, nRow);
}

CALXCellCtrl* CGridCtrl::GetCheckButton()
{
	if (!check)
	{
		check = new CALXButtonCtrl;
		check->CreateCtrl(WS_CHILD | BS_AUTOCHECKBOX | BS_FLAT | ES_CENTER, this, 1);
		check->EnableCtrl(FALSE);
		check->SetFocusRect(&m_MetricsCells.m_FocusRect);
		check->SetFontCtrl(GetGridFont());
	}
	return check;
}

CELL_INFO CGridCtrl::GetCellInfo(int nCol, int nRow)
{
	CELL_INFO ci = CALXGridCtrl::GetCellInfo(nCol, nRow);
	int nType = GA_CELLCTRL;
	int id = GetCellCtrlID(nCol, nRow);
	if (nGridId == GRIDID_RESTORE && id == RESTORE_SEL)
	{
		if (nRow >= 0 && nRow < rarr->arr.GetSize())
		{
			RESTORE_TYPE* r = &rarr->arr[nRow];
			if (r->nId != -1)nType = GA_BUTTONCTRL;
		}
	}
	if (nType == GA_COMBOBOXCTRL)
	{
		ci.m_nTypeCtrl = DFC_SCROLL;
		ci.m_nState = DFCS_SCROLLCOMBOBOX;
	}
	else if (nType == GA_BUTTONCTRL)
	{
		ci.m_nTypeCtrl = DFC_BUTTON;
		ci.m_nState = DFCS_BUTTONCHECK | DFCS_FLAT;
	}
	return ci;
}

void CGridCtrl::FillDataScalePoints(CNFC_Device* nfc)
{
	this->nfc = nfc;
	RemoveAllCol();
	CRect rt;
	GetClientRect(&rt);
	int nWidth = (rt.Width() - 10) / (SCALE_SIZE - SCALE_GAS);
	CString h[] = { S_O::LoadString(IDS_GAS),S_O::LoadString(IDS_VALUE),S_O::LoadString(IDS_UNITS) };
	for (int i = SCALE_GAS; i < SCALE_SIZE; i++)
	{
		if (i == SCALE_GAS)DefineColCtrl(AddCol(nWidth, h[i - SCALE_GAS], ACFF_CENTER, AHFF_LEFT, 0, 0, i), GA_CHECKCTRL, WS_CHILD | ES_LEFT | ES_AUTOHSCROLL, i);
		else DefineColCtrl(AddCol(nWidth, h[i - SCALE_GAS], ACFF_LEFT, AHFF_LEFT, 0, 0, i), GA_EDITCTRL, WS_CHILD | ES_LEFT | ES_AUTOHSCROLL, i);
	}
	LoadCellWidths();
	nData.RemoveAll();
	if (nfc)
	{
		if (!nfc->bManufact)
		{
			BYTE en[] = { nfc->sensors_info.bCH4En,nfc->sensors_info.bO2En, nfc->sensors_info.bCOEn,nfc->sensors_info.bH2SEn };
			for (int i = 0; i < 4; i++)
			{
				if (en[i])nData.Add(i);
			}
		}
		else
		{
			for (int i = 0; i < 4; i++)nData.Add(i);
		}
		SetGridRowCount(nData.GetSize());
	}
	UpdateScrollSizes();
	RedrawWindow();
}

BOOL CGridCtrl::SaveCellDataScalePoints(CELL_DATA& Data, int nCol, int nRow)
{
	int id = GetCellCtrlID(nCol, nRow);
	if (nRow >= nData.GetSize())return 0;
	GAS_LIMITS* r = &nfc->limits[nData[nRow]];
	switch (id)
	{
	case SCALE_DATA:
	{
		double val = S_O::ToDouble(Data.m_strText)*nfc->GetGasDevider(r->nGas, 1);
		if (r->nGas == GAS_O2)nfc->dev_settings.ScalePoint.O2 = val;
		else if (r->nGas == GAS_CO)nfc->dev_settings.ScalePoint.CO = val;
		else if (r->nGas == GAS_H2S)nfc->dev_settings.ScalePoint.H2S = val;
		else nfc->dev_settings.ScalePoint.CH4 = val;
		break;
	}
	}
	return 1;
}

CELL_DATA CGridCtrl::GetCellDataScalePoints(int nCol, int nRow)
{
	CELL_DATA CellData;
	int id = GetCellCtrlID(nCol, nRow);
	if (nRow >= nData.GetSize())return CellData;
	GAS_LIMITS* r = &nfc->limits[nData[nRow]];
	switch (id)
	{
	case SCALE_GAS:
		CellData.m_strText = nfc->GetGasString(r->nGas);
		break;
	case SCALE_UNITS:
		CellData.m_strText = nfc->GetUnitsString(r->nGas,1);
		break;
	case SCALE_DATA:
	{
		uint16_t val;
		if (r->nGas == GAS_O2)val = nfc->dev_settings.ScalePoint.O2;
		else if (r->nGas == GAS_CO)val = nfc->dev_settings.ScalePoint.CO;
		else if (r->nGas == GAS_H2S)val = nfc->dev_settings.ScalePoint.H2S;
		else val = nfc->dev_settings.ScalePoint.CH4;
		double v = val;
		v /= nfc->GetGasDevider(r->nGas, 1);
		CellData.m_strText.Format("%g", v);
		break;
	}
	}
	return CellData;
}

void CGridCtrl::FillDataRSSI(CNFC_Device* nfc)
{
	this->nfc = nfc;
	RemoveAllCol();
	CRect rt;
	GetClientRect(&rt);
	int nWidth = (rt.Width() - 10) / (RSSI_SIZE - RSSI_LEVEL);
	CString h[] = { S_O::LoadString(IDS_LEVEL),S_O::LoadString(IDS_RSSI_CURRENT),S_O::LoadString(IDS_RSSI_NEIGHBOR) };
	for (int i = RSSI_LEVEL; i < RSSI_SIZE; i++)
	{
		if (i == RSSI_LEVEL)DefineColCtrl(AddCol(nWidth, h[i - RSSI_LEVEL], ACFF_CENTER, AHFF_LEFT, 0, 0, i), GA_CHECKCTRL, WS_CHILD | ES_LEFT | ES_AUTOHSCROLL, i);
		else DefineColCtrl(AddCol(nWidth, h[i - RSSI_LEVEL], ACFF_LEFT, AHFF_LEFT, 0, 0, i), GA_EDITCTRL, WS_CHILD | ES_LEFT | ES_AUTOHSCROLL, i);
	}
	SetGridRowCount(6);
	LoadCellWidths();
	UpdateScrollSizes();
	RedrawWindow();
}

BOOL CGridCtrl::SaveCellDataRSSI(CELL_DATA& Data, int nCol, int nRow)
{
	int id = GetCellCtrlID(nCol, nRow);
	if (nRow >= 6)return 0;
	switch (id)
	{
	case RSSI_CURRENT:
		nfc->wifi.RSSI[nRow][0] = atoi(Data.m_strText);
		break;
	case RSSI_NEIGHBOR:
		nfc->wifi.RSSI[nRow][1] = atoi(Data.m_strText);
		break;
	}
	return 1;
}

CELL_DATA CGridCtrl::GetCellDataRSSI(int nCol, int nRow)
{
	CELL_DATA CellData;
	int id = GetCellCtrlID(nCol, nRow);
	if (nRow >= 6)return CellData;
	switch (id)
	{
	case RSSI_LEVEL:
		CellData.m_strText = S_O::LoadString(IDS_RSSI_LEVEL0+nRow);
		break;
	case RSSI_CURRENT:
		CellData.m_strText.Format("%d", (int)nfc->wifi.RSSI[nRow][0]);
		break;
	case RSSI_NEIGHBOR:
		CellData.m_strText.Format("%d", (int)nfc->wifi.RSSI[nRow][1]);
		break;
	}
	return CellData;
}

void CGridCtrl::AddRow(CString strHdr, CString strVal, CString strVal2)
{
	names.Add(strHdr);
	names.Add(strVal);
	if (columnnames.GetSize() == 3)names.Add(strVal2);
}

void CGridCtrl::FillData(DELAYED_COMMAND_ARRAY* arr, bool bAddStatus)
{
	this->dca = arr;
	bWidthDependsOnColumnCount = 1;
	RemoveAllCol();
	CRect rt;
	GetClientRect(&rt);
	UINT nEnd = bAddStatus ? DCA_SIZE : DCA_STATUS;
	int nWidth = (rt.Width() - 10) / (nEnd - DCA_N);
	CString h[] = { "N",S_O::LoadString(IDS_COMMAND),S_O::LoadString(IDS_FROMPREVCOMMAND),S_O::LoadString(IDS_FROMSTART),S_O::LoadString(IDS_STATUS) };
	for (int i = DCA_N; i < nEnd; i++)
	{
		if (i == DCA_N)DefineColCtrl(AddCol(nWidth, h[i - DCA_N], ACFF_CENTER, AHFF_LEFT, 0, 0, i), GA_CHECKCTRL, WS_CHILD | ES_LEFT | ES_AUTOHSCROLL, i);
		else DefineColCtrl(AddCol(nWidth, h[i - DCA_N], ACFF_LEFT, AHFF_LEFT, 0, 0, i), GA_CHECKCTRL, WS_CHILD | ES_LEFT | ES_AUTOHSCROLL, i);
	}
	SetGridRowCount(arr->arr.GetSize());
	LoadCellWidths();
	UpdateScrollSizes();
	RedrawWindow();
}

CELL_DATA CGridCtrl::GetCellDataDCA(int nCol, int nRow)
{
	CELL_DATA CellData;
	int id = GetCellCtrlID(nCol, nRow);
	if (nRow >= dca->arr.GetSize())return CellData;
	DELAYED_COMMAND& c = dca->arr[nRow];
	switch (id)
	{
	case DCA_N:
		CellData.m_strText = S_O::FormatUINT(nRow + 1);
		break;
	case DCA_STATUS:
	{
		UINT n = 0;
		if (c.nStatus == DCAS_DONE)n = IDS_DONE;
		else if (c.nStatus == DCAS_WAIT)n = IDS_WAIT;
		else if (c.nStatus == DCAS_NOTDONE)n = IDS_QUERYERROR;
		else if (c.nStatus == DCAS_EXECUTE)n = IDS_EXECUTING;
		if (n != 0)CellData.m_strText = S_O::LoadString(n);
		break;
	}
	case DCA_COMMAND:
		CellData.m_strText=c.strCommand;
		if (c.bT)CellData.m_strText += S_O::Format(" T=%g (%g)", c.dT, c.dTTime);
		if (c.bGas)CellData.m_strText += S_O::Format(" G=%u %g (%g)", c.nValve, c.dGasConc,c.dGasTime);
		break;
	case DCA_DELAY:
		CellData.m_strText.Format("%g", c.dTimeDelay);
		break;
	case DCA_DELAYFROMSTART:
		CellData.m_strText.Format("%g", c.dTimeDelayFromStart);
		break;
	}
	return CellData;
}

void CGridCtrl::FillData(DELAYED_COMMAND_RESULT_ARRAY* arr)
{
	this->dcar = arr;
	RemoveAllCol();
	CRect rt;
	GetClientRect(&rt);
	int nWidth = (rt.Width() - 10) / (DCAR_SIZE - DCAR_N);
	CString h[] = {"N", S_O::LoadString(IDS_TIME),S_O::LoadString(IDS_SERIALNO),S_O::LoadString(IDS_COMMAND),S_O::LoadString(IDS_VALUE) };
	for (int i = DCAR_N; i < DCAR_SIZE; i++)
	{
		if (i == DCAR_N)DefineColCtrl(AddCol(nWidth, h[i - DCAR_N], ACFF_CENTER, AHFF_LEFT, 0, 0, i), GA_CHECKCTRL, WS_CHILD | ES_LEFT | ES_AUTOHSCROLL, i);
		else DefineColCtrl(AddCol(nWidth, h[i - DCAR_N], ACFF_LEFT, AHFF_LEFT, 0, 0, i), GA_CHECKCTRL, WS_CHILD | ES_LEFT | ES_AUTOHSCROLL, i);
	}
	SetGridRowCount(dcar->arr.GetSize());
	LoadCellWidths();
	UpdateScrollSizes();
	RedrawWindow();
}

CELL_DATA CGridCtrl::GetCellDataDCAR(int nCol, int nRow)
{
	CELL_DATA CellData;
	int id = GetCellCtrlID(nCol, nRow);
	if (nRow >= dcar->arr.GetSize())
	{
		return CellData;
	}
	DELAYED_COMMAND_RESULT& c = dcar->arr[nRow];
	switch (id)
	{
	case DCAR_N:
		CellData.m_strText = S_O::FormatUINT(nRow + 1);
		break;
	case DCAR_TIME:
		CellData.m_strText = c.tm.FormatStandard(1);
		break;
	case DCAR_SERIAL:
		CellData.m_strText = c.strDeviceId;
		break;
	case DCAR_COMMAND:
		CellData.m_strText = c.strCommand;
		break;
	case DCAR_RESULT:
		CellData.m_strText = c.strResult;
		break;
	}
	return CellData;
}

void CGridCtrl::RemoveAllCol(bool bClearGroups)
{
	CALXGridCtrl::RemoveAllCol();
	if (bClearGroups)
	{
		gridrows.RemoveAll();
	}
	ClearAllHeadFilters();

	sortcols.RemoveAll();
	//	nLastGroupColumn=-1;
	av_columns.RemoveAll();
}