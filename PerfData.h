#ifndef AFX_PERFDATA_H
#define AFX_PERFDATA_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Winperf.h>


// CPerfData window
class CPerfData {
// Construction
public:
	CPerfData();
	
	void Init();
	BOOL GetReceivedAndSentOctets(DWORD *pRecv, DWORD *pSent);
	
// Attributes
public:
	
private:
	LPSTR m_lpNameStrings;
	LPSTR *m_lpNamesArray;
	BOOL m_bIs95;
	
	void GetNameStrings();
	void ReadData9x(DWORD *pRecv, DWORD *pSent);
	void ReadDataNT(DWORD *pRecv, DWORD *pSent);
	BOOL GetPerfStats9x(LPCSTR pKey, DWORD *dwValue);
	
// Implementation
public:
	virtual ~CPerfData();
	
	// Generated message map functions
protected:
	PPERF_OBJECT_TYPE FirstObject(PPERF_DATA_BLOCK PerfData) {
		return (PPERF_OBJECT_TYPE)((PBYTE)PerfData + PerfData->HeaderLength);
	}
	
	PPERF_OBJECT_TYPE NextObject(PPERF_OBJECT_TYPE PerfObj) {
		return (PPERF_OBJECT_TYPE)((PBYTE)PerfObj + PerfObj->TotalByteLength);
	}
	
	PPERF_INSTANCE_DEFINITION FirstInstance(PPERF_OBJECT_TYPE PerfObj) {
		return (PPERF_INSTANCE_DEFINITION)((PBYTE)PerfObj + PerfObj->DefinitionLength);
	}
	
	PPERF_INSTANCE_DEFINITION NextInstance(PPERF_INSTANCE_DEFINITION PerfInst) {
		PPERF_COUNTER_BLOCK PerfCntrBlk = (PPERF_COUNTER_BLOCK)((PBYTE)PerfInst + PerfInst->ByteLength);
		return (PPERF_INSTANCE_DEFINITION)((PBYTE)PerfCntrBlk + PerfCntrBlk->ByteLength);
	}
	
	PPERF_COUNTER_DEFINITION FirstCounter(PPERF_OBJECT_TYPE PerfObj) {
		return (PPERF_COUNTER_DEFINITION) ((PBYTE)PerfObj + PerfObj->HeaderLength);
	}
	
	PPERF_COUNTER_DEFINITION NextCounter(PPERF_COUNTER_DEFINITION PerfCntr) {
		return (PPERF_COUNTER_DEFINITION)((PBYTE)PerfCntr + PerfCntr->ByteLength);
	}
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
