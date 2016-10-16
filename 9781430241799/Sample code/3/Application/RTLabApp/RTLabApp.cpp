// RTLabApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "RtlabdrvrSDK.h"

LARGE_INTEGER liFrequency;
LARGE_INTEGER liCurrentTime;
LARGE_INTEGER liDrvrTime;
int uiCounterFreq;                  // count register frequency 
DWORD dwTickNS;
DWORD dwDiffInNS;

DWORD AnotherThread(LPVOID lpParameter)
{
	Trigger* pParam = (Trigger*)lpParameter;
	DWORD dwTickCount = 0;
	DWORD dwCount = 0;
	LARGE_INTEGER liStartTime;
	LARGE_INTEGER liDiffTime;

	while (1)
	{
			QueryPerformanceCounter(&liStartTime);
			Sleep(1000);

			QueryPerformanceCounter(&liDiffTime);
			liDiffTime.QuadPart = liDiffTime.QuadPart - liStartTime.QuadPart;
			dwDiffInNS = (DWORD) liDiffTime.QuadPart * dwTickNS; // Really simplified calculation by multiplying the tick difference by nanosecs per tick
			_tprintf(_T("Time span of AnotherThread is %u microseconds\r\n"), dwDiffInNS / 1000);
	}
}

DWORD ControlThread(LPVOID lpParameter)
{
	Trigger* pParam = (Trigger*)lpParameter;
	BOOL done = 0;
	DWORD dwTickCount = 0;
	DWORD dwCount = 0;
	DWORD dwLength = 0;
	LARGE_INTEGER liDiffTime;

	while (!done)
	{
		if (WaitForSingleObject(pParam->hEvent, INFINITE) == WAIT_OBJECT_0)
		{
			ResetEvent(pParam->hEvent);
			QueryPerformanceCounter(&liCurrentTime);

			DeviceIoControl(pParam->hGPIO,IOCTL_RTLABDRVR_GET_INTTIME, NULL, 0, (LPVOID)&liDrvrTime, sizeof(LARGE_INTEGER),&dwLength,NULL);	

			liDiffTime.QuadPart = liCurrentTime.QuadPart - liDrvrTime.QuadPart;

			dwDiffInNS = (DWORD) liDiffTime.QuadPart * dwTickNS; // Really simplified calculation by multiplying the tick difference by nanosecs per tick
			_tprintf(_T("Triggered!!! time span from IST to RT Application is %u microseconds\r\n"), dwDiffInNS / 1000);
			if (dwCount > 9)
			{
				done = 1;
			}
			dwCount++;
		}
	}
	return 0;
}

HANDLE CreateControlThread(Trigger* pTrig, int nPriority)
{
    HANDLE hIst = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ControlThread,(LPVOID)pTrig, CREATE_SUSPENDED, NULL);
    if (hIst == NULL)
    {
        _tprintf(_T("Failed to create control thread\r\n"));
        return INVALID_HANDLE_VALUE;
    }
    // Set the thread priority 
    // 
    if (nPriority != 999)
	{
		if( !CeSetThreadPriority(hIst, nPriority))
		{
			_tprintf((_T("Failed to set control thread priority\r\n")));
			return INVALID_HANDLE_VALUE;
		}
	}

	_tprintf(_T("ControlThread priority is %u\r\n"), CeGetThreadPriority(hIst));
    // Start RT thread 
    // 
    // ResumeThread(hIst);
	return hIst;
}

HANDLE CreateAnotherThread(Trigger* pTrig, int nPriority)
{
    HANDLE hIst = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)AnotherThread,(LPVOID)pTrig, CREATE_SUSPENDED, NULL);
    if (hIst == NULL)
    {
        _tprintf(_T("Failed to create Another thread\r\n"));
        return INVALID_HANDLE_VALUE;
    }
    // Set the thread priority 
    // 
    if (nPriority != 999)
	{
		if( !CeSetThreadPriority(hIst, nPriority))
		{
			_tprintf((_T("Failed to set another thread priority\r\n")));
			return INVALID_HANDLE_VALUE;
		}
	}

	_tprintf(_T("AnotherThread priority is %u\r\n"), CeGetThreadPriority(hIst));
    // Start RT thread 
    // 
    // ResumeThread(hIst);
	return hIst;
}

int _tmain(int argc, TCHAR *argv[], TCHAR *envp[])
{
	Trigger trig;
	HANDLE hTstDrvr = INVALID_HANDLE_VALUE;
	DWORD dwRes = 0;
	LPTSTR szError = NULL;
	TCHAR EventName[16];
	SYNCEVENTNAME GetEvnt;
	DWORD dwLength = 0;
	HANDLE hThread =  INVALID_HANDLE_VALUE;
	HANDLE hAThread =  INVALID_HANDLE_VALUE;
	int nCTprty = 999;
	int nATprty = 999;

	if (argc > 1)
	{
		nCTprty = _wtoi(argv[1]);
		if (argc > 2)
			nATprty = _wtoi(argv[2]);
	}

	if (QueryPerformanceFrequency(&liFrequency))
	{
		uiCounterFreq = (int) liFrequency.QuadPart;
		// 10^9 (MHZ) devided by returned frequency gives us how many ticks per nano second
		dwTickNS = (DWORD) ((__int64) 1000000000 / liFrequency.QuadPart);
		_tprintf(_T("Counter Frequency     = %u Hz\r\n"), uiCounterFreq);
		_tprintf(_T("Counter tick interval = %u ns\r\n"), dwTickNS);
	}

	// Try open an instance of Rttester
	hTstDrvr = CreateFile(L"RTL1:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == hTstDrvr) 
    {
		DWORD bdw = GetLastError();
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, bdw, NULL, 
						(TCHAR *)&szError, 80, NULL );
		_tprintf(_T("CreateFile failed to open DemoDrvr driver instance with error %s\r\n"), (TCHAR)szError);
        return FALSE;
    }
    DeviceIoControl(hTstDrvr,IOCTL_RTLABDRVR_GET_SYNCEVENT, NULL, 0, (LPVOID)&GetEvnt, sizeof(SYNCEVENTNAME),&dwLength,NULL);	
	wsprintf(EventName, GetEvnt.strEventName);

    // Create a trigger event for IOControl IO operation
    trig.hEvent = CreateEvent(NULL, TRUE, FALSE, EventName);
    if (!trig.hEvent)
    {
		DWORD bdw = GetLastError();
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, bdw, NULL, 
						(TCHAR *)&szError, 80, NULL );
		_tprintf(_T("CreateEvent failed to create a named event with error %s\r\n"), (TCHAR)szError);
		CloseHandle(hTstDrvr);
        return FALSE;
    }
	trig.hGPIO = hTstDrvr;
	hAThread = CreateAnotherThread(&trig, nATprty);
	if (hAThread == INVALID_HANDLE_VALUE)
	{
		_tprintf(_T("Failed to create and start AnotherThread\r\n"));
		CloseHandle(hTstDrvr);
        return FALSE;
	}
	hThread = CreateControlThread(&trig, nCTprty);
	if (hThread == INVALID_HANDLE_VALUE)
	{
		_tprintf(_T("Failed to create and start control thread\r\n"));
		CloseHandle(hTstDrvr);
        return FALSE;
	}
	dwRes = ResumeThread(hAThread);
	dwRes = ResumeThread(hThread);
	// We want to wait on thread exit
	if (WaitForSingleObject(hThread, INFINITE) == WAIT_OBJECT_0)
	{
		_tprintf(_T("Thread completed \r\n"));
	}
	TerminateThread(hAThread, 0);
	CloseHandle(hTstDrvr);
    return 0;
}
