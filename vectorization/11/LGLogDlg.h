#pragma once
#include "CLongGasMainDlg.h"

// CLGLogDlg dialog

class CLGLogDlg : public CLongGasBaseDlg
{
	DECLARE_DYNAMIC(CLGLogDlg)

public:
	CLGLogDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CLGLogDlg();
	virtual bool CommandDone(CNFC_Command* c);
	virtual CString GetName() { return S_O::LoadStringW(IDS_LG_LOG); }
	virtual bool OperationStarted(CNFC_Device* dev, UINT nType);
	virtual bool OperationEnded(CNFC_Device* dev, UINT nType);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LG__LOG };
#endif
	virtual UINT GetId() { return IDD_LG__LOG; };

protected:
	void OperationToControls(CNFC_Device* nfc, UINT nType, bool bOperationStarted);
	CDateTime tmStart;
	virtual void FillIdAllArray() {};
	CGraph* gr;
	int GetCurveCount();
	CString GetCurveName(int n);
	bool bCurveSelChanged;
	void ClickedRadioLog();
	void GetFolderName(bool bAll);
	bool bPrevShort2;
	bool bPrevMIPEX;
	CString GetSaveFileName(CNFC_Device * nfc,CString strDir);
	static int nCompareMode;
	static int compare(const void* arg1, const void* arg2);
	void FillGraph(bool bFromStart);
	void FillCurveList(bool bInit);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnTabSelected();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedGet();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedRadioLog();
	CListBox m_Curves;
	afx_msg void OnLbnSelchangeListCurves();
	void ShowCurves();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedForward();
	afx_msg void OnBnClickedReset();
	//afx_msg void OnBnClickedSavecsv();
	afx_msg void OnBnClickedGetAll();
	//afx_msg void OnBnClickedSaveall();
	bool SaveToDir(CNFC_Device* nfc, CString strPath,bool bAppend=1);
	virtual void ChangeAll(CNFC_Device* dev, UINT nMode, CHANGEALL* c);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CEdit m_FilePath;
	afx_msg void OnBnClickedOpenfolder();
	BOOL m_bShowBadData;
};
