// GroupInfoDlg.cpp : implementation file
//

#include "pch.h"
#include "LongGas.h"
#include "GroupInfoDlg.h"
#include "afxdialogex.h"


// CGroupInfoDlg dialog

IMPLEMENT_DYNAMIC(CGroupInfoDlg, CDialogEx)

CGroupInfoDlg::CGroupInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GROUPINFO, pParent)
{

}

CGroupInfoDlg::~CGroupInfoDlg()
{
}

void CGroupInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT21, m_Rich);
}


BEGIN_MESSAGE_MAP(CGroupInfoDlg, CDialogEx)
END_MESSAGE_MAP()


// CGroupInfoDlg message handlers
