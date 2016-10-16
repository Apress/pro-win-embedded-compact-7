//////////////////////////////////////////////////////////
//
// This file implements PDD functions
//


#include "rtllabdrvr.h"



//////////////////////////////////////////////////////////
HANDLE pdd_Init(int nGPIOpinID, PRTL_DEVCONTXT pDevCntxt)
{

    HANDLE RetVal = 0;
    
    // Todo: implement this function
    RetVal = GPIOOpen();
    GPIOSetMode(RetVal, nGPIOpinID, GPIO_DIR_INPUT|GPIO_INT_LOW_HIGH|GPIO_INT_LOW_HIGH);
	pDevCntxt->TrigParam.hGPIO = RetVal;

    return	RetVal;
}



DWORD pdd_DeInit(PRTL_DEVCONTXT pDevCntxt)
{

    DWORD	RetVal = 0;
    
    // Todo: implement this function
    GPIOClose(pDevCntxt->TrigParam.hGPIO);

    return	RetVal;
}


