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
	m_pPropertiesDlg = NULL;
	ZeroMemory(&m_SystemTray, sizeof(m_SystemTray));
	ResetData();
	m_dwStartTime = GetTickCount();
}


Cwinproc::~Cwinproc() {}


// Cwinproc
void Cwinproc::OnClose() {
	KillTimer(TIMER_ID_WINPROC);
	if (m_SystemTray.hWnd != NULL)
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
	if (!snmp.Init())
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
	
	// Write first few fields into array head element
	pStats[0].Bps = dwBps;
	pStats[0].total = dbTotal;
	pStats[0].time = dwTime;  // Current time interval (ms)
	
	// The index in the array for calculating averages
	int start = g_nAveragingWindow;
	while (start > 0 && (start >= MAX_SAMPLES || pStats[start].total == 0))
		start--;
	
	// Total bytes received/sent in our sampling window
	double dbSampleTotal = dbTotal - pStats[start].total;
	
	// Elapsed time (ms)
	DWORD dwElapsed = pStats[0].time - pStats[start].time;
	
	// Calculate average
	pStats[0].ave = dwElapsed > 0 ? (DWORD)(dbSampleTotal * 1000.0 / dwElapsed + 0.5) : 0;
}


DWORD Cwinproc::GetRecentMaximum(STATS_STRUCT *stats, int num, int type) {
	ASSERT(num <= MAX_SAMPLES);
	DWORD result = 1;  // To prevent division by zero when using the maximum for scaling
	if (type == 0) {
		for (int i = 0; i < num; i++)
			result = max(stats[i].Bps, result);
	} else if (type == 1) {
		for (int i = 0; i < num; i++)
			result = max(stats[i].ave, result);
	} else
		ASSERT(false);
	return result;
}


// Calculate samples
void Cwinproc::OnTimer(UINT /* nIDEvent */) {
	// Retrieve the total number of bytes received and sent by all network adapters, modulo 2^32.
	// Each network adapter counts from its own epoch, which is the last time the cable or wireless LAN was connected. 
	DWORD r, s;
	snmp.GetReceivedAndSentOctets(&r, &s);
	
	// Calculate the bytes transferred in the time interval that started a few seconds ago and ended just now.
	// Consider this calculation invalid if any of these conditions are true:
	// - The current total bytes r and s are 0 (network adapter disconnected/reset)
	// - The previous total bytes are 0 (NetPerSec counter was reset using ResetData())
	// - The data transferred in this interval exceeds 3 GiB (too fast to be plausible)
	DWORD curRecv, curSent;
	if (r == 0 && s == 0 || m_PrevBytesRecv == 0 && m_PrevBytesSent == 0
			|| r - m_PrevBytesRecv > 0xC0000000u || s - m_PrevBytesSent > 0xC0000000u) {
		curRecv = 0;
		curSent = 0;
	} else {
		curRecv = r - m_PrevBytesRecv;  // With 32-bit unsigned wraparound
		curSent = s - m_PrevBytesSent;
	}
	m_PrevBytesRecv = r;
	m_PrevBytesSent = s;
	m_TotalBytesRecv += curRecv;
	m_TotalBytesSent += curSent;
	
	// Don't depend upon timing of WM_TIMER messages. Get the true number of milliseconds elapsed
	DWORD dwTime = GetTickCount();
	DWORD elapsed = dwTime - m_dwStartTime;
	m_dwStartTime = dwTime;
	
	// Calculate bits per second
	DWORD dwRecv_bps = 0;
	DWORD dwSent_bps = 0;
	if (elapsed > 0) {
		dwRecv_bps = (DWORD)(curRecv * 1000.0 / elapsed + 0.5);
		dwSent_bps = (DWORD)(curSent * 1000.0 / elapsed + 0.5);
	}
	
	// Shift over previous data
	for (int i = MAX_SAMPLES - 1; i >= 1; i--) {
		RecvStats[i] = RecvStats[i - 1];
		SentStats[i] = SentStats[i - 1];
	}
	
	// Calc the average bps and add new entry to array
	CalcAverages(m_TotalBytesRecv, dwTime, dwRecv_bps, RecvStats);
	CalcAverages(m_TotalBytesSent, dwTime, dwSent_bps, SentStats);
	
	// Get the icon for the system tray
	HICON hIcon = theApp.m_Icons.GetIcon(RecvStats, SentStats, g_IconStyle);
	UpdateTrayIcon(hIcon);
	DestroyIcon(hIcon);
}


void Cwinproc::ShowPropertiesDlg() {
	if (m_pPropertiesDlg != NULL)
		m_pPropertiesDlg->SetForegroundWindow();
	else {
		// Fake out MFC in order to receive the "minimize all windows" syscommand message
		// The window is restored in initdialog
		theApp.m_pMainWnd = NULL;
		
		m_pPropertiesDlg = new DlgPropSheet(SZ_APPNAME, NULL);
		m_pPropertiesDlg->m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_MODELESS;
		
		if (m_pPropertiesDlg->DoModal() == IDOK)
			SaveSettings();
		else
			ReadSettings();
		
		delete m_pPropertiesDlg;
		m_pPropertiesDlg = NULL;
		
		SetTimer(TIMER_ID_WINPROC, g_nSampleRate, NULL);
	}
}


LRESULT Cwinproc::OnTaskbarNotify(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(wParam);
	
	switch (lParam) {
		case WM_MOUSEMOVE:
		{
			CString s, sRecvBPS, sRecvAVE;
			FormatBytes(RecvStats[0].Bps, sRecvBPS, true);
			FormatBytes(RecvStats[0].ave, sRecvAVE, true);
			s.Format("Current: %s   Average: %s", sRecvBPS, sRecvAVE);
			
			m_SystemTray.cbSize = sizeof(NOTIFYICONDATA);
			m_SystemTray.hWnd   = GetSafeHwnd();
			m_SystemTray.uID    = 1;
			m_SystemTray.uFlags = NIF_TIP;
			strcpy_s(m_SystemTray.szTip, s);
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
			
			CMenu &pMenu = *menu.GetSubMenu(0);
			pMenu.SetDefaultItem(0, TRUE);
			
			// See Q135788 "PRB: Menus for Notification Icons Do Not Work Correctly"
			SetForegroundWindow();
			int cmd = pMenu.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_NONOTIFY, pt.x, pt.y, this);
			PostMessage(WM_NULL, 0, 0);
			
			if (cmd == IDCLOSE) {
				// Save any settings if the user closes the tray icon while the dialog is open
				if (m_pPropertiesDlg != NULL) {
					SaveSettings();
					m_pPropertiesDlg->SendMessage(WM_CLOSE);
				}
				theApp.m_wnd.PostMessage(WM_CLOSE);
			} else if (cmd == ID_PROPERTIES)
				ShowPropertiesDlg();
		}
		break;
	}
	return 0;
}


void Cwinproc::UpdateTrayIcon(HICON hIcon) {
	ASSERT(hIcon != 0);
	if (m_SystemTray.hWnd != NULL && hIcon != NULL) {
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
	ZeroMemory(RecvStats, sizeof(RecvStats));
	ZeroMemory(SentStats, sizeof(SentStats));
	m_TotalBytesRecv = 0;
	m_TotalBytesSent = 0;
	m_PrevBytesRecv = 0;
	m_PrevBytesSent = 0;
}


void Cwinproc::WinHelp(DWORD /*dwData*/, UINT /*nCmd*/) {
	if (m_pPropertiesDlg != NULL) {
		switch (m_pPropertiesDlg->GetActiveIndex()) {
			case 0:  CWnd::WinHelp(IDH_connection_tab);  return;
			case 1:  CWnd::WinHelp(IDH_options_tab   );  return;
			case 2:  CWnd::WinHelp(IDH_colors_tab    );  return;
		}
	}
	CWnd::WinHelp(0, HELP_CONTENTS);
}
