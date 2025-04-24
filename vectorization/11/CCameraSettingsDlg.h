#pragma once


// CCameraSettingsDlg dialog

class CCameraSettingsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCameraSettingsDlg)

public:
	CCameraSettingsDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CCameraSettingsDlg();
	CComboBox m_CameraType;
	CEdit m_CameraPort;
	CEdit m_GasPort;
	int m_nSlaveId;
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CAMERASETTINGS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	BOOL m_b10Chan;
	double m_CameraAddTime;
	double m_CameraMaxTime;
	double m_Interval;
	double m_RoomTemp;
};
