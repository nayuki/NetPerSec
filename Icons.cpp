/* 
 * Implements the bar graph and histogram icons.
 */

#include "StdAfx.h"
#include "Icons.h"
#include "Globals.h"
#include "resource.h"


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
HICON CIcons::GetIcon(STATS_STRUCT *pRecv, STATS_STRUCT *pSent, int nIndex, ICON_STYLE nStyle) {
	if (nStyle == ICON_HISTOGRAM)
		return GetHistogramIcon(pRecv, pSent, nIndex);
	else if (nStyle == ICON_BARGRAPH)
		return GetBargraphIcon(pRecv, pSent, nIndex);
	else
		ASSERT(false);
}


// Draws the bar graph icon
void CIcons::FillBarIcon(CDC *pDC, STATS_STRUCT *pStats, COLORREF color, int nIndex, CRect *prc) {
	// Find maximum among most recent HIST_SIZE samples in circular buffer
	const int HIST_SIZE = 15;
	DWORD dwHigh = 1;
	for (int i = 0; i < HIST_SIZE; i++)
		dwHigh = max(pStats[(nIndex - HIST_SIZE + 1 + i + MAX_SAMPLES) % MAX_SAMPLES].Bps, dwHigh);
	
	int nIcon = MulDiv(pStats[nIndex].Bps, 14, dwHigh);
	prc->top = prc->bottom - nIcon;
	CBrush brush(color);
	pDC->FillRect(prc, &brush);
}


// Returns icon for the bar graph
HICON CIcons::GetBargraphIcon(STATS_STRUCT *pRecv, STATS_STRUCT *pSent, int nIndex) {
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
	
	FillBarIcon(&dcMem, pSent, g_ColorSent, nIndex, &rcSent);
	FillBarIcon(&dcMem, pRecv, g_ColorRecv, nIndex, &rcRecv);
	
	dcMem.SelectObject(pOld);
	HICON hIcon = CreateIconIndirect(&m_BarGraphIconInfo);
	dcMem.DeleteDC();
	return hIcon;  // Caller must delete this icon handle
}


// Draws the histogram icon
void CIcons::FillHistogramIcon(CDC *pDC, STATS_STRUCT *pStats, COLORREF color, int nIndex, CRect *prc) {
	CRect rc;
	rc.CopyRect(prc);
	
	int offset = 1;
	int width = 1;
	int size = rc.Width() - offset * 2;
	
	rc.bottom -= offset;
	
	rc.top += offset;
	rc.right -= offset;
	rc.left += offset;
	CBrush back(g_ColorIconBack);
	pDC->FillRect(rc, &back);
	
	rc.right = offset;
	rc.left = offset;
	CBrush brush(color);
	
	int start = nIndex - size;
	if (start < 0)
		start += MAX_SAMPLES;
	int savestart = start;
	
	DWORD dwHigh = max(pStats[nIndex].Bps, 1);
	for (int i = 0; i < size; i++) {
		dwHigh = max(pStats[start].Bps, dwHigh);
		start = (start + 1) % MAX_SAMPLES;
	}
	
	start = savestart;
	for (int i = 0; i < size; i++) {
		// 60 percent - icon is 6 pixels high
		int nIcon = MulDiv(pStats[start].Bps, 6, dwHigh);
		if (pStats[start].Bps != 0)  // For at least 1 pixel if there is traffic
			nIcon = max(nIcon, 1);
		ASSERT(nIcon < 7);
		
		rc.top = rc.bottom - nIcon;
		rc.right += width;
		pDC->FillRect(&rc, &brush);
		rc.left += width;
		
		start = (start + 1) % MAX_SAMPLES;
	}
}


// Returns a histogram icon
HICON CIcons::GetHistogramIcon(STATS_STRUCT *pRecv, STATS_STRUCT *pSent, int nIndex) {
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
	FillHistogramIcon(&dcMem, pSent, g_ColorSent, nIndex, &rc);
	
	rc.top = 0;
	rc.bottom -= rc.Height() / 2;
	FillHistogramIcon(&dcMem, pRecv, g_ColorRecv, nIndex, &rc);
	
	dcMem.SelectObject(pOld);
	HICON hIcon = CreateIconIndirect(&m_HistogramIconInfo);
	dcMem.DeleteDC();
	return hIcon;  // Caller must delete this icon handle
}
