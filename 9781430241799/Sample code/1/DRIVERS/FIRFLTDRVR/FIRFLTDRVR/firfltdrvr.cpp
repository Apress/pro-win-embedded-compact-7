#define _WIN32_WINNT 0x0400
#include "firfltdrvr.h"
#include "..\..\DEMODRVR\SDK\DemodrvrSDK.h"

#ifdef DEBUG

#define	DEBUGMASK(bit)		(1 << (bit))

#define	MASK_INIT		DEBUGMASK(0)
#define	MASK_OPEN		DEBUGMASK(1)
#define	MASK_IOCTL		DEBUGMASK(2)
#define	MASK_DEINIT		DEBUGMASK(3)
#define	MASK_ERROR		DEBUGMASK(15)

DBGPARAM dpCurSettings = {
	_T("Firfltdrvr"),
	{
		_T("H"), _T("Open"), _T("Ioctl"), _T("DeInit"),
		_T(""), _T(""), _T(""), _T(""),
		_T(""),_T(""),_T(""),_T(""),
		_T(""),_T(""),_T(""),_T("Error")
	},
	MASK_INIT | MASK_DEINIT
};
#endif //DEBUG

// Global variables
#define NTAPS 6

HANDLE g_hInstance;
static double g_dCoeffs[NTAPS];

static double g_dLine[2 * NTAPS]; // delay line array
static double g_input[3 * NTAPS]; // input data array

// function to convert the input data buffer to a doubles array
void ToDoubleArray(double* pBufIn, DWORD dwLenIn)
{
	double* pDbl = pBufIn;
	for (int i = 0; i < dwLenIn; i++)
	{
		g_input[i] = *pDbl;
		pDbl++;
	}
}

// function to set zero all members of an array
void ResetArray(int nCnt, double a[])
{
	for (int i = 0; i < nCnt; i++)
	{
		a[i] = 0;
	}
}

// This is the finite impulse response filter algorithm
// This is the basic algorithm:
// 1. store input sample
// 2. calculate output sample
// 3. move delay line 
//
// using global static arrays for this 
double FIRCalcAlg(double input)
{
    int indx;
    double accum;
    
    // store input at the beginning of the delay line
    g_dLine[0] = input;

    // calc FIR (Y(n) = co(0)*inp(n)+co(1)*inp(n)+....+co(N)*inp(n-N-1)) where N is the number of taps
    accum = 0;
	// sum the taps
    for (indx = 0; indx < NTAPS; indx++) {
        accum += g_dCoeffs[indx] * g_dLine[indx];
    }

    // shift delay line 
    for (indx = NTAPS - 2; indx >= 0; indx--) {
        g_dLine[indx + 1] = g_dLine[indx];
    }

    return accum; // return the computed result
}

//////////////////////////////////////////////////////////
BOOL WINAPI DllEntry(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            g_hInstance = hinstDLL;
            DEBUGREGISTER((HINSTANCE)hinstDLL);
            DEBUGMSG(ZONE_INIT,(_T("!PROCESS_ATTACH: Process: 0x%x, ID: 0x%x \r\n"),
                GetCurrentProcess(), GetCurrentProcessId()));
            DisableThreadLibraryCalls ((HMODULE)hinstDLL);
            return TRUE;
        }
        case DLL_PROCESS_DETACH:
        {
            DEBUGMSG(ZONE_INIT,(_T("!PROCESS_DETACH: Process: 0x%x, ID: 0x%x \r\n"),
                GetCurrentProcess(), GetCurrentProcessId()));
        }
        break;
    }
    return TRUE;
}

//////////////////////////////////////////////////////////
extern "C" PDRIVER_FILTER FIRFilterInit(LPCTSTR lpcFilterRegistryPath, LPCTSTR lpcDriverRegistryPath, PDRIVER_FILTER pNextFilter)
{
    Filter* pFilter = NULL;
    
    InitializeCriticalSection(&g_CriticalSection);
    EnterCriticalSection(&g_CriticalSection);
    
    DEBUGMSG(ZONE_INIT,(L"Filter: Creating new filter driver for %s\r\n",lpcDriverRegistryPath));
    
    pFilter = new Filter(lpcFilterRegistryPath,lpcDriverRegistryPath,pNextFilter);
    if (!pFilter) 
    {
        DEBUGMSG(ZONE_ERROR,(L"Filter: Error, Unable to allocate memory. %s cannot be initialized\r\n", lpcDriverRegistryPath));
        goto done;
    }
    
done:
    LeaveCriticalSection(&g_CriticalSection);
    return (PDRIVER_FILTER)pFilter;
}

//////////////////////////////////////////////////////////
Filter::Filter(LPCTSTR lpcFilterRegistryPath, LPCTSTR lpcDriverRegistryPath,
                                PDRIVER_FILTER pNextFilterParam) : DriverFilterBase(lpcFilterRegistryPath, pNextFilterParam)
{
    m_initReturn = 0;
}

//////////////////////////////////////////////////////////
Filter::~Filter()
{
    m_initReturn = 0;
    DeleteCriticalSection(&g_CriticalSection);
}

//////////////////////////////////////////////////////////
DWORD Filter::FilterInit(DWORD dwContext,LPVOID lpParam)
{
    __try
    {
        m_initReturn = pNextFilter->fnInit(dwContext, lpParam, this);
    }
    __except(ReportFault(GetExceptionInformation(),0), EXCEPTION_EXECUTE_HANDLER)
    {
        m_initReturn = 0;
    }
    
    return m_initReturn;
}

//////////////////////////////////////////////////////////
BOOL Filter::FilterPreDeinit(DWORD dwContext)
{
    BOOL bRet = FALSE;
    SetLastError(ERROR_NOT_SUPPORTED);
    return bRet;
}

//////////////////////////////////////////////////////////
BOOL Filter::FilterDeinit(DWORD dwContext)
{
    BOOL bRet = FALSE;
    
    __try
    {
        bRet = pNextFilter->fnDeinit(dwContext, this);
    }
    __except(ReportFault(GetExceptionInformation(),0), EXCEPTION_EXECUTE_HANDLER)
    {
        bRet = FALSE;
    }
    
    return bRet;
}

//////////////////////////////////////////////////////////
DWORD Filter::FilterOpen(DWORD dwContext, DWORD AccessCode, DWORD ShareMode)
{
    DWORD dwRet = 0;
    
    __try
    {
        dwRet = pNextFilter->fnOpen(dwContext, AccessCode, ShareMode, this);
    }
    __except(ReportFault(GetExceptionInformation(),0), EXCEPTION_EXECUTE_HANDLER)
    {
        dwRet = 0;
    }
    
    return dwRet;
}

//////////////////////////////////////////////////////////
BOOL Filter::FilterPreClose(DWORD dwOpenCont)
{
    BOOL bRet = FALSE;
    SetLastError(ERROR_NOT_SUPPORTED);
    return bRet;
}

//////////////////////////////////////////////////////////
BOOL Filter::FilterClose(DWORD dwOpenCont)
{
    BOOL bRet = FALSE;
    
    __try
    {
        bRet = pNextFilter->fnClose(dwOpenCont, this);
    }
    __except(ReportFault(GetExceptionInformation(),0), EXCEPTION_EXECUTE_HANDLER)
    {
        bRet = FALSE;
    }
    
    return bRet;
}

//////////////////////////////////////////////////////////
BOOL Filter::FilterControl(DWORD dwOpenCont,DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn,
                                PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut,HANDLE hAsyncRef)
{
    BOOL bRet = FALSE;
	// This IOCTL code is implemented in the filter driver only 
	// because the coefficients needed for the filter algorithm 
	// are not needed in the filtered driver.
	// All other UOCTLs are passed on without any change, however 
	// if an IOCTL would handle data that should be filtered you
	// would add filter code in the same manner that is implemented  
	// in FilterRead
	if (dwCode == IOCTL_DEMODRVR_SET_COEFFS)
	{
		double* pDbl = (double*)pBufIn;
		for (int i = 0; i < NTAPS; i++)
		{
			g_dCoeffs[i] = *pDbl;
			pDbl++;
		}
		return bRet;
	}    
    __try
    {
        bRet = pNextFilter->fnControl(dwOpenCont, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut, hAsyncRef, this);
    }
    __except(ReportFault(GetExceptionInformation(),0), EXCEPTION_EXECUTE_HANDLER)
    {
        bRet = FALSE;
    }
    
    return bRet;
}

//////////////////////////////////////////////////////////
void Filter::FilterPowerdn(DWORD dwConext)
{
    SetLastError(ERROR_NOT_SUPPORTED);
}

//////////////////////////////////////////////////////////
void Filter::FilterPowerup(DWORD dwConext)
{
    SetLastError(ERROR_NOT_SUPPORTED);
}

//////////////////////////////////////////////////////////
BOOL Filter::FilterCancelIo(DWORD dwOpenCont, HANDLE hAsyncHandle)
{
    BOOL bRet = FALSE;
    
    __try
    {
        bRet = pNextFilter->fnCancelIo(dwOpenCont, hAsyncHandle, this);
    }
    __except(ReportFault(GetExceptionInformation(),0), EXCEPTION_EXECUTE_HANDLER)
    {
        bRet = FALSE;
    }
    
    return bRet;
}

//////////////////////////////////////////////////////////
DWORD Filter::FilterRead(DWORD dwOpenCont, LPVOID pBuffer, DWORD Count, HANDLE hAsyncRef)
{
    DWORD dwRet = 0;
	double dRes = 0;
	double* output = (double*) malloc(Count*sizeof(double));

    __try
    {
		// First call the filtered device driver to retrieve its data
        dwRet = pNextFilter->fnRead(dwOpenCont, (double*)pBuffer, Count, hAsyncRef, this);
		// Filter data before sending it back
		ToDoubleArray((double*)pBuffer, Count); // This is just to convert the data buffer into an array
		ResetArray(NTAPS * 2, g_dLine);			// setting the delay line array to zeros

		// Using the the basic FIR algorithm: store input sample, calculate       
		// output sample, move delay line implemented in FIRCalcAlg
		for (int i = 0; i < Count; i++)
		{
			dRes = FIRCalcAlg(g_input[i]);
			output[i] = dRes;
		}
		// Return filtered result to caller process
		memcpy((double*)pBuffer, (double*)output, 144);
	}
    __except(ReportFault(GetExceptionInformation(),0), EXCEPTION_EXECUTE_HANDLER)
    {
        dwRet = 0;
    }
    
    return dwRet;
}

//////////////////////////////////////////////////////////
DWORD Filter::FilterWrite(DWORD dwOpenCont, LPCVOID pSourceBytes, DWORD NumberOfBytes, HANDLE hAsyncRef)
{
    DWORD dwRet = 0;
    
    __try
    {
        dwRet = pNextFilter->fnWrite(dwOpenCont, pSourceBytes, NumberOfBytes, hAsyncRef, this);
    }
    __except(ReportFault(GetExceptionInformation(),0), EXCEPTION_EXECUTE_HANDLER)
    {
        dwRet = 0;
    }
    
    return dwRet;
}

//////////////////////////////////////////////////////////
DWORD Filter::FilterSeek(DWORD dwOpenCont, long Amount, DWORD Type)
{
    DWORD dwRet = 0;
    
    __try
    {
        dwRet = pNextFilter->fnSeek(dwOpenCont, Amount, Type, this);
    }
    __except(ReportFault(GetExceptionInformation(),0), EXCEPTION_EXECUTE_HANDLER)
    {
        dwRet = 0;
    }
    
    return dwRet;
}
