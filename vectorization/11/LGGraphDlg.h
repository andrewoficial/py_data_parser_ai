#pragma once

#include "CLongGasMainDlg.h"
#include "NFC_Device.h"
// CLGGraphDlg dialog

class CLGGraphDlg : public CLongGasBaseDlg
{
	DECLARE_DYNAMIC(CLGGraphDlg)

public:
	CLGGraphDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CLGGraphDlg();

	virtual CString GetName() { return S_O::LoadStringW(IDS_LG_GRAPH); }
	virtual bool CommandDone(CNFC_Command* c);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LG_GRAPH };
#endif

	virtual UINT GetId() { return IDD_LG_GRAPH; };

protected:
	virtual void FillIdAllArray() {};
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnTabSelected();
private:
	CGraph* gr;	
	bool bCurveSelChanged;
	CString GetSaveFileName(CNFC_Device* dev,bool bAuto);
	//bool bChanged[7];
	//void ConcChanged(UINT n, UINT nId);
	bool bInSetAll;
	void GetFolderName(CNFC_Device* dev, bool bAll);
	void FillGraph(bool bFromStart);
	CString GetCurveName(UINT nId,bool bUnicode=0);
	void ShowCurves();	
	bool SaveCSV(CNFC_Device* dev, CString strPath, bool bAll, bool bLast);
	void ClickedRadioGraph();
	void UpdateCurveName(CUIntArray& arr,int nCurve);
	void SensStatusToControls();
	void UpdateMean();
public:
	afx_msg void OnBnClickedStart();
	CString m_PollTime;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CMyListBox m_ListCurves;
	BOOL m_CH4En;
	BOOL m_H2SEn;
	BOOL m_O2En;
	BOOL m_COEn;
	afx_msg void OnLbnSelchangeListCurves();
	afx_msg void OnDestroy();
	CString m_Mean;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedRadioGraph();
	afx_msg void OnBnClickedRadioData();
	afx_msg void OnBnClickedStart2();
	bool StartMonitoring(CNFC_Device* dev);
	void StopMonitoring(CNFC_Device * dev);
	virtual void ChangeAll(CNFC_Device* dev, UINT nMode, CHANGEALL* c);
	afx_msg void OnBnClickedGet();
	afx_msg void OnBnClickedGetall();
	BOOL m_bSendF;
	afx_msg void OnBnClickedFcommand();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual bool OperationEnded(CNFC_Device* dev, UINT nType);
	/*afx_msg void OnEnChangeConcO2();
	afx_msg void OnEnChangeConcCo();
	afx_msg void OnEnChangeConcH2s();
	afx_msg void OnEnChangeConcCh4();*/
	afx_msg void OnBnClickedClear();
	CString m_Last;
	CString m_Command;
	CEdit m_FilePath;
	afx_msg void OnBnClickedOpenfolder();
};
