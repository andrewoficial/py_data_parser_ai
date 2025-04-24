#pragma once


// CLongGasMainDlg dialog

#include "HidApi.h"
#include "CRichEditChildDlg.h"
#include <LayoutHelper.h>
#include <EnTabCtrl.h>
#include <ScrollHelper.h>
#include <ProgressCtrlX.h>

class HidApi;
class HidDeviceList;

class CLGMainDlg;
class CLongGasBaseDlg;
class CNFC_Device;
class CNFC_Command;
class CGroupInfoDlg;

class LG_WND_ARRAY
{
public:
	CNFC_Device* dev;	
	LG_WND_ARRAY()
	{
		dev = 0;
	};
	LG_WND_ARRAY& operator=(const LG_WND_ARRAY& a)
	{
		dev = a.dev;
		return *this;
	}
	void Clear();
};

enum
{
	CHANGEALL_MONITORINGONCE,
	CHANGEALL_MULTIPLESELECT
};

struct CHANGEALL
{
	CString str;
	int n;
	double d;
	CUIntArray arr;
};

struct REPORT_COMMAND_DONE
{
	CString strCommand;
	UINT nCount;
};

enum
{
	HKT_SYSTEM,
	HKT_TAB
};

struct HOTKEY
{
	BYTE nType;
	BYTE nTab;
	UINT nCtrlId;
	UINT nMessage;
	WPARAM wParam;
	LPARAM lParam;
	WORD wVirtualCode;
	WORD wModifiers;
	HOTKEY()
	{
		nType = 0;
		nTab = 0;
		nMessage = 0;
		nCtrlId = -1;
		wVirtualCode = wModifiers = 0;
		wParam = lParam = 0;
	}
	CString GetHotkeyName();
};

struct HOTKEY_ARRAY:public RM_VERSIONEDFILE
{
	HOTKEY_ARRAY();
	CArray< HOTKEY, HOTKEY&> arr;
	CArray< HOTKEY, HOTKEY&> arrSystem;
	bool Save();
	bool Load();
	bool IsCanAdd(HOTKEY& hk);
	bool AddHotkey(HOTKEY& hk);
	UINT GetHotkeyIndex(HOTKEY& hk);
};

class CProgressCtrlX;

interface ITaskbarList3;

class CLongGasMainDlg : public CDialog
{
	DECLARE_DYNAMIC(CLongGasMainDlg)

public:
	CLongGasMainDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CLongGasMainDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LONGGASDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg LRESULT OnCellSelected(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnGridRButtonUp(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnNFCCommandDone(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnNFCDeviceDetectionStatus(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnNFCLog(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnNFCOperationDone(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnNFCDeviceAdded(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnWmSwitchToManufact(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnWmSelectNextTab(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnWmSelectNextDevice(WPARAM wp, LPARAM lp);
	
	afx_msg LRESULT OnNFCMarkError(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnWmHotkey(WPARAM wp, LPARAM lp);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
private:
	HidApi *       api;
	static void hidApiErrorCb(HidError err);
	static void deviceAddedCb(HidDeviceDescr& descr);
	static void deviceRemovedCb(std::string& path);
	void SelectTab(int nTab, bool bSetFocus=0);
	void CreateWindows(LG_WND_ARRAY& a, HidDeviceDescr* descr,bool bVirtual=0);
	bool bInStartAll;
	bool bWasStartAll;
	bool bWasError;
	bool bWasTerminated;
	CGroupInfoDlg* group;
	CFont fRich;
	HHOOK keybdhook;
	static LRESULT __stdcall KeyboardProc(int code, WPARAM wParam, LPARAM lParam);
	void SelectManufact();
	bool bManufact;
	UINT nLastTabSel;
	CLayoutHelper layout;
	CRect rtInitial;
	CRect rtInitialLog;
	CRect rtInitialProg;
	CArray< REPORT_COMMAND_DONE, REPORT_COMMAND_DONE&> arrReportCommandDone;
	CScrollHelper scroll;
	HICON m_hIcon;
	void UpdateQualityBar();
	ITaskbarList3* itl;
	int nLastDeviceNum;
public:
	CEnTabCtrl m_Tab;
	CArray< LG_WND_ARRAY, LG_WND_ARRAY&> dlgs;
	CArray< CLongGasBaseDlg*, CLongGasBaseDlg*> arrWnd;
	CArray<CRichEditChildDlg*, CRichEditChildDlg* > logs;
	CArray< CProgressCtrlX*, CProgressCtrlX*> progress;
	CArray<CStatic*, CStatic*> statics;
	afx_msg void OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void StartAll(UINT nMode,CNFC_Device* from,bool bSkipDeviceFrom=0);
	afx_msg void OnClose();
	void BreakAll(CNFC_Device* from,int nOperation=-1, bool bForce=0);
	CComboBox m_Lang;
	afx_msg void OnCbnSelchangeComboLang();
	void ChangeAll(CLongGasBaseDlg * from,UINT nMode, CHANGEALL* c);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	//int SendAll(CLongGasBaseDlg* base, UINT nCommand);
	CLongGasBaseDlg* GetByClass(CNFC_Device* nfc, CString strClass);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	CFont fButton;
	CFont fButtonDefault;
	CFont fButtonBald;
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnPaint();
	CProgressCtrlX m_Quality;
	ITaskbarList3* GetTaskbarList();
	bool IsInDelayedMIPEX();
	CNFC_Device* GetDeviceByNum(UINT nNum);
};

#define TIMER_HOTKEYHIGHLIGHT 1000

struct HOTKEY_HIGHLIGHT
{
	int nStage;
	UINT nCtrlId;
	HOTKEY_HIGHLIGHT() {};
	HOTKEY_HIGHLIGHT(UINT nCtrl)
	{
		nStage = 0;
		nCtrlId = nCtrl;
	}
};

class CLongGasBaseDlg:public CDialogEx
{
public:
	CLongGasBaseDlg(UINT nIDTemplate, CWnd* pParent = nullptr) :CDialogEx(nIDTemplate, pParent)
	{
		main = 0;
		nfcCurrent = 0;
	};
	CLongGasMainDlg* main;
	CNFC_Device* nfcCurrent;
	CRect rtInitial;
	UINT nTabId;
	virtual CString GetName()=0;
	virtual UINT GetId() = 0;
	virtual bool CommandDone(CNFC_Command* c)=0;
	virtual bool OperationStarted(CNFC_Device* dev, UINT nType);
	virtual bool OperationEnded(CNFC_Device* dev, UINT nType);
	virtual void ChangeAll(CNFC_Device* dev,UINT nMode, CHANGEALL* c) {};
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	void HighlightHotkeys();
	bool HotkeyPressed(UINT nHotkey);
	void UpdateSaveFolder(CString strFolder, bool bSave)
	{
		this->strFolder = strFolder;
		this->bSave = bSave;
	}
protected:
	virtual void OnTabSelected() = 0;
	CLayoutHelper layout;	
	CToolTipCtrl m_wndToolTip;
	CArray< HOTKEY_HIGHLIGHT, HOTKEY_HIGHLIGHT&> hl;
	bool IsNumber(MSG* pMsg,bool bAllowFloat=1);
	CString strFolder;
	bool bSave;
	
public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnWmTabSelected(WPARAM, LPARAM);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};