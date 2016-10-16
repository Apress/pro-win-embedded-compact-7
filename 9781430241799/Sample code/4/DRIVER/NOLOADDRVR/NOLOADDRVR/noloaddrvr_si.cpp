#define _WIN32_WINNT 0x0400
#include "noloaddrvr.h"


#ifdef DEBUG

#include <dbgapi.h>

#define	DEBUGMASK(bit)		(1 << (bit))

#define	MASK_INIT		DEBUGMASK(0)
#define	MASK_OPEN		DEBUGMASK(1)
#define	MASK_IOCTL		DEBUGMASK(2)
#define	MASK_DEINIT		DEBUGMASK(3)
#define	MASK_ERROR		DEBUGMASK(15)

DBGPARAM dpCurSettings = {
	_T("Noloaddrvr"),
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
HANDLE g_hInstance;
CRITICAL_SECTION  g_CriticalSection;	// Global critical section

//////////////////////////////////////////////////////////
BOOL WINAPI DllEntry(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            g_hInstance = hinstDLL;
            DEBUGREGISTER((HINSTANCE)hinstDLL);
            DEBUGMSG(ZONE_INIT,(_T("NOL!PROCESS_ATTACH: Process: 0x%x, ID: 0x%x \r\n"),
                GetCurrentProcess(), GetCurrentProcessId()));
            DisableThreadLibraryCalls ((HMODULE)hinstDLL);
            return TRUE;
        }
        case DLL_PROCESS_DETACH:
        {
            DEBUGMSG(ZONE_INIT,(_T("NOL!PROCESS_DETACH: Process: 0x%x, ID: 0x%x \r\n"),
                GetCurrentProcess(), GetCurrentProcessId()));
        }
        break;
    }
    return TRUE;
}

//////////////////////////////////////////////////////////
DWORD NOL_Init(LPCTSTR pContext, LPCVOID lpvBusContext)
{
    HKEY hConfig = NULL;
    DWORD dwRet = 0;
    BOOL bRc = FALSE;
    DWORD dwStatus;
    DDKWINDOWINFO WinInfo;
    ULONG unIoSpace = 1;
    DDKISRINFO IsrInfo;
    PNOL_DEVCONTXT pDevContxt = NULL;
    
    // Allocate device instance context structure
    DWORD dwPhAdd;
    pDevContxt = (PNOL_DEVCONTXT)AllocPhysMem(sizeof(NOL_DEVCONTXT), PAGE_READWRITE, 0, 0, &dwPhAdd);
    if (pDevContxt != NULL)
    {
        InitializeCriticalSection(&g_CriticalSection);
        pDevContxt->hDriverMan = (HANDLE)GetCurrentProcessId();
    }
    else
    {
        DEBUGMSG(ZONE_INIT, (_T("NOL!not enough memory\r\n")));
        return dwRet;
    }
    
    // Open active device registry key to retrieve activation info
    DEBUGMSG(ZONE_INIT, (_T("NOL!Starting NOL_Init\r\n")));
    hConfig = OpenDeviceKey(pContext);
    if (hConfig != NULL)
    {
        
        // Read window information
        WinInfo.cbSize = sizeof(WinInfo);
        dwStatus = DDKReg_GetWindowInfo(hConfig, &WinInfo);
        if (dwStatus != ERROR_SUCCESS)
        {
            DEBUGMSG(ZONE_INIT, (_T("NOL!Error Getting window information\r\n")));
            FreePhysMem(pDevContxt);
            return dwRet;
        }
        pDevContxt->dwBusNumber = WinInfo.dwBusNumber;
        pDevContxt->dwInterfaceType = WinInfo.dwInterfaceType;
        
        // Read ISR information
        IsrInfo.cbSize = sizeof(IsrInfo);
        dwStatus = DDKReg_GetIsrInfo(hConfig, &IsrInfo);
        if (dwStatus != ERROR_SUCCESS)
        {
            DEBUGMSG(ZONE_INIT, (_T("NOL!Error Getting ISR information\r\n")));
            FreePhysMem(pDevContxt);
            return dwRet;
        }
    }
    else
    {
        DEBUGMSG(ZONE_INIT, (_T("NOL!Failed to open active device key\r\n")));
        FreePhysMem(pDevContxt);
        return dwRet;
    }
    RegCloseKey(hConfig);
    
    // Obtain hardware I/O address - initialized to memory mapped IO space
    pDevContxt->ioPhysicalBase.LowPart = WinInfo.memWindows[0].dwBase;
    pDevContxt->ioPhysicalBase.HighPart = 0;
    pDevContxt->dwIoRange = WinInfo.memWindows[0].dwLen;
    if (HalTranslateBusAddress((INTERFACE_TYPE)pDevContxt->dwInterfaceType, WinInfo.dwBusNumber, pDevContxt->ioPhysicalBase, &unIoSpace, &pDevContxt->ioPhysicalBase))
    {
        
        // Is it memory mapped IO or in IO space.
        if (!unIoSpace)
        {
            pDevContxt->bIoMemMapped = TRUE;
            if ((pDevContxt->dwIoAddr = (DWORD)MmMapIoSpace(pDevContxt->ioPhysicalBase, pDevContxt->dwIoRange, FALSE)) == NULL)
            {
                DEBUGMSG(ZONE_INIT, (_T("NOL!Error mapping IO Ports failure\r\n")));
                FreePhysMem(pDevContxt);
                
                // You may want to disable IO device here
                return dwRet;
            }
        }
        else
        {
            pDevContxt->bIoMemMapped = FALSE;
            pDevContxt->IoAddr = (UCHAR)pDevContxt->ioPhysicalBase.LowPart;
        }
    }
    else
    {
        DEBUGMSG(ZONE_INIT, (_T("NOL!Error HalTranslateBusAddress call failure\r\n")));
        FreePhysMem(pDevContxt);
        
        // You may want to disable IO device here
        return dwRet;
    }
    
    // We got here if all succeded so return the device context pointer
    dwRet = (DWORD)pDevContxt;
    return dwRet;
}

//////////////////////////////////////////////////////////
BOOL NOL_PreDeinit(DWORD hDeviceContext)
{
    BOOL bRet = TRUE;
    
    // TODO: Add code to implement Pre-Deinitialization to prevent race condition:
    return bRet;
}

//////////////////////////////////////////////////////////
BOOL NOL_Deinit(DWORD hDeviceContext)
{
    BOOL bRet = TRUE;
    PNOL_DEVCONTXT pDevContxt = (PNOL_DEVCONTXT)hDeviceContext;
    
    // TODO: Add code to implement Deinitialization:
    FreePhysMem(pDevContxt);
    return bRet;
}

//////////////////////////////////////////////////////////
DWORD NOL_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    DWORD dwRet = DRVR_INVALID_HANDLE;
    PNOL_DEVCONTXT pDevContxt = (PNOL_DEVCONTXT)hDeviceContext;
    if (pDevContxt == NULL)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return dwRet;
    }
    InterlockedIncrement(&pDevContxt->lOpenCount);
    EnterCriticalSection(&g_CriticalSection);
    
    // The following code prevents opening more than one instance
    if (pDevContxt->bOpenEx == TRUE)
    {
        if (pDevContxt->lOpenCount > 0)
        {
            DEBUGMSG(ZONE_OPEN, (_T("NOL!There is already an open instance.\r\n")));
        }
    }
    else
    {
        pDevContxt->bOpenEx = TRUE;
        
        // TODO: Add code to implement opening an instance:
        
        dwRet = (DWORD)pDevContxt;
    }
    LeaveCriticalSection(&g_CriticalSection);
    if (dwRet == DRVR_INVALID_HANDLE) // Not Opened
    {
        InterlockedDecrement(&pDevContxt->lOpenCount);
    }
    return dwRet;
}

//////////////////////////////////////////////////////////
BOOL NOL_PreClose(DWORD hOpenContext)
{
    BOOL bRet = TRUE;
    PNOL_DEVCONTXT pDevContxt = (PNOL_DEVCONTXT)hOpenContext;
    
    // TODO: Add code to implement pre-closing an instance to prevent race condition:
    return bRet;
}

//////////////////////////////////////////////////////////
BOOL NOL_Close(DWORD hOpenContext)
{
    BOOL bRet = TRUE;
    PNOL_DEVCONTXT pDevContxt = (PNOL_DEVCONTXT)hOpenContext;
    InterlockedDecrement(&pDevContxt->lOpenCount);
    InterlockedExchange((volatile LONG*)&pDevContxt->bOpenEx, FALSE);
    
    // TODO: Add code to implement closing an instance:
    return bRet;
}

//////////////////////////////////////////////////////////
void NOL_PowerUp(DWORD hDeviceContext)
{
    
    // TODO: Add code to implement PowerUp:
}

//////////////////////////////////////////////////////////
void NOL_PowerDown(DWORD hDeviceContext)
{
    
    // TODO: Add code to implement PowerDown:
}

//////////////////////////////////////////////////////////
BOOL NOL_Cancel(DWORD hOpenContext, HANDLE hAsyncHandle)
{
    DWORD dwRet = 0;
    PNOL_DEVCONTXT pDevContxt = (PNOL_DEVCONTXT)hOpenContext;
    
    // TODO: Add code to implement canceling IO:
    return dwRet;
}

//////////////////////////////////////////////////////////
BOOL NOL_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut, HANDLE hAsyncRef)
{
    PNOL_DEVCONTXT pDevContxt = (PNOL_DEVCONTXT)hOpenContext;
    BOOL bRet = TRUE;
    DWORD dwErr = 0;
    
    // TODO: Add code to implement IOCTL codes:
    // The following commented code should guide you to handle access to embedded pointers
    // typedef struct _INPUT_tag
    // {
    //     UCHAR *pEmbedded;
    //     DWORD dwSize;
    // }INPUT, *PINPUT;
    // PUCHAR g_pMappedEmbedded
    //
    // HRESULT hr = E_WASNEVERSET;
    // PINPUT pInput = pBufIn;
    // hr = CeOpenCallerBuffer((PVOID*)&g_pMappedEmbedded, pInput->pEmbedded, pInput->dwSize, ARG_I_PTR, FALSE);
    // Fail if FAILED(hr) == true
    // When done with pointer
    // hr = CeCloseCallerBuffer((PVOID)g_pMappedEmbedded, pInput->pEmbedded, pInput->dwSize, ARG_I_PTR);
    // For asynchronous marshailng of an embedded pointer call:
    // CeAllocAsynchronousBuffer and 
    // CeFreeAsynchronousBuffer when done!
    
    return bRet;
}

//////////////////////////////////////////////////////////
DWORD NOL_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count, HANDLE hAsyncRef)
{
    DWORD dwRet = 0;
    PNOL_DEVCONTXT pDevContxt = (PNOL_DEVCONTXT)hOpenContext;
    
    // TODO: Add code to implement read:
    return dwRet;
}

//////////////////////////////////////////////////////////
DWORD NOL_Write(DWORD hOpenContext, LPCVOID pSourceBytes, DWORD NumberOfBytes, HANDLE hAsyncRef)
{
    DWORD dwRet = 0;
    PNOL_DEVCONTXT pDevContxt = (PNOL_DEVCONTXT)hOpenContext;
    
    // TODO: Add code to implement write:
    return dwRet;
}

//////////////////////////////////////////////////////////
DWORD NOL_Seek(DWORD hOpenContext, long Amount, DWORD Type)
{
    DWORD dwRet = 0;
    
    // TODO: Add code to implement seek:
    return dwRet;
}
