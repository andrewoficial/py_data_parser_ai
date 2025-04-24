
// VegaConnect.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include <MainAppTemplate.h>
#include <XMessageBox.h>


// CVegaConnectApp:
// See VegaConnect.cpp for the implementation of this class
//


//#define LANGUAGE_RU 0
//#define LANGUAGE_ENG 1

struct VEGA_PREFERENCES:public PREFERENCES
{
	CArray< LANGID, LANGID> avLang;
	LANGID nLang;
	UINT nCameraType;
	CString strCameraAddr;
	CString strGasAddr;
	int nCameraSlaveId;
	double dCameraAddTime;
	double dCameraMaxTime;
	double dCameraInterval;
	double dRoomTemp;
	bool b10ChanBKM;
};

class CLongGasApp : public CMainAppTemplate
{
public:
	CLongGasApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual PREFERENCES* GetPreferences();
	virtual void LoadPreferences();
	void FillAvailableLanguages();
	bool ProcessSysCommand(UINT nID, LPARAM lParam);
	void FillSystemMenu(CWnd* w);
// Implementation

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	
protected:
	CWnd wndRunning;
	XMSGBOXPARAMS msgBox;
public:
	virtual int DoMessageBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt);
};

extern CLongGasApp theApp;
