/*=========================================================================*/
/*                           SNMP.CPP                                      */
/*                                                                         */
/*               Implements the SNMP and IPHLPAPI functions.               */
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
#include "winsock2.h"


HANDLE hPollForTrapEvent = NULL;
AsnObjectIdentifier SupportedView = {0,0};

///////////////////////////////////////////////////////////////////////////////////////////
//
LPVOID CSnmp::SnmpUtilMemAlloc( UINT nSize )
{
	if( m_fpSnmpUtilMemAlloc )
		return( m_fpSnmpUtilMemAlloc( nSize ) );
	else
		return( GlobalAlloc( GPTR, nSize ) );
}

///////////////////////////////////////////////////////////////////////////////////////////
//
void CSnmp::SnmpUtilMemFree( LPVOID pMem )
{
	if( m_fpSnmpUtilMemFree )
		m_fpSnmpUtilMemFree( pMem );
	else
		GlobalFree( pMem );
}


///////////////////////////////////////////////////////////////////////////////////////////
//
CSnmp::CSnmp( )
{
	m_fpSnmpUtilMemAlloc = NULL;
	m_fpSnmpUtilMemFree = NULL;
	m_pvarBindList = NULL;
	m_bUse_iphlpapi = FALSE;
	m_hInstIpHlp = 0;
	m_hInstSnmp = 0;
	m_dwInterfaces = 0;
	m_fpGetNumberOfInterfaces = 0;
	m_fpGetIfEntry = 0;
	m_bUseGetInterfaceInfo = FALSE; //win2k
}

///////////////////////////////////////////////////////////////////////////////////////////
//
CSnmp::~CSnmp( )
{
	if( m_pvarBindList )
		SnmpUtilMemFree( m_pvarBindList );
	
	if( m_hInst )
		FreeLibrary( m_hInst );
	
	if( m_hInstIpHlp )
		FreeLibrary( m_hInstIpHlp );
	
	if( m_hInstSnmp )
		FreeLibrary( m_hInstSnmp );
	
	m_hInstSnmp = 0;
	m_hInstIpHlp = 0;
	m_hInst = 0;
}


///////////////////////////////////////////////////////////////////////////////////////////
//if running under NT use iphlpapi -- requires SP4 for NT4 otherwise there is a memory leak in SNMP
//We could use this on Win98, however certain releases of IE5 cause iphlpapi to fail.
BOOL CSnmp::CheckNT( )
{
	m_dwInterfaces = 0;
	
	//check for NT
	OSVERSIONINFO os;
	os.dwOSVersionInfoSize = sizeof( os );
	GetVersionEx( &os );
	
	if( os.dwPlatformId == VER_PLATFORM_WIN32_NT )
	{
		m_hInstIpHlp = LoadLibraryEx( "iphlpapi.dll", NULL, 0 );
		
		if( m_hInstIpHlp )
		{
			m_fpGetIfEntry = (fpGetIfEntry)GetProcAddress(m_hInstIpHlp,"GetIfEntry" );
			m_fpGetNumberOfInterfaces = (fpGetNumberOfInterfaces)GetProcAddress( m_hInstIpHlp, "GetNumberOfInterfaces" );
			
			//if WinNT 4
			if( os.dwMajorVersion < 5 )
			{
				//requires SP4 or higher
				DWORD dwServicePack = GetServicePack( );
				if( dwServicePack > 0x300 )
				{
					if( m_fpGetNumberOfInterfaces != 0 )
						m_bUse_iphlpapi = TRUE;
				}
				
			} else {
				//windows 2000 and win98 use what appear to be scalar values in the lower word
				//and control flags in the upper word of the adapter[].index
				//GetInterfaceInfo is not supported by WinNT
				m_fpGetInterfaceInfo = (fpGetInterfaceInfo)GetProcAddress( m_hInstIpHlp, "GetInterfaceInfo" );
				if( m_fpGetInterfaceInfo )
				{
					m_bUseGetInterfaceInfo = TRUE;
					m_bUse_iphlpapi = TRUE;
				}
			}
		}
	}
	return( m_bUse_iphlpapi );
}

///////////////////////////////////////////////////////////////////////////////////////////
//check if an interface, such as DUN, has been added or removed
//Win2K does not report adapters until they are used
void CSnmp::GetInterfaces( )
{
	DWORD i;
	
	if( m_bUseGetInterfaceInfo == TRUE )
	{
		DWORD dwSize = 0;
		PIP_INTERFACE_INFO pInterface;
		if( m_fpGetInterfaceInfo )
		{
			//query size
			m_fpGetInterfaceInfo( NULL, &dwSize );
			
			if( dwSize )
			{
				pInterface = (PIP_INTERFACE_INFO)GlobalAlloc( GPTR, dwSize );
				if( pInterface )
				{
					m_fpGetInterfaceInfo ( pInterface, &dwSize );
					m_dwInterfaces = min( MAX_INTERFACES, pInterface->NumAdapters );
					
					for( i = 0; i < m_dwInterfaces; i++ )
						m_dwInterfaceArray[i] = pInterface->Adapter[i].Index;
					
					GlobalFree( pInterface );
				}
			}
		}
	} else {
		//WinNT 4 uses scalar "friendly" values for the GetIfEntry function
		//although this is poorly documented by Microsoft
		if( m_fpGetNumberOfInterfaces( &m_dwInterfaces ) == NO_ERROR )
		{
			m_dwInterfaces = min( MAX_INTERFACES, m_dwInterfaces );
			for( i = 0; i < m_dwInterfaces; i++ )
				m_dwInterfaceArray[i] = i + 1; //not zero based
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////
//Load the SNMP dlls.
BOOL CSnmp::Init( )
{
	CheckNT( );
	
	m_hInst = LoadLibraryEx( "inetmib1.dll", NULL, 0 );
	if( m_hInst == NULL )
	{
		ShowSystemError( IDS_INETMIB1_ERR );
		return FALSE;
	}
	
	m_fpExtensionInit  = (pSnmpExtensionInit)GetProcAddress( m_hInst ,"SnmpExtensionInit" );
	m_fpExtensionQuery = (pSnmpExtensionQuery)GetProcAddress( m_hInst ,"SnmpExtensionQuery" );
	
	if( !m_fpExtensionInit )
	{
		ShowSystemError( IDS_SNMPINIT_ERR );
		return( FALSE );
	}
	if( !m_fpExtensionQuery )
	{
		ShowSystemError( IDS_SNMPQUERY_ERR );
		return( FALSE );
	}
	
	//init
	if( !m_fpExtensionInit( GetTickCount(), &hPollForTrapEvent, &SupportedView ) )
	{
		ShowSystemError( IDS_SNMPFAIL_ERR );
		return( FALSE );
	}
	
	//check to see if the MemAlloc and MemFree functions are available
	m_hInstSnmp = LoadLibraryEx( "snmpapi.dll", NULL, 0 );
	
	if( m_hInstSnmp )
	{
		m_fpSnmpUtilMemAlloc = (SUALLOC)GetProcAddress( m_hInstSnmp,"SnmpUtilMemAlloc" );
		m_fpSnmpUtilMemFree  = (SUFREE)GetProcAddress( m_hInstSnmp, "SnmpUtilMemFree" );
		m_fpSnmpUtilOidFree = (pSnmpUtilOidFree)GetProcAddress( m_hInstSnmp,"SnmpUtilOidFree" );
		m_fpSnmpUtilVarBindFree = (pSnmpUtilVarBindFree)GetProcAddress( m_hInstSnmp, "SnmpUtilVarBindFree" );
		m_fpSnmpUtilOidNCmp = (pSnmpUtilOidNCmp)GetProcAddress( m_hInstSnmp, "SnmpUtilOidNCmp" );
		m_fpSnmpUtilOidCpy = (pSnmpUtilOidCpy)GetProcAddress( m_hInstSnmp, "SnmpUtilOidCpy" );
	} else {
		ShowSystemError( IDS_SNMPAPI_ERR );
		return( FALSE );
	}
	
	//alloc our bindlist
	m_pvarBindList = (SnmpVarBindList*)SnmpUtilMemAlloc( sizeof(SnmpVarBindList) );
	ASSERT( m_pvarBindList != NULL );
	
	return( TRUE );
}

///////////////////////////////////////////////////////////////////////////////////////////
// Use the IPHLPAPI interface to retrieve the transmitted and received bytes
int CSnmp::GetReceivedAndSentOctets_IPHelper( DWORD* pReceived, DWORD *pSent)
{
	DWORD i;
	MIB_IFROW mib;
	ZeroMemory( &mib, sizeof(mib) );
	
	*pReceived = 0;
	*pSent = 0;
	
	GetInterfaces( );
	
	for( i = 0; i < m_dwInterfaces; i++ )
	{
		mib.dwIndex = m_dwInterfaceArray[i];
		
		//monitor specific adapter?
		if( g_MonitorMode == MONITOR_ADAPTER )
			if( g_dwAdapter != mib.dwIndex )
				continue;
		
		if( m_fpGetIfEntry( &mib ) == NO_ERROR )
		{
			if( mib.dwType != MIB_IF_TYPE_LOOPBACK )
			{
				*pReceived += mib.dwInOctets;
				*pSent += mib.dwOutOctets;
			}
		}
	}
	return( TRUE );
}


///////////////////////////////////////////////////////////////////////////////////////////
// Returns the number of bytes received and transmitted through all network interfaces
BOOL  CSnmp::GetReceivedAndSentOctets_9x( DWORD* pReceived, DWORD *pSent )
{
	#define VAR_BINDS	3
	RFC1157VarBind      varBind[VAR_BINDS];
	AsnInteger          errorStatus;
	AsnInteger          errorIndex;
	AsnObjectIdentifier tempOid;
	int ret;
	
	static AsnObjectIdentifier MIB_NULL = {0,0};
	
	static UINT OID_ifInoctets[] = { 1,3,6,1,2,1,2,2,1,10,0 };
	AsnObjectIdentifier MIB_ifInoctets = { OID_SIZEOF(OID_ifInoctets), OID_ifInoctets };
	
	static UINT OID_ifOutoctets[] = { 1,3,6,1,2,1,2,2,1,16,0 };
	AsnObjectIdentifier MIB_ifOutoctets = { OID_SIZEOF(OID_ifOutoctets), OID_ifOutoctets};
	
	static UINT OID_ifType[] = { 1,3,6,1,2,1,2,2,1,3 };
	AsnObjectIdentifier MIB_ifType = { OID_SIZEOF(OID_ifType ), OID_ifType};
	
	ASSERT( m_pvarBindList != NULL );
	
	m_pvarBindList->list = varBind;
	m_pvarBindList->len  = VAR_BINDS;
	varBind[0].name = MIB_NULL;
	varBind[1].name = MIB_NULL;
	varBind[2].name = MIB_NULL;
	
	//monitor specific adapter?
	if( g_MonitorMode == MONITOR_ADAPTER )
	{
		m_pvarBindList->len = 2;
		OID_ifInoctets[10] = g_dwAdapter;
		OID_ifOutoctets[10] = g_dwAdapter;
		
		m_fpSnmpUtilOidCpy(&varBind[0].name,&MIB_ifInoctets );
		m_fpSnmpUtilOidCpy(&varBind[1].name,&MIB_ifOutoctets );
		
		ret = m_fpExtensionQuery( ASN_RFC1157_GETREQUEST, m_pvarBindList, &errorStatus, &errorIndex );
		if( ret && !errorStatus )
		{
			*pReceived = varBind[0].value.asnValue.number;
			*pSent = varBind[1].value.asnValue.number;
		}
		
		m_fpSnmpUtilOidFree( &varBind[0].name );
		m_fpSnmpUtilOidFree( &varBind[1].name );
		return 1;
	}
	
	//monitor all adapters
	OID_ifInoctets[10] = 0;
	OID_ifOutoctets[10] = 0;
	
	m_fpSnmpUtilOidCpy(&varBind[0].name,&MIB_ifInoctets );
	m_fpSnmpUtilOidCpy(&varBind[1].name,&MIB_ifOutoctets );
	m_fpSnmpUtilOidCpy(&varBind[2].name,&MIB_ifType );
	
	while( TRUE )
	{
		ret = m_fpExtensionQuery( ASN_RFC1157_GETNEXTREQUEST, m_pvarBindList, &errorStatus, &errorIndex );
		if( !ret )
			break;
		
		ret = m_fpSnmpUtilOidNCmp( &varBind[0].name, &MIB_ifInoctets, MIB_ifInoctets.idLength - 1 );
		if( ret != 0 )
			break;
		
		if( varBind[2].value.asnValue.number != MIB_IF_TYPE_LOOPBACK )
		{
			*pReceived += varBind[0].value.asnValue.number;
			*pSent += varBind[1].value.asnValue.number;
		}
		
		// Prepare for the next iteration.  Make sure returned oid is
		// preserved and the returned value is freed.
		for( int i = 0; i < VAR_BINDS; i++ )
		{
			m_fpSnmpUtilOidCpy( &tempOid, &varBind[i].name );
			m_fpSnmpUtilVarBindFree( &varBind[i] );
			m_fpSnmpUtilOidCpy( &varBind[i].name, &tempOid);
			varBind[i].value.asnType = ASN_NULL;
			m_fpSnmpUtilOidFree( &tempOid );
		}
	}
	
	for( int i = 0; i < VAR_BINDS; i++ )
		m_fpSnmpUtilOidFree( &varBind[i].name );
	
	return 1;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Returns a list of adapter names and index values
void CSnmp::GetInterfaceDescriptions( CStringArray *sArray, CUIntArray *nAdapter )
{
	#define VAR_BINDS_DESCRIPTIONS	3
	AsnInteger          errorStatus;
	AsnInteger          errorIndex;
	AsnObjectIdentifier tempOid;
	SnmpVarBindList varBindList;
	RFC1157VarBind   varBind[VAR_BINDS_DESCRIPTIONS];
	int ret;
	
	static AsnObjectIdentifier MIB_NULL = {0,0};
	
	static UINT OID_ifDesc[] = { 1,3,6,1,2,1,2,2,1,2 };
	AsnObjectIdentifier MIB_ifDesc = { OID_SIZEOF(OID_ifDesc), OID_ifDesc };
	
	static UINT OID_ifIndex[] = {1,3,6,1,2,1,2,2,1,1 };
	AsnObjectIdentifier MIB_ifIndex = { OID_SIZEOF(OID_ifIndex), OID_ifIndex };
	
	static UINT OID_ifType[] = { 1,3,6,1,2,1,2,2,1,3 };
	AsnObjectIdentifier MIB_ifType = { OID_SIZEOF(OID_ifType), OID_ifType };
	
	varBindList.list = varBind;
	varBindList.len  = VAR_BINDS_DESCRIPTIONS;
	varBind[0].name = MIB_NULL;
	varBind[1].name = MIB_NULL;
	varBind[2].name = MIB_NULL;
	
	m_fpSnmpUtilOidCpy(&varBind[0].name,&MIB_ifDesc);
	m_fpSnmpUtilOidCpy(&varBind[1].name,&MIB_ifIndex);
	m_fpSnmpUtilOidCpy(&varBind[2].name,&MIB_ifType);
	
	while( 1 )
	{
		ret = m_fpExtensionQuery( ASN_RFC1157_GETNEXTREQUEST, &varBindList, &errorStatus, &errorIndex );
		if( !ret )
			break;
		
		ret = m_fpSnmpUtilOidNCmp( &varBind[0].name, &MIB_ifDesc, MIB_ifDesc.idLength );
		if( ret != 0 )
			break;
		
		if( !errorStatus )
		{
			//Win9x occasionally fails to truncate the ifDesc string (and leaks memory when this happens)
			//Limit the output string to 32 characters max.
			if( varBind[2].value.asnValue.number != MIB_IF_TYPE_LOOPBACK )
			{
				char s[32];
				int len = min( sizeof(s)-1, varBind[0].value.asnValue.string.length );
				strncpy( s, (char*)varBind[0].value.asnValue.string.stream, len );
				s[len] = 0;
				sArray->Add( s );
				nAdapter->Add( varBind[1].value.asnValue.number );
			}
		}
		
		for( int i = 0; i < VAR_BINDS_DESCRIPTIONS; i++ )
		{
			// Prepare for the next iteration.  Make sure returned oid is
			// preserved and the returned value is freed.
			m_fpSnmpUtilOidCpy( &tempOid, &varBind[i].name );
			m_fpSnmpUtilVarBindFree( &varBind[i]);
			m_fpSnmpUtilOidCpy( &varBind[i].name, &tempOid );
			varBind[i].value.asnType = ASN_NULL;
			m_fpSnmpUtilOidFree( &tempOid );
		}
	}
	
	for(int i = 0; i < VAR_BINDS_DESCRIPTIONS; i++ )
		m_fpSnmpUtilOidFree( &varBind[i].name );
}


///////////////////////////////////////////////////////////////////////////////////////////
//
void CSnmp::ShowSystemError( int nID )
{
	LPVOID lpMsgBuf;
	CString sErr;
	DWORD rc;
	DWORD dwErr = GetLastError( );

	rc = FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL, dwErr,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPSTR)&lpMsgBuf, 0, NULL);
	
	CString sMsg;
	sMsg.LoadString( nID );
	sErr.Format( "%s\n\nError code = %u.  %s\nPlease review the Troubleshooting section in the online help.",
			sMsg, dwErr, lpMsgBuf );
	
	AfxMessageBox( sErr, MB_OK | MB_ICONHAND | MB_SETFOREGROUND );
	
	LocalFree( lpMsgBuf );
}


///////////////////////////////////////////////////////////////////////////////////////////
// Returns the number of bytes received and transmitted
BOOL CSnmp::GetReceivedAndSentOctets( DWORD* pReceived, DWORD *pSent )
{
	*pReceived = 0;
	*pSent = 0;
	
	//use performance data from the registry
	if( g_MonitorMode == MONITOR_DUN )
		return( perfdata.GetReceivedAndSentOctets( pReceived, pSent ) );
	
	//use IPHLPAPI.DLL
	if( m_bUse_iphlpapi )
		return( GetReceivedAndSentOctets_IPHelper( pReceived, pSent ) );
	
	//use INETMIB1.DLL
	return( GetReceivedAndSentOctets_9x( pReceived, pSent ) );
}
