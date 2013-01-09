// PerfData.cpp : implementation file
//
#include "stdafx.h"
#include "netpersec.h"
#include "PerfData.h"
#include <atlbase.h>

#define TOTALBYTES    4096
#define BYTEINCREMENT 1024

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPerfData
CPerfData::CPerfData()
{
	m_lpNameStrings = NULL;
	m_lpNamesArray = NULL;
	m_bIs95 = FALSE;
}

CPerfData::~CPerfData()
{
	if( m_lpNameStrings )
		free( m_lpNameStrings );
	if( m_lpNamesArray )
		free( m_lpNamesArray );
}


/////////////////////////////////////////////////////////////////////////////
//
BOOL CPerfData::GetPerfStats9x( LPCSTR pKey, DWORD *dwValue )
{
	CRegKey reg;
	LONG lErr = !ERROR_SUCCESS;
	*dwValue = 0;
	
	if( ERROR_SUCCESS == reg.Open(HKEY_DYN_DATA, "PerfStats\\StatData") )
	{
		DWORD dwType = NULL;
		DWORD dwCount = sizeof(DWORD);
		lErr = RegQueryValueEx( reg.m_hKey, pKey, NULL, &dwType,(LPBYTE)dwValue, &dwCount );
		reg.Close( );
	}
	
	return lErr == ERROR_SUCCESS;
}


/*****************************************************************
 *                                                               *
 * Load the counter and object names from the registry to the    *
 * global variable m_lpNamesArray.                                 *
 *                                                               *
 *****************************************************************/
void CPerfData::GetNameStrings( )
{
	HKEY hKeyPerflib;      // handle to registry key
	HKEY hKeyPerflib009;   // handle to registry key
	DWORD dwMaxValueLen;   // maximum size of key values
	DWORD dwBuffer;        // bytes to allocate for buffers
	DWORD dwBufferSize;    // size of dwBuffer
	LPSTR lpCurrentString; // pointer for enumerating data strings
	DWORD dwCounter;       // current counter index
	
	// Get the number of Counter items.
	RegOpenKeyEx( HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib",
		0,
		KEY_READ,
		&hKeyPerflib);
	
	dwBufferSize = sizeof(dwBuffer);
	
	RegQueryValueEx( hKeyPerflib,
		"Last Counter",
		NULL,
		NULL,
		(LPBYTE) &dwBuffer,
		&dwBufferSize );
	
	RegCloseKey( hKeyPerflib );
	
	// Allocate memory for the names array.
	m_lpNamesArray = (char**)malloc( (dwBuffer+1) * sizeof(LPSTR) );
	
	// Open key containing counter and object names.
	RegOpenKeyEx( HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\009",
		0,
		KEY_READ,
		&hKeyPerflib009);
	
	// Get the size of the largest value in the key (Counter or Help).
	RegQueryInfoKey( hKeyPerflib009,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		&dwMaxValueLen,
		NULL,
		NULL);
	
	// Allocate memory for the counter and object names.
	dwBuffer = dwMaxValueLen + 1;
	
	m_lpNameStrings = (char*)malloc( dwBuffer * sizeof(CHAR) );
	
	// Read Counter value.
	RegQueryValueEx( hKeyPerflib009,
		"Counter",
		NULL,
		NULL,
		(BYTE*)m_lpNameStrings, &dwBuffer );
	
	// Load names into an array, by index.
	for( lpCurrentString = m_lpNameStrings; *lpCurrentString;
	     lpCurrentString += (lstrlen(lpCurrentString)+1) )
	{
		dwCounter = atol( lpCurrentString );
		lpCurrentString += (lstrlen(lpCurrentString)+1);
		m_lpNamesArray[dwCounter] = (LPSTR) lpCurrentString;
	}
}


/////////////////////////////////////////////////////////////////////////////
//
void CPerfData::ReadData9x( DWORD* pReceived, DWORD* pSent )
{
	static BOOL bErrorShown = FALSE;
	
	if( GetPerfStats9x( "Dial-Up Adapter\\TotalBytesXmit" , pSent ) )
	{
		GetPerfStats9x( "Dial-Up Adapter\\TotalBytesRecvd", pReceived );
	} else {
		if( !bErrorShown )
		{
			//requires the Dial-up networking update
			bErrorShown = TRUE;
			ShowError( IDS_DUN_ERR, MB_OK );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//
void CPerfData::ReadDataNT( DWORD* pReceived, DWORD* pSent )
{
	PPERF_DATA_BLOCK PerfData = NULL;
	PPERF_OBJECT_TYPE PerfObj;
	PPERF_COUNTER_DEFINITION PerfCntr;
	PPERF_COUNTER_BLOCK PtrToCntr;
	DWORD BufferSize = TOTALBYTES;
	DWORD i, j;
	
	// Allocate the buffer
	PerfData = (PPERF_DATA_BLOCK) malloc( BufferSize );
	
	if( !PerfData )
		return;
	
	while( RegQueryValueEx( HKEY_PERFORMANCE_DATA,
			"906",
			NULL,
			NULL,
			(LPBYTE) PerfData,
			&BufferSize ) == ERROR_MORE_DATA )
	{
		// Get a buffer that is big enough.
		BufferSize += BYTEINCREMENT;
		PerfData = (PPERF_DATA_BLOCK) realloc( PerfData, BufferSize );
	}
	
	// Get the first object type.
	PerfObj = FirstObject( PerfData );
	
	// Process all objects.
	for( i=0; i < PerfData->NumObjectTypes; i++ )
	{
		// Get the first counter.
		PerfCntr = FirstCounter( PerfObj );
		
		// don't enumerate instances
		if( PerfObj->NumInstances < 1 )
		{
			// Get the counter block.
			PtrToCntr = (PPERF_COUNTER_BLOCK) ((PBYTE)PerfObj + PerfObj->DefinitionLength );
			
			// Retrieve all counters.
			for( j=0; j < PerfObj->NumCounters; j++ )
			{
				if( !strcmp("Bytes Transmitted", m_lpNamesArray[PerfCntr->CounterNameTitleIndex] ) )
				{
					*pSent =*( (DWORD*)((BYTE*)PtrToCntr+PerfCntr->CounterOffset) );
				}
				
				if( !strcmp("Bytes Received", m_lpNamesArray[PerfCntr->CounterNameTitleIndex] ) )
				{
					*pReceived =*( (DWORD*)((BYTE*)PtrToCntr+PerfCntr->CounterOffset) );
				}
				
				// Get the next counter.
				PerfCntr = NextCounter( PerfCntr );
			}
		}
		// Get the next object type.
		PerfObj = NextObject( PerfObj );
	}
	free( PerfData );
}

/////////////////////////////////////////////////////////////////////////////
//
BOOL CPerfData::GetReceivedAndSentOctets( DWORD* pReceived, DWORD* pSent )
{
	static BOOL bInitPerfData = FALSE;
	
	if( !bInitPerfData )
	{
		Init( );
		bInitPerfData = TRUE;
	}
	
	if( m_bIs95 )
		ReadData9x( pReceived, pSent );
	else
		ReadDataNT( pReceived, pSent );
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//
void CPerfData::Init(void)
{
	DWORD dwVersion = GetVersion();
	
	// Windows NT
	if( dwVersion >> 31 == 0 )
	{
		m_bIs95 = FALSE;
		GetNameStrings( );
	} else {
		// Windows 95 - 98
		// requires Dial-up Networking Update v1.3 for Win95 users
		// since these functions use the registry you should close NPS before installing or updating system
		m_bIs95 = TRUE;
	}
}
