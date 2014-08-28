#pragma once

#include <snmp.h>
#include "iphlpapi.h"  // Requires the platform SDK
#include "PerfData.h"

#define MAX_INTERFACES 64


class CSnmp {
	typedef BOOL (WINAPI *pSnmpExtensionInit)(
		IN DWORD dwTimeZeroReference,
		OUT HANDLE *hPollForTrapEvent,
		OUT AsnObjectIdentifier *supportedView);
	
	typedef BOOL (WINAPI *pSnmpExtensionTrap)(
		OUT AsnObjectIdentifier *enterprise,
		OUT AsnInteger *genericTrap,
		OUT AsnInteger *specificTrap,
		OUT AsnTimeticks *timeStamp,
		OUT RFC1157VarBindList *variableBindings);
	
	typedef BOOL (WINAPI *pSnmpExtensionQuery)(
		IN BYTE requestType,
		IN OUT RFC1157VarBindList *variableBindings,
		OUT AsnInteger *errorStatus,
		OUT AsnInteger *errorIndex);
	
	typedef BOOL (WINAPI *pSnmpExtensionInitEx)(
		OUT AsnObjectIdentifier *supportedView);
	
	typedef LPVOID (CALLBACK *SUALLOC)(UINT);
	typedef VOID (CALLBACK *SUFREE)(LPVOID);
	
	typedef DWORD (WINAPI *fpGetNumberOfInterfaces)(LPDWORD);
	typedef DWORD (WINAPI *fpGetIfEntry)(LPVOID);
	typedef DWORD (WINAPI *fpGetInterfaceInfo)(PIP_INTERFACE_INFO, LPDWORD);
	
	typedef int (WINAPI *pSnmpUtilOidFree)(AsnObjectIdentifier *pOid);
	typedef int (WINAPI *pSnmpUtilVarBindFree)(SnmpVarBind *pVb);
	typedef int (WINAPI *pSnmpUtilOidNCmp)(AsnObjectIdentifier *pOid1,
	                                       AsnObjectIdentifier *pOid2,
	                                       UINT nSubIds);
	typedef int (WINAPI *pSnmpUtilOidCpy)(AsnObjectIdentifier *pOidDst,
	                                      AsnObjectIdentifier *pOidSrc);
	
public:
	CSnmp();
	~CSnmp();
	
private:
	HINSTANCE m_hInst, m_hInstIpHlp;
	pSnmpExtensionInit m_fpExtensionInit;
	pSnmpExtensionQuery m_fpExtensionQuery;
	fpGetIfEntry m_fpGetIfEntry;
	fpGetNumberOfInterfaces m_fpGetNumberOfInterfaces;
	fpGetInterfaceInfo m_fpGetInterfaceInfo;
	
	pSnmpUtilOidFree m_fpSnmpUtilOidFree;
	pSnmpUtilVarBindFree m_fpSnmpUtilVarBindFree;
	pSnmpUtilOidNCmp m_fpSnmpUtilOidNCmp;
	pSnmpUtilOidCpy m_fpSnmpUtilOidCpy;
	HINSTANCE m_hInstSnmp;
	
	DWORD m_dwInterfaces;
	BOOL m_bUseGetInterfaceInfo;
	
	SUALLOC m_fpSnmpUtilMemAlloc;
	SUFREE m_fpSnmpUtilMemFree;
	RFC1157VarBindList *m_pvarBindList;
	BOOL m_bUse_iphlpapi;
	DWORD m_dwInterfaceArray[MAX_INTERFACES];
	
	CPerfData perfdata;
	
	// Overloaded functions to accommodate Win95
	LPVOID SnmpUtilMemAlloc(UINT nSize);
	void SnmpUtilMemFree(LPVOID pMem);
	
public:
	BOOL Init();
	void GetReceivedAndSentOctets(DWORD *pRecv, DWORD *pSent);
	void GetInterfaceDescriptions(CStringArray *sArray, CUIntArray *nAdapter);
	
private:
	void GetReceivedAndSentOctets_9x(DWORD *pRecv, DWORD *pSent);
	void GetReceivedAndSentOctets_IPHelper(DWORD *pReceived, DWORD *pSent);
	void GetInterfaces();
	void ShowSystemError(int nID);
	BOOL CheckNT();
};
