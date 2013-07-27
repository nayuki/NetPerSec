/* 
 * About property page.
 */

#include "StdAfx.h"
#include "NetPerSec.h"
#include "About.h"

#pragma comment(lib, "version.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/* CAboutPage message handlers */

void InitDlg(HWND hDlg) {
	// Get version information from the application
	HINSTANCE hInst = AfxGetInstanceHandle();
	TCHAR szFullPath[256];
	GetModuleFileName(hInst, szFullPath, sizeof(szFullPath));
	DWORD dwVerHnd;
	DWORD dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
	
	if (dwVerInfoSize != 0) {
		// If we were able to get the information, process it
		HANDLE hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		LPVOID lpvMem = GlobalLock(hMem);
		GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpvMem);
		TCHAR szGetName[256];
		lstrcpy(szGetName, _T("\\StringFileInfo\\040904b0\\"));
		int cchRoot = lstrlen(szGetName);
		
		// Walk through the dialog items that we want to replace
		for (int i = 0; i < 3; i++) {
			WORD wID;
			if (i == 0)
				wID = IDC_COPYRIGHT;
			else if (i == 1)
				wID = IDC_VERSION;
			else
				wID = 0;
			
			if (wID != 0) {
				TCHAR szResult[256];
				::GetDlgItemText(hDlg, wID, szResult, sizeof(szResult));
				lstrcpy(&szGetName[cchRoot], szResult);
				LPTSTR lszVer = NULL;
				UINT cchVer = 0;
				BOOL fRet = VerQueryValue(lpvMem, szGetName, (void**)&lszVer, &cchVer);
				
				if (fRet && cchVer != 0 && lszVer != NULL) {
					// Replace dialog item text with version info
					lstrcpy(szResult, lszVer);
					::SetDlgItemText(hDlg, wID, szResult);
				}
			}
		}
		GlobalUnlock(hMem);
		GlobalFree(hMem);
	}
	return;
}


// CAboutPage property page
IMPLEMENT_DYNCREATE(CAboutPage, CPropertyPage)


CAboutPage::CAboutPage() : CPropertyPage(CAboutPage::IDD) {
	//{{AFX_DATA_INIT(CAbout)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CAboutPage::~CAboutPage() {}

void CAboutPage::DoDataExchange(CDataExchange *pDX) {
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


BOOL CAboutPage::OnInitDialog() {
	CDialog::OnInitDialog();
	InitDlg(GetSafeHwnd());
	return TRUE;
}
