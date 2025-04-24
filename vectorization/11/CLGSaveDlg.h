#pragma once

#include "CLongGasMainDlg.h"

// CLGSaveDlg dialog

struct DEVICE_SAVED_SETTINGS;

struct RESTORE_TYPE
{
	UINT nId;
	CString strDescr;
	bool bSelect;
	RESTORE_TYPE()
	{
		bSelect = 0;
	};
};

struct RESTORE_TYPE_ARRAY
{
	CArray< RESTORE_TYPE, RESTORE_TYPE&> arr;
};

class CLGSaveDlg : public CLongGasBaseDlg
{
	DECLARE_DYNAMIC(CLGSaveDlg)

public:
	CLGSaveDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CLGSaveDlg();
	virtual CString GetName() { return S_O::LoadStringW(IDS_LG_SAVE); }
	virtual bool CommandDone(CNFC_Command* c);
	virtual UINT GetId() { return IDD_LG_SAVE; };

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LG_SAVE };
#endif

protected:
	afx_msg LRESULT OnWmGridCellSelected(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnWmGridHeadClicked(WPARAM wp, LPARAM lp);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnTabSelected();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedSave();
private:
	void Save(bool bUser);
	void FillTypesGrid();
	RESTORE_TYPE_ARRAY rarr;
	void FillSettingsInfo();
	void FillList();
	DEVICE_SAVED_SETTINGS* settings;
	void GetRestoredSettings(UINT nId, CUIntArray& arr);
	void FillRestoredSettingsInfo(UINT nId, CRichEditCtrl& rich, bool bAddDescr);
	void WriteLogInfo(CRichEditCtrl& rich, CString strDescr, CString strInfo, bool bAddCR=1);
	bool IsUserSetting(UINT nId);
public:
	afx_msg void OnBnClickedRestore();
	CEdit m_Info;
	CRichEditCtrl m_Rich;
	CRichEditCtrl m_Base;
	CRichEditCtrl m_SelInfo;
	afx_msg void OnBnClickedSave2();
};
