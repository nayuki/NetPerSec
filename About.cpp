/*=========================================================================*/
/*                           ABOUT.CPP                                     */
/*                                                                         */
/*                       About property page.                              */
/*                                                                         */
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
#include "About.h"

#pragma comment(lib, "version.lib" )

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CAboutPage message handlers


void InitDlg( HWND hDlg )
{
	TCHAR szFullPath[256];
	DWORD dwVerHnd;
	DWORD dwVerInfoSize;
	HINSTANCE hInst = AfxGetInstanceHandle( );
	
	// Get version information from the application
	GetModuleFileName( hInst, szFullPath, sizeof(szFullPath) );
	dwVerInfoSize = GetFileVersionInfoSize( szFullPath, &dwVerHnd );
	
	if( dwVerInfoSize )
	{
		// If we were able to get the information, process it:
		HANDLE  hMem;
		LPVOID  lpvMem;
		TCHAR   szGetName[256];
		int     cchRoot;
		int     i;
		
		hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		lpvMem = GlobalLock(hMem);
		GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpvMem);
		lstrcpy(szGetName, _T("\\StringFileInfo\\040904b0\\") );
		cchRoot = lstrlen( szGetName );
		
		// Walk through the dialog items that we want to replace:
		for( i = 0; i < 3; i++ )
		{
			BOOL  fRet;
			UINT  cchVer = 0;
			LPTSTR lszVer = NULL;
			TCHAR  szResult[256];
			WORD  wID = 0;
			
			switch( i )
			{
				case 0:
					wID = IDC_COPYRIGHT;
					break;
				case 1:
					wID = IDC_VERSION;
					break;
			}
			
			if( wID )
			{
				::GetDlgItemText(hDlg, wID, szResult, sizeof(szResult));
				lstrcpy( &szGetName[cchRoot], szResult );
				fRet = VerQueryValue( lpvMem, szGetName, (void**)&lszVer, &cchVer );
				
				if( fRet && cchVer && lszVer )
				{
					// Replace dialog item text with version info
					lstrcpy( szResult, lszVer );
					::SetDlgItemText(hDlg, wID, szResult );
				}
			}
		}
		GlobalUnlock( hMem );
		GlobalFree( hMem );
	}
	
	return;
	
}


/////////////////////////////////////////////////////////////////////////////
// CAboutPage property page

IMPLEMENT_DYNCREATE(CAboutPage, CPropertyPage)


///////////////////////////////////////////////////////////////////////////////////////////
//
CAboutPage::CAboutPage( ) : CPropertyPage(CAboutPage::IDD )
{
	//{{AFX_DATA_INIT(CAbout)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

///////////////////////////////////////////////////////////////////////////////////////////
//
CAboutPage::~CAboutPage()
{
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void CAboutPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAboutPage, CPropertyPage)
	//{{AFX_MSG_MAP(CAboutPage)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////////////////////////
//
BOOL CAboutPage::OnInitDialog( )
{
	CDialog::OnInitDialog();
	
	InitDlg( GetSafeHwnd( ) );
	return( TRUE );
}
