// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
////////////////////////////////////////////////////////////////////////////////
//
//  Demodrvrtest TUX DLL
//
//  Module: test.cpp
//          Contains the test functions.
//
//  Revision History:
//
////////////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "globals.h"
#include "..\SDK\DemodrvrSDK.h"

HANDLE g_hDevice;
#define WRITE_TEST_STRING_SIZE 65536
TCHAR szBuf[WRITE_TEST_STRING_SIZE];

BOOL TestAsynchWrite(int nTest)
{
	BOOL bRc = FALSE;
    DEVMGR_DEVICE_INFORMATION ddiDemo;
	TCHAR strDrvrName[5] = {'D', 'M', 'O', '*', 0};
	volatile OVERLAPPED ovlpd;
	HANDLE hCompltEvent = NULL;
	DWORD dwWaitRet = WAIT_FAILED, dwBytes = 0;
	DWORD dwOldTime = 0, dwTimeElapsed = 0;

    memset(&ddiDemo, 0, sizeof(DEVMGR_DEVICE_INFORMATION));
    ddiDemo.dwSize = sizeof(DEVMGR_DEVICE_INFORMATION);

    g_hDevice = FindFirstDevice(DeviceSearchByLegacyName, &strDrvrName, &ddiDemo);
    if (g_hDevice == INVALID_HANDLE_VALUE)
    {
        return bRc;
    }
    else
    {
		g_hDevice = CreateFile(L"DMO1:", 0, 0, NULL, 0, 0, NULL);
		if (g_hDevice == INVALID_HANDLE_VALUE)
		{
			return bRc;
		}
		bRc = TRUE;
    }
	// Create a completion event for IOControl IO operation
	ovlpd.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!ovlpd.hEvent)
	{
		return FALSE;
	}
	for (int i = 0; i <  WRITE_TEST_STRING_SIZE; i++)
	{
		szBuf[i] = i;
	}

	memset((void*)&ovlpd, 0, sizeof(ovlpd));

	dwOldTime = GetTickCount();
	bRc = DeviceIoControl(g_hDevice,IOCTL_DEMODRVR_ASYNC_WRITE, szBuf, 
							WRITE_TEST_STRING_SIZE, NULL, 0,NULL,(LPOVERLAPPED)&ovlpd);
	while (!bRc)	// I/O is not done yet
	{
		bRc = GetOverlappedResult(g_hDevice, (LPOVERLAPPED)&ovlpd, &dwBytes, FALSE);
		if (!!bRc)
		{
			g_pKato->Log(LOG_COMMENT, TEXT("Asynch IO pending and %d bytes written\r\n"), ovlpd.InternalHigh);
		}
	}
	dwTimeElapsed = GetTickCount() - dwOldTime;

	CloseHandle(ovlpd.hEvent);
	CloseHandle(g_hDevice);
	switch (nTest)
	{
	case 1:
		if (dwBytes < WRITE_TEST_STRING_SIZE)
		{
			bRc = FALSE;
		}
		else if (dwBytes == WRITE_TEST_STRING_SIZE) 
		{
			bRc = TRUE;
		}
		break;
	case 2:
		if (dwTimeElapsed > 250)
		{
			bRc = FALSE;
		}
		else
		{
			bRc = TRUE;
		}
		break;
	}
	return bRc;
}

////////////////////////////////////////////////////////////////////////////////
// TestProc
//  Executes one test.
//
// Parameters:
//  uMsg            Message code.
//  tpParam         Additional message-dependent data.
//  lpFTE           Function table entry that generated this call.
//
// Return value:
//  TPR_PASS if the test passed, TPR_FAIL if the test fails, or possibly other
//  special conditions.

TESTPROCAPI TestProc(UINT uMsg, TPPARAM tpParam, LPFUNCTION_TABLE_ENTRY lpFTE)
{
    INT TP_Status = TPR_FAIL;

	// The shell doesn't necessarily want us to execute the test. Make sure
    // first.
    if(uMsg != TPM_EXECUTE)
    {
        return TPR_NOT_HANDLED;
    }

    // TODO: Replace the following line with your own test code here. Also,
    //       change the return value from TPR_SKIP to the appropriate code.

    if(TestAsynchWrite(1))
    {
        TP_Status = TPR_PASS;
		g_pKato->Log(LOG_COMMENT, TEXT("This test passed."));
    }
	else
	{
		g_pKato->Log(LOG_COMMENT, TEXT("This test failed."));
	}

    return TP_Status;
}

////////////////////////////////////////////////////////////////////////////////
// PerfomanceTestProc
//  Executes one test.
//
// Parameters:
//  uMsg            Message code.
//  tpParam         Additional message-dependent data.
//  lpFTE           Function table entry that generated this call.
//
// Return value:
//  TPR_PASS if the test passed, TPR_FAIL if the test fails, or possibly other
//  special conditions.

TESTPROCAPI PerfomanceTestProc(UINT uMsg, TPPARAM tpParam, LPFUNCTION_TABLE_ENTRY lpFTE)
{
    INT TP_Status = TPR_FAIL;

	// The shell doesn't necessarily want us to execute the test. Make sure
    // first.
    if(uMsg != TPM_EXECUTE)
    {
        return TPR_NOT_HANDLED;
    }

    // TODO: Replace the following line with your own test code here. Also,
    //       change the return value from TPR_SKIP to the appropriate code.

    if(TestAsynchWrite(2))
    {
        TP_Status = TPR_PASS;
		g_pKato->Log(LOG_COMMENT, TEXT("This test passed."));
    }
	else
	{
		g_pKato->Log(LOG_COMMENT, TEXT("This test failed."));
	}

    return TP_Status;
}

////////////////////////////////////////////////////////////////////////////////
