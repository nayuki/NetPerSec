/* 
 * Implements the options property page.
 */

#include "stdafx.h"
#include "NetPerSec.h"
#include "OptionsDlg.h"
#include "snmp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// Tick marks for the averaging window are a multiple of 5 times the sample interval
#define AVERAGING_MULTIPLIER 5


// Milliseconds to set the timer for sampling SNMP
static UINT SampleIntervals[] = {
	250,
	500,
	1000,
	2000,
	3000,
	4000,
	5000,
};


// COptionsDlg property page

IMPLEMENT_DYNCREATE(COptionsDlg, CPropertyPage)

COptionsDlg::COptionsDlg() : CPropertyPage(COptionsDlg::IDD) {
	//{{AFX_DATA_INIT(COptionsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

COptionsDlg::~COptionsDlg() {}

void COptionsDlg::DoDataExchange(CDataExchange *pDX) {
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsDlg)
	DDX_Control(pDX, IDC_INTERFACES, m_Interfaces);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsDlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsDlg)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_USE_SNMP, OnUseSnmp)
	ON_BN_CLICKED(IDC_USE_DUN, OnUseDun)
	ON_BN_CLICKED(IDC_MONITOR_ADAPTER, OnMonitorAdapter)
	ON_CBN_SELCHANGE(IDC_INTERFACES, OnSelchangeInterfaces)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/* COptionsDlg message handlers */

BOOL COptionsDlg::OnInitDialog() {
	CPropertyPage::OnInitDialog();
	
	CSliderCtrl *pSampleSlider = (CSliderCtrl*)GetDlgItem(IDC_SAMPLE_SLIDER);
	ASSERT(pSampleSlider != NULL);
	
	pSampleSlider->SetRange(0, ELEMENTS(SampleIntervals) - 1);  // Milliseconds
	pSampleSlider->SetTicFreq(1);
	pSampleSlider->SetPageSize(1);
	pSampleSlider->SetLineSize(1);
	int nPos = 0;
	for (int i = 0; i < ELEMENTS(SampleIntervals); i++) {
		if ((UINT)g_nSampleRate >= SampleIntervals[i])
			nPos = i;
	}
	pSampleSlider->SetPos(nPos);
	
	CSliderCtrl *pWindowSlider = (CSliderCtrl*)GetDlgItem(IDC_AVERAGE_SLIDER);
	ASSERT(pWindowSlider != NULL);
	pWindowSlider->SetRange(1, (MAX_SAMPLES - 1) / AVERAGING_MULTIPLIER);  // Seconds
	pWindowSlider->SetTicFreq(1);
	pWindowSlider->SetPageSize(1);
	pWindowSlider->SetLineSize(1);
	pWindowSlider->SetPos(g_nAveragingWindow / AVERAGING_MULTIPLIER);
	
	int nID;
	switch (g_MonitorMode) {
		case MONITOR_DUN    :  nID = IDC_USE_DUN;          break;
		case MONITOR_ADAPTER:  nID = IDC_MONITOR_ADAPTER;  break;
		case MONITOR_ALL    :  nID = IDC_USE_SNMP;         break;
		default             :  ASSERT(false);              break;
	}
	
	CheckRadioButton(IDC_USE_SNMP, IDC_MONITOR_ADAPTER, nID);
	m_Interfaces.EnableWindow(g_MonitorMode == MONITOR_ADAPTER);
	
	UpdateDlg();
	
	// Return TRUE unless you set the focus to a control
	// Exception: OCX Property Pages should return FALSE
	return TRUE;
}


void COptionsDlg::UpdateAveragingWindow() {
	CString s;
	CSliderCtrl *pWindowSlider = (CSliderCtrl*)GetDlgItem(IDC_AVERAGE_SLIDER);
	
	int max = (MAX_SAMPLES - 1) / AVERAGING_MULTIPLIER;
	pWindowSlider->SetRange(1, max, TRUE);  // Blocks of samples
	int nPos = pWindowSlider->GetPos();
	
	g_nAveragingWindow = max(nPos * AVERAGING_MULTIPLIER, 1);
	ASSERT(g_nAveragingWindow <= MAX_SAMPLES);
	
	s.Format("%.5g", g_nSampleRate * AVERAGING_MULTIPLIER / 1000.0);
	SetDlgItemText(IDC_AVERAGE_MIN, s);
	s.Format("%.5g", max * g_nSampleRate * AVERAGING_MULTIPLIER / 1000.0);
	SetDlgItemText(IDC_AVERAGE_MAX, s);
}


void COptionsDlg::UpdateDlg() {
	CString s;
	
	UpdateAveragingWindow();
	
	if (g_nSampleRate == 1000)
		s = "1 second";
	else
		s.Format("%.5g seconds", g_nSampleRate / 1000.0);
	
	s = "Sampling Rate:   " + s;
	SetDlgItemText(IDC_SAMPLE_GROUP, s);
	
	double ave = g_nAveragingWindow * (g_nSampleRate / 1000.0);
	if (ave == 1)
		s = "1 second";
	else
		s.Format("%.5g seconds", ave);
	
	s = "Averaging Window:   " + s;
	SetDlgItemText(IDC_AVERAGE_GROUP, s);
}


void COptionsDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar) {
	int nControl = pScrollBar->GetDlgCtrlID();
	CSliderCtrl *pCtrl = (CSliderCtrl*)GetDlgItem(nControl);
	ASSERT(pCtrl != NULL);
	
	if (nControl == IDC_SAMPLE_SLIDER) {
		int nPos = pCtrl->GetPos();
		nPos = min(nPos, ELEMENTS(SampleIntervals));
		g_nSampleRate = SampleIntervals[nPos];
		theApp.m_wnd.SetTimer(TIMER_ID_WINPROC, g_nSampleRate, NULL);
	}
	
	UpdateDlg();
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}


void COptionsDlg::OnUseSnmp() {
	g_MonitorMode = MONITOR_ALL;
	
	if (IsDlgButtonChecked(IDC_USE_DUN))
		g_MonitorMode = MONITOR_DUN;
	
	if (IsDlgButtonChecked(IDC_MONITOR_ADAPTER)) {
		g_MonitorMode = MONITOR_ADAPTER;
		g_dwAdapter = m_Interfaces.GetItemData(m_Interfaces.GetCurSel());
	}
	
	m_Interfaces.EnableWindow(g_MonitorMode == MONITOR_ADAPTER);
	
	// Reset totals
	theApp.m_wnd.ResetData();
}

void COptionsDlg::OnUseDun() {
	OnUseSnmp();
}

BOOL COptionsDlg::OnSetActive() {
	CSnmp *pSnmp = &theApp.m_wnd.snmp;
	if (pSnmp != NULL) {
		CStringArray s;
		CUIntArray nAdapterArray;
		pSnmp->GetInterfaceDescriptions(&s, &nAdapterArray);
		m_Interfaces.ResetContent();
		
		int active = 0;
		for (int i = 0; i <= s.GetUpperBound(); i++) {
			int index = m_Interfaces.AddString(s.GetAt(i));
			if (index != CB_ERR) {
				m_Interfaces.SetItemData(index, nAdapterArray.GetAt(i));
				if (nAdapterArray.GetAt(i) == g_dwAdapter)
					active = i;
			}
		}
		m_Interfaces.SetCurSel(active);
	}
	return CPropertyPage::OnSetActive();
}

void COptionsDlg::OnMonitorAdapter() {
	OnUseSnmp();
}

void COptionsDlg::OnSelchangeInterfaces() {
	OnUseSnmp();
}
