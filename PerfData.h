#if !defined(AFX_PERFDATA_H__8AC19065_EECB_11D4_A181_004033572A05__INCLUDED_)
#define AFX_PERFDATA_H__8AC19065_EECB_11D4_A181_004033572A05__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PerfData.h : header file

#include <Winperf.h>


// CPerfData window
class CPerfData
{
// Construction
public:
	CPerfData();
	
	void Init(void);
	BOOL GetReceivedAndSentOctets(DWORD* pRecv, DWORD* pSent);
	
// Attributes
public:
	
private:
	LPSTR m_lpNameStrings;
	LPSTR* m_lpNamesArray;
	BOOL  m_bIs95;
	
	void GetNameStrings();
	void ReadData9x(DWORD* pReceived, DWORD* pSent);
	void ReadDataNT(DWORD* pReceived, DWORD* pSent);
	BOOL GetPerfStats9x(LPCSTR pKey, DWORD *dwValue);
	
// Implementation
public:
	virtual ~CPerfData();
	
	// Generated message map functions
protected:
	PPERF_OBJECT_TYPE FirstObject(PPERF_DATA_BLOCK PerfData)
	{
		return (PPERF_OBJECT_TYPE)((PBYTE)PerfData + PerfData->HeaderLength);
	}
	
	PPERF_OBJECT_TYPE NextObject(PPERF_OBJECT_TYPE PerfObj)
	{
		return (PPERF_OBJECT_TYPE)((PBYTE)PerfObj + PerfObj->TotalByteLength);
	}
	
	PPERF_INSTANCE_DEFINITION FirstInstance(PPERF_OBJECT_TYPE PerfObj)
	{
		return (PPERF_INSTANCE_DEFINITION)((PBYTE)PerfObj + PerfObj->DefinitionLength);
	}
	
	PPERF_INSTANCE_DEFINITION NextInstance(PPERF_INSTANCE_DEFINITION PerfInst)
	{
		PPERF_COUNTER_BLOCK PerfCntrBlk;
	
		PerfCntrBlk = (PPERF_COUNTER_BLOCK)((PBYTE)PerfInst + PerfInst->ByteLength);
	
		return (PPERF_INSTANCE_DEFINITION)((PBYTE)PerfCntrBlk + PerfCntrBlk->ByteLength);
	}
	
	PPERF_COUNTER_DEFINITION FirstCounter(PPERF_OBJECT_TYPE PerfObj)
	{
		return (PPERF_COUNTER_DEFINITION) ((PBYTE)PerfObj +  PerfObj->HeaderLength);
	}
	
	PPERF_COUNTER_DEFINITION NextCounter(PPERF_COUNTER_DEFINITION PerfCntr)
	{
		return (PPERF_COUNTER_DEFINITION)((PBYTE)PerfCntr + PerfCntr->ByteLength);
	}
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PERFDATA_H__8AC19065_EECB_11D4_A181_004033572A05__INCLUDED_)
