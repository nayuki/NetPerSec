#pragma once

#ifndef __AFXWIN_H__
#error "include 'StdAfx.h' before including this file for PCH"
#endif

#include "resource.h"  // Main symbols
#include "Snmp.h"
#include "winproc.h"
#include "Globals.h"
#include "Icons.h"


// CNetPerSecApp:
// See NetPerSec.cpp for the implementation of this class
class CNetPerSecApp : public CWinApp {
public:
	CNetPerSecApp();
	~CNetPerSecApp();
	
	Cwinproc m_wnd;
	CIcons m_Icons;
	HANDLE m_hMutex;  // Single instance
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNetPerSecApp)
public:
	virtual BOOL InitInstance();
	virtual void WinHelp(DWORD dwData, UINT nCmd=HELP_CONTEXT);
	//}}AFX_VIRTUAL
	
// Implementation
	//{{AFX_MSG(CNetPerSecApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		// DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


extern CNetPerSecApp theApp;


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
