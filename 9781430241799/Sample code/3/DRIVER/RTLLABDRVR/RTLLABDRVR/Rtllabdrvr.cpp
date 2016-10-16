//////////////////////////////////////////////////
//Rtllabdrvr.cpp"

#include "rtllabdrvr.h"

extern LARGE_INTEGER g_liCurrentTime;

DWORD RtllabdrvrISTProc(LPVOID lpParameter)
{
    PRTL_DEVCONTXT pParam = (PRTL_DEVCONTXT)lpParameter;
	HANDLE hSyncEvent = pParam->TrigParam.hEvent;
    BOOL bDone = FALSE;
    
    while(!bDone)
    {
        if (pParam->bShutDown)
        {
            bDone = TRUE;
        }

        // TODO: Write code to process interrupt
		if (WaitForSingleObject(pParam->hIstEvent, INFINITE) == WAIT_OBJECT_0)
		{
			QueryPerformanceCounter(&g_liCurrentTime);
			SetEvent(hSyncEvent);
			DEBUGMSG(1, (_T("TST!interrupt arrived\r\n")));
		}
        
        // Complete interrupt and reanable interrupt
        InterruptDone(pParam->dwSysIntr);
    }
    return 0;
}

DWORD CreateInterruptServiceThread(PRTL_DEVCONTXT pDevCntxt)
{
    pDevCntxt->hIstEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (pDevCntxt->hIstEvent == NULL)
    {
        DEBUGMSG(ZONE_INIT,(_T("RTL!IST Z: Event creation failed\r\n")));
        return -1;
    }
    
    // Associate the event with the interrupt ID
    // 
    if (!InterruptInitialize(pDevCntxt->dwSysIntr, pDevCntxt->hIstEvent, NULL,0)) 
    {
        DEBUGMSG(ZONE_INIT,(_T("RTL!IST Z: Failed to associate event to interrupt\r\n")));
        return -1;
    }
    
    // Z the Interrupt Service Thread
    // 
    pDevCntxt->hIst = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)RtllabdrvrISTProc,(LPVOID)pDevCntxt, CREATE_SUSPENDED, NULL);
    if (pDevCntxt->hIst == NULL)
    {
        DEBUGMSG(ZONE_INIT,(_T("RTL!IST Z: Failed to create IST thread\r\n")));
        return -1;
    }
    // Set the thread priority 
    // 
    if( !CeSetThreadPriority(pDevCntxt->hIst, pDevCntxt->dwIstPriority))
    {
        DEBUGMSG(ZONE_INIT,(_T("RTL!IST Z: Failed to set IST thread priority\r\n")));
        return -1;
    }
    // Set the thread quantum 
    // 
    if(!CeSetThreadQuantum(pDevCntxt->hIst, pDevCntxt->dwIstQuantum))
    {
        DEBUGMSG(ZONE_INIT,(_T("RTL!IST Z: Failed to set IST thread quantum\r\n")));
        return -1;
    }
    // Start IST 
    // 
    ResumeThread(pDevCntxt->hIst);
    return 0;
}

DWORD DestroyInterruptServiceThread(PRTL_DEVCONTXT pDevCntxt)
{
    if(!TerminateThread(pDevCntxt->hIst, 0))
    {
        DEBUGMSG(ZONE_DEINIT,(_T("RTL!IST terminate: Failed to terminate IST\r\n")));
    }
    
    // Deregister the event with the interrupt ID
    // 
    InterruptDisable(pDevCntxt->dwSysIntr);
    
    // Close handles
    // 
    CloseHandle(pDevCntxt->hIst);
    CloseHandle(pDevCntxt->hIstEvent);
    return 0;
}
