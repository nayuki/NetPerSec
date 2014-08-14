#include "StdAfx.h"
#include "NetPerSec.h"
#include "PerfData.h"
#include <atlbase.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TOTALBYTES    4096
#define BYTEINCREMENT 1024


// CPerfData
CPerfData::CPerfData() {
	m_bIs95 = FALSE;
}

CPerfData::~CPerfData() {}


BOOL CPerfData::GetPerfStats9x(LPCSTR pKey, DWORD *dwValue) {
	CRegKey reg;
	LONG lErr = !ERROR_SUCCESS;
	*dwValue = 0;
	
	if (ERROR_SUCCESS == reg.Open(HKEY_DYN_DATA, "PerfStats\\StatData")) {
		DWORD dwType = NULL;
		DWORD dwCount = sizeof(DWORD);
		lErr = RegQueryValueEx(reg.m_hKey, pKey, NULL, &dwType, (LPBYTE)dwValue, &dwCount);
		reg.Close();
	}
	
	return lErr == ERROR_SUCCESS;
}


// Load the counter and object names from the registry to the global variable m_lpNamesArray.
void CPerfData::GetNameStrings() {
	HKEY hKeyPerflib;      // Handle to registry key
	HKEY hKeyPerflib009;   // Handle to registry key
	DWORD dwMaxValueLen;   // Maximum size of key values
	DWORD dwBuffer;        // Bytes to allocate for buffers
	DWORD dwBufferSize;    // Size of dwBuffer
	LPSTR lpCurrentString; // Pointer for enumerating data strings
	DWORD dwCounter;       // Current counter index
	
	// Get the number of Counter items
	RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib",
		0,
		KEY_READ,
		&hKeyPerflib);
	
	dwBufferSize = sizeof(dwBuffer);
	
	RegQueryValueEx(hKeyPerflib,
		"Last Counter",
		NULL,
		NULL,
		(LPBYTE)&dwBuffer,
		&dwBufferSize);
	
	RegCloseKey(hKeyPerflib);
	
	// Allocate memory for the names array
	m_lpNamesArray.resize(dwBuffer + 1);
	
	// Open key containing counter and object names
	RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\009",
		0,
		KEY_READ,
		&hKeyPerflib009);
	
	// Get the size of the largest value in the key (Counter or Help)
	RegQueryInfoKey(hKeyPerflib009,
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
	
	// Allocate memory for the counter and object names
	dwBuffer = dwMaxValueLen + 1;
	m_lpNameStrings.resize(dwBuffer);
	
	// Read counter value
	RegQueryValueEx(hKeyPerflib009,
		"Counter",
		NULL,
		NULL,
		(BYTE*)m_lpNameStrings.data(), &dwBuffer);
	
	// Load names into an array, by index
	lpCurrentString = m_lpNameStrings.data();
	while (*lpCurrentString != '\0') {
		dwCounter = atol(lpCurrentString);
		lpCurrentString += lstrlen(lpCurrentString) + 1;
		m_lpNamesArray[dwCounter] = (LPSTR)lpCurrentString;
		lpCurrentString += lstrlen(lpCurrentString) + 1;
	}
}


void CPerfData::ReadData9x(DWORD *pRecv, DWORD *pSent) {
	static BOOL bErrorShown = FALSE;
	if (GetPerfStats9x("Dial-Up Adapter\\TotalBytesXmit" , pSent))
		GetPerfStats9x("Dial-Up Adapter\\TotalBytesRecvd", pRecv);
	else if (!bErrorShown) {
		// Requires the dial-up networking update
		bErrorShown = TRUE;
		ShowError(IDS_DUN_ERR, MB_OK);
	}
}

void CPerfData::ReadDataNT(DWORD *pRecv, DWORD *pSent) {
	DWORD BufferSize = TOTALBYTES;
	
	// Allocate the buffer
	PPERF_DATA_BLOCK PerfData = (PPERF_DATA_BLOCK)malloc(BufferSize);
	if (!PerfData)
		return;
	
	while (RegQueryValueEx(HKEY_PERFORMANCE_DATA, "906", NULL, NULL, (LPBYTE)PerfData, &BufferSize) == ERROR_MORE_DATA) {
		// Get a buffer that is big enough
		BufferSize += BYTEINCREMENT;
		PerfData = (PPERF_DATA_BLOCK)realloc(PerfData, BufferSize);
	}
	
	// Get the first object type
	PPERF_OBJECT_TYPE PerfObj = FirstObject(PerfData);
	
	// Process all objects
	for (DWORD i = 0; i < PerfData->NumObjectTypes; i++) {
		// Get the first counter
		PPERF_COUNTER_DEFINITION PerfCntr = FirstCounter(PerfObj);
		
		// Don't enumerate instances
		if (PerfObj->NumInstances < 1) {
			// Get the counter block
			PPERF_COUNTER_BLOCK PtrToCntr = (PPERF_COUNTER_BLOCK)((PBYTE)PerfObj + PerfObj->DefinitionLength);
			
			// Retrieve all counters
			for (DWORD j = 0; j < PerfObj->NumCounters; j++) {
				if (strcmp("Bytes Transmitted", m_lpNamesArray[PerfCntr->CounterNameTitleIndex]) == 0)
					*pSent = *(DWORD*)((BYTE*)PtrToCntr + PerfCntr->CounterOffset);
				
				if (strcmp("Bytes Received", m_lpNamesArray[PerfCntr->CounterNameTitleIndex]) == 0)
					*pRecv = *(DWORD*)((BYTE*)PtrToCntr + PerfCntr->CounterOffset);
				
				// Get the next counter
				PerfCntr = NextCounter(PerfCntr);
			}
		}
		// Get the next object type
		PerfObj = NextObject(PerfObj);
	}
	free(PerfData);
}

void CPerfData::GetReceivedAndSentOctets(DWORD *pReceived, DWORD *pSent) {
	static BOOL bInitPerfData = FALSE;
	if (!bInitPerfData) {
		DWORD ver = GetVersion();
		m_bIs95 = (ver & 0x80000000u) != 0;
		if (!m_bIs95)
			GetNameStrings();  // Windows NT
		// Otherwise Windows 95/98: Requires Dial-up Networking Update v1.3 for Win95 users.
		// Since these functions use the registry, you should close NetPerSec before installing or updating the system.
		bInitPerfData = TRUE;
	}
	
	if (m_bIs95)
		ReadData9x(pReceived, pSent);
	else
		ReadDataNT(pReceived, pSent);
}
