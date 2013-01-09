#if !defined(AFX_SESSIONDLG_H__7FE7EC26_9C41_11D4_A181_004033572A05__INCLUDED_)
#define AFX_SESSIONDLG_H__7FE7EC26_9C41_11D4_A181_004033572A05__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SessionDlg.h : header file
//
#include "Graphs.h"

typedef enum {
	RECV_DATA = 0x01,
	SENT_DATA = 0x02,
} UPDATE_MODE;


/////////////////////////////////////////////////////////////////////////////
// CSessionDlg dialog

class CSessionDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CSessionDlg)
	
// Construction
public:
	CSessionDlg();
	~CSessionDlg();
	
	void UpdateDlg( );
	
	void DisplayNumber( int nID, DWORD dwNumber );
	void SetGraphRangeSent( );
	void SetGraphRangeRecv( );
	void DrawGraph( int nIndex, UPDATE_MODE update);
	void SetOptions( );
	void UpdateGraphTextSent( DWORD );
	void UpdateGraphTextRecv( DWORD );
	void UpdateGraph( );
	void SetGraphStyle( );
	BOOL CalcAutoScale( UINT* pAutoScale,  STATS_STRUCT* pStats, UPDATE_MODE update );
	void UpdateScrollPos( WORD wControl, DWORD dwValue );
	DWORD CalcMax(STATS_STRUCT* pStats, int start );
	
	
	CToolTipCtrl m_ToolTip;
	CGraphs m_RecvGraph;
	CGraphs m_SentGraph;
	UINT m_AutoScale_Recv;
	UINT m_AutoScale_Sent;
	CBrush* m_pbrBackground;
	
// Dialog Data
	//{{AFX_DATA(CSessionDlg)
	enum { IDD = IDD_SESSION_DLG };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA
	
	
// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CSessionDlg)
public:
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	
// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CSessionDlg)
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnAveRecvOption();
	afx_msg void OnAveSentOption();
	afx_msg void OnCurrentRecvOption();
	afx_msg void OnCurrentSentOption();
	afx_msg void OnMaxRecvOption();
	afx_msg void OnMaxSentOption();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBargraph();
	afx_msg void OnLinegraph();
	afx_msg void OnResetData();
	afx_msg void OnAutoscale();
	afx_msg void OnBps();
	afx_msg void OnBytes();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SESSIONDLG_H__7FE7EC26_9C41_11D4_A181_004033572A05__INCLUDED_)
