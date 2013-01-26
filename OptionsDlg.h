#ifndef AFX_OPTIONSDLG_H
#define AFX_OPTIONSDLG_H

#if _MSC_VER > 1000
#pragma once
#endif


// COptionsDlg dialog
class COptionsDlg : public CPropertyPage {
	DECLARE_DYNCREATE(COptionsDlg)
	
// Construction
public:
	COptionsDlg();
	~COptionsDlg();
	void UpdateDlg();
	void UpdateAveragingWindow();
	
// Dialog Data
	//{{AFX_DATA(COptionsDlg)
	enum { IDD = IDD_OPTIONS_DLG };
	CComboBox m_Interfaces;
	//}}AFX_DATA
	
// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsDlg)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);  // DDX/DDV support
	//}}AFX_VIRTUAL
	
// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
	afx_msg void OnUseSnmp();
	afx_msg void OnUseDun();
	afx_msg void OnMonitorAdapter();
	afx_msg void OnSelchangeInterfaces();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
