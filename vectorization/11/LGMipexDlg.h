#pragma once

#include "CLongGasMainDlg.h"
#include "NFC_Device.h"
// CLGMipexDlg dialog


class CLGMipexDlg : public CLongGasBaseDlg
{
	DECLARE_DYNAMIC(CLGMipexDlg)

public:
	CLGMipexDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CLGMipexDlg();

	virtual CString GetName() { return "MIPEX"; }
	virtual bool CommandDone(CNFC_Command* c);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LG_MIPEX };
#endif

	virtual UINT GetId() { return IDD_LG_MIPEX; };

protected:
	void FillGrid();
	virtual void FillIdAllArray() {};
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	bool SendCommand(CNFC_Device* dev,bool bAll);
	bool UpdateMIPEXCommand(CNFC_Device* dev);
	CString GetSaveFileName(CNFC_Device * nfc,bool bAuto);
	bool SaveData(CNFC_Device* nfc, CString strPath, bool bLast);
	void ShowControls();
	void LogAnswer(CNFC_Command* c, DELAYED_COMMAND_RESULT& r,bool bShowIfError);
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CString m_Command;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedSendall();
//	int m_nRepeatTime;
	afx_msg void OnBnClickedRepeat();
private:		
	CDateTime dtStart;
	void GetFolderName(CNFC_Device* nfc, bool bAll);
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedClear();
	afx_msg void OnDestroy();
	virtual void ChangeAll(CNFC_Device* dev, UINT nMode, CHANGEALL* c);
	void StartRepeat(CNFC_Device * dev);
	void StopRepeat(CNFC_Device* dev);
	afx_msg void OnBnClickedRepeatall();
	CString m_RepeatTime;
//	BOOL m_bAutosave;
	afx_msg void OnBnClickedMipex4();
	BOOL m_bMipex4;
	virtual void OnTabSelected();
	afx_msg void OnBnClickedFromfile();
	CEdit m_FilePath;
	afx_msg void OnBnClickedOpenfolder();
	afx_msg void OnBnClickedBackup();
	afx_msg void OnBnClickedRestore();
	afx_msg void OnBnClickedCoeff();
	afx_msg void OnBnClickedLastcommand();
};
