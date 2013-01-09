#ifndef _COLORCUBE_H
#define _COLORCUBE_H


class CColorCubeDlg : public CDialog {
// Construction
public:
	CColorCubeDlg(CWnd* pParent = NULL);   // standard constructor
	
	CButton *m_hParent;
	int m_ColorIndex;
	
// Dialog Data
	//{{AFX_DATA(CColorCubeDlg)
		enum { IDD = IDD_COLORCUBE_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorCubeDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	
// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CColorCubeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDrawItem(int nID, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	
	void OnColorSelect(UINT);
	void EndDialog(int);
	
	DECLARE_MESSAGE_MAP()
};



class CColorCube : public CButton {
public:
	CColorCube();
	COLORREF m_crCurrentColor;
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorCube)
	//}}AFX_VIRTUAL
	
// Implementation
public:
	virtual ~CColorCube();
	
	// Generated message map functions
	//protected:
	//{{AFX_MSG(CColorCube)
		afx_msg BOOL OnClick() ;
	//}}AFX_MSG
	
	void DrawItem(LPDRAWITEMSTRUCT);
	DECLARE_MESSAGE_MAP()
	
private:
	CColorCubeDlg dlg;
};

#endif
