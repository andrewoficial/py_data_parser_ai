#pragma once


// CRCTranslateDlg dialog

class CRCTranslateDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRCTranslateDlg)

public:
	CRCTranslateDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CRCTranslateDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RCTRANSLATE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	CComboBox m_Lang;
	virtual BOOL OnInitDialog();
};
