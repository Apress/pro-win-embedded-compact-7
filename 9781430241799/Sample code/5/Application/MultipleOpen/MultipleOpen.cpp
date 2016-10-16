// MultipleOpen.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, TCHAR *argv[], TCHAR *envp[])
{

	HANDLE hTstDrvr = INVALID_HANDLE_VALUE;			// Handle for first instance
	HANDLE hTstDrvr1 = INVALID_HANDLE_VALUE;		// Handle for second instance
	HANDLE hTstDrvr2 = INVALID_HANDLE_VALUE;		// Handle for third instance
	LPTSTR szError = NULL;
	DWORD bdw = 0;
	DWORD dwData = 546;
	DWORD dwWritten = 0;

	// Open the first instance of multi open instnaces device driver
	hTstDrvr = CreateFile(L"MID1:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == hTstDrvr) 
    {
		bdw = GetLastError();
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, bdw, NULL, 
						(TCHAR *)&szError, 124, NULL );
		_tprintf(_T("CreateFile failed to open Midevdrvr driver instance with error %s\r\n"), (TCHAR)szError);
        return FALSE;
    }
	// Open the second instance of multi open instnaces device driver
	hTstDrvr1 = CreateFile(L"MID1:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == hTstDrvr1) 
    {
		bdw = GetLastError();
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, bdw, NULL, 
						(TCHAR *)&szError, 124, NULL );
		_tprintf(_T("CreateFile failed to open Midevdrvr driver instance with error %s\r\n"), (TCHAR)szError);
        return FALSE;
    }	
	// Try to open the third instance of multi open instnaces device driver
	// This should fail since the device driver will only allow two open instances
	hTstDrvr2 = CreateFile(L"MID1:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == hTstDrvr2) 
    {
		bdw = GetLastError();
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, bdw, NULL, 
						(TCHAR *)&szError, 124, NULL );
		_tprintf(_T("CreateFile failed to open a third Midevdrvr driver instance with error %s\r\n"), (TCHAR)szError);
    }

	// Write a DWORD of data value (546) to instance 2
	WriteFile(hTstDrvr1, &dwData, sizeof(DWORD), &dwWritten, NULL);
	dwData = 9999;
	// Write a DWORD of data value (9999) to instance 1
	WriteFile(hTstDrvr, &dwData, sizeof(DWORD), &dwWritten, NULL);
	// Read the data in istance 1 and verify it is 546
	ReadFile(hTstDrvr1, &dwData, sizeof(DWORD), &dwWritten, NULL);
	if (dwData != 546)
	{
		_tprintf(_T("Actual value returned is %d should have been 546"), dwData);
	}
	// Lose instance 2
	CloseHandle(hTstDrvr1);
	// Try topen another instance - should work this time
	hTstDrvr2 = CreateFile(L"MID1:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == hTstDrvr2) 
    {
		bdw = GetLastError();
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, bdw, NULL, 
						(TCHAR *)&szError, 124, NULL );
		_tprintf(_T("CreateFile failed to open another Midevdrvr driver instance after one instance was closed with error %s\r\n"), (TCHAR)szError);
		CloseHandle(hTstDrvr); // Close instance 1 since we are returning and it will not reach the end of the code
        return FALSE;
    }
	// Before returning close both instances, since we got here hTstDrvr2 represents an open instance
	CloseHandle(hTstDrvr);
	CloseHandle(hTstDrvr2); 
	return 0;
}
