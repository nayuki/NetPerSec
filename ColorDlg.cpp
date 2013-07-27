/* 
 * Implements the color selection dialog.
 */

#include "StdAfx.h"
#include "resource.h"
#include "ColorDlg.h"
#include "Globals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CColorCube
CColorCube::CColorCube() {
	dlg.m_hParent = this;
	m_crCurrentColor = RGB(255,255,255);
}


CColorCube::~CColorCube() {}


BEGIN_MESSAGE_MAP(CColorCube, CButton)
	//{{AFX_MSG_MAP(CColorCube)
	ON_CONTROL_REFLECT_EX(BN_CLICKED, OnClick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// CColorCube message handlers
void CColorCube::DrawItem(LPDRAWITEMSTRUCT lpd) {
	CDC dc;
	dc.Attach(lpd->hDC);
	
	int top    = lpd->rcItem.top;
	int left   = lpd->rcItem.left;
	int bottom = lpd->rcItem.bottom;
	int right  = lpd->rcItem.right;
	
	CBrush br_CurrentColor;
	CBrush br_BackColor;
	CPen graypen;
	br_CurrentColor.CreateSolidBrush(m_crCurrentColor);
	br_BackColor.CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	graypen.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
	
	CBrush *oldbrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
	CPen *oldpen = (CPen*)dc.SelectStockObject(NULL_PEN);
	
	dc.SelectObject(br_BackColor);
	dc.Rectangle(&lpd->rcItem);
	
	dc.SelectStockObject(BLACK_PEN);
	dc.MoveTo(left, bottom-1);
	dc.LineTo(right-1, bottom-1);
	dc.LineTo(right-1, top);
	
	dc.SelectObject(&graypen);
	dc.MoveTo(left+1, bottom-2);
	dc.LineTo(right-2, bottom-2);
	dc.LineTo(right-2, top+1);
	
	dc.SelectStockObject(WHITE_PEN);
	dc.LineTo(left+1, top+1);
	dc.LineTo(left+1, bottom-2);
	dc.MoveTo(right-10, top+4);
	dc.LineTo(right-10, bottom-4);
	
	dc.SelectObject(&graypen);
	dc.MoveTo(right-11, top+4);
	dc.LineTo(right-11, bottom-4);
	
	dc.SelectStockObject(BLACK_PEN);
	dc.MoveTo(right-4, (bottom/2)-1);
	dc.LineTo(right-9, (bottom/2)-1);
	
	dc.MoveTo(right-5, (bottom/2));
	dc.LineTo(right-8, (bottom/2));
	
	dc.SetPixel(right-6, (bottom/2)+1, RGB(0,0,0));
	
	dc.SelectObject(&br_CurrentColor);
	dc.Rectangle(left+5, top+4, right-15, bottom-4);
	
	if ((lpd->itemState & ODS_FOCUS) != 0) {
		for (int i = left+3; i < right-4; i += 2) {
			dc.SetPixel(i, top+3, RGB(0,0,0));
			dc.SetPixel(i, bottom-4, RGB(0,0,0));
		}
		for (int i = top+3; i < bottom-4; i += 2) {
			dc.SetPixel(left+3, i, RGB(0,0,0));
			dc.SetPixel(right-4, i, RGB(0,0,0));
		}
	}
	
	dc.SelectObject(oldpen);
	dc.SelectObject(oldbrush);
	dc.Detach();
}


BOOL CColorCube::OnClick() {
	if (dlg.DoModal() == IDOK) {
		m_crCurrentColor = IconColors[dlg.m_ColorIndex];
		InvalidateRect(NULL);
	}
	return FALSE;
}


// CColorCubeDlg dialog

CColorCubeDlg::CColorCubeDlg(CWnd *pParent /*=NULL*/)
	: CDialog(CColorCubeDlg::IDD, pParent) {
	//{{AFX_DATA_INIT(CColorCubeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CColorCubeDlg::DoDataExchange(CDataExchange *pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CColorCubeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CColorCubeDlg, CDialog)
	//{{AFX_MSG_MAP(CColorCubeDlg)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_DRAWITEM()
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(IDC_COLOR1,IDC_COLOR16,OnColorSelect)
END_MESSAGE_MAP()


/* CColorCubeDlg message handlers */

BOOL CColorCubeDlg::OnInitDialog() {
	CDialog::OnInitDialog();
	RECT rc, r2;
	m_hParent->GetWindowRect(&rc);
	SetWindowPos(NULL, rc.left, rc.bottom, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
	GetWindowRect(&r2);
	
	if (r2.bottom > GetSystemMetrics(SM_CYSCREEN))
		r2.top = rc.top - (r2.bottom - r2.top);
	if (r2.right > GetSystemMetrics(SM_CXSCREEN))
		r2.left = GetSystemMetrics(SM_CXSCREEN) - (r2.right - r2.left);
	
	SetWindowPos(NULL, r2.left, r2.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
	SetCapture();
	return TRUE;
}


void CColorCubeDlg::EndDialog(int nResult) {
	ReleaseCapture();
	CDialog::EndDialog(nResult);
}


void CColorCubeDlg::OnLButtonDown(UINT nFlags, CPoint pt) {
	POINT p;
	p.x = pt.x;
	p.y = pt.y;
	ClientToScreen(&p);
	
	RECT rc;
	GetWindowRect(&rc);
	
	if (!PtInRect(&rc, p)) {
		EndDialog(IDCANCEL);
	} else {
		CWnd *pWnd = ChildWindowFromPoint(pt);
		if (pWnd != NULL && pWnd != this)
			pWnd->SendMessage(WM_LBUTTONDOWN, 0, 0);
	}
	
	CDialog::OnLButtonDown(nFlags, pt);
}


void CColorCubeDlg::OnDrawItem(int nID, LPDRAWITEMSTRUCT lpd) {
	CDC dc;
	CPen nullpen;
	CBrush brush;
	CPen *oldpen;
	CBrush *oldbrush;
	
	nullpen.CreateStockObject(NULL_PEN);
	brush.CreateSolidBrush(IconColors[nID-IDC_COLOR1]);
	
	dc.Attach(lpd->hDC);
	oldpen = dc.SelectObject(&nullpen);
	oldbrush = dc.SelectObject(&brush);
	
	lpd->rcItem.right++;
	lpd->rcItem.bottom++;
	
	dc.Rectangle(&lpd->rcItem);
	dc.SelectObject(oldpen);
	dc.SelectObject(oldbrush);
	dc.Detach();
	
	CDialog::OnDrawItem(nID, lpd);
}


void CColorCubeDlg::OnColorSelect(UINT id) {
	m_ColorIndex = id - IDC_COLOR1;
	EndDialog(IDOK);
}


void CColorCubeDlg::OnLButtonUp(UINT nFlags, CPoint pt) {
	CWnd *pWnd = ChildWindowFromPoint(pt, CWP_ALL);
	if (pWnd && pWnd != this)
		pWnd->SendMessage(WM_LBUTTONDOWN, 0, 0);
	CDialog::OnLButtonUp(nFlags, pt);
}
