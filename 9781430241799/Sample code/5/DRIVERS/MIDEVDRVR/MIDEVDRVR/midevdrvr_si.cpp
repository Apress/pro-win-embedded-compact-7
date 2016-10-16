#define _WIN32_WINNT 0x0400
#include "midevdrvr.h"


#ifdef DEBUG

#include <dbgapi.h>

#define	DEBUGMASK(bit)		(1 << (bit))

#define	MASK_INIT		DEBUGMASK(0)
#define	MASK_OPEN		DEBUGMASK(1)
#define	MASK_IOCTL		DEBUGMASK(2)
#define	MASK_DEINIT		DEBUGMASK(3)
#define	MASK_ERROR		DEBUGMASK(15)

DBGPARAM dpCurSettings = {
	_T("Midevdrvr"),
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
PINSTANCE_DATA g_pInstances[2]; // we allow only two instances for our example

//////////////////////////////////////////////////////////
BOOL WINAPI DllEntry(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            g_hInstance = hinstDLL;
            DEBUGREGISTER((HINSTANCE)hinstDLL);
            DEBUGMSG(ZONE_INIT,(_T("MID!PROCESS_ATTACH: Process: 0x%x, ID: 0x%x \r\n"),
                GetCurrentProcess(), GetCurrentProcessId()));
            DisableThreadLibraryCalls ((HMODULE)hinstDLL);
            return TRUE;
        }
        case DLL_PROCESS_DETACH:
        {
            DEBUGMSG(ZONE_INIT,(_T("MID!PROCESS_DETACH: Process: 0x%x, ID: 0x%x \r\n"),
                GetCurrentProcess(), GetCurrentProcessId()));
        }
        break;
    }
    return TRUE;
}

//////////////////////////////////////////////////////////
DWORD MID_Init(LPCTSTR pContext, LPCVOID lpvBusContext)
{
    HKEY hConfig = NULL;
    DWORD dwRet = 0;
    BOOL bRc = FALSE;
    DWORD dwStatus;
    DDKWINDOWINFO WinInfo;
    ULONG unIoSpace = 1;
    DDKISRINFO IsrInfo;
    PMID_DEVCONTXT pDevContxt = NULL;
    
    // Allocate device instance context structure
    DWORD dwPhAdd;
    pDevContxt = (PMID_DEVCONTXT)AllocPhysMem(sizeof(MID_DEVCONTXT), PAGE_READWRITE, 0, 0, &dwPhAdd);
    if (pDevContxt != NULL)
    {
        InitializeCriticalSection(&g_CriticalSection);
        pDevContxt->hDriverMan = (HANDLE)GetCurrentProcessId();
    }
    else
    {
        DEBUGMSG(ZONE_INIT, (_T("MID!not enough memory\r\n")));
        return dwRet;
    }
    
    // Open active device registry key to retrieve activation info
    DEBUGMSG(ZONE_INIT, (_T("MID!Starting MID_Init\r\n")));
    hConfig = OpenDeviceKey(pContext);
    if (hConfig != NULL)
    {
        
        // Read window information
        WinInfo.cbSize = sizeof(WinInfo);
        dwStatus = DDKReg_GetWindowInfo(hConfig, &WinInfo);
        if (dwStatus != ERROR_SUCCESS)
        {
            DEBUGMSG(ZONE_INIT, (_T("MID!Error Getting window information\r\n")));
            FreePhysMem(pDevContxt);
            return dwRet;
        }
        pDevContxt->dwBusNumber = WinInfo.dwBusNumber;
        pDevContxt->dwInterfaceType = WinInfo.dwInterfaceType;
        
    }
    else
    {
        DEBUGMSG(ZONE_INIT, (_T("MID!Failed to open active device key\r\n")));
        FreePhysMem(pDevContxt);
        return dwRet;
    }
    RegCloseKey(hConfig);
    
	pDevContxt->bOpenEx = FALSE; // allowing more than one open instance
	// We got here if all succeded so return the device context pointer
    dwRet = (DWORD)pDevContxt;
    return dwRet;
}

//////////////////////////////////////////////////////////
BOOL MID_PreDeinit(DWORD hDeviceContext)
{
    BOOL bRet = TRUE;
    
    // TODO: Add code to implement Pre-Deinitialization to prevent race condition:
    return bRet;
}

//////////////////////////////////////////////////////////
BOOL MID_Deinit(DWORD hDeviceContext)
{
    BOOL bRet = TRUE;
    PMID_DEVCONTXT pDevContxt = (PMID_DEVCONTXT)hDeviceContext;
    
    // TODO: Add code to implement Deinitialization:
    
    FreePhysMem(pDevContxt);
    return bRet;
}

//////////////////////////////////////////////////////////
//
//	This xxx_open implementation allows opening two instances
//  It increments the open instances count
//  It allocates a new INSTANCE_DATA object for each instance
//  puts it into the instances list and returns a handle to this object
//
//////////////////////////////////////////////////////////
DWORD MID_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    DWORD dwRet = DRVR_INVALID_HANDLE;
    PMID_DEVCONTXT pDevContxt = (PMID_DEVCONTXT)hDeviceContext;
	PINSTANCE_DATA pInstance = NULL;
    if (pDevContxt == NULL)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return dwRet;
    }
	if (pDevContxt->lOpenCount < 2)
	{
		InterlockedIncrement(&pDevContxt->lOpenCount);
	}
	else
	{
		SetLastError(ERROR_ALREADY_ASSIGNED);
		return 0;
	}
    EnterCriticalSection(&g_CriticalSection);
 	pInstance = (PINSTANCE_DATA)new (INSTANCE_DATA);
	if (pInstance != NULL)
	{
		pInstance->pDevContxt = pDevContxt;
		g_pInstances[pDevContxt->lOpenCount - 1] = pInstance;
		dwRet = (DWORD)pInstance;
	}
    LeaveCriticalSection(&g_CriticalSection);

    if (dwRet == DRVR_INVALID_HANDLE) // Not Opened
    {
        InterlockedDecrement(&pDevContxt->lOpenCount);
		if (pDevContxt->lOpenCount > 0)
		{
			g_pInstances[pDevContxt->lOpenCount - 1] = NULL;
		}
        SetLastError(ERROR_INVALID_HANDLE);
    }
    return dwRet;
}

//////////////////////////////////////////////////////////
BOOL MID_PreClose(DWORD hOpenContext)
{
    BOOL bRet = TRUE;
    PMID_DEVCONTXT pDevContxt = (PMID_DEVCONTXT)hOpenContext;
    
    // TODO: Add code to implement pre-closing an instance to prevent race condition:
    return bRet;
}

//////////////////////////////////////////////////////////
//
//	Traverses the instance object list and finds the object
//  Deletes the object and removes the list entry and
//  decrements the opne instance count
//
//////////////////////////////////////////////////////////
BOOL MID_Close(DWORD hOpenContext)
{
    BOOL bRet = TRUE;
    PINSTANCE_DATA pInstance = (PINSTANCE_DATA)hOpenContext;

    // TODO: Add code to implement closing an instance:
    PMID_DEVCONTXT pDevContxt = pInstance->pDevContxt;

	long lOpenCnt = pDevContxt->lOpenCount;
	if (lOpenCnt > 0)
	{
		for (int i = 0; i < lOpenCnt; i++)
		{
			if (g_pInstances[i] == pInstance)
			{
				delete pInstance;
				g_pInstances[i] = NULL;  
				InterlockedDecrement(&pDevContxt->lOpenCount);
			}
			
		}
	}
    return bRet;
}

//////////////////////////////////////////////////////////
void MID_PowerUp(DWORD hDeviceContext)
{
    
    // TODO: Add code to implement PowerUp:
}

//////////////////////////////////////////////////////////
void MID_PowerDown(DWORD hDeviceContext)
{
    
    // TODO: Add code to implement PowerDown:
}

//////////////////////////////////////////////////////////
BOOL MID_Cancel(DWORD hOpenContext, HANDLE hAsyncHandle)
{
    DWORD dwRet = 0;
    PMID_DEVCONTXT pDevContxt = (PMID_DEVCONTXT)hOpenContext;
    
    // TODO: Add code to implement canceling IO:
    return dwRet;
}

//////////////////////////////////////////////////////////
BOOL MID_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut, HANDLE hAsyncRef)
{
    PMID_DEVCONTXT pDevContxt = (PMID_DEVCONTXT)hOpenContext;
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
    
    switch (dwCode)
    {
        case IOCTL_MIDEVDRVR_FOO1:
            // Enter critical section if needed otherwise erase
            EnterCriticalSection(&g_CriticalSection);
            // Add implementation code
            LeaveCriticalSection(&g_CriticalSection);
            break;
        default:
            break;
    }
    
    // pass back appropriate response codes
    SetLastError(dwErr);
    if ((dwErr != ERROR_SUCCESS) && (dwErr != ERROR_IO_PENDING))
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
//
// Very simple implementation that thraverses the list of
// open instances and returns the instance local data
//
//////////////////////////////////////////////////////////
DWORD MID_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count, HANDLE hAsyncRef)
{
    DWORD dwRet = 0;
    PINSTANCE_DATA pInstance = (PINSTANCE_DATA)hOpenContext;

    // TODO: Add code to implement closing an instance:
    PMID_DEVCONTXT pDevContxt = pInstance->pDevContxt;

	long lOpenCnt = pDevContxt->lOpenCount;
	if (lOpenCnt > 0)
	{
		for (int i = 0; i < lOpenCnt; i++)
		{
			if (g_pInstances[i] == pInstance)
			{
				memcpy(pBuffer, pInstance->pInstanceData, Count);
				dwRet = Count;
			}
		}
	}
	return dwRet;
}

//////////////////////////////////////////////////////////
//
// Very simple implementation that thraverses the list of
// open instances and allocates memory for the instance local
// data and writes the data to into this memory
//
//////////////////////////////////////////////////////////
DWORD MID_Write(DWORD hOpenContext, LPCVOID pSourceBytes, DWORD NumberOfBytes, HANDLE hAsyncRef)
{
    DWORD dwRet = 0;
	LPVOID pVoid = NULL;
    PINSTANCE_DATA pInstance = (PINSTANCE_DATA)hOpenContext;

    // TODO: Add code to implement closing an instance:
    PMID_DEVCONTXT pDevContxt = pInstance->pDevContxt;

	long lOpenCnt = pDevContxt->lOpenCount;
	if (lOpenCnt > 0)
	{
		for (int i = 0; i < lOpenCnt; i++)
		{
			if (g_pInstances[i] == pInstance)
			{
				pVoid = VirtualAlloc(NULL, NumberOfBytes, MEM_COMMIT, PAGE_READWRITE);
				if (pVoid == NULL)
				{
					pInstance->pInstanceData = NULL;
					dwRet = -1;
				}
				else
				{
					pInstance->pInstanceData = (DWORD*)pVoid;
					memcpy(pInstance->pInstanceData, pSourceBytes, NumberOfBytes);
					dwRet = NumberOfBytes;
				}
			}
		}
	}    
	return dwRet;
}

//////////////////////////////////////////////////////////
DWORD MID_Seek(DWORD hOpenContext, long Amount, DWORD Type)
{
    DWORD dwRet = 0;
    
    // TODO: Add code to implement seek:
    return dwRet;
}
