//////////////////////////////////////////////////
//Demodrvr.cpp"

#include "demodrvr.h"



DWORD DemodrvrISTProc(LPVOID lpParameter)
{
    PDMO_DEVCONTXT pParam = (PDMO_DEVCONTXT)lpParameter;
    while(WaitForSingleObject(pParam->hIstEvent, INFINITE))
    {
        if (pParam->bShutDown)
        {
            ExitThread(0);
        }
        // TODO: Write code to process interrupt
        
        // Complete interrupt and reanable interrupt
        InterruptDone(pParam->dwSysIntr);
    }
    return 0;
}

DWORD CreateInterruptServiceThread(PDMO_DEVCONTXT pDevCntxt)
{
    pDevCntxt->hIstEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (pDevCntxt->hIstEvent == NULL)
    {
        DEBUGMSG(ZONE_INIT,(_T("DMO!IST Create: Event creation failed\r\n")));
        return -1;
    }
    
    // Associate the event with the interrupt ID
    // 
    if (!InterruptInitialize(pDevCntxt->dwSysIntr, pDevCntxt->hIstEvent, NULL,0)) 
    {
        DEBUGMSG(ZONE_INIT,(_T("DMO!IST Create: Failed to associate event to interrupt\r\n")));
        return -1;
    }
    
    // Create the Interrupt Service Thread
    // 
    pDevCntxt->hIst = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)DemodrvrISTProc,(LPVOID)pDevCntxt, CREATE_SUSPENDED, NULL);
    if (pDevCntxt->hIst == NULL)
    {
        DEBUGMSG(ZONE_INIT,(_T("DMO!IST Create: Failed to create IST thread\r\n")));
        return -1;
    }
    // Set the thread priority 
    // 
    if( !CeSetThreadPriority(pDevCntxt->hIst, pDevCntxt->dwIstPriority))
    {
        DEBUGMSG(ZONE_INIT,(_T("DMO!IST Create: Failed to set IST thread priority\r\n")));
        return -1;
    }
    // Set the thread quantum 
    // 
    if(!CeSetThreadQuantum(pDevCntxt->hIst, pDevCntxt->dwIstQuantum))
    {
        DEBUGMSG(ZONE_INIT,(_T("DMO!IST Create: Failed to set IST thread quantum\r\n")));
        return -1;
    }
    // Start IST 
    // 
    ResumeThread(pDevCntxt->hIst);
    return 0;
}

DWORD DestroyInterruptServiceThread(PDMO_DEVCONTXT pDevCntxt)
{
    if(!TerminateThread(pDevCntxt->hIst, 0))
    {
        DEBUGMSG(ZONE_DEINIT,(_T("DMO!IST terminate: Failed to terminate IST\r\n")));
    }
    
    // Deregister the event with the interrupt ID
    // 
    InterruptDisable(pDevCntxt->dwSysIntr);
    
    // Unload interrupt handler
    // 
    FreeIntChainHandler(pDevCntxt->hInstISR);
    
    // Close handles
    // 
    CloseHandle(pDevCntxt->hIst);
    CloseHandle(pDevCntxt->hIstEvent);
    return 0;
}
