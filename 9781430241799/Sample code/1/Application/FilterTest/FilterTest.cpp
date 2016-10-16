// FilterTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\DRIVERS\DEMODRVR\SDK\DemodrvrSDK.h" // you may want to check path here or even copy header 

#define DATA_WINDOW_SIZE 18

int _tmain(int argc, TCHAR *argv[], TCHAR *envp[])
{
	BOOL bRet = FALSE;
	HANDLE hTstDrvr = INVALID_HANDLE_VALUE;
    double coeffs[6] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 }; // An example set of 6 coefficients 
	double FilteredData[DATA_WINDOW_SIZE];
	DWORD dwDatadbls = 0;
	LPTSTR szError = NULL;

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

	// Setup the filtr's coefficients 
	bRet = DeviceIoControl(hTstDrvr,IOCTL_DEMODRVR_SET_COEFFS, coeffs, 
									6 * sizeof(double), NULL, 0, NULL, NULL);

	// Read filtered data and display it
	ReadFile(hTstDrvr, (double*)&FilteredData, DATA_WINDOW_SIZE, &dwDatadbls, NULL);

	// Make sure to close device driver instance
	CloseHandle(hTstDrvr);
		     
	for (int n = 0; n < 18; n++)
	{
		_tprintf(_T("%3.1lf "), FilteredData[n]);
	}
	return 0;
}
