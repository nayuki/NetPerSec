/* 
 * Implements the display property page.
 */

#include "StdAfx.h"
#include "NetPerSec.h"
#include "DisplayDlg.h"
#include "Globals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CDisplayDlg property page

IMPLEMENT_DYNCREATE(CDisplayDlg, CPropertyPage)

CDisplayDlg::CDisplayDlg() : CPropertyPage(CDisplayDlg::IDD) {
	//{{AFX_DATA_INIT(CDisplayDlg)
	//}}AFX_DATA_INIT
}

CDisplayDlg::~CDisplayDlg() {}

void CDisplayDlg::DoDataExchange(CDataExchange *pDX) {
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDisplayDlg)
	DDX_Control(pDX, IDC_COLOR_ICON_BACK, m_IconBtn);
	DDX_Control(pDX, IDC_COLOR_SENT, m_SentBtn);
	DDX_Control(pDX, IDC_COLOR_RECV, m_RecvBtn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDisplayDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CDisplayDlg)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_COLOR_AVE, OnColorAve)
	ON_BN_CLICKED(IDC_COLOR_BACK, OnColorBack)
	ON_BN_CLICKED(IDC_COLOR_RECV, OnColorRecv)
	ON_BN_CLICKED(IDC_COLOR_SENT, OnColorSent)
	ON_BN_CLICKED(IDC_STARTWITHWINDOWS, OnStartwithwindows)
	ON_BN_CLICKED(IDC_ONTOP, OnOntop)
	ON_BN_CLICKED(IDC_DEFAULT_COLORS, OnDefaultColors)
	ON_BN_CLICKED(IDC_UNDO, OnUndo)
	ON_BN_CLICKED(IDC_COLOR_ICON_BACK, OnColorIconBack)
	ON_BN_CLICKED(IDC_ICON_BARGRAPH, OnIconBargraph)
	ON_BN_CLICKED(IDC_ICON_HISTOGRAM, OnIconHistogram)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/* CDisplayDlg message handlers */

BOOL CDisplayDlg::GetColor(COLORREF *pColorRef) {
	CColorDialog dlg;
	dlg.m_cc.Flags |= CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
	dlg.m_cc.rgbResult = *pColorRef;
	if (dlg.DoModal() == IDOK) {
		*pColorRef = dlg.m_cc.rgbResult;
		return TRUE;
	} else
		return FALSE;
}


void CDisplayDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) {
	HBRUSH hbr;
	COLORREF cr;
	
	int iStyle;
	if ((lpDrawItemStruct->itemAction & ODA_SELECT) != 0 && (lpDrawItemStruct->itemState & ODS_SELECTED) != 0)
		iStyle = EDGE_SUNKEN;
	else
		iStyle = EDGE_RAISED;
	
	switch (nIDCtl) {
		case IDC_COLOR_BACK:
			cr = g_ColorBack;
			break;
		
		case IDC_COLOR_AVE:
			cr = g_ColorAve;
			break;
		
		default:
			CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
			return;
		
		case IDC_COLOR_SENT:
			m_SentBtn.DrawItem(lpDrawItemStruct);
			return;
			
		case IDC_COLOR_RECV:
			m_RecvBtn.DrawItem(lpDrawItemStruct);
			return;
			
		case IDC_COLOR_ICON_BACK:
			m_IconBtn.DrawItem(lpDrawItemStruct);
			return;
	}
	
	hbr = CreateSolidBrush(cr);
	FillRect(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, hbr);
	DrawEdge(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, iStyle, BF_RECT);
	DeleteObject(hbr);
}

void CDisplayDlg::OnColorAve() {
	if (GetColor(&g_ColorAve)) {
		GetDlgItem(IDC_COLOR_AVE)->InvalidateRect(NULL);
		GetDlgItem(IDC_UNDO)->EnableWindow(TRUE);
	}
}

void CDisplayDlg::OnColorBack() {
	if (GetColor(&g_ColorBack)) {
		GetDlgItem(IDC_COLOR_BACK)->InvalidateRect(NULL);
		GetDlgItem(IDC_UNDO)->EnableWindow(TRUE);
	}
}


BOOL CDisplayDlg::OnInitDialog() {
	CPropertyPage::OnInitDialog();
	
	m_Restore_ColorSent = g_ColorSent;
	m_Restore_ColorRecv = g_ColorRecv;
	m_Restore_ColorAve  = g_ColorAve;
	m_Restore_ColorBack = g_ColorBack;
	m_Restore_ColorIconBack = g_ColorIconBack;
	
	CheckDlgButton(IDC_ICON_HISTOGRAM, g_IconStyle == ICON_HISTOGRAM);
	CheckDlgButton(IDC_ICON_BARGRAPH , g_IconStyle == ICON_BARGRAPH );
	CheckDlgButton(IDC_ONTOP, g_bOnTop);
	CheckDlgButton(IDC_STARTWITHWINDOWS, g_bStartWithWindows);
	return TRUE;  // Return TRUE unless you set the focus to a control
}

BOOL CDisplayDlg::OnSetActive() {
	m_RecvBtn.m_crCurrentColor = g_ColorRecv;
	m_SentBtn.m_crCurrentColor = g_ColorSent;
	m_IconBtn.m_crCurrentColor = g_ColorIconBack;
	ShowSampleIcon();
	return CPropertyPage::OnSetActive();
}

void CDisplayDlg::OnColorRecv() {
	g_ColorRecv = m_RecvBtn.m_crCurrentColor;
	ShowSampleIcon();
	GetDlgItem(IDC_UNDO)->EnableWindow(TRUE);
}

void CDisplayDlg::OnColorIconBack() {
	g_ColorIconBack = m_IconBtn.m_crCurrentColor;
	ShowSampleIcon();
	GetDlgItem(IDC_UNDO)->EnableWindow(TRUE);
}


void CDisplayDlg::OnColorSent() {
	g_ColorSent = m_SentBtn.m_crCurrentColor;
	ShowSampleIcon();
	GetDlgItem(IDC_UNDO)->EnableWindow(TRUE);
}

void CDisplayDlg::ShowSampleIcon() {
	STATS_STRUCT r[MAX_SAMPLES];
	STATS_STRUCT s[MAX_SAMPLES];
	
	// Fill the stats array with artificial data for the sample icon in the dialog
	if (g_IconStyle == ICON_BARGRAPH) {
		for (int i = 0; i <= 16; i++) {
			r[i].Bps = 70;
			s[i].Bps = 35;
		}
		r[1].Bps = 100;
		s[1].Bps = 100;
	} else if (g_IconStyle == ICON_HISTOGRAM) {
		for (int i = 0; i < 16; i++) {
			r[i].Bps = (16 - 1 - i) * 4;
			s[i].Bps = (16 - 1 - i) * 4;
		}
	} else
		ASSERT(false);
	
	HICON hIcon = theApp.m_Icons.GetIcon(s, r, g_IconStyle);
	HICON hOld = (HICON)GetDlgItem(IDC_SAMPLE_ICON)->SendMessage(STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
	if (hOld)
		DestroyIcon(hOld);
}


void CDisplayDlg::OnStartwithwindows() {
	g_bStartWithWindows = IsDlgButtonChecked(IDC_STARTWITHWINDOWS);
}

void CDisplayDlg::OnOntop() {
	g_bOnTop = IsDlgButtonChecked(IDC_ONTOP);
	GetParent()->SetWindowPos(g_bOnTop ? &wndTopMost : &wndNoTopMost, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
}

void CDisplayDlg::OnDefaultColors() {
	g_ColorSent = COLOR_ICON_SENT;
	g_ColorRecv = COLOR_ICON_RECV;
	g_ColorAve  = COLOR_AVERAGE;
	g_ColorBack = g_ColorIconBack = COLOR_ICON_BACK;
	
	m_RecvBtn.m_crCurrentColor = g_ColorRecv;
	m_SentBtn.m_crCurrentColor = g_ColorSent;
	m_IconBtn.m_crCurrentColor = g_ColorIconBack;
	
	ShowSampleIcon();
	InvalidateRect(NULL);
	GetDlgItem(IDC_UNDO)->EnableWindow(TRUE);
}

void CDisplayDlg::OnUndo() {
	g_ColorSent = m_Restore_ColorSent;
	g_ColorRecv = m_Restore_ColorRecv;
	g_ColorAve  = m_Restore_ColorAve;
	g_ColorBack = m_Restore_ColorBack;
	g_ColorIconBack = m_Restore_ColorIconBack;
	
	m_RecvBtn.m_crCurrentColor = g_ColorRecv;
	m_SentBtn.m_crCurrentColor = g_ColorSent;
	m_IconBtn.m_crCurrentColor = g_ColorIconBack;
	
	ShowSampleIcon();
	InvalidateRect(NULL);
	GetDlgItem(IDC_UNDO)->EnableWindow(FALSE);
}

void CDisplayDlg::OnCancel() {
	OnUndo();
	CPropertyPage::OnCancel();
}


void CDisplayDlg::OnIconBargraph() {
	OnIconHistogram();
}

void CDisplayDlg::OnIconHistogram() {
	if (IsDlgButtonChecked(IDC_ICON_BARGRAPH))
		g_IconStyle = ICON_BARGRAPH;
	else if (IsDlgButtonChecked(IDC_ICON_HISTOGRAM))
		g_IconStyle = ICON_HISTOGRAM;
	else
		ASSERT(false);
	
	ShowSampleIcon();
}
