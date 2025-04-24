
// VegaConnect.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "LongGas.h"
#include "GridCtrl.h"
#include <EnumProcess.h>
#include <DialogOperations.h>
#include <locale.h>
#include <kd_doc.h>
//#include "CLongGasMainDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "CLongGasMainDlg.h"
#include "RCTranslateDlg.h"

CString g_szLastSavePath;
CString g_szCurrentDir;
CString g_szLastOpenPath;
CString g_szSaveMeasPath;
extern CString MyClass;
extern CString TransparentWndClass;
extern CString SaveBitsClass;
extern COLORREF clrYellow;
extern HBRUSH brYellowBrush;
extern CLIPFORMAT g_DropCurve, g_DropGraph;
CString APP_EXE = "LongGas.exe";
CString APP_NAME = "LongGas";
CString g_szDBError = "DB error";
CString strMainClass = "LongGasClass";
CMySQLDatabase g_db;
bool g_dbAvailable;
VEGA_PREFERENCES g_pref;
GRAPH_PREF* g_prefptr = &g_pref;

UINT & nCurrentDeviceId = g_pref.nDeviceId;
WORKERINFO wi;
WORKERINFO & wiCurrent = wi;
WORKER_AUTH_BASE* g_workerptr = 0;

extern TRANSLATION g_Translation;
extern TRANSLATION g_EngTranslation;

#include <CommCtrl.h>
#include <XMessageBox.h>
#include <CRC.h>
#include <UniversalDevice.h>
#include "CLGDelayedCommandDlg.h"
#pragma comment(lib,"comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


// CVegaConnectApp

BEGIN_MESSAGE_MAP(CLongGasApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CVegaConnectApp construction

CLongGasApp::CLongGasApp()
{
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

CLongGasApp theApp;


// CVegaConnectApp initialization
extern CRC_32 g_crc;
#pragma comment(lib, "ws2_32.lib")

BOOL CLongGasApp::InitInstance()
{
	if (!CMainAppTemplate::InitInstance())return 0;
	/*TERMOCAMERA_PROC t;
	t.nCameraType = CAMERA_BINDER;
	BYTE bt[] = { 0x01,0x10,0x15,0xff,0xff,0xff,0x81,0 };
	CString str = S_O::MakeString(bt, 8);
	t.CheckChecksum(str);*/
	CreateDirectory(g_szCurrentDir + "\\save", 0);
	CWnd* pWndPrev=0;
	if ((pWndPrev = CWnd::FindWindow(strMainClass, NULL)) != NULL)
	{
		Sleep(2000);
		if ((pWndPrev = CWnd::FindWindow(strMainClass, NULL)) != NULL)
		{
			DWORD res;
			bool b = SendMessageTimeout(pWndPrev->m_hWnd, WM_MOUSEMOVE, 0, 0, SMTO_ABORTIFHUNG, 10000, &res);
			DWORD pId = 0;
			GetWindowThreadProcessId(pWndPrev->GetSafeHwnd(), &pId);
			if (b)
			{
				HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION, 0, pId);
				if (h)
				{
					HWND w = CEnumProcess::GetProcessWindow(h, APP_NAME);
					if (w)D_O::RestoreWindow(CWnd::FromHandle(w));
					CloseHandle(h);
				}
				return FALSE;
			}
			else
			{
				if(AfxMessageBox(S_O::LoadString(IDS_APPNOTRESPONDING), MB_YESNO) == IDNO)return 0;
				if (pId)
				{
					HANDLE h = OpenProcess(PROCESS_TERMINATE, 0, pId);
					if (h)
					{
						TerminateProcess(h, 0);
						if (CWnd::FindWindow(strMainClass, NULL) != NULL)return 0;
					}
					else
					{
						AfxMessageBox(IDS_ERROR);
						return 0;
					}
				}
				else return 0;
			}
		}
	}

	wndRunning.Create(strMainClass, "test", 0, CRect(0, 10, 0, 10), CWnd::GetDesktopWindow(), 0);
	setlocale(LC_ALL, "");
	SetThreadUILanguage(MAKELANGID(LANG_RUSSIAN, SUBLANG_NEUTRAL));
	SetThreadLocale(MAKELCID(MAKELANGID(LANG_RUSSIAN, SUBLANG_NEUTRAL),SORT_DEFAULT));
//	g_Translation.LoadTranslation(0);

	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
	AfxInitRichEdit2();
	CGridCtrl::RegisterClass();
#ifdef _DEBUG
	//CRCTranslateDlg rc;
	//rc.DoModal();
#endif
	CLongGasMainDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();


	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

PREFERENCES* CLongGasApp::GetPreferences()
{
	return &g_pref;
}

int CLongGasApp::ExitInstance()
{
	return CWinApp::ExitInstance();
}

typedef LANGID(WINAPI* PFNGETUSERDEFAULTUILANGUAGE)();

void CLongGasApp::FillAvailableLanguages()
{
	CString str;
	g_pref.avLang.RemoveAll();
	g_pref.avLang.Add(MAKELANGID(LANG_RUSSIAN, SUBLANG_NEUTRAL));
	LANGID	id[] = {
		0x0436,0x041c,0x0401,0x0801,0x0c01,0x1001,0x1401,0x1801,0x1c01,0x2001,0x2401,0x2801,
		0x2c01,0x3001,0x3401,0x3801,0x3c01,0x4001,0x042b,0x042c,0x082c,0x042d,0x0423,0x0445,
		0x141a,0x0402,0x0455,0x0403,0x0404,0x0804,0x0c04,0x1004,0x1404,0x041a,0x101a,0x0405,
		0x0406,0x0465,0x0413,0x0813,0x0409,0x0809,0x0c09,0x1009,0x1409,0x1809,0x1c09,0x2009,
		0x2409,0x2809,0x2c09,0x3009,0x3409,0x0425,0x0438,0x0429,0x040b,0x040c,0x080c,0x0c0c,
		0x100c,0x140c,0x180c,0x0456,0x0437,0x0407,0x0807,0x0c07,0x1007,0x1407,0x0408,0x0447,
		0x040d,0x0439,0x040e,0x040f,0x0421,0x0434,0x0435,0x0410,0x0810,0x0411,0x044b,0x0457,
		0x0412,0x0812,0x0440,0x0426,0x0427,0x0827,0x042f,0x043e,0x083e,0x044c,0x0481,0x043a,
		0x044e,0x0450,0x0414,0x0814,0x0415,0x0416,0x0816,0x0446,0x046b,0x086b,0x0c6b,0x0418,
		0x0419,0x044f,0x043b,0x083b,0x0c3b,0x103b,0x143b,0x183b,0x1c3b,0x203b,0x243b,0x0c1a,
		0x1c1a,0x081a,0x181a,0x046c,0x0432,0x041b,0x0424,0x040a,0x080a,0x0c0a,0x100a,0x140a,
		0x180a,0x1c0a,0x200a,0x240a,0x280a,0x2c0a,0x300a,0x340a,0x380a,0x3c0a,0x400a,0x440a,
		0x480a,0x4c0a,0x500a,0x0430,0x0441,0x041d,0x081d,0x045a,0x0449,0x0444,0x044a,0x041e,
		0x041f,0x0422,0x0420,0x0820,0x0443,0x0843,0x042a,0x0452 };
	for (int i = 0; i < sizeof(id) / sizeof(id[0]); i++)
	{
		CFile f;
		if (f.Open(g_Translation.GetTranslationPath(id[i]), CFile::modeRead))
		{
			g_pref.avLang.Add(id[i]);
		}
	}
}

void CLongGasApp::LoadPreferences()
{
	CMainAppTemplate::LoadPreferences();
	FillAvailableLanguages();
	g_pref.nLang = theApp.GetProfileInt("System", "LanguageId", 0);
	if (g_pref.nLang == 0)
	{
		PFNGETUSERDEFAULTUILANGUAGE pfnGetUserDefaultUILanguage;
		HINSTANCE hKernel32= ::GetModuleHandle(_T("kernel32.dll"));
		pfnGetUserDefaultUILanguage =(PFNGETUSERDEFAULTUILANGUAGE)::GetProcAddress(hKernel32,"GetUserDefaultUILanguage");
		if (pfnGetUserDefaultUILanguage != NULL)
		{
			LANGID langid = pfnGetUserDefaultUILanguage();
			for (int i = 0; i < g_pref.avLang.GetSize(); i++)
			{
				if (PRIMARYLANGID(langid) == PRIMARYLANGID(g_pref.avLang[i]))
				{
					g_pref.nLang = g_pref.avLang[i];
					break;
				}
			}
		}
	}
	if (g_pref.nLang == 0)g_pref.nLang = MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL);
	SetThreadUILanguage(MAKELANGID(LANG_RUSSIAN, SUBLANG_NEUTRAL));
	SetThreadLocale(MAKELCID(MAKELANGID(LANG_RUSSIAN, SUBLANG_NEUTRAL), SORT_DEFAULT));
	if (PRIMARYLANGID(g_pref.nLang) != LANG_RUSSIAN)
	{
		g_Translation.LoadTranslation(g_pref.nLang);
		if (PRIMARYLANGID(g_pref.nLang) != LANG_ENGLISH)g_EngTranslation.LoadTranslation(2057);
	}
	msgBox.bUseUserDefinedButtonCaptions = 1;
	strcpy_s(msgBox.UserDefinedButtonCaptions.szYes , S_O::LoadString(IDS_YES_));
	strcpy_s(msgBox.UserDefinedButtonCaptions.szCancel, S_O::LoadString(IDS_CANCEL_));
	strcpy_s(msgBox.UserDefinedButtonCaptions.szNo, S_O::LoadString(IDS_NO_));

	g_pref.nCameraSlaveId = GetProfileInt("DelayedCommandDlg", "SlaveId", 1);
	g_pref.b10ChanBKM = GetProfileInt("DelayedCommandDlg", "b10Chan", 0);
	g_pref.strCameraAddr=GetProfileString("DelayedCommandDlg", "CameraPort", "");
	g_pref.strGasAddr=GetProfileString("DelayedCommandDlg", "GasPort", "");
	g_pref.nCameraType=GetProfileInt("DelayedCommandDlg", "CameraType", CAMERA_BINDER);
	g_pref.dCameraAddTime = GetProfileDouble("DelayedCommandDlg", "CameraAddTime", 30);
	g_pref.dCameraMaxTime = GetProfileDouble("DelayedCommandDlg", "CameraMaxTime", 120);
	g_pref.dCameraInterval = GetProfileDouble("DelayedCommandDlg", "CameraInterval", 0.3);
	g_pref.dRoomTemp= GetProfileDouble("DelayedCommandDlg", "RoomTemp", 20);

}

BOOL CLongGasApp::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message >= WM_MOUSEFIRST &&
		pMsg->message <= WM_MOUSELAST) || (pMsg->message>=WM_KEYFIRST && pMsg->message<=WM_KEYLAST))
	{
//		g_pref.tmLastUpdate = CTime::GetCurrentTime();
	}
	return CMainAppTemplate::PreTranslateMessage(pMsg);
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

bool CLongGasApp::ProcessSysCommand(UINT nID, LPARAM lParam)
{
	UINT nId = (nID & 0xFFF0);
	if (nId == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if (nId >= 32 && nId < 0xf000)
	{
		int n = (nId >> 5) - 1;
		WriteProfileInt("System", "AppMode", n);
		ReloadApplication();
	}
	else return 0;
	return 1;
}

void CLongGasApp::FillSystemMenu(CWnd* w)
{
	CMenu* pSysMenu = w->GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
//			pSysMenu->AppendMenu(MF_SEPARATOR);
//			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
			/*char* h[] = { "LongGas","VegaConnect"};
			for (int i = 0; i < 2; i++)
			{
				pSysMenu->AppendMenu(MF_STRING, (i + 1) << 5, h[i]);
				if (g_pref.nAppMode == i)pSysMenu->CheckMenuItem((i + 1) << 5, MF_CHECKED | MF_BYCOMMAND);
			}*/
		}
	}
}

bool KD_Notification::Add(bool bSendMail, bool bOnlyMail)//dummy function to ignore ssl libraries
{
	return 1;
}

int CLongGasApp::DoMessageBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt)
{
	CWnd* pParentWnd = CWnd::GetActiveWindow();

	// Check whether an active window was retrieved successfully.
	if (pParentWnd == NULL)
	{
		// Try to retrieve a handle to the last active popup.
		CWnd* main = GetMainWnd();
		if (main)pParentWnd = main->GetLastActivePopup();
	}
	return XMessageBox(pParentWnd->GetSafeHwnd(), lpszPrompt, APP_NAME, nType,&msgBox);
}
