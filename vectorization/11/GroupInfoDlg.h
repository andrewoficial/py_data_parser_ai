#pragma once


// CGroupInfoDlg dialog

class CGroupInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CGroupInfoDlg)

public:
	CGroupInfoDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CGroupInfoDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GROUPINFO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CRichEditCtrl m_Rich;
};
