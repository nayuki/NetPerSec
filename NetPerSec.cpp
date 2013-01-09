/*=========================================================================*/
/*                          NETPERSEC.CPP                                  */
/*                                                                         */
/*           Creates a hidden window to manage the system tray             */
/*           icons. see winproc.cpp.                                       */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/*                   NetPerSec 1.1 Copyright (c) 2000                      */
/*                      Ziff Davis Media, Inc.							   */
/*                       All rights reserved.							   */
/*																		   */
/*                     Programmed by Mark Sweeney                          */
/*=========================================================================*/

#include "stdafx.h"
#include "NetPerSec.h"
#include "winsock2.h"
#pragma comment(lib, "Ws2_32.lib" )

CNetPerSecApp* pTheApp;

//private message for the taskbar
extern UINT TaskbarCallbackMsg;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNetPerSecApp

BEGIN_MESSAGE_MAP(CNetPerSecApp, CWinApp)
	//{{AFX_MSG_MAP(CNetPerSecApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNetPerSecApp construction
CNetPerSecApp::CNetPerSecApp()
{
	pTheApp = this;
	m_hMutex = NULL;
}


CNetPerSecApp::~CNetPerSecApp()
{
	if(m_hMutex)
		::CloseHandle(m_hMutex);
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CNetPerSecApp object
CNetPerSecApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CNetPerSecApp initialization
BOOL CNetPerSecApp::InitInstance()
{
	//single instance
	m_hMutex = ::CreateMutex(NULL, FALSE, SZ_APPNAME);
	if( m_hMutex != NULL )
	{
		if(::GetLastError()==ERROR_ALREADY_EXISTS)
		{
			HWND hWnd = FindWindow(NULL, SZ_APPNAME);
			if( hWnd ) {
				PostMessage( hWnd, TaskbarCallbackMsg ,0 ,WM_LBUTTONDBLCLK );
				SetForegroundWindow( hWnd );
			}
			return FALSE;
		}
	}
	
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
	
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
	
	WSADATA WinsockData;
	if( WSAStartup(MAKEWORD(1,1), &WinsockData) != 0)
	{
		AfxMessageBox("This program requires Winsock 2.x", MB_ICONHAND );
	}
	
	//read in saved settings from ini file
	ReadSettings( );
	
	//create a hidden window to receive system tray messages
	LPCTSTR pClass = AfxRegisterWndClass( 0 );
	CRect rc;
	rc.SetRectEmpty( );
	
	if( m_wnd.CreateEx( WS_EX_TOOLWINDOW, pClass, SZ_APPNAME, WS_OVERLAPPED, rc, NULL, 0) == 0 )
	{
		ShowError( IDS_CREATEWINDOW_ERR, MB_OK | MB_ICONHAND );
		return FALSE;	//bail out
	}
	
	m_pMainWnd = &m_wnd;
	m_wnd.StartUp( );
	
	WSACleanup( );
	
	return TRUE;
}


void CNetPerSecApp::WinHelp(DWORD dwData, UINT nCmd)
{
	m_wnd.WinHelp( dwData, nCmd );
}
