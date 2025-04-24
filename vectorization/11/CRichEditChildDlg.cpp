// CRichEditChildDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "CRichEditChildDlg.h"
#include "afxdialogex.h"


// CRichEditChildDlg dialog

IMPLEMENT_DYNAMIC(CRichEditChildDlg, CDialogEx)

CRichEditChildDlg::CRichEditChildDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_RICHEDIT, pParent)
{

}

CRichEditChildDlg::~CRichEditChildDlg()
{
}

void CRichEditChildDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT21, m_Rich);
}


BEGIN_MESSAGE_MAP(CRichEditChildDlg, CDialogEx)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CRichEditChildDlg message handlers


BOOL CRichEditChildDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	layout.AttachBasic(this);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CRichEditChildDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	layout.OnSize(nType, cx, cy);
}
