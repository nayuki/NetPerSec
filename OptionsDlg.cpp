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


//tick marks for the averaging window are a multiple of 5 times the sample rate
#define AVERAGING_MULTIPLIER 5


//milliseconds to set the timer for sampling SNMP
UINT SampleRates[]={
	250,
	500,
	1000,
	2000,
	3000,
	4000,
	5000
};


/////////////////////////////////////////////////////////////////////////////
// COptionsDlg property page

IMPLEMENT_DYNCREATE(COptionsDlg, CPropertyPage)

COptionsDlg::COptionsDlg() : CPropertyPage(COptionsDlg::IDD)
{
	//{{AFX_DATA_INIT(COptionsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

///////////////////////////////////////////////////////////////////////////////////////////
//
COptionsDlg::~COptionsDlg()
{
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
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

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg message handlers

BOOL COptionsDlg::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	
	CSliderCtrl* pSampleSlider = (CSliderCtrl*)GetDlgItem(IDC_SAMPLE_SLIDER);
	CSliderCtrl* pWindowSlider = (CSliderCtrl*)GetDlgItem(IDC_AVERAGE_SLIDER);
	ASSERT(pSampleSlider != NULL);
	ASSERT(pWindowSlider != NULL);
	
	pSampleSlider->SetRange(0, ELEMENTS(SampleRates)-1);   //milliseconds
	pWindowSlider->SetRange(1, (MAX_SAMPLES -1) / AVERAGING_MULTIPLIER);   //seconds
	
	pSampleSlider->SetTicFreq(1);
	pWindowSlider->SetTicFreq(1);
	
	pSampleSlider->SetPageSize(1);
	pSampleSlider->SetLineSize(1);
	
	pWindowSlider->SetPageSize(1);
	pWindowSlider->SetLineSize(1);
	
	int nPos = 0;
	for (int i = 0; i < ELEMENTS(SampleRates); i++)
	{
		if ((UINT)g_nSampleRate >= SampleRates[i])
			nPos = i;
	}
	
	pSampleSlider->SetPos(nPos);
	pWindowSlider->SetPos(g_nAveragingWindow / AVERAGING_MULTIPLIER);
	
	int nID;
	nID = IDC_USE_SNMP;
	if (g_MonitorMode == MONITOR_DUN)
		nID = IDC_USE_DUN;
	
	if (g_MonitorMode == MONITOR_ADAPTER)
		nID = IDC_MONITOR_ADAPTER ;
	
	CheckRadioButton(IDC_USE_SNMP, IDC_MONITOR_ADAPTER , nID);
	m_Interfaces.EnableWindow(g_MonitorMode == MONITOR_ADAPTER);
	
	UpdateDlg();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



///////////////////////////////////////////////////////////////////////////////////////////
//
void COptionsDlg::UpdateAveragingWindow()
{
	CString s;
	CSliderCtrl* pWindowSlider = (CSliderCtrl*)GetDlgItem(IDC_AVERAGE_SLIDER);
	
	int max = (MAX_SAMPLES -1) / AVERAGING_MULTIPLIER;
	pWindowSlider->SetRange(1, max, TRUE);   //seconds
	int nPos = pWindowSlider->GetPos();
	
	g_nAveragingWindow = max(1, nPos * AVERAGING_MULTIPLIER);
	ASSERT(g_nAveragingWindow <= MAX_SAMPLES);
	
	s.Format("%.5g", (double)(g_nSampleRate * AVERAGING_MULTIPLIER) / 1000);
	SetDlgItemText(IDC_AVERAGE_MIN, s);
	s.Format("%.5g", (double)(max * g_nSampleRate * AVERAGING_MULTIPLIER) / 1000);
	SetDlgItemText(IDC_AVERAGE_MAX, s);
}



///////////////////////////////////////////////////////////////////////////////////////////
//
void COptionsDlg::UpdateDlg()
{
	CString s;
	
	UpdateAveragingWindow();
	
	if (g_nSampleRate == 1000)
		s = "1 second";
	else
		s.Format("%.5g seconds", (double)((double)g_nSampleRate / (double)1000));
	
	s = "Sampling Rate:   " + s;
	SetDlgItemText(IDC_SAMPLE_GROUP, s);
	
	double ave = (double)((double)g_nAveragingWindow * (double)((double)g_nSampleRate / 1000));
	
	if (ave == 1)
		s = "1 second";
	else
		s.Format("%.5g seconds", ave);
	
	s = "Averaging Window:   " + s;
	SetDlgItemText(IDC_AVERAGE_GROUP, s);
}


///////////////////////////////////////////////////////////////////////////////////////////
//
void COptionsDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int nControl = pScrollBar->GetDlgCtrlID();
	CSliderCtrl* pCtrl = (CSliderCtrl*)GetDlgItem(nControl);
	ASSERT(pCtrl != NULL);
	
	switch (nControl)
	{
		case IDC_SAMPLE_SLIDER:
		{
			int nPos = pCtrl->GetPos();
			nPos = min(nPos, ELEMENTS(SampleRates));
			
			g_nSampleRate = SampleRates[nPos];
			pTheApp->m_wnd.SetTimer(TIMER_ID_WINPROC, g_nSampleRate, NULL);
			break;
		}
		
		case IDC_AVERAGE_SLIDER:
			break;
	}
	
	UpdateDlg();
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}


void COptionsDlg::OnUseSnmp()
{
	g_MonitorMode = MONITOR_ALL;
	
	if (IsDlgButtonChecked(IDC_USE_DUN))
		g_MonitorMode = MONITOR_DUN;
	
	if (IsDlgButtonChecked(IDC_MONITOR_ADAPTER))
	{
		g_MonitorMode = MONITOR_ADAPTER;
		g_dwAdapter = m_Interfaces.GetItemData(m_Interfaces.GetCurSel());
	}
	
	m_Interfaces.EnableWindow(g_MonitorMode == MONITOR_ADAPTER);
	
	//reset totals
	pTheApp->m_wnd.ResetData();
	
	g_dbResetRecv =
	g_dbResetSent =
	pTheApp->m_wnd.m_dbRecvWrap =
	pTheApp->m_wnd.m_dbSentWrap =
	pTheApp->m_wnd.m_dbTotalBytesRecv =
	pTheApp->m_wnd.m_dbTotalBytesSent = 0;
}

void COptionsDlg::OnUseDun()
{
	OnUseSnmp();
}

BOOL COptionsDlg::OnSetActive()
{
	CSnmp* pSnmp = &pTheApp->m_wnd.snmp;
	
	if (pSnmp)
	{
		CStringArray s;
		CUIntArray  nAdapterArray;
		pSnmp->GetInterfaceDescriptions(&s, &nAdapterArray);
		m_Interfaces.ResetContent();
		
		int active = 0;
		
		for (int i = 0; i <= s.GetUpperBound(); i++)
		{
			int index = m_Interfaces.AddString(s.GetAt(i));
			if (index != CB_ERR)
			{
				m_Interfaces.SetItemData(index, nAdapterArray.GetAt(i));
				if (nAdapterArray.GetAt(i) == g_dwAdapter)
					active = i;
			}
		}
		m_Interfaces.SetCurSel(active);
	}
	return CPropertyPage::OnSetActive();
}

void COptionsDlg::OnMonitorAdapter()
{
	OnUseSnmp();
}

void COptionsDlg::OnSelchangeInterfaces()
{
	OnUseSnmp();
}
