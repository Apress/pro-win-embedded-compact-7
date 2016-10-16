#define _WIN32_WINNT 0x0400
#include "demodrvr.h"


#ifdef DEBUG

#include <dbgapi.h>

#define	DEBUGMASK(bit)		(1 << (bit))

#define	MASK_INIT		DEBUGMASK(0)
#define	MASK_OPEN		DEBUGMASK(1)
#define	MASK_IOCTL		DEBUGMASK(2)
#define	MASK_DEINIT		DEBUGMASK(3)
#define	MASK_ERROR		DEBUGMASK(15)

DBGPARAM dpCurSettings = {
	_T("Demodrvr"),
	{
		_T("Init"), _T("Open"), _T("Ioctl"), _T("DeInit"),
		_T(""), _T(""), _T(""), _T(""),
		_T(""),_T(""),_T(""),_T(""),
		_T(""),_T(""),_T(""),_T("Error")
	},
	MASK_INIT | MASK_DEINIT
};
#endif //DEBUG

// Global variables
HANDLE g_hInstance;
IOCTLWTPARAMS  g_AsyncIOParams;
HANDLE g_hAsyncThread;

//////////////////////////////////////////////////////////
BOOL WINAPI DllEntry(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            g_hInstance = hinstDLL;
            DEBUGREGISTER((HINSTANCE)hinstDLL);
            DEBUGMSG(ZONE_INIT,(_T("DMO!PROCESS_ATTACH: Process: 0x%x, ID: 0x%x \r\n"),
                GetCurrentProcess(), GetCurrentProcessId()));
            DisableThreadLibraryCalls ((HMODULE)hinstDLL);
            return TRUE;
        }
        case DLL_PROCESS_DETACH:
        {
            DEBUGMSG(ZONE_INIT,(_T("DMO!PROCESS_DETACH: Process: 0x%x, ID: 0x%x \r\n"),
                GetCurrentProcess(), GetCurrentProcessId()));
        }
        break;
    }
    return TRUE;
}

//////////////////////////////////////////////////////////
DWORD DMO_Init(LPCTSTR pContext, LPCVOID lpvBusContext)
{
    HKEY hConfig;
    DWORD dwRet = 0;
    BOOL bRc = FALSE;
    DWORD dwStatus;
    DDKWINDOWINFO WinInfo;
    ULONG unIoSpace = 1;
    DDKISRINFO IsrInfo;
    PDMO_DEVCONTXT pDevContxt = NULL;
    
    // Allocate device instance context structure
    DWORD dwPhAdd;
    pDevContxt = (PDMO_DEVCONTXT)AllocPhysMem(sizeof(DMO_DEVCONTXT), PAGE_READWRITE, 0, 0, &dwPhAdd);
    if (pDevContxt != NULL)
    {
        InitializeCriticalSection(&pDevContxt->CriticalSection);
        pDevContxt->hDriverMan = (HANDLE)GetCurrentProcessId();
    }
    else
    {
        DEBUGMSG(ZONE_INIT, (_T("DMO!not enough memory\r\n")));
        return dwRet;
    }
    
    // Open active device registry key to retrieve activation info
    DEBUGMSG(ZONE_INIT, (_T("DMO!Starting DMO_Init\r\n")));
    if (hConfig = OpenDeviceKey(pContext))
    {
        
        // Read window information
        WinInfo.cbSize = sizeof(WinInfo);
        dwStatus = DDKReg_GetWindowInfo(hConfig, &WinInfo);
        if (dwStatus != ERROR_SUCCESS)
        {
            DEBUGMSG(ZONE_INIT, (_T("DMO!Error Getting window information\r\n")));
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
            DEBUGMSG(ZONE_INIT, (_T("DMO!Error Getting ISR information\r\n")));
            FreePhysMem(pDevContxt);
            return dwRet;
        }
    }
    else
    {
        DEBUGMSG(ZONE_INIT, (_T("DMO!Failed to open active device key\r\n")));
        FreePhysMem(pDevContxt);
        return dwRet;
    }
    RegCloseKey(hConfig);
    
    // Obtain hardware I/O address - initialized to IO space
    pDevContxt->ioPhysicalBase.LowPart = WinInfo.ioWindows[0].dwBase;
    pDevContxt->ioPhysicalBase.HighPart = 0;
    pDevContxt->dwIoRange = WinInfo.ioWindows[0].dwLen;
    if (HalTranslateBusAddress((INTERFACE_TYPE)pDevContxt->dwInterfaceType, WinInfo.dwBusNumber, pDevContxt->ioPhysicalBase, &unIoSpace, &pDevContxt->ioPhysicalBase))
    {
        
        // Is it memory mapped IO or in IO space.
        if (!unIoSpace)
        {
            pDevContxt->bIoMemMapped = TRUE;
            if ((pDevContxt->dwIoAddr = (DWORD)MmMapIoSpace(pDevContxt->ioPhysicalBase, pDevContxt->dwIoRange, FALSE)) == NULL)
            {
                DEBUGMSG(ZONE_INIT, (_T("DMO!Error mapping IO Ports failure\r\n")));
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
        DEBUGMSG(ZONE_INIT, (_T("DMO!Error HalTranslateBusAddress call failure\r\n")));
        FreePhysMem(pDevContxt);
        
        // You may want to disable IO device here
        return dwRet;
    }
    
    // Install ISR handler
    
    // We make sure we got a good result from DDKReg_GetIsrInfo
    if (IsrInfo.szIsrDll[0] != 0)
    {
        if (IsrInfo.szIsrHandler[0]!= 0 && IsrInfo.dwIrq == IRQ_UNSPECIFIED && IsrInfo.dwSysintr == SYSINTR_UNDEFINED)
        {
            DEBUGMSG(ZONE_INIT, (_T("DMO!Corrupted or missing installable ISR settings\r\n")));
            FreePhysMem(pDevContxt);
            return dwRet;
        }
        else
        {
            pDevContxt->hInstISR = LoadIntChainHandler(IsrInfo.szIsrDll,IsrInfo.szIsrHandler, (BYTE)IsrInfo.dwIrq);
            if (!pDevContxt->hInstISR)
            {
                DEBUGMSG(ZONE_INIT, (_T("DMO!Failed to install ISR handler\r\n")));
                FreePhysMem(pDevContxt);
                return dwRet;
            }
            else
            {
                GIISR_INFO Info;
                PHYSICAL_ADDRESS PortAddress = pDevContxt->ioPhysicalBase;
                DEBUGMSG(ZONE_INIT, (_T("DMO!ISR handler installed , Dll: '%s', Handler: '%s', Irq: %d\r\n"),
                    						IsrInfo.szIsrDll, IsrInfo.szIsrHandler, IsrInfo.dwIrq));
                pDevContxt->dwSysIntr = IsrInfo.dwSysintr;
                
                // Set up ISR handler
                Info.SysIntr = IsrInfo.dwSysintr;
                Info.CheckPort = TRUE;
                Info.PortIsIO = pDevContxt->bIoMemMapped;
                Info.UseMaskReg =  FALSE;
                Info.PortAddr = pDevContxt->dwIoAddr;
                Info.PortSize = sizeof(DWORD);
                Info.MaskAddr = 0x0000;
                if (!KernelLibIoControl(pDevContxt->hInstISR, IOCTL_GIISR_INFO, &Info, sizeof(Info), NULL, 0, NULL))
                {
                    DEBUGMSG(ZONE_ERROR, (_T("DMO!KernelLibIoControl call failed.\r\n")));
                }
            }
        }
    }
    pDevContxt->dwIstPriority = 100;
    pDevContxt->dwIstQuantum = 100;
    
    // Setup SYSINTR ID from registry
    pDevContxt->dwSysIntr = IsrInfo.dwSysintr;
    
    // Creat IST procedure
    dwRet = CreateInterruptServiceThread((PDMO_DEVCONTXT)pDevContxt);
    if (dwRet != 0)
    {
        FreePhysMem(pDevContxt);
        return 0;
    }
    
    // We got here if all succeded so return the device context pointer
    dwRet = (DWORD)pDevContxt;
    return dwRet;
}

//////////////////////////////////////////////////////////
BOOL DMO_PreDeinit(DWORD hDeviceContext)
{
    BOOL bRet = TRUE;
    
    // TODO: Add code to implement Pre-Deinitialization to prevent race condition:
    return bRet;
}

//////////////////////////////////////////////////////////
BOOL DMO_Deinit(DWORD hDeviceContext)
{
    BOOL bRet = TRUE;
    PDMO_DEVCONTXT pDevContxt = (PDMO_DEVCONTXT)hDeviceContext;
    
    // TODO: Add code to implement Deinitialization:
    
    // Destroy IST procedure
    DestroyInterruptServiceThread(pDevContxt);
    FreePhysMem(pDevContxt);
    return bRet;
}

//////////////////////////////////////////////////////////
DWORD DMO_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    DWORD dwRet = DRVR_INVALID_HANDLE;
    PDMO_DEVCONTXT pDevContxt = (PDMO_DEVCONTXT)hDeviceContext;
    if (pDevContxt == NULL)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return dwRet;
    }
    InterlockedIncrement(&pDevContxt->lOpenCount);
    EnterCriticalSection(&pDevContxt->CriticalSection);
    
    // The following code prevents opening more than one instance
    if (pDevContxt->bOpenEx == TRUE)
    {
        if (pDevContxt->lOpenCount > 0)
        {
            DEBUGMSG(ZONE_OPEN, (_T("DMO!There is already an open instance.\r\n")));
        }
    }
    else
    {
        pDevContxt->bOpenEx = TRUE;
        
        // TODO: Add code to implement opening an instance:
        
        dwRet = (DWORD)pDevContxt;
    }
    LeaveCriticalSection(&pDevContxt->CriticalSection);
    if (dwRet == DRVR_INVALID_HANDLE) // Not Opened
    {
        InterlockedDecrement(&pDevContxt->lOpenCount);
    }
    return dwRet;
}

//////////////////////////////////////////////////////////
BOOL DMO_PreClose(DWORD hOpenContext)
{
    BOOL bRet = TRUE;
    PDMO_DEVCONTXT pDevContxt = (PDMO_DEVCONTXT)hOpenContext;
    
    // TODO: Add code to implement pre-closing an instance to prevent race condition:
    return bRet;
}

//////////////////////////////////////////////////////////
BOOL DMO_Close(DWORD hOpenContext)
{
    BOOL bRet = TRUE;
    PDMO_DEVCONTXT pDevContxt = (PDMO_DEVCONTXT)hOpenContext;
    InterlockedDecrement(&pDevContxt->lOpenCount);
    InterlockedExchange((volatile LONG*)&pDevContxt->bOpenEx, FALSE);
    
    // TODO: Add code to implement closing an instance:
    return bRet;
}

//////////////////////////////////////////////////////////
void DMO_PowerUp(DWORD hDeviceContext)
{
    
    // TODO: Add code to implement PowerUp:
}

//////////////////////////////////////////////////////////
void DMO_PowerDown(DWORD hDeviceContext)
{
    
    // TODO: Add code to implement PowerDown:
}

//////////////////////////////////////////////////////////
BOOL DMO_Cancel(DWORD hOpenContext, HANDLE hAsyncHandle)
{
    DWORD dwRet = 0;
    PDMO_DEVCONTXT pDevContxt = (PDMO_DEVCONTXT)hOpenContext;
    
    // TODO: Add code to implement canceling IO:
    return dwRet;
}

//////////////////////////////////////////////////////////
BOOL DMO_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut, HANDLE hAsyncRef)
{
    PDMO_DEVCONTXT pDevContxt = (PDMO_DEVCONTXT)hOpenContext;
    BOOL bRet = TRUE;
    DWORD dwErr = 0;
	HANDLE hAsyncIO = NULL;
    
    switch (dwCode)
    {
        case IOCTL_DEMODRVR_ASYNC_WRITE:
			if (hAsyncRef)
			{
				hAsyncIO = CreateAsyncIoHandle(hAsyncRef, (LPVOID*)pBufIn, 0);
			}  

			g_AsyncIOParams.pSrcBufIn = (PBYTE)pBufIn;

			CeOpenCallerBuffer((PVOID*)(&g_AsyncIOParams.pBufIn), (PVOID)pBufIn,  dwLenIn, ARG_I_PTR, TRUE);

			g_AsyncIOParams.hAsyncIO = hAsyncIO;
			g_AsyncIOParams.dwInLen = dwLenIn;

			g_hAsyncThread = CreateThread(NULL,163840, (LPTHREAD_START_ROUTINE)AsyncIOThread,
											(LPVOID)&g_AsyncIOParams, CREATE_SUSPENDED | STACK_SIZE_PARAM_IS_A_RESERVATION, 
											NULL);
			if (g_hAsyncThread == NULL)
			{
				DEBUGMSG(ZONE_IOCTL,(_T("TST!failed …\r\n")));
				return FALSE;
			}

			// Raise asyn thread priority if you want IO operation 
			// to not last ethernity 
			CeSetThreadPriority(g_hAsyncThread, 150);

			ResumeThread(g_hAsyncThread);  
			dwErr = ERROR_IO_PENDING;
            break;
        default:
            break;
    }
    
    // pass back appropriate response codes
    SetLastError(dwErr);
    if((dwErr != ERROR_SUCCESS) && (dwErr != ERROR_IO_PENDING))
    {
        bRet = FALSE;
    }
    else
    {
        bRet = TRUE;
    }
    
    return bRet;
}

//////////////////////////////////////////////////////////
DWORD DMO_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count, HANDLE hAsyncRef)
{
    DWORD dwRet = 0;
    HANDLE hAsyncIO = NULL;
    PDMO_DEVCONTXT pDevContxt = (PDMO_DEVCONTXT)hOpenContext;
	double a[18];
    
    // TODO: Add code to implement read:
	for (int i = 0; i < 18; i++)
	{
		a[i] = 0;
	}
	a[5] = 1.0;

	memcpy((double*)pBuffer, (double*)&a, 144);
	dwRet = Count = 18;
    return dwRet;
}

//////////////////////////////////////////////////////////
DWORD DMO_Write(DWORD hOpenContext, LPCVOID pSourceBytes, DWORD NumberOfBytes, HANDLE hAsyncRef)
{
    DWORD dwRet = 0;
    HANDLE hAsyncIO = NULL;
    PDMO_DEVCONTXT pDevContxt = (PDMO_DEVCONTXT)hOpenContext;
    
    // TODO: Add code to implement write:
    if (hAsyncRef)
    {
        hAsyncIO = CreateAsyncIoHandle(hAsyncRef, (LPVOID*)pSourceBytes, 0);
    }
    g_AsyncIOParams.pSrcBufIn = (PBYTE)pSourceBytes;
    CeOpenCallerBuffer((PVOID*)(&g_AsyncIOParams.pBufIn), (PVOID)pSourceBytes,  NumberOfBytes, ARG_I_PTR, TRUE);
    g_AsyncIOParams.hAsyncIO = hAsyncIO;
    g_AsyncIOParams.dwInLen = NumberOfBytes;
    
    // You may want to modify the stack size if input buffer is huge
    g_hAsyncThread = CreateThread(NULL, 65536,(LPTHREAD_START_ROUTINE)AsyncIOThread,(LPVOID)&g_AsyncIOParams, CREATE_SUSPENDED | STACK_SIZE_PARAM_IS_A_RESERVATION, NULL);
    if (g_hAsyncThread == NULL)
    {
        DEBUGMSG(ZONE_WRITE,(_T("DMO! AsyncIOThread Create: Failed to create thread\r\n")));
        return FALSE;
    }
    CeSetThreadPriority(g_hAsyncThread, 150);
    ResumeThread(g_hAsyncThread);
	dwRet = ERROR_IO_PENDING;
    return dwRet;
}

//////////////////////////////////////////////////////////
DWORD DMO_Seek(DWORD hOpenContext, long Amount, DWORD Type)
{
    DWORD dwRet = 0;
    
    // TODO: Add code to implement seek:
    return dwRet;
}
