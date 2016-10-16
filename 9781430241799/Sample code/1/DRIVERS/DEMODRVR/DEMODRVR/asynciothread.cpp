#define _WIN32_WINNT 0x0400
#include "Demodrvr.H"

extern IOCTLWTPARAMS  g_AsyncIOParams;

/////////////////////////////////////////////////////////////////////
// Asynchronous IO request handler thread
//
DWORD AsyncIOThread(LPVOID lpParameter)
{
    PIOCTLWTPARAMS pParam = (PIOCTLWTPARAMS)lpParameter;
    HRESULT hRes = NULL;
	TCHAR buf[65536];
	BOOL bComplete = 0;    
	PBYTE pBuf = pParam->pBufIn;

    for (int i = 0; i < (int)pParam->dwInLen; i++)
    {
		buf[i] = *pBuf++;
        if (i % 100 == 0)
        {
            SetIoProgress(pParam->hAsyncIO, i);
			Sleep(50);
        }
    }
    if (pParam->hAsyncIO != NULL)
    {
        bComplete = CompleteAsyncIo(pParam->hAsyncIO, pParam->dwInLen, 0);
    }
    // Depending on the IO direction: if input then de-comment the first call to CeCloseCallerBuffer
    // otherwise de-comment the second call to CeCloseCallerBuffer
    //
    hRes = CeCloseCallerBuffer(pParam->pBufIn, pParam->pSrcBufIn, pParam->dwInLen, ARG_I_PTR);
  
    return 0;
}
    
