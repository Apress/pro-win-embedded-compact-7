// DemoAsync.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\DRIVERS\DEMODRVR\SDK\DemodrvrSDK.h"

#define WRITE_TEST_STRING_SIZE 65536
TCHAR szBuf[WRITE_TEST_STRING_SIZE];

int _tmain(int argc, TCHAR *argv[], TCHAR *envp[])
{
	BOOL bRet = FALSE;
	volatile OVERLAPPED ovlpd;
	HANDLE hCompltEvent = NULL;
	HANDLE hTstDrvr = INVALID_HANDLE_VALUE;
	DWORD dwWaitRet = WAIT_FAILED;
	DWORD dwBytes = 0, dwOldTime = 0, dwTimeElapsed = 0;
	LPTSTR szError = NULL;

	memset((void*)&ovlpd, 0, sizeof(ovlpd));
	
	// Try open an instance of TestDrvr
	hTstDrvr = CreateFile(L"DMO1:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == hTstDrvr) 
    {
		DWORD bdw = GetLastError();
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, bdw, NULL, 
						(TCHAR *)&szError, 80, NULL );
		_tprintf(_T("CreateFile failed to open DemoDrvr driver instance with error %s\r\n"), (TCHAR)szError);
        return FALSE;
    }

    // Create a completion event for IOControl IO operation
    ovlpd.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!ovlpd.hEvent)
    {
		DWORD bdw = GetLastError();
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, bdw, NULL, 
						(TCHAR *)&szError, 80, NULL );
		_tprintf(_T("CreateEvent failed to create an event with error %s\r\n"), (TCHAR)szError);
        return FALSE;
    }
    for (int i = 0; i <  WRITE_TEST_STRING_SIZE; i++)
    {
	  szBuf[i] = i;
    }
	dwOldTime = GetTickCount();
	// Write I/O request using an IOCTL code
    bRet = DeviceIoControl(hTstDrvr,IOCTL_DEMODRVR_ASYNC_WRITE, szBuf, 
							WRITE_TEST_STRING_SIZE, NULL, 
							0,NULL,(LPOVERLAPPED)&ovlpd);

	_tprintf(_T("DeviceIoControl returned %d\r\n"), bRet);
	// This is not the only procedure to wait on completion
	// You can wait on the event with a minimal wait and do something else if
	// the wait returns ERROR_IO_PENDING. This method uses the GetOverlappedResult
	// API.
     while (!bRet)	// I/O is not done yet
     {
			bRet = GetOverlappedResult(hTstDrvr, (LPOVERLAPPED)&ovlpd, &dwBytes, FALSE);
			if (!bRet )
			{
				// You may perform other computations while waiting for lengthy IO to complete
				// however it is worthwile to break this work up so that you can continue as soon as
				// IO completion
				_tprintf(_T("Asynch IO is not yet completed %d bytes written\r\n"), ovlpd.InternalHigh);
			}
			else
			{
				_tprintf(_T("DeviceIoControl has completed operation transfered %d\r\n"), dwBytes);
			}
	}
	CloseHandle(hTstDrvr);
	CloseHandle(ovlpd.hEvent);
	dwTimeElapsed = GetTickCount() - dwOldTime;
	_tprintf(_T("Testing operation lasted %d ticks\r\n"), dwTimeElapsed);
	return 0;
}


