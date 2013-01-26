/* 
 * Creates a hidden window to manage the system tray
 * icons. See winproc.cpp.
 */

#include "StdAfx.h"
#include "NetPerSec.h"
#include "winsock2.h"
#pragma comment(lib, "Ws2_32.lib")

CNetPerSecApp *pTheApp;

// Private message for the taskbar
extern UINT TaskbarCallbackMsg;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CNetPerSecApp

BEGIN_MESSAGE_MAP(CNetPerSecApp, CWinApp)
	//{{AFX_MSG_MAP(CNetPerSecApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		// DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

// CNetPerSecApp construction
CNetPerSecApp::CNetPerSecApp() {
	pTheApp = this;
	m_hMutex = NULL;
}


CNetPerSecApp::~CNetPerSecApp() {
	if (m_hMutex)
		::CloseHandle(m_hMutex);
}

// The one and only CNetPerSecApp object
CNetPerSecApp theApp;

// CNetPerSecApp initialization
BOOL CNetPerSecApp::InitInstance() {
	// Single instance
	m_hMutex = ::CreateMutex(NULL, FALSE, SZ_APPNAME);
	if (m_hMutex != NULL && ::GetLastError() == ERROR_ALREADY_EXISTS) {
		HWND hWnd = FindWindow(NULL, SZ_APPNAME);
		if (hWnd) {
			PostMessage(hWnd, TaskbarCallbackMsg, 0, WM_LBUTTONDBLCLK);
			SetForegroundWindow(hWnd);
		}
		return FALSE;
	}
	
	WSADATA WinsockData;
	if (WSAStartup(MAKEWORD(1, 1), &WinsockData) != 0)
		AfxMessageBox("This program requires Winsock 2.x", MB_ICONHAND);
	
	// Read in saved settings from .ini file
	ReadSettings();
	
	// Create a hidden window to receive system tray messages
	LPCTSTR pClass = AfxRegisterWndClass(0);
	CRect rc;
	rc.SetRectEmpty();
	
	if (m_wnd.CreateEx(WS_EX_TOOLWINDOW, pClass, SZ_APPNAME, WS_OVERLAPPED, rc, NULL, 0) == 0) {
		ShowError(IDS_CREATEWINDOW_ERR, MB_OK | MB_ICONHAND);
		return FALSE;  // Bail out
	}
	
	m_pMainWnd = &m_wnd;
	m_wnd.StartUp();
	WSACleanup();
	return TRUE;
}


void CNetPerSecApp::WinHelp(DWORD dwData, UINT nCmd) {
	m_wnd.WinHelp(dwData, nCmd);
}
