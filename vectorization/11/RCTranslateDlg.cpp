// RCTranslateDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "RCTranslateDlg.h"
#include "afxdialogex.h"
#include <DialogOperations.h>
extern VEGA_PREFERENCES g_pref;

// CRCTranslateDlg dialog

IMPLEMENT_DYNAMIC(CRCTranslateDlg, CDialogEx)

CRCTranslateDlg::CRCTranslateDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_RCTRANSLATE, pParent)
{

}

CRCTranslateDlg::~CRCTranslateDlg()
{
}

void CRCTranslateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_Lang);
}


BEGIN_MESSAGE_MAP(CRCTranslateDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CRCTranslateDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CRCTranslateDlg message handlers

extern CString  g_szCurrentDir;
void CRCTranslateDlg::OnBnClickedButton1()
{
	CString str, buf,buf1;
	str.Format("%s\\*.rc", CMainAppTemplate::GetLastSavePath());
	CFileDialog fd(1, "*.rc", str, OFN_EXPLORER | OFN_HIDEREADONLY, "Resource Files (*.rc)|*.rc||", NULL);
	if (fd.DoModal() == IDCANCEL)return;
	CMainAppTemplate::SetLastSavePath(fd.GetPathName());
	CFile f;
	if (!f.Open(fd.GetPathName(), CFile::modeRead))return;
	UINT nL = (UINT)f.GetLength();
	if (!nL)return;
	char* b = buf.GetBufferSetLength(nL);
	f.Read(b, nL);
	buf.ReleaseBuffer(nL);
	f.Close();
	UINT nLang = D_O::GetSelectedItemData(m_Lang);
	TRANSLATION t;
	if (nLang != -1)
	{
		t.LoadTranslation(nLang);
		TCHAR	szLangCode[128];
		::GetLocaleInfo(nLang, LOCALE_SENGLANGUAGE, szLangCode, 128);
		str.Format("%s\\totranslate - %s.xls", g_szCurrentDir, szLangCode);
	}
	else str.Format("%s\\totranslate.xls", g_szCurrentDir);
	if (!f.Open(str, CFile::modeCreate | CFile::modeReadWrite))return;
	str = buf;
	int nF = str.Find("\""),nc=0;
	CStringArray arr;
	while (nF != -1)
	{
		int nFE = str.Find("\"", nF + 1);
		if (nFE == -1)break;
		buf = str.Mid(nF, nFE - nF);
		bool bF = 0;
		for (int i = 0; i < buf.GetLength(); i++)
		{
			BYTE c = buf.GetAt(i);
			if (c >= 0xc0 && c <= 0xff)
			{
				bF = 1;
				break;
			}
		}
		if (bF)
		{
			S_O::Trim(buf);
			if (nLang != -1)
			{
				if (!t.map.Lookup(buf, buf1))S_O::AddToArray(buf, arr);
			}
			else S_O::AddToArray(buf, arr);
		}
		nF = nFE + 1;
	}
	for (int i = 0; i < arr.GetSize(); i++)
	{
		str.Format("%s\t\r\n", arr[i]);
		f.Write(str, str.GetLength());
	}
	str.Format("%d found", arr.GetSize());
	AfxMessageBox(str);

}


BOOL CRCTranslateDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	TCHAR	szLangCode[128];
	D_O::AddStringWithId(&m_Lang, "Новый", -1, 1);
	for (int i = 0; i < g_pref.avLang.GetSize(); i++)
	{
		if (PRIMARYLANGID(g_pref.avLang[i]) == LANG_RUSSIAN)continue;
		::GetLocaleInfo(g_pref.avLang[i], LOCALE_SENGLANGUAGE, szLangCode, 128);
		D_O::AddStringWithId(&m_Lang, szLangCode, g_pref.avLang[i], g_pref.avLang[i] == g_pref.nLang);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}
