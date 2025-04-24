#pragma once


// CGridInfoDlg dialog
class CNFC_Device;

enum GRIDINFO_MODE
{
	MIPEX
};

class CGridInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CGridInfoDlg)

public:
	CGridInfoDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CGridInfoDlg();
	CNFC_Device* nfc;
	GRIDINFO_MODE mode;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GRIDINFO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
