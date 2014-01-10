#ifndef _ICONS_H_
#define _ICONS_H_

#include "Globals.h"


class CIcons {
public:
	CIcons();
	~CIcons();
	
	HICON GetIcon(STATS_STRUCT*, STATS_STRUCT*, ICON_STYLE);
	
private:
	HICON m_hHistogramIcon;
	HICON m_hBarGraphIcon;
	ICONINFO m_BarGraphIconInfo;
	ICONINFO m_HistogramIconInfo;
	CBitmap m_bmpBarGraph;
	CBitmap m_bmpHistogram;
	
	HICON GetBargraphIcon (STATS_STRUCT *pSent, STATS_STRUCT *pRecv);
	HICON GetHistogramIcon(STATS_STRUCT *pRecv, STATS_STRUCT *pSent);
	void FillHistogramIcon(CDC &pDC, STATS_STRUCT *pRecv , COLORREF color, CRect &prc);
	void FillBarIcon      (CDC &pDC, STATS_STRUCT *pStats, COLORREF color, CRect &prc);
};

#endif
