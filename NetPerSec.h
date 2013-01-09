#if !defined(AFX_NETPERSEC_H__6D695825_9A18_11D4_A181_004033572A05__INCLUDED_)
#define AFX_NETPERSEC_H__6D695825_9A18_11D4_A181_004033572A05__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "snmp.h"
#include "winproc.h"
#include "globals.h"
#include "Icons.h"

// CNetPerSecApp:
// See NetPerSec.cpp for the implementation of this class
class CNetPerSecApp : public CWinApp
{
public:
	CNetPerSecApp();
	~CNetPerSecApp();
	
	Cwinproc m_wnd;
	CIcons m_Icons;
	HANDLE m_hMutex; //single instance
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNetPerSecApp)
public:
	virtual BOOL InitInstance();
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
	//}}AFX_VIRTUAL
	
// Implementation
	
	//{{AFX_MSG(CNetPerSecApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CNetPerSecApp* pTheApp;



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NETPERSEC_H__6D695825_9A18_11D4_A181_004033572A05__INCLUDED_)
