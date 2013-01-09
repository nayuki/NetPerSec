/*=========================================================================*/
/*                            GLOBAL.CPP                                   */
/*                                                                         */
/*         Shared functions and variables used by all modules.  Variables  */
/*         beginning with a g_ underscore are global.                      */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/*                   NetPerSec 1.0 Copyright (c) 2000                      */
/*                      Ziff Davis Media, Inc.							   */
/*                       All rights reserved.							   */
/*																		   */
/*                     Programmed by Mark Sweeney                          */
/*=========================================================================*/
#include "stdafx.h"
#include "NetPerSec.h"
#include <atlbase.h>

//loaded via ini file
int g_nSampleRate;       //milliseconds
int g_nAveragingWindow;  //seconds
int g_Range_Recv;        //for graph window (in Bps)
int g_Range_Sent;        //for graph window (in Bps)
int g_GraphOptions;
int g_DisplayBytes;
BOOL g_bStartWithWindows;
BOOL g_bOnTop;
BOOL g_bShowBarGraph;
BOOL g_bAutoScaleRecv;
BOOL g_bAutoScaleSent;
double g_dbResetRecv;   //when user clicks reset these
double g_dbResetSent;   //values are subtracted from the total
DWORD g_dwAdapter;
COLORREF g_ColorBack;
COLORREF g_ColorRecv;
COLORREF g_ColorSent;
COLORREF g_ColorAve ;
COLORREF g_ColorIconBack;
ICON_STYLE g_IconStyle;
MONITOR_MODE g_MonitorMode;

COLORREF IconColors[MAX_ICON_COLORS] =
{
	RGB( 0xFF, 0x00, 0x00 ),
	RGB( 0x00, 0xFF, 0x00 ),
	RGB( 0xFF, 0xFF, 0x00 ),
	RGB( 0x00, 0x00, 0xFF ),
	RGB( 0xFF, 0x00, 0xFF ),
	RGB( 0x00, 0xFF, 0xFF ),
	RGB( 0xFF, 0xFF, 0xFF ),
	RGB( 0xC0, 0xC0, 0xC0 ),
	RGB( 0x80, 0x00, 0x00 ),
	RGB( 0x00, 0x80, 0x00 ),
	RGB( 0x80, 0x80, 0x00 ),
	RGB( 0x00, 0x00, 0x80 ),
	RGB( 0x80, 0x00, 0x80 ),
	RGB( 0x00, 0x80, 0x80 ),
	RGB( 0x80, 0x80, 0x80 ),
	RGB( 0x00, 0x00, 0x00 )
};


///////////////////////////////////////////////////////////////////////////////////////////
//
void ShowError( UINT nID, int nType )
{
	AfxMessageBox( nID, nType );
}


///////////////////////////////////////////////////////////////////////////////////////////
// GetServicePack -- returns the hex value of the current NT Service Pack
DWORD GetServicePack( )
{
	CRegKey key;
	#define SZ_SPKEY "System\\CurrentControlSet\\Control\\Windows"
	
	DWORD dwVersion = 0;
	if( key.Open( HKEY_LOCAL_MACHINE, SZ_SPKEY ) == ERROR_SUCCESS )
	{
		key.QueryValue( dwVersion, "CSDVersion" );
		key.Close( );
	}
	
	return dwVersion;
}


/////////////////////////////////////////////////////////////////////////////
//  SetStartupOptions( )
//  creates a shortcut in the startup folder.
//
void SetStartupOptions( )
{
	TCHAR szPath[MAX_PATH] = {0};
	TCHAR szLinkFile[MAX_PATH] = {0};
	WCHAR wszLinkFile[MAX_PATH] = {0};
	LPITEMIDLIST pidl;
	LPMALLOC pMalloc;
	
	IShellLink* pShellLink = NULL;
	IPersistFile* pPF = NULL;
	
	//required for Win95
	CoInitialize( NULL );
	
	//Create the COM server
	HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL,
			CLSCTX_INPROC_SERVER, IID_IShellLink,
			reinterpret_cast<LPVOID*>(&pShellLink));
	
	if( FAILED( hr ) )
	{
		return;
	}
	
	GetModuleFileName( NULL, szPath, sizeof(szPath) );
	GetShortPathName( szPath, szPath, sizeof(szPath) );
	
	// Set attributes
	pShellLink->SetPath( szPath );
	pShellLink->SetDescription( SZ_APPNAME );
	pShellLink->SetHotkey( 0 );
	pShellLink->SetIconLocation( szPath, 0 );
	
	// Get the IPersistFile interface to save
	hr = pShellLink->QueryInterface(IID_IPersistFile, reinterpret_cast<LPVOID*>(&pPF));
	
	if(FAILED(hr))
	{
		pShellLink->Release();
		return;
	}
	
	if( SUCCEEDED( SHGetMalloc( &pMalloc ) ) )
	{
		SHGetSpecialFolderLocation( NULL, CSIDL_STARTUP, &pidl );
		SHGetPathFromIDList( pidl, szPath );
		pMalloc->Free( pidl );
		pMalloc->Release( );
	}
	
	//create a .lnk file
	wsprintf( szLinkFile, "%s.lnk", SZ_APPNAME );
	
	if( szPath[lstrlen(szPath) -1] != '\\' )
		lstrcat( szPath, "\\" ) ;
	lstrcat( szPath, szLinkFile );
	
	if( g_bStartWithWindows )
	{
		// Save Unicode LNK file
		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szPath, -1, wszLinkFile, MAX_PATH );
		hr = pPF->Save( wszLinkFile, TRUE );
		if( FAILED( hr ) )
		{
			ShowError( IDS_STARTUP_ERR, MB_ICONHAND );
		}
		
	} else {
		DeleteFile( szPath );
	}
	
	//Clean up
	pPF->Release( );
	pShellLink->Release( );
	
	CoUninitialize( );
}

///////////////////////////////////////////////////////////////////////////////////////////
//  Format BYTES into a string,  the function will convert to bits if it is the default option
void FormatBytes( double dbBytes, CString *pString, BOOL bPerSecond /* bPerSecond = TRUE */ )
{
	static char s[256];
	char ch;
	char* b = "Bytes";
	double num = dbBytes;
	
	//decimal format
	#define GIGABITS 1000 * 1000 * 1000
	#define MEGABITS 1000 * 1000
	#define KILOBITS 1000
	
	//binary format
	#define GIGABYTE 1024 * 1024 * 1024
	#define MEGABYTE 1024 * 1024
	#define KILOBYTE 1024
	
	UINT GIGA = GIGABYTE;
	UINT MEGA = MEGABYTE;
	UINT KILO = KILOBYTE;
	
	//convert to bits
	if( g_DisplayBytes == 0 )
	{
		num *= 8;
		b = "bits";
		GIGA = GIGABITS;
		MEGA = MEGABITS;
		KILO = KILOBITS;
	}
	
	if( num >= GIGA )
	{
		sprintf( s, "%.1f", ( (double)num / (double)(GIGA) ) );
		*pString = s;
		ch = 'G';
	} else {
		if ( num >= MEGA )
		{
			sprintf( s, "%.1f", ( (double)num / (double)(MEGA) ) );
			ch = 'M';
		} else {
			if ( num >= KILO )
			{
				sprintf( s, "%.1f", ( (double)num / (double)(KILO) ) );
				*pString = s;
				if( g_DisplayBytes )
					ch = 'K';
				else
					ch = 'k';
			} else {
				sprintf( s, "%g", num );
				*pString = s;
				ch = ' ';
			}
		}
	}
	
	if( bPerSecond )
		pString->Format( "%s %c%s/s", s, ch, b );
	else
		pString->Format( "%s %c%s", s, ch, b );
}


///////////////////////////////////////////////////////////////////////////////////////////
//  returns a comma formatted number
LPSTR FormatNumber( DWORD N )
{
	#define BUF_SIZE 128
	static char buf[BUF_SIZE+1];
	
	int len = 1, posn = 1, sign = 1;
	char *ptr = buf + BUF_SIZE - 1;
	*ptr-- = NULL;
	
	for ( ; len <= BUF_SIZE; ++len, ++posn)
	{
		*ptr-- = (char)((N % 10L) + '0');
		if (0L == (N /= 10L))
			break;
		
		if (0 == (posn % 3))
		{
			*ptr-- = ',';
			++len;
		}
		
		if (len >= BUF_SIZE)
			return "";
	}
	
	if (0 > sign)
	{
		if (len >= BUF_SIZE)
		{
			return "";
		}
		
		*ptr-- = '-';
		++len;
	}
	
	memmove(buf, ++ptr, len + 1);
	return buf;
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void QualifyPathName( CString *pFile, LPCSTR pIni )
{
	char szName[MAX_PATH];
	LPSTR p;
	
	//Qualify the INI file to the same location as our exe
	GetModuleFileName( AfxGetInstanceHandle( ), szName, sizeof( szName ) );
	p = strrchr( szName, '\\' );
	
	if( p )
		*(++p) = 0;
	else
		p = szName;
	
	strcat( p, pIni );
	*pFile = szName;
}

///////////////////////////////////////////////////////////////////////////////////////////
//
int GetPrivateProfileInt( LPCSTR pKey, int nDefault )
{
	CString sFileName;
	QualifyPathName( &sFileName, SZ_NETPERSEC_INI );
	return GetPrivateProfileInt( SZ_CONFIG, pKey, nDefault, sFileName );
}

///////////////////////////////////////////////////////////////////////////////////////////
//
int GetPrivateProfileString( LPCSTR pKey,LPCSTR lpDefault, LPSTR lpReturn, int nSize )
{
	CString sFileName;
	QualifyPathName( &sFileName, SZ_NETPERSEC_INI );
	return GetPrivateProfileString( SZ_CONFIG, pKey, lpDefault, lpReturn, nSize, sFileName );
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void WritePrivateProfileInt( LPCSTR pSection, int nValue )
{
	CString sFileName;
	char buf[256];
	QualifyPathName( &sFileName, SZ_NETPERSEC_INI );
	wsprintf( buf, "%u", nValue );
	WritePrivateProfileString( SZ_CONFIG, pSection, buf, sFileName );
}


///////////////////////////////////////////////////////////////////////////////////////////
//
void WritePrivateProfileString( LPCSTR pSection, LPCSTR pValue )
{
	CString sFileName;
	QualifyPathName( &sFileName, SZ_NETPERSEC_INI );
	WritePrivateProfileString( SZ_CONFIG, pSection, pValue, sFileName );
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void SaveWindowPosition( CRect *pRect )
{
	WritePrivateProfileInt( SZ_WINPOS_TOP, pRect->top );
	WritePrivateProfileInt( SZ_WINPOS_LEFT, pRect->left );
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void LoadWindowPosition( CRect *pRect )
{
	pRect->top = (int)GetPrivateProfileInt( SZ_WINPOS_TOP, -1 );
	pRect->left = (int)GetPrivateProfileInt( SZ_WINPOS_LEFT, -1 );
}


///////////////////////////////////////////////////////////////////////////////////////////
//
void ReadSettings( )
{
	g_nSampleRate = GetPrivateProfileInt( SZ_SAMPLERATE, 1000 );
	g_nAveragingWindow = GetPrivateProfileInt( SZ_AVERAGEWINDOW, 30 );
	g_Range_Recv = GetPrivateProfileInt( SZ_RANGE_RECV, 1 );
	g_Range_Sent = GetPrivateProfileInt( SZ_RANGE_SENT, 1 );
	g_GraphOptions = GetPrivateProfileInt( SZ_GRAPHOPTIONS, -1 );
	g_DisplayBytes = GetPrivateProfileInt( SZ_DISPLAYBYTES, 0 );
	g_bStartWithWindows = GetPrivateProfileInt( SZ_STARTUP, 0 );
	g_bOnTop = GetPrivateProfileInt( SZ_ONTOP, 0 );
	g_bShowBarGraph =  GetPrivateProfileInt( SZ_BARGRAPH,1 );
	g_bAutoScaleRecv = GetPrivateProfileInt( SZ_AUTOSCALE_RECV,1 );
	g_bAutoScaleSent = GetPrivateProfileInt( SZ_AUTOSCALE_SENT,1 );
	g_IconStyle = (ICON_STYLE)GetPrivateProfileInt( SZ_ICON_STYLE, ICON_HISTOGRAM );
	g_ColorBack = GetPrivateProfileInt( SZ_COLOR_BACK, COLOR_ICON_BACK );
	g_ColorRecv = GetPrivateProfileInt( SZ_COLOR_RECV, COLOR_ICON_RECV );
	g_ColorSent = GetPrivateProfileInt( SZ_COLOR_SENT, COLOR_ICON_SENT );
	g_ColorAve  = GetPrivateProfileInt( SZ_COLOR_AVE , COLOR_AVERAGE );
	g_ColorIconBack = GetPrivateProfileInt( SZ_COLOR_ICON, COLOR_ICON_BACK );
	g_MonitorMode = (MONITOR_MODE)GetPrivateProfileInt( SZ_MONITOR_MODE, 0 );
	g_dwAdapter = GetPrivateProfileInt( SZ_ADAPTER_INDEX, 0 );
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void SaveSettings( )
{
	WritePrivateProfileInt( SZ_SAMPLERATE, g_nSampleRate );
	WritePrivateProfileInt( SZ_AVERAGEWINDOW, g_nAveragingWindow );
	WritePrivateProfileInt( SZ_RANGE_RECV, g_Range_Recv);
	WritePrivateProfileInt( SZ_RANGE_SENT, g_Range_Sent);
	WritePrivateProfileInt( SZ_GRAPHOPTIONS, g_GraphOptions );
	WritePrivateProfileInt( SZ_DISPLAYBYTES, g_DisplayBytes );
	WritePrivateProfileInt( SZ_STARTUP, g_bStartWithWindows );
	WritePrivateProfileInt( SZ_ONTOP, g_bOnTop );
	WritePrivateProfileInt( SZ_BARGRAPH, g_bShowBarGraph );
	WritePrivateProfileInt( SZ_AUTOSCALE_RECV, g_bAutoScaleRecv );
	WritePrivateProfileInt( SZ_AUTOSCALE_SENT, g_bAutoScaleSent );
	WritePrivateProfileInt( SZ_COLOR_BACK, g_ColorBack );
	WritePrivateProfileInt( SZ_COLOR_RECV, g_ColorRecv );
	WritePrivateProfileInt( SZ_COLOR_SENT, g_ColorSent );
	WritePrivateProfileInt( SZ_COLOR_AVE , g_ColorAve );
	WritePrivateProfileInt( SZ_COLOR_ICON, g_ColorIconBack );
	WritePrivateProfileInt( SZ_ICON_STYLE, g_IconStyle );
	WritePrivateProfileInt( SZ_MONITOR_MODE, g_MonitorMode );
	WritePrivateProfileInt( SZ_ADAPTER_INDEX, g_dwAdapter );
	
	SetStartupOptions( );
}
