/*=========================================================================*/
/*                        SESSIONDLG.CPP                                   */
/*                                                                         */
/*                 Implements the graphs property page and displays        */
/*                 statistics about the current session.                   */
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
#include "SessionDlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//which graph to display -- bps and/or average
enum {
    OPTION_BPS = 0x01,
    OPTION_AVE = 0x02,
} OPTIONS;


//this is for the line graph, the graphing functions need to save the previous
//graph end points for the line
enum {
    LINEGRAPH_BPS = 0,
    LINEGRAPH_AVE,
    LINE_GRAPH_SIZE,
} LINE_GRAPH;

//array of bps values for the sliders
//divide by 8 since the formatting routine will multiply by 8 to convert to bits
static UINT bpsArray[] = {
    14400 / 8,
    28800 / 8,
    33600 / 8,
    56000 / 8,
    70000 / 8,
    80000 / 8,
    90000 / 8,
   100000 / 8,
   125000 / 8,
   150000 / 8,
   200000 / 8,
   300000 / 8,
   500000 / 8,
  1000000 / 8,
  1500000 / 8,
  2000000 / 8,
 10000000 / 8 };


/////////////////////////////////////////////////////////////////////////////
// CSessionDlg property page

IMPLEMENT_DYNCREATE(CSessionDlg, CPropertyPage)

CSessionDlg::CSessionDlg() : CPropertyPage(CSessionDlg::IDD)
{
	//{{AFX_DATA_INIT(CSessionDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

    m_pbrBackground = 0;

}

///////////////////////////////////////////////////////////////////////////////////////////
//
CSessionDlg::~CSessionDlg()
{
    if( m_pbrBackground )
        delete m_pbrBackground;
    m_pbrBackground = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void CSessionDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSessionDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSessionDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CSessionDlg)
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_AVE_RECV_OPTION, OnAveRecvOption)
	ON_BN_CLICKED(IDC_AVE_SENT_OPTION, OnAveSentOption)
	ON_BN_CLICKED(IDC_CURRENT_RECV_OPTION, OnCurrentRecvOption)
	ON_BN_CLICKED(IDC_CURRENT_SENT_OPTION, OnCurrentSentOption)
	ON_BN_CLICKED(IDC_MAX_RECV_OPTION, OnMaxRecvOption)
	ON_BN_CLICKED(IDC_MAX_SENT_OPTION, OnMaxSentOption)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BARGRAPH, OnBargraph)
	ON_BN_CLICKED(IDC_LINEGRAPH, OnLinegraph)
	ON_BN_CLICKED(IDC_RESET_DATA, OnResetData)
	ON_BN_CLICKED(IDC_AUTOSCALE_SENT, OnAutoscale)
	ON_BN_CLICKED(IDC_BPS, OnBps)
	ON_BN_CLICKED(IDC_BYTES, OnBytes)
    ON_BN_CLICKED(IDC_AUTOSCALE_RECV, OnAutoscale)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSessionDlg message handlers

BOOL CSessionDlg::PreTranslateMessage(MSG* pMsg)
{

    //show the color dialog if a graph is right clicked
    if( pMsg->message == WM_RBUTTONUP )
    {
        WORD wID = (WORD)GetWindowLong( pMsg->hwnd, GWL_ID );
        if( wID == IDC_RECV_GRAPH_WINDOW || wID == IDC_SENT_GRAPH_WINDOW )
        {
            pTheApp->m_wnd.m_pPropertiesDlg->SetActivePage( 2 );
            return( TRUE );
        }
    }

	return CPropertyPage::PreTranslateMessage(pMsg);
}


///////////////////////////////////////////////////////////////////////////////////////////
//
BOOL CSessionDlg::OnSetActive()
{
    if( m_pbrBackground )
        delete m_pbrBackground;

    m_pbrBackground = new CBrush( g_ColorBack );

    SetTimer( TIMER_ID_SESSION, g_nSampleRate, NULL );

    UpdateDlg( );
    UpdateGraph( );
    SetGraphRangeRecv( );
    SetGraphRangeSent( );
	return CPropertyPage::OnSetActive();
}



/////////////////////////////////////////////////////////////////////////////
// display a formatted number
void CSessionDlg::DisplayNumber( int nID, DWORD dwBytes)
{
    CString s;
   
    FormatBytes( dwBytes, &s );
    SetDlgItemText( nID, s );
}

/////////////////////////////////////////////////////////////////////////////
//
DWORD CSessionDlg::CalcMax(STATS_STRUCT* pStats, int index )
{
	int total = m_SentGraph.GetTotalElements( );

    //calc max
    DWORD dwMax = 0;
    
    for( int i = 0; i < total; i++ )
    {
        if( index < 0 )
            index = MAX_SAMPLES-1;

        if( pStats[index].Bps > dwMax )
            dwMax = pStats[index].Bps;
        index--;
    }

    return( dwMax );
}

/////////////////////////////////////////////////////////////////////////////
//
void CSessionDlg::UpdateDlg( )
{
    int i = pTheApp->m_wnd.GetArrayIndex( );

    DisplayNumber( IDC_RECV_CURRENT, pTheApp->m_wnd.RecvStats[i].Bps );
    DisplayNumber( IDC_RECV_AVERAGE, pTheApp->m_wnd.RecvStats[i].ave );

    DisplayNumber( IDC_SENT_CURRENT, pTheApp->m_wnd.SentStats[i].Bps );
    DisplayNumber( IDC_SENT_AVERAGE, pTheApp->m_wnd.SentStats[i].ave );

    DisplayNumber( IDC_RECV_MAXIMUM, CalcMax( pTheApp->m_wnd.RecvStats, i ) );
    DisplayNumber( IDC_SENT_MAXIMUM, CalcMax( pTheApp->m_wnd.SentStats, i ) );
}


/////////////////////////////////////////////////////////////////////////////
// update the total sent and received amounts
void CSessionDlg::OnTimer(UINT /* nIDEvent */)
{
    UpdateDlg( );
    UpdateGraph( );

    CString s;
    FormatBytes( (pTheApp->m_wnd.m_dbTotalBytesRecv - g_dbResetRecv ), &s, FALSE );
    SetDlgItemText( IDC_GROUP_RECV, "Received: " + s );

    FormatBytes( (pTheApp->m_wnd.m_dbTotalBytesSent - g_dbResetSent), &s, FALSE );
    SetDlgItemText( IDC_GROUP_SENT, "Sent: " + s );

}



/////////////////////////////////////////////////////////////////////////////
//
BOOL CSessionDlg::OnKillActive()
{
    if( m_pbrBackground )
        delete m_pbrBackground;
    m_pbrBackground = 0;

    KillTimer( TIMER_ID_SESSION );
	return CPropertyPage::OnKillActive();
}


/////////////////////////////////////////////////////////////////////////////
//
BOOL CSessionDlg::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	
	CRect rect;
	GetDlgItem(IDC_SENT_GRAPH)->GetWindowRect(rect);
	ScreenToClient(rect);
	m_SentGraph.Create( WS_VISIBLE | WS_CHILD, rect, this, IDC_SENT_GRAPH_WINDOW );

	GetDlgItem(IDC_RECV_GRAPH)->GetWindowRect(rect);
	ScreenToClient(rect);
	m_RecvGraph.Create( WS_VISIBLE | WS_CHILD, rect, this, IDC_RECV_GRAPH_WINDOW );
   
    m_RecvGraph.SetSize( LINE_GRAPH_SIZE );
    m_SentGraph.SetSize( LINE_GRAPH_SIZE );
    
    m_AutoScale_Recv = 0;
    m_AutoScale_Sent = 0;
  
    CheckDlgButton( IDC_AUTOSCALE_RECV, g_bAutoScaleRecv );
    CheckDlgButton( IDC_AUTOSCALE_SENT, g_bAutoScaleSent );

	CheckDlgButton( IDC_BYTES, g_DisplayBytes );
	CheckDlgButton( IDC_BPS, g_DisplayBytes == 0 );

    OnAutoscale();

    CSliderCtrl* pCtrl = (CSliderCtrl*)GetDlgItem(IDC_SCALE_SLIDER_RECV);
    pCtrl->SetRange( 0, ELEMENTS(bpsArray)-1 );
    pCtrl->SetTicFreq( 1 );
	pCtrl->SetPos( g_Range_Recv );

    CSliderCtrl* pCtrl2 = (CSliderCtrl*)GetDlgItem(IDC_SCALE_SLIDER_SENT);
    pCtrl2->SetRange( 0, ELEMENTS(bpsArray)-1 );
    pCtrl2->SetTicFreq( 1 );
	pCtrl2->SetPos( g_Range_Sent );

    CheckDlgButton( IDC_BARGRAPH, g_bShowBarGraph != 0 );
    CheckDlgButton( IDC_LINEGRAPH, g_bShowBarGraph == 0 );
    m_SentGraph.SetStyle( g_bShowBarGraph );
    m_RecvGraph.SetStyle( g_bShowBarGraph );

    UpdateGraphTextRecv( bpsArray[g_Range_Recv] );
    UpdateGraphTextSent( bpsArray[g_Range_Sent] );

	CheckDlgButton( IDC_CURRENT_RECV_OPTION, g_GraphOptions & OPTION_BPS );
	CheckDlgButton( IDC_AVE_RECV_OPTION, g_GraphOptions & OPTION_AVE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


/////////////////////////////////////////////////////////////////////////////
// draw the graphs
void CSessionDlg::DrawGraph( int nIndex, UPDATE_MODE update )
{

    if ( update & RECV_DATA )
    {
        m_RecvGraph.ShiftLeft( );
    
        if( g_GraphOptions & OPTION_AVE )
            m_RecvGraph.SetPos( pTheApp->m_wnd.RecvStats[nIndex].ave, g_ColorAve, LINEGRAPH_AVE );

	    if( g_GraphOptions & OPTION_BPS )
            m_RecvGraph.SetPos( pTheApp->m_wnd.RecvStats[nIndex].Bps, g_ColorRecv, LINEGRAPH_BPS );
    }


    if ( update & SENT_DATA )
    {
        m_SentGraph.ShiftLeft( );

        if( g_GraphOptions & OPTION_AVE )
            m_SentGraph.SetPos( pTheApp->m_wnd.SentStats[nIndex].ave, g_ColorAve, LINEGRAPH_AVE );

        if( g_GraphOptions & OPTION_BPS )
	        m_SentGraph.SetPos( pTheApp->m_wnd.SentStats[nIndex].Bps, g_ColorSent, LINEGRAPH_BPS );
    }
    
}

/////////////////////////////////////////////////////////////////////////////
// adjust the auto scale position
void CSessionDlg::UpdateScrollPos( WORD wControl, DWORD dwValue )
{
    CSliderCtrl* pCtrl = (CSliderCtrl*)GetDlgItem( wControl );

    if( pCtrl )
    {
        int nStart = ELEMENTS(bpsArray) - 1;
        int nPos = nStart;

        for( int i = nStart; i >= 0; i-- )
        {
            if( bpsArray[i] >= dwValue )
                nPos = i;
        }
        pCtrl->SetPos( nPos );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void CSessionDlg::UpdateGraph( )
{
    //check autosize
    if( !g_bAutoScaleRecv || CalcAutoScale( &m_AutoScale_Recv, pTheApp->m_wnd.RecvStats, RECV_DATA ) == FALSE )
        DrawGraph( pTheApp->m_wnd.GetArrayIndex(), RECV_DATA );

    if( !g_bAutoScaleSent || CalcAutoScale( &m_AutoScale_Sent, pTheApp->m_wnd.SentStats, SENT_DATA ) == FALSE )
        DrawGraph( pTheApp->m_wnd.GetArrayIndex(), SENT_DATA );

}


///////////////////////////////////////////////////////////////////////////////////////////
// determine the range of the graph based upon recent samples. returns TRUE if the graph should
// be updated.
BOOL CSessionDlg::CalcAutoScale( UINT* pAutoScale,  STATS_STRUCT* pStats, UPDATE_MODE update )
{
    
    DWORD dwHigh = 0;

	int start = pTheApp->m_wnd.GetArrayIndex( );

	int total = m_SentGraph.GetTotalElements( );
	start -= total;
	if( start < 0 )
		start += MAX_SAMPLES;
   
	for( int i = 0; i < total; i++ )
	{
        if( start >= MAX_SAMPLES )
			start = 0;
		if( g_GraphOptions & OPTION_BPS )
        {
            if( pStats[start].Bps > dwHigh )
			    dwHigh = pStats[start].Bps;
        }

        if( g_GraphOptions & OPTION_AVE )
        {
            if( pStats[start].ave > dwHigh )
			    dwHigh = pStats[start].ave;
        }
		start++;
	}

    //scale the highest point in the graph to 80% of the graphs total height
    //you don't want the bars or lines always to be bumping the top of the graph

    //convert to bits
    dwHigh = MulDiv( dwHigh, 100, 80 );

    BOOL bUpdate = FALSE;
    if( dwHigh > *pAutoScale )
    {
        bUpdate = TRUE;
        *pAutoScale = dwHigh;
    } else {

        if( *pAutoScale > dwHigh && *pAutoScale > 1000 )
        {
            *pAutoScale = dwHigh;
            *pAutoScale = max( 1000, *pAutoScale );
            bUpdate = TRUE;
        }
    }


    if( bUpdate )
    {
        //move the sliders
        if( update == SENT_DATA )
        {
            UpdateScrollPos( IDC_SCALE_SLIDER_SENT, *pAutoScale );
            UpdateGraphTextSent( *pAutoScale );
        } else {
            UpdateScrollPos( IDC_SCALE_SLIDER_RECV, *pAutoScale );
            UpdateGraphTextRecv( *pAutoScale );
        }

    } else {
        return( FALSE );    //graph not updated
    }
    return( TRUE );
    

}



///////////////////////////////////////////////////////////////////////////////////////////
// draws the transmitted samples
void CSessionDlg::SetGraphRangeSent( )
{

    DWORD dwNumber;

    if( !g_bAutoScaleSent )
        dwNumber = bpsArray[g_Range_Sent];
    else
        dwNumber = m_AutoScale_Sent;

    m_SentGraph.SetGraphRange( dwNumber );
    m_SentGraph.ClearGraph();
    
	//init the graphs
    int start = pTheApp->m_wnd.GetArrayIndex( );

	int total = m_SentGraph.GetTotalElements( );
	start -= total;
	if( start < 0 )
		start += MAX_SAMPLES;

	for( int i = 0; i <= total; i++ )
	{
        if( start >= MAX_SAMPLES )
			start = 0;
        DrawGraph( start++, SENT_DATA );
	}

}

///////////////////////////////////////////////////////////////////////////////////////////
// draws the received samples
void CSessionDlg::SetGraphRangeRecv( )
{
    DWORD dwNumber;

    if( !g_bAutoScaleRecv )
        dwNumber = bpsArray[g_Range_Recv];
    else
        dwNumber = m_AutoScale_Recv;

    m_RecvGraph.SetGraphRange( dwNumber );
    m_RecvGraph.ClearGraph();

	//init the graphs
    int start = pTheApp->m_wnd.GetArrayIndex( );

	int total = m_SentGraph.GetTotalElements( );
	start -= total;
	if( start < 0 )
		start += MAX_SAMPLES;
   

	for( int i = 0; i <= total; i++ )
	{
        if( start >= MAX_SAMPLES )
			start = 0;
        DrawGraph( start++, RECV_DATA );
	}

}


///////////////////////////////////////////////////////////////////////////////////////////
// sets the colors for the graph labels
HBRUSH CSessionDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
	COLORREF cr;
	
    //set the foreground and background colors for the text labels
	if( nCtlColor == CTLCOLOR_STATIC )
	{
		int iID = pWnd->GetDlgCtrlID( );
		switch( iID )
		{
            case IDC_SENT_MAXIMUM:
            case IDC_SENT_CURRENT: cr = g_ColorSent; break;
            
            case IDC_RECV_MAXIMUM:
            case IDC_RECV_CURRENT: cr = g_ColorRecv; break;
            
            case IDC_SENT_AVERAGE:
            case IDC_RECV_AVERAGE: cr = g_ColorAve; break;

			default:
				return hbr;
		}
        pDC->SetBkMode( TRANSPARENT );
        pDC->SetTextColor( cr );
        return( (HBRUSH)m_pbrBackground->m_hObject );
	}
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void CSessionDlg::SetOptions( )
{
    g_GraphOptions = 0;

    if( IsDlgButtonChecked( IDC_CURRENT_RECV_OPTION ) )
        g_GraphOptions += OPTION_BPS;

    if( IsDlgButtonChecked( IDC_AVE_RECV_OPTION ) )
        g_GraphOptions += OPTION_AVE;

    SetGraphRangeRecv(  );	//redraw the graphs
    SetGraphRangeSent(  );	//redraw the graphs

}

///////////////////////////////////////////////////////////////////////////////////////////
//
void CSessionDlg::OnAveRecvOption()
{
    SetOptions( );
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void CSessionDlg::OnAveSentOption()
{
    SetOptions( );
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void CSessionDlg::OnCurrentRecvOption()
{
    SetOptions( );
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void CSessionDlg::OnCurrentSentOption()
{
    SetOptions( );
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void CSessionDlg::OnMaxRecvOption()
{
    SetOptions( );
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void CSessionDlg::OnMaxSentOption()
{
    SetOptions( );
}

///////////////////////////////////////////////////////////////////////////////////////////
//draw the labels on the right side of the graph
void CSessionDlg::UpdateGraphTextRecv( DWORD dwNumber )
{
    CString s;
    FormatBytes( dwNumber, &s );

    SetDlgItemText( IDC_SCALE_MAX_RECV, s );

    FormatBytes( dwNumber / 2,  &s );

    SetDlgItemText( IDC_SCALE_MID_RECV, s );

    FormatBytes( 0, &s );
    SetDlgItemText( IDC_RECV_ZERO, s );
    
	SetGraphRangeRecv( );
}



///////////////////////////////////////////////////////////////////////////////////////////
// draw the labels on the right side of the graph
void CSessionDlg::UpdateGraphTextSent( DWORD dwNumber )
{

    CString s;
    FormatBytes( dwNumber, &s );

    SetDlgItemText( IDC_SCALE_MAX_SENT, s );

    FormatBytes( dwNumber / 2,  &s );

    SetDlgItemText( IDC_SCALE_MID_SENT, s );

    FormatBytes( 0, &s );
    SetDlgItemText( IDC_SENT_ZERO, s );

	SetGraphRangeSent(  );
}


///////////////////////////////////////////////////////////////////////////////////////////
//respond to the slider commands
void CSessionDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    int nControl = pScrollBar->GetDlgCtrlID( );
    CSliderCtrl* pCtrl = (CSliderCtrl*)GetDlgItem(nControl);
    ASSERT( pCtrl != NULL );

    nPos = pCtrl->GetPos( );
	if( nPos < 0 )
		nPos = 0;
	if( nPos >= ELEMENTS(bpsArray) )
		nPos = ELEMENTS(bpsArray)-1;

    if( nControl == IDC_SCALE_SLIDER_RECV )
    {
        m_AutoScale_Recv = 0;
        g_Range_Recv = nPos;
        UpdateGraphTextRecv( bpsArray[nPos] );
    }

    if( nControl == IDC_SCALE_SLIDER_SENT )
    {
        m_AutoScale_Sent = 0;
        g_Range_Sent = nPos;
        UpdateGraphTextSent( bpsArray[nPos] );
    }

	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void CSessionDlg::OnBargraph()
{
	SetGraphStyle( );
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void CSessionDlg::OnLinegraph()
{
    SetGraphStyle( );
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void CSessionDlg::SetGraphStyle( )
{
    g_bShowBarGraph = IsDlgButtonChecked( IDC_BARGRAPH );

    m_SentGraph.SetStyle( g_bShowBarGraph );
    m_RecvGraph.SetStyle( g_bShowBarGraph );
    SetGraphRangeRecv( );
    SetGraphRangeSent( );
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void CSessionDlg::OnResetData()
{
    pTheApp->m_wnd.ResetData( );

    m_AutoScale_Recv = 0;
    m_AutoScale_Sent = 0;
    
    g_dbResetRecv = pTheApp->m_wnd.m_dbTotalBytesRecv;   //when user clicks reset
    g_dbResetSent = pTheApp->m_wnd.m_dbTotalBytesSent;   //these values are subtracted from the total

    SetGraphRangeRecv( );
    SetGraphRangeSent( );

    OnTimer( 0 );   //update the display

}

///////////////////////////////////////////////////////////////////////////////////////////
//
void CSessionDlg::OnAutoscale()
{
    g_bAutoScaleRecv = IsDlgButtonChecked( IDC_AUTOSCALE_RECV );
    g_bAutoScaleSent = IsDlgButtonChecked( IDC_AUTOSCALE_SENT );
    GetDlgItem( IDC_SCALE_SLIDER_SENT )->EnableWindow( g_bAutoScaleSent == FALSE );
    GetDlgItem( IDC_SCALE_SLIDER_RECV )->EnableWindow( g_bAutoScaleRecv == FALSE );
}


///////////////////////////////////////////////////////////////////////////////////////////
//
void CSessionDlg::OnBps()
{
    OnBytes( );
}

///////////////////////////////////////////////////////////////////////////////////////////
// sets the display type to either Bps or bps
void CSessionDlg::OnBytes()
{
    g_DisplayBytes = IsDlgButtonChecked( IDC_BYTES );
    if( g_bAutoScaleRecv )
        UpdateGraphTextRecv(  m_AutoScale_Recv );
    else
        UpdateGraphTextRecv( bpsArray[g_Range_Recv] );
    
    if( g_bAutoScaleSent )
        UpdateGraphTextSent( m_AutoScale_Sent  );
    else
        UpdateGraphTextSent( bpsArray[g_Range_Sent] );
    
}

