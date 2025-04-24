#pragma once
#include "CLongGasMainDlg.h"

// CLGFirmwareDlg dialog

class CLGFirmwareDlg : public CLongGasBaseDlg
{
	DECLARE_DYNAMIC(CLGFirmwareDlg)

public:
	CLGFirmwareDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CLGFirmwareDlg();
	virtual bool CommandDone(CNFC_Command* c);
	virtual CString GetName() { return S_O::LoadStringW(IDS_LG_FIRMWARE); }
	virtual void OnTabSelected();
//	virtual void FirmwareStarted();
//	virtual void FirmwareEnded();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LG_FIRMWARE };
#endif
	virtual UINT GetId() { return IDD_LG_FIRMWARE; };

protected:
	virtual void FillIdAllArray() {};
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CProgressCtrl m_Progress;
	afx_msg void OnBnClickedSendfirmware();
	afx_msg void OnBnClickedSendfirmwareall();
protected:
	void StartFirmware(bool bAll);
};
