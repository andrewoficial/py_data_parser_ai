// GridCtrl.h: interface for the CGridCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRIDCTRL_H__33B17C55_9DFA_4CC8_9392_36513DDB0C3B__INCLUDED_)
#define AFX_GRIDCTRL_H__33B17C55_9DFA_4CC8_9392_36513DDB0C3B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ALXGridCtrl.h"

#define WM_CELLSELECTED WM_USER+1
#define WM_NOTIFYRBUTTONUP WM_USER+2
#define WM_NOTIFYLDBLCLK WM_USER+3
#define WM_HEADCLICKED WM_USER+6
#define WM_LOADALLDATA WM_USER+7
#define WM_CELLMODIFIED WM_USER+8

enum
{
	GRIDID_DEFAULT=0,
	GRIDID_DEVICES,
	GRIDID_DATA,
	GRIDID_DEVICESSEL,
	GRIDID_LOG,
	GRIDID_GRAPH,
	GRIDID_LIMITS,
	GRIDID_RESTORE,
	GRIDID_SCALEPOINT,
	GRIDID_RSSI,
	GRIDID_DCA,
	GRIDID_DCAR
};

enum
{
	DEV_N=10,
	DEV_SEL,
	DEV_NAME,
	DEV_GROUP,
	DEV_LASTTIME,
	DEV_DATACOUNT,
	DEV_ADDRESS,
	DEV_SETASVIEWED,
	DEV_SIZE
};

enum
{
	DEVDATA_N=30,
	DEVDATA_DATA,
	DEVDATA_GAS1,
	DEVDATA_GAS2,
	DEVDATA_GAS3,
	DEVDATA_GAS4,
	DEVDATA_T,
	DEVDATA_TIME,
	DEVDATA_SIZE
};

enum
{
	LOG_N=50,
	LOG_DATE,
	LOG_FLAG,
	LOG_TEMP,
	LOG_O2,
	LOG_CO,
	LOG_H2S,
	LOG_CH4,
	LOG_O2ADC,
	LOG_O2VOLT,
	LOG_COADC,
	LOG_COVOLT,
	LOG_H2SADC,
	LOG_H2SVOLT,
	LOG_PRESSURE,
	LOG_BATTLOG,
	LOG_RSSI,
	LOG_SNR,
	LOG_EXT,
	LOG_FREEZE,
	LOG_SIZE
};

enum
{
	GRAPH_N = 70,
	GRAPH_DATE,
	GRAPH_TEMP,
	GRAPH_O2,
	GRAPH_CO,
	GRAPH_H2S,
	GRAPH_CH4,
	GRAPH_O2ADC,
	GRAPH_O2VOLT,
	GRAPH_COADC,
	GRAPH_COVOLT,
	GRAPH_H2SADC,
	GRAPH_H2SVOLT,
	GRAPH_BATTERY,
	GRAPH_PRESSURE,
	GRAPH_CPU,
	GRAPH_SIZE
};

enum
{
	LIM_GAS=90,
	LIM_LIM1,
	LIM_LIM2,
	LIM_LIM1TEST,
	LIM_LIM2TEST,
	LIM_SIZE
};

enum
{
	RESTORE_SEL=100,
	RESTORE_DESCR
};

enum
{
	SCALE_GAS=120,
	SCALE_DATA,
	SCALE_UNITS,
	SCALE_SIZE
};

enum
{
	RSSI_LEVEL=130,
	RSSI_CURRENT,
	RSSI_NEIGHBOR,
	RSSI_SIZE
};

enum
{
	DCA_N=140,
	DCA_COMMAND,
	DCA_DELAY,
	DCA_DELAYFROMSTART,
	DCA_STATUS,
	DCA_SIZE
};

enum
{
	DCAR_N = 150,
	DCAR_TIME,
	DCAR_SERIAL,
	DCAR_COMMAND,
	DCAR_RESULT,
	DCAR_SIZE
};

struct GRID_GROUP
{
	CUIntArray rows;
	CString strName;
	CString strAlternativeInfo;
	BYTE bExpand;
	GRID_GROUP()
	{
		bExpand = 0;
	};
};

struct GRID_ROW
{
	int nRealRow;
	GRID_GROUP* group;
	GRID_ROW()
	{
		group = 0;
		nRealRow = 0;
	};
};


struct DEVICE_LOG;
class CNFC_Device;
struct RESTORE_TYPE_ARRAY;
struct DELAYED_COMMAND_ARRAY;
struct DELAYED_COMMAND_RESULT_ARRAY;

class CGridCtrl : public CALXGridCtrl
{
public:
	CGridCtrl();
	virtual ~CGridCtrl();
	CStringArray names;
	CStringArray columnnames;
	CUIntArray nStatus;
	CUIntArray nData;
	CUIntArray changed;
	union
	{
		void* ptrData;
		DEVICE_LOG* log;
		CNFC_Device* nfc;
		RESTORE_TYPE_ARRAY* rarr;
		DELAYED_COMMAND_ARRAY* dca;
		DELAYED_COMMAND_RESULT_ARRAY* dcar;
	};
	CUIntArray nNumeric;
	CUIntArray nDateColumns;
	RM_DATABASEIDLIST strDateFormat;
	int nSelColumn;
	bool bSortEnabled;
	DECLARE_DYNCREATE(CGridCtrl)
	DECLARE_REGISTER()

public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridCtrl)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnEndResizeCol(int nCol, int nNewWidth);
	//}}AFX_VIRTUAL
// Generated message map functions
		// Implementation
	virtual BOOL OnSaveCellData(int nCol, int nRow);
public:
	bool bChangeDataAllowed;
	bool bFirstColNarrow;
	bool bWidthDependsOnColumnCount;
	void AddRow(CString strHdr, CString strVal, CString strVal2="");
	void GetLogColumns(DEVICE_LOG* log, CUIntArray& ids, CStringArray& names);
	CString GetCellDataLog(DEVICE_LOG* log, int nColId, int nRow);
	void FillData();
	void FillData(DEVICE_LOG* log);
	void FillData(CNFC_Device* nfc);
	void FillDataRSSI(CNFC_Device* nfc);
	void FillDataLimits(CNFC_Device* nfc);
	void FillDataScalePoints(CNFC_Device* nfc);
	void FillData(RESTORE_TYPE_ARRAY* arr);
	void FillData(DELAYED_COMMAND_ARRAY* arr, bool bAddStatus);
	void FillData(DELAYED_COMMAND_RESULT_ARRAY* arr);
	BOOL SaveCellDataLog(CELL_DATA& Data, int nCol, int nRow);
	CELL_DATA GetCellDataLog(int nCol, int nRow);
	CELL_DATA GetCellDataGraph(int nCol, int nRow);
	BOOL SaveCellDataLimits(CELL_DATA& Data, int nCol, int nRow);
	CELL_DATA GetCellDataLimits(int nCol, int nRow);
	BOOL SaveCellDataRestore(CELL_DATA& Data, int nCol, int nRow);
	CELL_DATA GetCellDataRestore(int nCol, int nRow);
	BOOL SaveCellDataScalePoints(CELL_DATA& Data, int nCol, int nRow);
	CELL_DATA GetCellDataScalePoints(int nCol, int nRow);
	BOOL SaveCellDataRSSI(CELL_DATA& Data, int nCol, int nRow);
	CELL_DATA GetCellDataRSSI(int nCol, int nRow);
	CELL_DATA GetCellDataDCA(int nCol, int nRow);
	CELL_DATA GetCellDataDCAR(int nCol, int nRow);
	virtual bool IsUnicode(int nCol, int nRow);
	int GetActiveRow();
	int GetRealActiveRow();
	void LoadCellWidths();
	void ClearCellWidths();
	bool bSaveColWidth;
	void RemoveAllCol(bool bClearGroups = 1);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
protected:
	CALXCellCtrl* check;
	CALXCellCtrl* GetCheckButton();
	virtual CALXCellCtrl* GetCellCtrl(int nCol, int nRow);
	virtual CELL_INFO GetCellInfo(int nCol, int nRow);
	int GetSaveColumnsCount();
	//{{AFX_MSG(CGridCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	RM_DATABASEIDLIST av_columns;
	bool IsShowColumn(UINT nColId);
	bool bCellWidthLoaded;
	void SaveCellWidths();
	virtual CELL_DATA GetCellData(int nCol, int nRow);
	virtual HBITMAP GetBitmap(int nCol, int nRow, BOOL bSelected);
	virtual void OnHeadButtonDown(int nHeadCol, int nHeadRow);
	virtual CALXCellMetrics* GetCellMetrics(int nCol, int nRow);
	CBitmap m_BMP;
	virtual bool OnDrawCellContent(CDC *pDC,int nCol,int nRow,CRect& rectCell);
	CArray<GRID_ROW, GRID_ROW&> gridrows;
	CArray<GRID_SORT, GRID_SORT&> sortcols;
	void SortByColumn(int nCol, bool bAsc, bool bForceDest, bool bControlPressed, bool bRedraw=1);
	void SortGroups(bool bRedraw);
	bool bForceGetData;
	static CUIntArray staticNumeric;
	static CStringArray staticDate;
	static CArray<GRID_SORT, GRID_SORT&> staticSort;
	static int compare(const void* arg1, const void* arg2);
	bool IsColumnNumeric(int nCol);
	CString IsColumnDate(int nCol);
	CALXCellMetrics m_Normal;
	CALXCellMetrics m_Attention;
	CALXCellMetrics m_Error;
	CALXCellMetrics m_DeviceError;
	CALXCellMetrics m_AttentionData;
	CALXCellMetrics m_ErrorData;
};


#endif // !defined(AFX_GRIDCTRL_H__33B17C55_9DFA_4CC8_9392_36513DDB0C3B__INCLUDED_)
