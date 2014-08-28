/* 
 * Shared functions and variables used by all modules.
 * Variables beginning with g+underscore are global.
 */

#include "StdAfx.h"
#include "NetPerSec.h"
#include <atlbase.h>


// Loaded via ini file
int g_nSampleRate;       // Milliseconds
int g_nAveragingWindow;  // Samples
int g_Range_Recv;        // For graph window (in Bps)
int g_Range_Sent;        // For graph window (in Bps)
int g_GraphOptions;
int g_DisplayBytes;
BOOL g_bStartWithWindows;
BOOL g_bOnTop;
BOOL g_bShowBarGraph;
BOOL g_bAutoScaleRecv;
BOOL g_bAutoScaleSent;
DWORD g_dwAdapter;
COLORREF g_ColorBack;
COLORREF g_ColorRecv;
COLORREF g_ColorSent;
COLORREF g_ColorAve ;
COLORREF g_ColorIconBack;
ICON_STYLE g_IconStyle;
MONITOR_MODE g_MonitorMode;

// The 16 VGA colors
COLORREF IconColors[MAX_ICON_COLORS] = {
	RGB(0xFF,0x00,0x00),
	RGB(0x00,0xFF,0x00),
	RGB(0xFF,0xFF,0x00),
	RGB(0x00,0x00,0xFF),
	RGB(0xFF,0x00,0xFF),
	RGB(0x00,0xFF,0xFF),
	RGB(0xFF,0xFF,0xFF),
	RGB(0xC0,0xC0,0xC0),
	RGB(0x80,0x00,0x00),
	RGB(0x00,0x80,0x00),
	RGB(0x80,0x80,0x00),
	RGB(0x00,0x00,0x80),
	RGB(0x80,0x00,0x80),
	RGB(0x00,0x80,0x80),
	RGB(0x80,0x80,0x80),
	RGB(0x00,0x00,0x00)
};


void ShowError(UINT nID, int nType) {
	AfxMessageBox(nID, nType);
}


// Returns the hex value of the current NT Service Pack
DWORD GetServicePack() {
	CRegKey key;
	#define SZ_SPKEY "System\\CurrentControlSet\\Control\\Windows"
	
	DWORD dwVersion = 0;
	if (key.Open(HKEY_LOCAL_MACHINE, SZ_SPKEY) == ERROR_SUCCESS) {
		key.QueryValue(dwVersion, "CSDVersion");
		key.Close();
	}
	return dwVersion;
}


// Creates a shortcut in the startup folder.
void SetStartupOptions() {
	// Required for Win95
	CoInitialize(NULL);
	
	// Create the COM server
	IShellLink *pShellLink = NULL;
	HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL,
			CLSCTX_INPROC_SERVER, IID_IShellLink,
			reinterpret_cast<LPVOID*>(&pShellLink));
	if (FAILED(hr))
		return;
	
	TCHAR szPath[MAX_PATH] = {0};
	GetModuleFileName(NULL, szPath, sizeof(szPath));
	GetShortPathName(szPath, szPath, sizeof(szPath));
	
	// Set attributes
	pShellLink->SetPath(szPath);
	pShellLink->SetDescription(SZ_APPNAME);
	pShellLink->SetHotkey(0);
	pShellLink->SetIconLocation(szPath, 0);
	
	// Get the IPersistFile interface to save
	IPersistFile *pPF = NULL;
	hr = pShellLink->QueryInterface(IID_IPersistFile, reinterpret_cast<LPVOID*>(&pPF));
	
	if (FAILED(hr)) {
		pShellLink->Release();
		return;
	}
	
	LPMALLOC pMalloc;
	if (SUCCEEDED(SHGetMalloc(&pMalloc))) {
		LPITEMIDLIST pidl;
		SHGetSpecialFolderLocation(NULL, CSIDL_STARTUP, &pidl);
		SHGetPathFromIDList(pidl, szPath);
		pMalloc->Free(pidl);
		pMalloc->Release();
	}
	
	// Create a .lnk file
	TCHAR szLinkFile[MAX_PATH] = {0};
	wsprintf(szLinkFile, "%s.lnk", SZ_APPNAME);
	
	if (szPath[lstrlen(szPath) - 1] != '\\')
		lstrcat(szPath, "\\");
	lstrcat(szPath, szLinkFile);
	
	if (g_bStartWithWindows) {
		// Save Unicode LNK file
		WCHAR wszLinkFile[MAX_PATH] = {0};
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szPath, -1, wszLinkFile, MAX_PATH);
		hr = pPF->Save(wszLinkFile, TRUE);
		if (FAILED(hr))
			ShowError(IDS_STARTUP_ERR, MB_ICONHAND);
	} else {
		DeleteFile(szPath);
	}
	
	// Clean up
	pPF->Release();
	pShellLink->Release();
	CoUninitialize();
}


// Format val into a string, the function will convert to bits if it is the default option
void FormatBytes(double val, CString &outStr, BOOL perSecond) {
	if (g_DisplayBytes != 0) {
		// Binary prefixes
		UINT GIGA = 1 << 30;
		UINT MEGA = 1 << 20;
		UINT KILO = 1 << 10;
		if      (val >= GIGA) outStr.Format("%.3f GiB", val / GIGA);
		else if (val >= MEGA) outStr.Format("%.2f MiB", val / MEGA);
		else if (val >= KILO) outStr.Format("%.2f KiB", val / KILO);
		else                  outStr.Format("%d bytes", (int)val);
		
	} else {  // Bits
		// Decimal prefixes
		UINT GIGA = 1000000000;
		UINT MEGA =    1000000;
		UINT KILO =       1000;
		val *= 8;
		if      (val >= GIGA) outStr.Format("%.3f Gbit", val / GIGA);
		else if (val >= MEGA) outStr.Format("%.2f Mbit", val / MEGA);
		else if (val >= KILO) outStr.Format("%.2f Kbit", val / KILO);
		else                  outStr.Format("%d bit", (int)val);
	}
	
	if (perSecond)
		outStr += "/s";
}


void QualifyPathName(CString *pFile, LPCSTR pIni) {
	// Qualify the INI file to the same location as our EXE
	char szName[MAX_PATH];
	GetModuleFileName(AfxGetInstanceHandle(), szName, sizeof(szName));
	LPSTR p = strrchr(szName, '\\');
	if (p != NULL)
		p[1] = '\0';
	strcat_s(szName, pIni);
	*pFile = szName;
}


int GetPrivateProfileInt(LPCSTR pKey, int nDefault) {
	CString sFileName;
	QualifyPathName(&sFileName, SZ_NETPERSEC_INI);
	return GetPrivateProfileInt(SZ_CONFIG, pKey, nDefault, sFileName);
}


int GetPrivateProfileString(LPCSTR pKey, LPCSTR lpDefault, LPSTR lpReturn, int nSize) {
	CString sFileName;
	QualifyPathName(&sFileName, SZ_NETPERSEC_INI);
	return GetPrivateProfileString(SZ_CONFIG, pKey, lpDefault, lpReturn, nSize, sFileName);
}


void WritePrivateProfileInt(LPCSTR pSection, int nValue) {
	char buf[256];
	wsprintf(buf, "%u", nValue);
	WritePrivateProfileString(pSection, buf);
}


void WritePrivateProfileString(LPCSTR pSection, LPCSTR pValue) {
	CString sFileName;
	QualifyPathName(&sFileName, SZ_NETPERSEC_INI);
	WritePrivateProfileString(SZ_CONFIG, pSection, pValue, sFileName);
}


void SaveWindowPosition(CRect &pRect) {
	WritePrivateProfileInt(SZ_WINPOS_TOP , pRect.top );
	WritePrivateProfileInt(SZ_WINPOS_LEFT, pRect.left);
}


void LoadWindowPosition(CRect &pRect) {
	pRect.top  = (int)GetPrivateProfileInt(SZ_WINPOS_TOP , -1);
	pRect.left = (int)GetPrivateProfileInt(SZ_WINPOS_LEFT, -1);
}


void ReadSettings() {
	g_nSampleRate       = GetPrivateProfileInt(SZ_SAMPLERATE, 1000);
	g_nAveragingWindow  = GetPrivateProfileInt(SZ_AVERAGEWINDOW, 10);
	g_Range_Recv        = GetPrivateProfileInt(SZ_RANGE_RECV, 1);
	g_Range_Sent        = GetPrivateProfileInt(SZ_RANGE_SENT, 1);
	g_GraphOptions      = GetPrivateProfileInt(SZ_GRAPHOPTIONS, -1);
	g_DisplayBytes      = GetPrivateProfileInt(SZ_DISPLAYBYTES, 0);
	g_bStartWithWindows = GetPrivateProfileInt(SZ_STARTUP, 0);
	g_bOnTop            = GetPrivateProfileInt(SZ_ONTOP, 0);
	g_bShowBarGraph     = GetPrivateProfileInt(SZ_BARGRAPH, 1);
	g_bAutoScaleRecv    = GetPrivateProfileInt(SZ_AUTOSCALE_RECV, 1);
	g_bAutoScaleSent    = GetPrivateProfileInt(SZ_AUTOSCALE_SENT, 1);
	g_IconStyle         = (ICON_STYLE)GetPrivateProfileInt(SZ_ICON_STYLE, ICON_HISTOGRAM);
	g_ColorBack         = GetPrivateProfileInt(SZ_COLOR_BACK, COLOR_BACK);
	g_ColorRecv         = GetPrivateProfileInt(SZ_COLOR_RECV, COLOR_ICON_RECV);
	g_ColorSent         = GetPrivateProfileInt(SZ_COLOR_SENT, COLOR_ICON_SENT);
	g_ColorAve          = GetPrivateProfileInt(SZ_COLOR_AVE , COLOR_AVERAGE);
	g_ColorIconBack     = GetPrivateProfileInt(SZ_COLOR_ICON, COLOR_ICON_BACK);
	g_MonitorMode       = (MONITOR_MODE)GetPrivateProfileInt(SZ_MONITOR_MODE, 0);
	g_dwAdapter         = GetPrivateProfileInt(SZ_ADAPTER_INDEX, 0);
}


void SaveSettings() {
	WritePrivateProfileInt(SZ_SAMPLERATE    , g_nSampleRate);
	WritePrivateProfileInt(SZ_AVERAGEWINDOW , g_nAveragingWindow);
	WritePrivateProfileInt(SZ_RANGE_RECV    , g_Range_Recv);
	WritePrivateProfileInt(SZ_RANGE_SENT    , g_Range_Sent);
	WritePrivateProfileInt(SZ_GRAPHOPTIONS  , g_GraphOptions);
	WritePrivateProfileInt(SZ_DISPLAYBYTES  , g_DisplayBytes);
	WritePrivateProfileInt(SZ_STARTUP       , g_bStartWithWindows);
	WritePrivateProfileInt(SZ_ONTOP         , g_bOnTop);
	WritePrivateProfileInt(SZ_BARGRAPH      , g_bShowBarGraph);
	WritePrivateProfileInt(SZ_AUTOSCALE_RECV, g_bAutoScaleRecv);
	WritePrivateProfileInt(SZ_AUTOSCALE_SENT, g_bAutoScaleSent);
	WritePrivateProfileInt(SZ_COLOR_BACK    , g_ColorBack);
	WritePrivateProfileInt(SZ_COLOR_RECV    , g_ColorRecv);
	WritePrivateProfileInt(SZ_COLOR_SENT    , g_ColorSent);
	WritePrivateProfileInt(SZ_COLOR_AVE     , g_ColorAve);
	WritePrivateProfileInt(SZ_COLOR_ICON    , g_ColorIconBack);
	WritePrivateProfileInt(SZ_ICON_STYLE    , g_IconStyle);
	WritePrivateProfileInt(SZ_MONITOR_MODE  , g_MonitorMode);
	WritePrivateProfileInt(SZ_ADAPTER_INDEX , g_dwAdapter);
	SetStartupOptions();
}
