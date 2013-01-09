#if !defined(AFX_ABOUT_H__EE7BB821_9D35_11D4_A181_004033572A05__INCLUDED_)
#define AFX_ABOUT_H__EE7BB821_9D35_11D4_A181_004033572A05__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// About.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAboutPage dialog

class CAboutPage : public CPropertyPage
{

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
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
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

#endif // !defined(AFX_ABOUT_H__EE7BB821_9D35_11D4_A181_004033572A05__INCLUDED_)
