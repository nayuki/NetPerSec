#ifndef __GRAPHS_H__
#define __GRAPHS_H__


// CGraphs window
class CGraphs : public CWnd {
// Construction
public:
	CGraphs();
	
// Attributes
public:
	void SetPos(UINT nPos, COLORREF crColor, int nLineIndex=-1);
	void SetGraphRange(UINT nRange);
	void RedrawGraph();
	void DrawGraph(UINT nPos, COLORREF crColor, int nLineIndex=-1);
	void DrawGrid(CDC *pDC, CRect *pRect);
	void SetSize(int nPoints);
	void ShiftLeft();
	void SetStyle(BOOL nStyle);
	void ClearGraph();
	int GetTotalElements();
	
// Operations
public:
	void StepIt();
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGraphs)
	public:
	virtual BOOL Create(DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID, CCreateContext *pContext=NULL);
	//}}AFX_VIRTUAL
	
// Implementation
public:
	virtual ~CGraphs();
	
// Generated message map functions
protected:
	//{{AFX_MSG(CGraphs)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	UINT m_nGraphRange;
	int m_nGraphScale;
	CDC m_MemDC;
	CBitmap m_Bitmap;
	BOOL m_bBarGraph;
	CUIntArray m_GraphArray;
};

#endif
