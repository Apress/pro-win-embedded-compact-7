// LoadandOpenDriver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define DEVICEKEY _T("\\Drivers\\BuiltIn\\Noloaddrvr")

// A very basic function to change the load flag so the device driver loads
// because the original device driver registry prevents it from loading
BOOL ResetLoadFlag()
{
	BOOL bRet = FALSE;
	HKEY hKey = NULL;
    DWORD dwStatus = 0;
	LONG lRet = 0;
	DWORD dwVal = 0, dwcbData = 0;
	DWORD dwType = 0;

	dwStatus = RegOpenKeyEx (HKEY_LOCAL_MACHINE, DEVICEKEY, 0, 0, &hKey);
	if (hKey != NULL)
	{
		lRet = RegSetValueEx(hKey, _T("Flags"), NULL, dwType, (PUCHAR)&dwVal, sizeof(DWORD));
		if (lRet != 0)
		{
			return FALSE;
		}		
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

int _tmain(int argc, TCHAR *argv[], TCHAR *envp[])
{
	BOOL bRet = FALSE;
	HANDLE hActiveDrvr = NULL;
	HANDLE hTstDrvr = INVALID_HANDLE_VALUE;
	DWORD dwLastErr = 0;
	LPTSTR szError = NULL;

	// Reset LOAD flag so that device manager loads the device driver
	if (!ResetLoadFlag())
	{
		return -1;
	}
	// Request device manager to load the devive driver
	hActiveDrvr = ActivateDeviceEx(DEVICEKEY, NULL, 0, NULL);
	_tprintf(_T("No boot load device driver was activated successfuly with handle %d\r\n"), hActiveDrvr);


	// Try open an instance of TestDrvr
	if (hActiveDrvr != NULL)
	{
		hTstDrvr = CreateFile(L"NOL1:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (INVALID_HANDLE_VALUE == hTstDrvr) 
		{
			DWORD bdw = GetLastError();
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, bdw, NULL, 
							(TCHAR *)&szError, 80, NULL );
			_tprintf(_T("CreateFile failed to open DemoDrvr driver instance with error %s\r\n"), (TCHAR)szError);
			return FALSE;
		}
	}
	// Add code to test device driver such as ReadFile or WriteFile
	// ......................
	_tprintf(_T("No boot load device driver was opened successfuly with handle %d\r\n"), hTstDrvr);


	// Deactivate and unload driver so it can be activated again whithout downloading the OS image again
	// First close driver instance
	CloseHandle(hTstDrvr);
	if (hActiveDrvr != NULL)
	{
		bRet = DeactivateDevice(hActiveDrvr);
	}
    return 0;
}
