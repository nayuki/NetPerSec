#pragma once

#include "SessionDlg.h"
#include "OptionsDlg.h"
#include "DisplayDlg.h"
#include "About.h"


// DlgPropSheet
class DlgPropSheet : public CPropertySheet {
	DECLARE_DYNAMIC(DlgPropSheet)
	
// Construction
public:
	DlgPropSheet(UINT nIDCaption, CWnd *pParentWnd=NULL, UINT iSelectPage=0);
	DlgPropSheet(LPCTSTR pszCaption, CWnd *pParentWnd=NULL, UINT iSelectPage=0);
	
// Attributes
public:
	CSessionDlg *m_pSessionDlg;
	CDisplayDlg *m_pDisplayDlg;
	COptionsDlg *m_pOptionsDlg;
	CAboutPage *m_pAboutDlg;
	
// Operations
public:
	void AddPropPages();
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgPropSheet)
public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pLResult);
protected:
	virtual void PostNcDestroy();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL
	
// Implementation
public:
	virtual ~DlgPropSheet();
	
// Generated message map functions
protected:
	//{{AFX_MSG(DlgPropSheet)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
