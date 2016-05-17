#pragma once

#include "Globals.h"
#include "DlgPropSheet.h"
#include "About.h"


// Cwinproc window

class Cwinproc : public CWnd {
// Construction
public:
	Cwinproc();
	void StartUp();
	LRESULT OnTaskbarNotify(WPARAM wParam, LPARAM lParam);
	void UpdateTrayIcon(HICON hIcon);
	void ShowPropertiesDlg();
	
// Attributes
public:
	CSnmp snmp;
	DlgPropSheet *m_pPropertiesDlg;
	
	DWORD m_dwStartTime;
	bool m_PrevBytesOk;
	DWORD m_PrevBytesRecv;
	DWORD m_PrevBytesSent;
	double m_TotalBytesRecv;
	double m_TotalBytesSent;
	
	NOTIFYICONDATA m_SystemTray;
	STATS_STRUCT RecvStats[MAX_SAMPLES];
	STATS_STRUCT SentStats[MAX_SAMPLES];
	
	void ResetData();
	void CalcAverages(double dwTotal, DWORD dwTime, DWORD dwBPS, STATS_STRUCT *pStats);
	static DWORD GetRecentMaximum(STATS_STRUCT *stats, int num, int type);
	
// Operations
public:
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Cwinproc)
public:
	virtual void WinHelp(DWORD dwData, UINT nCmd=HELP_CONTEXT);
	//}}AFX_VIRTUAL
	
// Implementation
public:
	virtual ~Cwinproc();
	
	// Generated message map functions
protected:
	//{{AFX_MSG(Cwinproc)
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
