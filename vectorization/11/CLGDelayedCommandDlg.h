#pragma once

#include "CLongGasMainDlg.h"
#include <UniversalDevice.h>
#include "CGraphDlg.h"
#include "NFC_Device.h"

// CLGDelayedCommandDlg dialog



enum
{
	CAMERA_TESTA=0,
	CAMERA_BINDER,
	CAMERA_BINDERMODBUS,
	CAMERA_2500
};

enum
{
	TC_UNKNOWN,
	TC_SLOPE,
	TC_WAIT,
	TC_GASSTARTED,
	TC_GASREACHED,
	TC_DONE
};

class CLGDelayedCommandDlg;

struct DELAYED_COMMAND_STATUS
{
	CNFC_Device* nfc;
	UINT nStatus;
	DELAYED_COMMAND_STATUS()
	{
		nStatus = 0;
		nfc = 0;
	};
};

class CTCamera
{
public:
	CTCamera()
	{
	};
	virtual bool Open(CString strAddr)=0;
	virtual bool SetTemp(double dTemp)=0;
	virtual bool GetTemp(double& dTemp)=0;
	virtual void Close()
	{
		camera.Close();
	}
	virtual bool IsNeedSetTemp() { return 0; };
	virtual bool IsOpened()
	{
		return camera.IsOpened();
	}
protected:
	CUniversalDevice camera;

};

class CBinderTCamera:public CTCamera
{
public:
	CBinderTCamera()
	{
		nHumidityControlIsOn = -1;
	};
	virtual bool Open(CString strAddr);
	virtual bool SetTemp(double dTemp);
	virtual bool GetTemp(double& dTemp) ;
protected:
	void CheckHumidityControl(double dTemp);
	WORD  CalcCheckSum(CString& str);
	bool CheckChecksum(CString& str);
	int nHumidityControlIsOn;
	bool WriteRead(CString& str, CString& read, int nSize,CString strDescr);
};

class CModbusTCamera :public CTCamera,public CModbusDevice
{
public:
	CModbusTCamera();
	virtual bool Open(CString strAddr);
	virtual bool SetTemp(double dTemp);
	virtual bool GetTemp(double& dTemp);
protected:
	bool TurnOn(bool bOn);
	void LogError();
};


class CTestaTCamera :public CTCamera
{
public:
	CTestaTCamera()
	{
		dTSet = -1000;
		dTSetRead = 0;
	};
	virtual bool Open(CString strAddr);
	virtual bool SetTemp(double dTemp);
	virtual bool GetTemp(double& dTemp);
	virtual bool IsNeedSetTemp();
	virtual void Close()
	{
		camera.Close();
		cameraUDPRcv.Close();
	}
protected:
	CUniversalDevice cameraUDPRcv;
	double dTSet;
	double dTSetRead;
};

class TERMOCAMERA_PROC
{
public:
	static CWnd* wNotify;
	UINT nBkValve;
	double dT;
	double dBkConc;
	UINT nCurrentValve;
	double dCurrentConc;
	DELAYED_COMMAND * c;
	CDateTime tmAllStarted;
	UINT nStatus;
	bool bRunning;
	bool bStopping;
	bool bGasOffAllowed;
	bool bWaitingForGasOff;
	bool bBkGas;
	bool bInBk;
	TERMOCAMERA_PROC();
	CEvent evBreak;
	CArray< DELAYED_COMMAND_STATUS, DELAYED_COMMAND_STATUS&> done;
	bool Open();
	bool StartCamera();
	void StopCamera();
	void SetCommandStatus(CNFC_Device* nfc, UINT nStatus);
	bool IsAllDone();
	bool IsDone(CNFC_Device* nfc);
	bool OffGas();
	bool CheckChecksum(CString& str);
	CGraphDlg* graph;
	static bool WriteLog(CString& str,bool bError=0);
	bool SetTemp(double dTemp);
	bool IsCameraOpened() { return camera ? camera->IsOpened() : 0; };
private:
	CTCamera* camera;
	CUniversalDevice gas;

	bool GetTemp(double& dTemp);
	static DWORD WINAPI CameraThread(void* ptr);
	WORD CalcCheckSum(CString& str);

	bool StartGas(UINT nValve, UINT nConc, UINT nTime);
	bool CloseValve();
	bool SendGasCommand(CString str, bool bInitialization=0);
};

class CLGDelayedCommandDlg : public CLongGasBaseDlg
{
	friend class CLongGasMainDlg;
	DECLARE_DYNAMIC(CLGDelayedCommandDlg)

public:
	CLGDelayedCommandDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CLGDelayedCommandDlg();
	virtual CString GetName() { return S_O::LoadStringW(IDS_LG_DELAYEDCOMMAND); }
	virtual UINT GetId() { return IDD_LG_DELAYED; };
	virtual bool CommandDone(CNFC_Command* c);
	virtual void ChangeAll(CNFC_Device* dev, UINT nMode, CHANGEALL* c);
	virtual void OnTabSelected();
	void Start(CNFC_Device* dev);
	void Stop(CNFC_Device * dev,bool bStopCamera);
	virtual bool OperationEnded(CNFC_Device* dev, UINT nType);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LG_DELAYED };
#endif

protected:
	DELAYED_COMMAND_ARRAY dca;
	static TERMOCAMERA_PROC t_proc;
	CDateTime dtStart;
	CFont fT;
	CFont fTSet;
	CFont fTStatus;
	CFont fValve;
	CFont fConc;
	bool bShowStatus;
	bool bProfileSelected;
	void FillGrid();
	void FillResultGrid();
	void SaveDCA();
	virtual void FillIdAllArray() {};
	void GetFolderName(bool bAll);
	CString GetSaveFileName(CNFC_Device* dev,bool bAuto);
	bool GetCommandParam(DELAYED_COMMAND& c);
	bool UpdateMIPEXCommand(CNFC_Device* dev, CString str);
	void LogAnswer(CNFC_Command* c, bool bShowIfError);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void LoadProfiles(CString strSelect);
	void ProfileChanged(bool bSendAll);
	void SaveToFile(CString& strPath, DELAYED_COMMAND_RESULT& r);
	void StartCamera(DELAYED_COMMAND& c);
	void CheckDelayedExecution(CNFC_Device* dev);
	void SelectNextCommand(CNFC_Device* dev,bool bStartCamera);
	void ClearIfWasMeasure();
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedMod();
	afx_msg void OnBnClickedDel();
	afx_msg void OnBnClickedSend();
	afx_msg void OnBnClickedSendall();
	afx_msg void OnBnClickedClear();
	afx_msg void OnBnClickedOpenfolder();
	afx_msg LRESULT OnWmCameraStatusChanged(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnWmCameraLog(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnWmCameraError(WPARAM wp, LPARAM lp);
	CComboBox m_Profile;
	CString m_Command;
	CEdit m_FilePath;
	CString m_RepeatTime;
	CStatic m_Elapsed;
	afx_msg void OnBnClickedModProfile();
	afx_msg void OnBnClickedDelProfile();
	afx_msg void OnCbnSelchangeProfile();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedMenu();
	CStatic m_T;
	CStatic m_TSet;
	CStatic m_TStatus;
	CStatic m_CurrentValve;
	CStatic m_CurrentConc;
	BOOL m_BkGas;
	BOOL m_BkOffAfter;
	BOOL m_StopAfter;
	CComboBox m_BkValve;
	double m_BkGasConc;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedCamerasettings();
	afx_msg void OnBnClickedOffafter();
	afx_msg void OnBnClickedStopallafter();
};

class CDelayedCommandEditor : public CDialogEx
{
	DECLARE_DYNAMIC(CDelayedCommandEditor)

public:
	CDelayedCommandEditor(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDelayedCommandEditor();
	DELAYED_COMMAND c;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DELAYEDCOMMANDEDITOR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_Command;
	double m_FromPrev;
	double m_T;
	BOOL m_bT;
	double m_TTime;
	CComboBox m_Valve;
	BOOL m_bGas;
	double m_GasConc;
	double m_GasTime;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};

