#if !defined(AFX_COLORS_H__80593906_ADAF_11D4_A181_004033572A05__INCLUDED_)
#define AFX_COLORS_H__80593906_ADAF_11D4_A181_004033572A05__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

#include "ColorDlg.h"


// CDisplayDlg dialog
class CDisplayDlg : public CPropertyPage {
	DECLARE_DYNCREATE(CDisplayDlg)
	
// Construction
public:
	CDisplayDlg();
	~CDisplayDlg();
	
	BOOL GetColor(COLORREF *pColorRef);
	void ShowSampleIcon();
	
	COLORREF m_Restore_ColorSent,
	         m_Restore_ColorRecv,
	         m_Restore_ColorAve,
	         m_Restore_ColorBack,
	         m_Restore_ColorIconBack;
	
// Dialog Data
	//{{AFX_DATA(CDisplayDlg)
	enum { IDD = IDD_COLOR_DLG };
	CColorCube m_IconBtn;
	CColorCube m_SentBtn;
	CColorCube m_RecvBtn;
	//}}AFX_DATA
	
// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CDisplayDlg)
public:
	virtual BOOL OnSetActive();
	virtual void OnCancel();
protected:
	virtual void DoDataExchange(CDataExchange *pDX);  // DDX/DDV support
	//}}AFX_VIRTUAL
	
// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDisplayDlg)
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnColorAve();
	afx_msg void OnColorBack();
	afx_msg void OnColorRecv();
	afx_msg void OnColorSent();
	afx_msg void OnStartwithwindows();
	afx_msg void OnOntop();
	virtual BOOL OnInitDialog();
	afx_msg void OnDefaultColors();
	afx_msg void OnUndo();
	afx_msg void OnColorIconBack();
	afx_msg void OnIconBargraph();
	afx_msg void OnIconHistogram();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
