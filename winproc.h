#if !defined(AFX_WINPROC_H__6564E3DE_EB5F_4094_9AEC_CF329AE35837__INCLUDED_)
#define AFX_WINPROC_H__6564E3DE_EB5F_4094_9AEC_CF329AE35837__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

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
	double m_dbTotalBytesRecv;
	double m_dbTotalBytesSent;
	double m_dbRecvWrap;
	double m_dbSentWrap;
	
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

#endif
