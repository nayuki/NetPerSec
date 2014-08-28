#pragma once


// CAboutPage dialog
class CAboutPage : public CPropertyPage {
	DECLARE_DYNCREATE(CAboutPage)
	
// Construction
public:
	CAboutPage();
	~CAboutPage();
	
// Dialog Data
	//{{AFX_DATA(CAboutPage)
	enum { IDD = IDD_ABOUTBOX };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutPage)
protected:
	virtual void DoDataExchange(CDataExchange *pDX);  // DDX/DDV support
	//}}AFX_VIRTUAL
	
// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CAboutPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
