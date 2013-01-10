/* 
 * Collects SNMP statistics and updates the system tray icons.
 */

#include "StdAfx.h"
#include "NetPerSec.h"
#include "winproc.h"
#include "resource.h"
#include "hlp\helpids.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


UINT TaskbarCallbackMsg = RegisterWindowMessage("NPSTaskbarMsg");


// Cwinproc
Cwinproc::Cwinproc() {
	m_dwStartTime = 0;
	m_dbTotalBytesRecv = 0;
	m_dbTotalBytesSent = 0;
	m_dbRecvWrap = 0;
	m_dbSentWrap = 0;
	m_pPropertiesDlg = 0;
	ZeroMemory(&m_SystemTray, sizeof(m_SystemTray));
	ResetData();
}

Cwinproc::~Cwinproc() {}


// Cwinproc
void Cwinproc::OnClose() {
	KillTimer(TIMER_ID_WINPROC);
	if (m_SystemTray.hWnd)
		Shell_NotifyIcon(NIM_DELETE, &m_SystemTray);
	CWnd::OnClose();
}

// Cwinproc
BEGIN_MESSAGE_MAP(Cwinproc, CWnd)
	//{{AFX_MSG_MAP(Cwinproc)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(TaskbarCallbackMsg, OnTaskbarNotify)
END_MESSAGE_MAP()


/* Cwinproc message handlers */

// Startup - called when the invisible window is created in netpersec.cpp.
// Initializes SNMP and the system tray icon
void Cwinproc::StartUp() {
	if (snmp.Init() == FALSE)
		PostQuitMessage(0);
	else {
		SetTimer(TIMER_ID_WINPROC, g_nSampleRate, NULL);
		
		// 1.1 init using NIF_ICON
		HICON hIcon;
		if (g_IconStyle == ICON_HISTOGRAM)
			hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_HISTOGRAM), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		else
			hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_BARGRAPH), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		
		m_SystemTray.cbSize = sizeof(NOTIFYICONDATA);
		m_SystemTray.hWnd   = GetSafeHwnd();
		m_SystemTray.uID    = 1;
		m_SystemTray.hIcon  = hIcon;
		m_SystemTray.uFlags = NIF_MESSAGE | NIF_ICON;
		m_SystemTray.uCallbackMessage = TaskbarCallbackMsg;
		if (!Shell_NotifyIcon(NIM_ADD, &m_SystemTray))
			AfxMessageBox("System tray error.");
	}
}

void Cwinproc::CalcAverages(double dbTotal, DWORD dwTime, DWORD dwBps, STATS_STRUCT *pStats) {
	ASSERT(g_nAveragingWindow <= MAX_SAMPLES);
	
	// Set current bps
	pStats[m_nArrayIndex].Bps = dwBps;
	
	// Set total bytes sent or recv
	pStats[m_nArrayIndex].total = dbTotal;
	
	// Set the current time (milliseconds)
	pStats[m_nArrayIndex].time = dwTime;
	
	// Start is the index in the array for calculating averages
	int start = m_nArrayIndex - g_nAveragingWindow;
	if (start < 0)
		start += MAX_SAMPLES;
	
	// The array entry may not have been filled in yet
	if (pStats[start].total == 0)
		start = 0;
	
	// Set average based upon sampling window size
	double dbSampleTotal = 0;
	
	// Total bytes received/sent in our sampling window
	if (pStats[start].total)
		dbSampleTotal = dbTotal - pStats[start].total;
	
	// Elapsed milliseconds
	DWORD dwElapsed = pStats[m_nArrayIndex].time - pStats[start].time;
	
	// Calc average
	if (dwElapsed)
		pStats[m_nArrayIndex].ave = MulDiv((DWORD)dbSampleTotal, 1000, dwElapsed);
}


// Calculate samples
void Cwinproc::OnTimer(UINT /* nIDEvent */) {
	DWORD s, r;
	snmp.GetReceivedAndSentOctets(&r, &s);
	
	// Init data?
	if (!m_dbTotalBytesRecv || !m_dbTotalBytesSent) {
		m_dbTotalBytesRecv = r + m_dbRecvWrap;
		m_dbTotalBytesSent = s + m_dbSentWrap;
		m_dwStartTime = GetTickCount();
	}
	
	// Don't depend upon exact WM_TIMER messages, get the true elapsed number of milliseconds
	DWORD dwTime = GetTickCount();
	DWORD elapsed = dwTime - m_dwStartTime;
	m_dwStartTime = dwTime;
	
	double dbRecv = r + m_dbRecvWrap;
	double dbSent = s + m_dbSentWrap;
	// Check for integer wrap
	if (dbRecv < m_dbTotalBytesRecv) {
		m_dbRecvWrap += 0xffffffff;
		dbRecv = r + m_dbRecvWrap;
	}
	if (dbSent < m_dbTotalBytesSent) {
		m_dbSentWrap += 0xffffffff;
		dbSent = s + m_dbSentWrap;
	}
	
	// Total bytes sent or received during this interval
	DWORD total_recv = (DWORD)(dbRecv - m_dbTotalBytesRecv);
	DWORD total_sent = (DWORD)(dbSent - m_dbTotalBytesSent);
	
	DWORD dwRecv_bps = 0;
	DWORD dwSent_bps = 0;
	
	// Calc bits per second
	if (elapsed) {
		dwRecv_bps = MulDiv(total_recv, 1000, elapsed);
		dwSent_bps = MulDiv(total_sent, 1000, elapsed);
	}
	
	// Convert to double
	double recv_bits = (double)r;
	double sent_bits = (double)s;
	
	// Calc the average bps
	CalcAverages(recv_bits, dwTime, dwRecv_bps, &RecvStats[0]);
	CalcAverages(sent_bits, dwTime, dwSent_bps, &SentStats[0]);
	
	// Get the icon for the system tray
	HICON hIcon = pTheApp->m_Icons.GetIcon(&RecvStats[0], &SentStats[0], m_nArrayIndex, g_IconStyle);
	UpdateTrayIcon(hIcon);
	DestroyIcon(hIcon);
	
	// Increment the circular buffer index
	m_nArrayIndex++;
	if (m_nArrayIndex >= MAX_SAMPLES)
		m_nArrayIndex = 0;
	
	// Save the totals
	m_dbTotalBytesRecv = dbRecv;
	m_dbTotalBytesSent = dbSent;
}


void Cwinproc::ShowPropertiesDlg() {
	if (m_pPropertiesDlg)
		m_pPropertiesDlg->SetForegroundWindow();
	else {
		// Fake out MFC in order to receive the "minimize all windows" syscommand message
		// The window is restored in initdialog
		pTheApp->m_pMainWnd = NULL;
		
		m_pPropertiesDlg = new DlgPropSheet(SZ_APPNAME, NULL);
		
		m_pPropertiesDlg->m_psh.dwFlags |= PSH_NOAPPLYNOW;
		m_pPropertiesDlg->m_psh.dwFlags |= PSH_MODELESS;
		
		if (m_pPropertiesDlg->DoModal() == IDOK)
			SaveSettings();
		else
			ReadSettings();
		
		delete m_pPropertiesDlg;
		m_pPropertiesDlg = 0;
		
		SetTimer(TIMER_ID_WINPROC, g_nSampleRate, NULL);
	}
}


LRESULT Cwinproc::OnTaskbarNotify(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(wParam);
	
	switch (lParam) {
		case WM_MOUSEMOVE:
		{
			CString s, sRecvBPS, sRecvAVE;
			int index = GetArrayIndex();
			
			FormatBytes(RecvStats[index].Bps, &sRecvBPS);
			FormatBytes(RecvStats[index].ave, &sRecvAVE);
			s.Format("Current: %s   Average: %s", sRecvBPS, sRecvAVE);
			
			m_SystemTray.cbSize = sizeof(NOTIFYICONDATA);
			m_SystemTray.hWnd   = GetSafeHwnd();
			m_SystemTray.uID    = 1;
			m_SystemTray.uFlags = NIF_TIP;
			strncpy(m_SystemTray.szTip, s, sizeof(m_SystemTray.szTip));
			Shell_NotifyIcon(NIM_MODIFY, &m_SystemTray);
		}
		break;
		
		case WM_LBUTTONDBLCLK:
			ShowPropertiesDlg();
			break;
			
		case WM_RBUTTONUP:
		{
			CMenu menu;
			POINT pt;
			
			GetCursorPos(&pt);
			
			menu.LoadMenu(IDR_MENU1);
			menu.SetDefaultItem(0, TRUE);
			
			CMenu *pMenu = menu.GetSubMenu(0);
			pMenu->SetDefaultItem(0, TRUE);
			
			// See Q135788 "PRB: Menus for Notification Icons Do Not Work Correctly"
			SetForegroundWindow();
			int cmd = pMenu->TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_NONOTIFY , pt.x, pt.y, this);
			PostMessage(WM_NULL, 0, 0);
			
			if (cmd == IDCLOSE) {
				// Save any settings if the user closes the tray icon while the dialog is open
				if (m_pPropertiesDlg) {
					SaveSettings();
					m_pPropertiesDlg->SendMessage(WM_CLOSE);
				}
				pTheApp->m_wnd.PostMessage(WM_CLOSE);
			} else if (cmd == ID_PROPERTIES)
				ShowPropertiesDlg();
		}
		break;
	}
	return 0;
}

// The index to the array will always be one greater than the actual current value
int Cwinproc::GetArrayIndex() {
	int i = m_nArrayIndex - 1;
	if (i < 0)
		i = MAX_SAMPLES + i;
	return i;
}

void Cwinproc::UpdateTrayIcon(HICON hIcon) {
	ASSERT(hIcon != 0);
	if (m_SystemTray.hWnd && hIcon) {
		m_SystemTray.cbSize = sizeof(NOTIFYICONDATA);
		m_SystemTray.hWnd = GetSafeHwnd();
		m_SystemTray.uID = 1;
		m_SystemTray.hIcon = hIcon;
		m_SystemTray.uFlags = NIF_ICON;
		m_SystemTray.uCallbackMessage = TaskbarCallbackMsg;
		Shell_NotifyIcon(NIM_MODIFY, &m_SystemTray);
	}
}


// Init all arrays
void Cwinproc::ResetData() {
	m_nArrayIndex = 0;
	ZeroMemory(RecvStats, sizeof(RecvStats));
	ZeroMemory(SentStats, sizeof(SentStats));
}


void Cwinproc::WinHelp(DWORD /*dwData*/, UINT /*nCmd*/) {
	if (m_pPropertiesDlg) {
		switch (m_pPropertiesDlg->GetActiveIndex()) {
			case 0:
				CWnd::WinHelp(IDH_connection_tab);
				return;
			case 1:
				CWnd::WinHelp(IDH_options_tab);
				return;
			case 2:
				CWnd::WinHelp(IDH_colors_tab);
				return;
		}
	}
	CWnd::WinHelp(0, HELP_CONTENTS);
}
