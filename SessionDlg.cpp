/* 
 * Implements the graphs property page and displays
 * statistics about the current session.
 */

#include "StdAfx.h"
#include "NetPerSec.h"
#include "SessionDlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// Which graph to display - bps and/or average
enum {
	OPTION_BPS = 0x01,
	OPTION_AVE = 0x02,
} OPTIONS;


// This is for the line graph - the graphing functions need
// to save the previous graph endpoints for the line
enum {
	LINEGRAPH_BPS = 0,
	LINEGRAPH_AVE,
	LINE_GRAPH_SIZE,
} LINE_GRAPH;

static UINT bpsArray[] = {  // In bytes
	1000 / 8,  // Kilobit
	3000 / 8,
	10000 / 8,
	30000 / 8,
	100000 / 8,
	300000 / 8,
	1000000 / 8,  // Megabit
	3000000 / 8,
	10000000 / 8,
	30000000 / 8,
	100000000 / 8,
	300000000 / 8,
	1000000000 / 8,  // Gigabit
};


// CSessionDlg property page

IMPLEMENT_DYNCREATE(CSessionDlg, CPropertyPage)

CSessionDlg::CSessionDlg() : CPropertyPage(CSessionDlg::IDD) {
	//{{AFX_DATA_INIT(CSessionDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pbrBackground = NULL;
}

CSessionDlg::~CSessionDlg() {
	if (m_pbrBackground != NULL) {
		delete m_pbrBackground;
		m_pbrBackground = NULL;
	}
}

void CSessionDlg::DoDataExchange(CDataExchange *pDX) {
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSessionDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSessionDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CSessionDlg)
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_AVE_RECV_OPTION, OnAveRecvOption)
	ON_BN_CLICKED(IDC_AVE_SENT_OPTION, OnAveSentOption)
	ON_BN_CLICKED(IDC_CURRENT_RECV_OPTION, OnCurrentRecvOption)
	ON_BN_CLICKED(IDC_CURRENT_SENT_OPTION, OnCurrentSentOption)
	ON_BN_CLICKED(IDC_MAX_RECV_OPTION, OnMaxRecvOption)
	ON_BN_CLICKED(IDC_MAX_SENT_OPTION, OnMaxSentOption)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BARGRAPH, OnBargraph)
	ON_BN_CLICKED(IDC_LINEGRAPH, OnLinegraph)
	ON_BN_CLICKED(IDC_RESET_DATA, OnResetData)
	ON_BN_CLICKED(IDC_AUTOSCALE_SENT, OnAutoscale)
	ON_BN_CLICKED(IDC_BPS, OnBps)
	ON_BN_CLICKED(IDC_BYTES, OnBytes)
	ON_BN_CLICKED(IDC_AUTOSCALE_RECV, OnAutoscale)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// CSessionDlg message handlers

BOOL CSessionDlg::PreTranslateMessage(MSG *pMsg) {
	// Show the color dialog if a graph is right clicked
	if (pMsg->message == WM_RBUTTONUP) {
		WORD wID = (WORD)GetWindowLong(pMsg->hwnd, GWL_ID);
		if (wID == IDC_RECV_GRAPH_WINDOW || wID == IDC_SENT_GRAPH_WINDOW) {
			theApp.m_wnd.m_pPropertiesDlg->SetActivePage(2);
			return TRUE;
		}
	}
	return CPropertyPage::PreTranslateMessage(pMsg);
}


BOOL CSessionDlg::OnSetActive() {
	if (m_pbrBackground != NULL)
		delete m_pbrBackground;
	m_pbrBackground = new CBrush(g_ColorBack);
	
	SetTimer(TIMER_ID_SESSION, g_nSampleRate, NULL);
	UpdateDlg();
	UpdateGraph();
	SetGraphRangeRecv();
	SetGraphRangeSent();
	return CPropertyPage::OnSetActive();
}


// Display a formatted number
void CSessionDlg::DisplayNumber(int nID, DWORD dwBytes) {
	CString s;
	FormatBytes(dwBytes, s, true);
	SetDlgItemText(nID, s);
}


DWORD CSessionDlg::CalcMax(STATS_STRUCT *pStats) {
	int total = min(m_SentGraph.GetTotalElements(), MAX_SAMPLES);
	return Cwinproc::GetRecentMaximum(pStats, total, 0);
}


void CSessionDlg::UpdateDlg() {
	DisplayNumber(IDC_RECV_CURRENT, theApp.m_wnd.RecvStats[0].Bps);
	DisplayNumber(IDC_RECV_AVERAGE, theApp.m_wnd.RecvStats[0].ave);
	
	DisplayNumber(IDC_SENT_CURRENT, theApp.m_wnd.SentStats[0].Bps);
	DisplayNumber(IDC_SENT_AVERAGE, theApp.m_wnd.SentStats[0].ave);
	
	DisplayNumber(IDC_RECV_MAXIMUM, CalcMax(theApp.m_wnd.RecvStats));
	DisplayNumber(IDC_SENT_MAXIMUM, CalcMax(theApp.m_wnd.SentStats));
}


// Update the total sent and received amounts
void CSessionDlg::OnTimer(UINT /* nIDEvent */) {
	UpdateDlg();
	UpdateGraph();
	
	CString s;
	FormatBytes(theApp.m_wnd.m_dbTotalBytesRecv - g_dbResetRecv, s, false);
	SetDlgItemText(IDC_GROUP_RECV, "Received: " + s);
	
	FormatBytes(theApp.m_wnd.m_dbTotalBytesSent - g_dbResetSent, s, false);
	SetDlgItemText(IDC_GROUP_SENT, "Sent: " + s);
}


BOOL CSessionDlg::OnKillActive() {
	if (m_pbrBackground != NULL) {
		delete m_pbrBackground;
		m_pbrBackground = NULL;
	}
	KillTimer(TIMER_ID_SESSION);
	return CPropertyPage::OnKillActive();
}


BOOL CSessionDlg::OnInitDialog() {
	CPropertyPage::OnInitDialog();
	
	CRect rect;
	GetDlgItem(IDC_SENT_GRAPH)->GetWindowRect(rect);
	ScreenToClient(rect);
	m_SentGraph.Create(WS_VISIBLE | WS_CHILD, rect, this, IDC_SENT_GRAPH_WINDOW);
	
	GetDlgItem(IDC_RECV_GRAPH)->GetWindowRect(rect);
	ScreenToClient(rect);
	m_RecvGraph.Create(WS_VISIBLE | WS_CHILD, rect, this, IDC_RECV_GRAPH_WINDOW);
	
	m_RecvGraph.SetSize(LINE_GRAPH_SIZE);
	m_SentGraph.SetSize(LINE_GRAPH_SIZE);
	
	m_AutoScale_Recv = 0;
	m_AutoScale_Sent = 0;
	
	CheckDlgButton(IDC_AUTOSCALE_RECV, g_bAutoScaleRecv);
	CheckDlgButton(IDC_AUTOSCALE_SENT, g_bAutoScaleSent);
	
	CheckDlgButton(IDC_BYTES, g_DisplayBytes);
	CheckDlgButton(IDC_BPS, g_DisplayBytes == 0);
	
	OnAutoscale();
	
	CSliderCtrl *pCtrl = (CSliderCtrl*)GetDlgItem(IDC_SCALE_SLIDER_RECV);
	pCtrl->SetRange(0, ELEMENTS(bpsArray) - 1);
	pCtrl->SetTicFreq(1);
	pCtrl->SetPos(g_Range_Recv);
	
	CSliderCtrl *pCtrl2 = (CSliderCtrl*)GetDlgItem(IDC_SCALE_SLIDER_SENT);
	pCtrl2->SetRange(0, ELEMENTS(bpsArray) - 1);
	pCtrl2->SetTicFreq(1);
	pCtrl2->SetPos(g_Range_Sent);
	
	CheckDlgButton(IDC_BARGRAPH, g_bShowBarGraph != 0);
	CheckDlgButton(IDC_LINEGRAPH, g_bShowBarGraph == 0);
	m_SentGraph.SetStyle(g_bShowBarGraph);
	m_RecvGraph.SetStyle(g_bShowBarGraph);
	
	UpdateGraphTextRecv(bpsArray[g_Range_Recv]);
	UpdateGraphTextSent(bpsArray[g_Range_Sent]);
	
	CheckDlgButton(IDC_CURRENT_RECV_OPTION, g_GraphOptions & OPTION_BPS);
	CheckDlgButton(IDC_AVE_RECV_OPTION, g_GraphOptions & OPTION_AVE);
	
	// Return TRUE unless you set the focus to a control
	// Exception: OCX Property Pages should return FALSE
	return TRUE;
}


// Draw the graphs
void CSessionDlg::DrawGraph(int nIndex, UPDATE_MODE update) {
	if (update & RECV_DATA) {
		m_RecvGraph.ShiftLeft();
		if (g_GraphOptions & OPTION_AVE)
			m_RecvGraph.SetPos(theApp.m_wnd.RecvStats[nIndex].ave, g_ColorAve, LINEGRAPH_AVE);
		if (g_GraphOptions & OPTION_BPS)
			m_RecvGraph.SetPos(theApp.m_wnd.RecvStats[nIndex].Bps, g_ColorRecv, LINEGRAPH_BPS);
	}
	
	if (update & SENT_DATA) {
		m_SentGraph.ShiftLeft();
		if (g_GraphOptions & OPTION_AVE)
			m_SentGraph.SetPos(theApp.m_wnd.SentStats[nIndex].ave, g_ColorAve, LINEGRAPH_AVE);
		if (g_GraphOptions & OPTION_BPS)
			m_SentGraph.SetPos(theApp.m_wnd.SentStats[nIndex].Bps, g_ColorSent, LINEGRAPH_BPS);
	}
}

// Adjust the auto scale position
void CSessionDlg::UpdateScrollPos(WORD wControl, DWORD dwValue) {
	CSliderCtrl *pCtrl = (CSliderCtrl*)GetDlgItem(wControl);
	
	if (pCtrl != NULL) {
		int nStart = ELEMENTS(bpsArray) - 1;
		int nPos = nStart;
		
		for (int i = nStart; i >= 0; i--) {
			if (bpsArray[i] >= dwValue)
				nPos = i;
		}
		pCtrl->SetPos(nPos);
	}
}

void CSessionDlg::UpdateGraph() {
	// Check autosize
	if (!g_bAutoScaleRecv || CalcAutoScale(&m_AutoScale_Recv, theApp.m_wnd.RecvStats, RECV_DATA) == FALSE)
		DrawGraph(0, RECV_DATA);
	if (!g_bAutoScaleSent || CalcAutoScale(&m_AutoScale_Sent, theApp.m_wnd.SentStats, SENT_DATA) == FALSE)
		DrawGraph(0, SENT_DATA);
}


// Determine the range of the graph based upon recent samples.
// Returns TRUE if the graph should be updated.
BOOL CSessionDlg::CalcAutoScale(UINT *pAutoScale, STATS_STRUCT *pStats, UPDATE_MODE update) {
	int total = m_SentGraph.GetTotalElements();
	DWORD dwHigh = 1;
	if (g_GraphOptions & OPTION_BPS)
		dwHigh = max(Cwinproc::GetRecentMaximum(pStats, total, 0), dwHigh);
	if (g_GraphOptions & OPTION_AVE)
		dwHigh = max(Cwinproc::GetRecentMaximum(pStats, total, 1), dwHigh);
	
	// Preferred top-of-scale numbers for binary-prefix bytes
	static unsigned int byteScaleTops[] = {
		100,
		150,
		250,
		400,
		600,
		10 * 1024 / 10,
		15 * 1024 / 10,
		25 * 1024 / 10,
		40 * 1024 / 10,
		60 * 1024 / 10,
		10 * 1024,
		15 * 1024,
		25 * 1024,
		40 * 1024,
		60 * 1024,
		100 * 1024,
		150 * 1024,
		250 * 1024,
		400 * 1024,
		600 * 1024,
		10 * 1024 * 1024 / 10,
		15 * 1024 * 1024 / 10,
		25 * 1024 * 1024 / 10,
		40 * 1024 * 1024 / 10,
		60 * 1024 * 1024 / 10,
		10 * 1024 * 1024,
		15 * 1024 * 1024,
		25 * 1024 * 1024,
		40 * 1024 * 1024,
		60 * 1024 * 1024,
		100 * 1024 * 1024,
		150 * 1024 * 1024,
		250 * 1024 * 1024,
		400 * 1024 * 1024,
		600 * 1024 * 1024,
	};
	
	// Preferred top-of-scale numbers for binary-prefix bits
	static unsigned int bitScaleTops[] = {
		1000 / 8,  // 1 Kb/s
		1500 / 8,
		2500 / 8,
		4000 / 8,
		6000 / 8,
		10000 / 8,  // 10 Kb/s
		15000 / 8,
		25000 / 8,
		40000 / 8,
		60000 / 8,
		100000 / 8,  // 100 Kb/s
		150000 / 8,
		250000 / 8,
		400000 / 8,
		600000 / 8,
		1000000 / 8,  // 1 Mb/s
		1500000 / 8,
		2500000 / 8,
		4000000 / 8,
		6000000 / 8,
		10000000 / 8,  // 10 Mb/s
		15000000 / 8,
		25000000 / 8,
		40000000 / 8,
		60000000 / 8,
		100000000 / 8,  // 100 Mb/s
		150000000 / 8,
		250000000 / 8,
		400000000 / 8,
		600000000 / 8,
		1000000000 / 8,  // 1 Gb/s
		1500000000 / 8,
	};
	
	unsigned int *scaletops;
	int len;
	if (g_DisplayBytes != 0) {
		scaletops = byteScaleTops;
		len = ELEMENTS(byteScaleTops);
	} else {
		scaletops = bitScaleTops;
		len = ELEMENTS(bitScaleTops);
	}
	
	// Pick the smallest top such that the current max is under 90% of the top
	int index = 0;
	while (index < len - 1 && scaletops[index] * 0.9 < dwHigh)
		index++;
	dwHigh = max(scaletops[index], dwHigh);
	
	bool updated = dwHigh != *pAutoScale;
	if (updated) {
		*pAutoScale = dwHigh;
		// Move the sliders
		if (update == SENT_DATA) {
			UpdateScrollPos(IDC_SCALE_SLIDER_SENT, *pAutoScale);
			UpdateGraphTextSent(*pAutoScale);
		} else {
			UpdateScrollPos(IDC_SCALE_SLIDER_RECV, *pAutoScale);
			UpdateGraphTextRecv(*pAutoScale);
		}
	}
	return updated;
}


// Draws the transmitted samples
void CSessionDlg::SetGraphRangeSent() {
	DWORD dwNumber;
	if (!g_bAutoScaleSent)
		dwNumber = bpsArray[g_Range_Sent];
	else
		dwNumber = m_AutoScale_Sent;
	
	m_SentGraph.SetGraphRange(dwNumber);
	m_SentGraph.ClearGraph();
	
	// Init the graphs
	int total = m_SentGraph.GetTotalElements();
	for (int i = 0; i <= total; i++)
		DrawGraph(total - i, SENT_DATA);
}

// Draws the received samples
void CSessionDlg::SetGraphRangeRecv() {
	DWORD dwNumber;
	if (!g_bAutoScaleRecv)
		dwNumber = bpsArray[g_Range_Recv];
	else
		dwNumber = m_AutoScale_Recv;
	
	m_RecvGraph.SetGraphRange(dwNumber);
	m_RecvGraph.ClearGraph();
	
	// Init the graphs
	int total = m_SentGraph.GetTotalElements();
	for (int i = 0; i <= total; i++)
		DrawGraph(total - i, RECV_DATA);
}


// Sets the colors for the graph labels
HBRUSH CSessionDlg::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor) {
	HBRUSH hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
	COLORREF cr;
	
	// Set the foreground and background colors for the text labels
	if (nCtlColor == CTLCOLOR_STATIC) {
		int iID = pWnd->GetDlgCtrlID();
		switch (iID) {
			case IDC_SENT_MAXIMUM: case IDC_SENT_CURRENT:  cr = g_ColorSent;  break;
			case IDC_RECV_MAXIMUM: case IDC_RECV_CURRENT:  cr = g_ColorRecv;  break;
			case IDC_SENT_AVERAGE: case IDC_RECV_AVERAGE:  cr = g_ColorAve ;  break;
			default:  return hbr;
		}
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(cr);
		return (HBRUSH)m_pbrBackground->m_hObject;
	}
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

void CSessionDlg::SetOptions() {
	g_GraphOptions = 0;
	if (IsDlgButtonChecked(IDC_CURRENT_RECV_OPTION))
		g_GraphOptions += OPTION_BPS;
	if (IsDlgButtonChecked(IDC_AVE_RECV_OPTION))
		g_GraphOptions += OPTION_AVE;
	
	SetGraphRangeRecv();  // Redraw the graphs
	SetGraphRangeSent();  // Redraw the graphs
}

void CSessionDlg::OnAveRecvOption() {
	SetOptions();
}

void CSessionDlg::OnAveSentOption() {
	SetOptions();
}

void CSessionDlg::OnCurrentRecvOption() {
	SetOptions();
}

void CSessionDlg::OnCurrentSentOption() {
	SetOptions();
}

void CSessionDlg::OnMaxRecvOption() {
	SetOptions();
}

void CSessionDlg::OnMaxSentOption() {
	SetOptions();
}

// Draw the labels on the right side of the graph
void CSessionDlg::UpdateGraphTextRecv(DWORD dwNumber) {
	CString s;
	FormatBytes(dwNumber, s, true);
	SetDlgItemText(IDC_SCALE_MAX_RECV, s);
	
	FormatBytes(dwNumber / 2, s, true);
	SetDlgItemText(IDC_SCALE_MID_RECV, s);
	
	FormatBytes(0, s, true);
	SetDlgItemText(IDC_RECV_ZERO, s);
	
	SetGraphRangeRecv();
}


// Draw the labels on the right side of the graph
void CSessionDlg::UpdateGraphTextSent(DWORD dwNumber) {
	CString s;
	FormatBytes(dwNumber, s, true);
	SetDlgItemText(IDC_SCALE_MAX_SENT, s);
	
	FormatBytes(dwNumber / 2, s, true);
	SetDlgItemText(IDC_SCALE_MID_SENT, s);
	
	FormatBytes(0, s, true);
	SetDlgItemText(IDC_SENT_ZERO, s);
	
	SetGraphRangeSent();
}


// Respond to the slider commands
void CSessionDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar) {
	int nControl = pScrollBar->GetDlgCtrlID();
	CSliderCtrl *pCtrl = (CSliderCtrl*)GetDlgItem(nControl);
	ASSERT(pCtrl != NULL);
	
	nPos = pCtrl->GetPos();
	if (nPos < 0)
		nPos = 0;
	if (nPos >= ELEMENTS(bpsArray))
		nPos = ELEMENTS(bpsArray) - 1;
	
	if (nControl == IDC_SCALE_SLIDER_RECV) {
		m_AutoScale_Recv = 0;
		g_Range_Recv = nPos;
		UpdateGraphTextRecv(bpsArray[nPos]);
	}
	if (nControl == IDC_SCALE_SLIDER_SENT) {
		m_AutoScale_Sent = 0;
		g_Range_Sent = nPos;
		UpdateGraphTextSent(bpsArray[nPos]);
	}
	
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CSessionDlg::OnBargraph() {
	SetGraphStyle();
}

void CSessionDlg::OnLinegraph() {
	SetGraphStyle();
}

void CSessionDlg::SetGraphStyle() {
	g_bShowBarGraph = IsDlgButtonChecked(IDC_BARGRAPH);
	m_SentGraph.SetStyle(g_bShowBarGraph);
	m_RecvGraph.SetStyle(g_bShowBarGraph);
	SetGraphRangeRecv();
	SetGraphRangeSent();
}

void CSessionDlg::OnResetData() {
	theApp.m_wnd.ResetData();
	
	m_AutoScale_Recv = 0;
	m_AutoScale_Sent = 0;
	
	g_dbResetRecv = theApp.m_wnd.m_dbTotalBytesRecv;  // When user clicks reset
	g_dbResetSent = theApp.m_wnd.m_dbTotalBytesSent;  // These values are subtracted from the total
	
	SetGraphRangeRecv();
	SetGraphRangeSent();
	
	OnTimer(0);  // Update the display
}

void CSessionDlg::OnAutoscale() {
	g_bAutoScaleRecv = IsDlgButtonChecked(IDC_AUTOSCALE_RECV);
	g_bAutoScaleSent = IsDlgButtonChecked(IDC_AUTOSCALE_SENT);
	GetDlgItem(IDC_SCALE_SLIDER_SENT)->EnableWindow(g_bAutoScaleSent == FALSE);
	GetDlgItem(IDC_SCALE_SLIDER_RECV)->EnableWindow(g_bAutoScaleRecv == FALSE);
}


void CSessionDlg::OnBps() {
	OnBytes();
}

// Sets the display type to either Bps or bps
void CSessionDlg::OnBytes() {
	g_DisplayBytes = IsDlgButtonChecked(IDC_BYTES);
	if (g_bAutoScaleRecv) {
		CalcAutoScale(&m_AutoScale_Recv, theApp.m_wnd.RecvStats, RECV_DATA);
		UpdateGraphTextRecv(m_AutoScale_Recv);
	} else
		UpdateGraphTextRecv(bpsArray[g_Range_Recv]);
	
	if (g_bAutoScaleSent) {
		CalcAutoScale(&m_AutoScale_Sent, theApp.m_wnd.SentStats, SENT_DATA);
		UpdateGraphTextSent(m_AutoScale_Sent);
	} else
		UpdateGraphTextSent(bpsArray[g_Range_Sent]);
}
