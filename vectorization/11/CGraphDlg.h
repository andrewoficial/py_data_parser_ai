#pragma once
#include "NFC_Device.h"


// CGraphDlg dialog
class CGraph;

class CGraphDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CGraphDlg)

public:
	CGraphDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CGraphDlg();
	CGraph* gr;
	void FillExpectedCurve(DELAYED_COMMAND_ARRAY* dca);
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GRAPH };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CRichEditCtrl m_Rich;
	afx_msg void OnBnClickedExport();
};
