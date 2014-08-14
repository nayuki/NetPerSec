/* 
 * Main property sheet.
 */

#include "StdAfx.h"
#include "NetPerSec.h"
#include "DlgPropSheet.h"
#include "About.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int m_nLastTab = 0;

// DlgPropSheet

IMPLEMENT_DYNAMIC(DlgPropSheet, CPropertySheet)


DlgPropSheet::DlgPropSheet(UINT nIDCaption, CWnd *pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage) {
	AddPropPages();
}

DlgPropSheet::DlgPropSheet(LPCTSTR pszCaption, CWnd *pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage) {
	AddPropPages();
}

DlgPropSheet::~DlgPropSheet() {
	delete m_pSessionDlg;
	delete m_pOptionsDlg;
	delete m_pDisplayDlg;
	delete m_pAboutDlg;
}


BEGIN_MESSAGE_MAP(DlgPropSheet, CPropertySheet)
	//{{AFX_MSG_MAP(DlgPropSheet)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/* DlgPropSheet message handlers */

void DlgPropSheet::AddPropPages() {
	m_pSessionDlg = new CSessionDlg;
	m_pOptionsDlg = new COptionsDlg;
	m_pDisplayDlg = new CDisplayDlg;
	m_pAboutDlg  = new CAboutPage;
	
	AddPage(m_pSessionDlg);
	AddPage(m_pOptionsDlg);
	AddPage(m_pDisplayDlg);
	AddPage(m_pAboutDlg);
	
	SetActivePage(m_nLastTab);
}


void DlgPropSheet::OnPaint() {
	CPaintDC dc(this);  // Device context for painting
	// Do not call CPropertySheet::OnPaint() for painting messages
}

BOOL DlgPropSheet::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pLResult) {
	// Save last tab
	m_nLastTab = GetActiveIndex();
	return CPropertySheet::OnChildNotify(message, wParam, lParam, pLResult);
}


BOOL DlgPropSheet::OnInitDialog() {
	BOOL bResult = CPropertySheet::OnInitDialog();
	
	HICON m_hIcon = AfxGetApp()->LoadIcon(IDI_MAINFRAME);
	SetIcon(m_hIcon, TRUE );  // Set big icon
	SetIcon(m_hIcon, FALSE);  // Set small icon
	
	// Position the dialog
	CRect rc;
	GetClientRect(rc);
	LoadWindowPosition(rc);
	BOOL bCenter = FALSE;
	if (rc.left < 0 || rc.left + rc.right >= GetSystemMetrics(SM_CXFULLSCREEN))
		bCenter = TRUE;
	
	if (rc.top < 0 || rc.top + rc.bottom >= GetSystemMetrics(SM_CYFULLSCREEN))
		bCenter = TRUE;
	
	if (bCenter) {
		GetWindowRect(rc);
		rc.OffsetRect(-rc.left, -rc.top);
		MoveWindow(((GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2 + 4) & ~7,
		            (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2, rc.right, rc.bottom, 0);
	} else {
		SetWindowPos(NULL, rc.left, rc.top, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);
	}
	
	if (g_bOnTop)
		SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
	
	return bResult;
}


void DlgPropSheet::OnDestroy() {
	CRect rc;
	GetWindowRect(rc);
	SaveWindowPosition(rc);
	theApp.m_pMainWnd = &theApp.m_wnd;
	CPropertySheet::OnDestroy();
}


void DlgPropSheet::PostNcDestroy() {
	// Restore MFC's main window handle
	theApp.m_pMainWnd = &theApp.m_wnd;
	CPropertySheet::PostNcDestroy();
}


LRESULT DlgPropSheet::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == WM_SIZE && wParam == SIZE_MINIMIZED) {
		SaveSettings();
		PostMessage(WM_CLOSE, 0, 0);
	}
	return CPropertySheet::DefWindowProc(message, wParam, lParam);
}


int DlgPropSheet::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	ModifyStyle(DS_CONTEXTHELP | DS_MODALFRAME, WS_MINIMIZEBOX);
	// Configure our system menu
	GetSystemMenu(TRUE);  // Revert the menu
	CMenu &pSysMenu = *GetSystemMenu(FALSE);  // And grab the handle
	pSysMenu.EnableMenuItem(SC_MINIMIZE, MF_ENABLED);
	pSysMenu.DeleteMenu(SC_MAXIMIZE, MF_BYCOMMAND);
	pSysMenu.DeleteMenu(SC_SIZE, MF_BYCOMMAND);
	pSysMenu.EnableMenuItem(SC_RESTORE, MF_DISABLED | MF_GRAYED);
	
	return 0;
}
