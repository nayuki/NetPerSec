/* 
 * Implements the bar graph and histogram icons.
 */

#include "StdAfx.h"
#include "Icons.h"
#include "Globals.h"
#include "resource.h"
#include "Snmp.h"
#include "winproc.h"


CIcons::CIcons() {
	m_hHistogramIcon = 0;
	m_hBarGraphIcon  = 0;
	ZeroMemory(&m_BarGraphIconInfo , sizeof(m_BarGraphIconInfo));
	ZeroMemory(&m_HistogramIconInfo, sizeof(m_HistogramIconInfo));
}


CIcons::~CIcons() {
	if (m_hHistogramIcon != NULL) DestroyIcon(m_hHistogramIcon);
	if (m_hBarGraphIcon  != NULL) DestroyIcon(m_hBarGraphIcon);
	if (m_BarGraphIconInfo.hbmColor != NULL) DeleteObject(m_BarGraphIconInfo.hbmColor);
	if (m_BarGraphIconInfo.hbmMask  != NULL) DeleteObject(m_BarGraphIconInfo.hbmMask);
}


// Return a handle to a histogram or bar graph icon.
// The calling function must delete this handle
HICON CIcons::GetIcon(STATS_STRUCT *pRecv, STATS_STRUCT *pSent, ICON_STYLE nStyle) {
	if (nStyle == ICON_HISTOGRAM)
		return GetHistogramIcon(pRecv, pSent);
	else if (nStyle == ICON_BARGRAPH)
		return GetBargraphIcon(pRecv, pSent);
	else
		ASSERT(false);
}


// Draws one graph of the bar graph icon
void CIcons::FillBarIcon(CDC &pDC, STATS_STRUCT *pStats, COLORREF color, CRect &prc) {
	CBrush back(g_ColorIconBack);
	pDC.FillRect(prc, &back);
	
	DWORD dwHigh = Cwinproc::GetRecentMaximum(pStats, 15, 0);
	prc.top = prc.bottom - MulDiv(pStats[0].Bps, 14, dwHigh);
	CBrush brush(color);
	pDC.FillRect(prc, &brush);
}


// Returns a bar graph icon
HICON CIcons::GetBargraphIcon(STATS_STRUCT *pRecv, STATS_STRUCT *pSent) {
	if (m_hBarGraphIcon == 0) {
		m_hBarGraphIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_BARGRAPH), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		GetIconInfo(m_hBarGraphIcon, &m_BarGraphIconInfo);
		m_bmpBarGraph.Attach(m_BarGraphIconInfo.hbmColor);
		ASSERT(m_hBarGraphIcon != 0);
	}
	
	CDC dcMem;
	dcMem.CreateCompatibleDC(NULL);
	CBitmap *pOld = dcMem.SelectObject(&m_bmpBarGraph);
	
	// Regions for the left and right halves of the icon
	CRect rcSent(1, 1, 7, 15);
	CRect rcRecv(9, 1, 15, 15);
	FillBarIcon(dcMem, pSent, g_ColorSent, rcSent);
	FillBarIcon(dcMem, pRecv, g_ColorRecv, rcRecv);
	
	dcMem.SelectObject(pOld);
	HICON hIcon = CreateIconIndirect(&m_BarGraphIconInfo);
	dcMem.DeleteDC();
	return hIcon;  // Caller must delete this icon handle
}


static int InterpolateColors(int x, int y, double t) {
	return (int)((x >> 16 & 0xFF) * (1 - t) + (y >> 16 & 0xFF) * t + 0.5) << 16
	     | (int)((x >>  8 & 0xFF) * (1 - t) + (y >>  8 & 0xFF) * t + 0.5) <<  8
	     | (int)((x >>  0 & 0xFF) * (1 - t) + (y >>  0 & 0xFF) * t + 0.5) <<  0;
}


static int gradientColor = 0xC0C0C0;
static int gradientHeight = 5;

static int GradientColor(int y) {
	double t = min((double)y / gradientHeight, 1);
	t = 1 - (1 - t) * (1 - t);
	return InterpolateColors(gradientColor, g_ColorIconBack, t);
}


// Draws one graph of the histogram icon
void CIcons::FillHistogramIcon(CDC &pDC, STATS_STRUCT *pStats, COLORREF color, CRect &prc) {
	// Slight gradient background at the top
	int height = prc.Height();
	for (int i = 0; i < height; i++) {
		CRect rect(prc.left, prc.top + i, prc.right, prc.top + i + 1);
		CBrush brush(GradientColor(i));
		pDC.FillRect(&rect, &brush);
	}
	
	int width = prc.Width();
	CBrush brush(color);
	DWORD dwHigh = Cwinproc::GetRecentMaximum(pStats, width, 0);
	for (int i = 0; i < width; i++) {
		// Compute and clamp
		double barheight = (double)pStats[width - 1 - i].Bps / dwHigh * height;
		if (barheight < 0)
			barheight = 0;
		else if (barheight > height)
			barheight = height;
		
		CRect temprect(i, prc.bottom - (int)barheight, i + 1, prc.bottom);
		pDC.FillRect(&temprect, &brush);
		
		// One pixel of antialiasing at top of bar
		double t = barheight - (int)barheight;
		if (t > 0) {
			CBrush tempbrush(InterpolateColors(GradientColor(height - 1 - (int)barheight), color, t));
			temprect.bottom = temprect.top;
			temprect.top--;
			pDC.FillRect(&temprect, &tempbrush);
		}
	}
}


// Returns a histogram icon
HICON CIcons::GetHistogramIcon(STATS_STRUCT *pRecv, STATS_STRUCT *pSent) {
	if (m_hHistogramIcon == 0) {
		m_hHistogramIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_HISTOGRAM), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		ASSERT(m_hHistogramIcon != 0);
		GetIconInfo(m_hHistogramIcon, &m_HistogramIconInfo);
		m_bmpHistogram.Attach(m_HistogramIconInfo.hbmColor);
	}
	
	CDC dcMem;
	dcMem.CreateCompatibleDC(NULL);
	CBitmap *pOld = dcMem.SelectObject(&m_bmpHistogram);
	
	// Top half: Received graph
	CRect rc(0, 0, 16, 8);
	FillHistogramIcon(dcMem, pRecv, g_ColorRecv, rc);
	
	// Bottom half: Sent graph
	rc.SetRect(0, 8, 16, 16);
	FillHistogramIcon(dcMem, pSent, g_ColorSent, rc);
	
	dcMem.SelectObject(pOld);
	HICON hIcon = CreateIconIndirect(&m_HistogramIconInfo);
	dcMem.DeleteDC();
	return hIcon;  // Caller must delete this icon handle
}
