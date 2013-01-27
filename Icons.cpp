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
	if (m_hHistogramIcon) DestroyIcon(m_hHistogramIcon);
	if (m_hBarGraphIcon ) DestroyIcon(m_hBarGraphIcon);
	if (m_BarGraphIconInfo.hbmColor) DeleteObject(m_BarGraphIconInfo.hbmColor);
	if (m_BarGraphIconInfo.hbmMask ) DeleteObject(m_BarGraphIconInfo.hbmMask);
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


// Draws the bar graph icon
void CIcons::FillBarIcon(CDC *pDC, STATS_STRUCT *pStats, COLORREF color, CRect *prc) {
	DWORD dwHigh = Cwinproc::GetRecentMaximum(pStats, 15, 0);
	int nIcon = MulDiv(pStats[0].Bps, 14, dwHigh);
	prc->top = prc->bottom - nIcon;
	CBrush brush(color);
	pDC->FillRect(prc, &brush);
}


// Returns icon for the bar graph
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
	
	// Offsets for the left and right halves of the icon
	CRect rcSent(1, 1, 7, 15);
	CRect rcRecv(9, 1, 15, 15);
	
	CBrush back(g_ColorIconBack);
	dcMem.FillRect(rcSent, &back);
	dcMem.FillRect(rcRecv, &back);
	
	FillBarIcon(&dcMem, pSent, g_ColorSent, &rcSent);
	FillBarIcon(&dcMem, pRecv, g_ColorRecv, &rcRecv);
	
	dcMem.SelectObject(pOld);
	HICON hIcon = CreateIconIndirect(&m_BarGraphIconInfo);
	dcMem.DeleteDC();
	return hIcon;  // Caller must delete this icon handle
}


// Draws the histogram icon
void CIcons::FillHistogramIcon(CDC *pDC, STATS_STRUCT *pStats, COLORREF color, CRect *prc) {
	CBrush back(g_ColorIconBack);
	pDC->FillRect(*prc, &back);
	
	int width = prc->Width();
	int height = prc->Height();
	CRect temprect(*prc);
	CBrush brush(color);
	
	DWORD dwHigh = Cwinproc::GetRecentMaximum(pStats, width, 0);
	for (int i = 0; i < width; i++) {
		// Compute and clamp
		double barheight = (double)pStats[width - 1 - i].Bps / dwHigh * height;
		if (barheight < 0)
			barheight = 0;
		else if (barheight > height)
			barheight = height;
		
		temprect.left = i;
		temprect.right = i + 1;
		temprect.bottom = prc->bottom;
		temprect.top = temprect.bottom - (int)barheight;
		pDC->FillRect(&temprect, &brush);
		
		// One pixel of antialiasing at top of bar
		double t = barheight - (int)barheight;
		if (t > 0) {
			int tempcolor = (int)((color >> 16 & 0xFF) * t + (g_ColorIconBack >> 16 & 0xFF) * (1 - t) + 0.5) << 16
			              | (int)((color >>  8 & 0xFF) * t + (g_ColorIconBack >>  8 & 0xFF) * (1 - t) + 0.5) <<  8
			              | (int)((color >>  0 & 0xFF) * t + (g_ColorIconBack >>  0 & 0xFF) * (1 - t) + 0.5) <<  0;
			CBrush tempbrush(tempcolor);
			temprect.bottom = temprect.top;
			temprect.top--;
			pDC->FillRect(&temprect, &tempbrush);
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
	
	// Top half is for received graph
	CRect rc(0, 0, 16, 8);
	FillHistogramIcon(&dcMem, pRecv, g_ColorRecv, &rc);
	
	// Bottom half (slightly less) is for sent graph
	rc.SetRect(0, 9, 16, 16);
	FillHistogramIcon(&dcMem, pSent, g_ColorSent, &rc);
	
	// Line between the two graphs
	CRect linerect(0, 8, 16, 9);
	CBrush linebrush(RGB(128,128,128));
	dcMem.FillRect(&linerect, &linebrush);
	
	dcMem.SelectObject(pOld);
	HICON hIcon = CreateIconIndirect(&m_HistogramIconInfo);
	dcMem.DeleteDC();
	return hIcon;  // Caller must delete this icon handle
}
