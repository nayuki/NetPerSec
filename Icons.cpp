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
	CRect rc;
	rc.CopyRect(prc);
	
	int offset = 1;
	int width = 1;
	int size = rc.Width() - offset * 2;
	const int HEIGHT = 6;
	
	rc.bottom -= offset;
	
	rc.top += offset;
	rc.right -= offset;
	rc.left += offset;
	CBrush back(g_ColorIconBack);
	pDC->FillRect(rc, &back);
	
	rc.right = offset;
	rc.left = offset;
	CBrush brush(color);
	
	DWORD dwHigh = Cwinproc::GetRecentMaximum(pStats, size, 0);
	for (int i = size - 1; i >= 0; i--) {
		// Compute and clamp
		double barheight = (double)pStats[i].Bps / dwHigh * HEIGHT;
		if (barheight < 0)
			barheight = 0;
		else if (barheight > HEIGHT)
			barheight = HEIGHT;
		
		rc.top = rc.bottom - (int)barheight;
		rc.right += width;
		pDC->FillRect(&rc, &brush);
		
		double t = barheight - (int)barheight;
		if (t > 0) {
			int tempcolor = (int)((color >> 16 & 0xFF) * t + (g_ColorIconBack >> 16 & 0xFF) * (1 - t) + 0.5) << 16
			              | (int)((color >>  8 & 0xFF) * t + (g_ColorIconBack >>  8 & 0xFF) * (1 - t) + 0.5) <<  8
			              | (int)((color >>  0 & 0xFF) * t + (g_ColorIconBack >>  0 & 0xFF) * (1 - t) + 0.5) <<  0;
			CBrush tempbrush(tempcolor);
			CRect temprc;
			temprc.top = rc.top - 1;
			temprc.bottom = rc.top;
			temprc.left = rc.left;
			temprc.right = rc.right;
			pDC->FillRect(&temprc, &tempbrush);
		}
		
		rc.left += width;
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
	
	// Icon is 16 pixels high
	CRect rc(0, 0, 16, 16);
	rc.top = rc.bottom / 2;
	FillHistogramIcon(&dcMem, pSent, g_ColorSent, &rc);
	
	rc.top = 0;
	rc.bottom -= rc.Height() / 2;
	FillHistogramIcon(&dcMem, pRecv, g_ColorRecv, &rc);
	
	dcMem.SelectObject(pOld);
	HICON hIcon = CreateIconIndirect(&m_HistogramIconInfo);
	dcMem.DeleteDC();
	return hIcon;  // Caller must delete this icon handle
}
