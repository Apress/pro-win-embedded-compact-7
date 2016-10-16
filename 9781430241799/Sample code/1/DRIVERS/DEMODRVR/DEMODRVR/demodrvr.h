// Demodrvr.h
//
//

#ifndef __DEMODRVR_H__
#define __DEMODRVR_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef DEBUG
#define DEBUG
#endif //DEBUG

#ifdef DEBUG

#define	ZONE_INIT		DEBUGZONE(0)
#define	ZONE_OPEN		DEBUGZONE(1)
#define	ZONE_IOCTL		DEBUGZONE(2)
#define	ZONE_DEINIT		DEBUGZONE(3)
#define	ZONE_READ		DEBUGZONE(4)
#define	ZONE_WRITE		DEBUGZONE(5)
#define	ZONE_ERROR		DEBUGZONE(15)

#endif //DEBUG

#include <windows.h>
#include <types.h>
#include <Pkfuncs.h>
#include <ceddk.h>
#include <devload.h>
#include <pm.h>
#include <nkintr.h>
#include <Ddkreg.h>
#include <giisr.h>
#include "..\SDK\DemodrvrSDK.h"

#define DRVR_INVALID_HANDLE 0x0

BOOL DMO_Cancel(DWORD hOpenContext, HANDLE hAsyncHandle);
DWORD DMO_Init(LPCTSTR pContext, LPCVOID lpvBusContext);
BOOL DMO_PreDeinit(DWORD hDeviceContext);
BOOL DMO_Deinit(DWORD hDeviceContext);
DWORD DMO_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode);
BOOL DMO_PreClose(DWORD hOpenContext);
BOOL DMO_Close(DWORD hOpenContext);
void DMO_PowerUp(DWORD hDeviceContext);
void DMO_PowerDown(DWORD hDeviceContext);
BOOL DMO_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut, HANDLE hAsyncHandle);
DWORD DMO_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count, HANDLE hAsyncHandle);
DWORD DMO_Write(DWORD hOpenContext, LPCVOID pSourceBytes, DWORD NumberOfBytes, HANDLE hAsyncHandle);
DWORD DMO_Seek(DWORD hOpenContext, long Amount, DWORD Type);

typedef struct _DMO_DEVCONTXT_tag
{
	PHYSICAL_ADDRESS ioPhysicalBase;		// physical IO address
	DWORD       dwIoRange;			// IO range in bytes
	BOOL        bIoMemMapped;		// memory mapped flag 
	volatile UCHAR  IoAddr;			// virtual address of base port 
	volatile DWORD  dwIoAddr;			// virtual address of registers 
	DWORD       dwSysIntr;			// system interrupt ID 
	WCHAR       RegPath[64];		// true registry path 
	BOOL        bShutDown;			// driver shutting down 
	BOOL        bHWInitialized;		// hardware initialized flag 
	CRITICAL_SECTION  CriticalSection;	// critical section  
	BOOL        bOpenEx;				// Exclusive open mode
	LONG       lOpenCount;		// number of active open handles 
	HANDLE        hDriverMan;			// Device manager handle.
	HANDLE        hIstEvent;			// interrupt event 
	HANDLE        hIst;				// IST handle 
	DWORD       dwIstPriority;		// IST priority 
	DWORD       dwIstQuantum;		// IST quantum 
	HANDLE        hInstISR;			// ISR handle 
	DWORD       dwInterfaceType;	// interface type 
	DWORD       dwBusNumber;		// bus number 
} DMO_DEVCONTXT, *PDMO_DEVCONTXT;


DWORD DMOInterruptServiceThread(LPVOID lpParameter);
DWORD CreateInterruptServiceThread(PDMO_DEVCONTXT pDevCntxt);
DWORD DestroyInterruptServiceThread(PDMO_DEVCONTXT pDevCntxt);

///////////////////////////////////////////////////////////////
//
// Additional declarations for Asynchronous I/O requests
//
typedef struct _IOCTLWTPARAMS_tag
{
	volatile HANDLE hAsyncIO;
	PBYTE pSrcBufIn;
	volatile PBYTE pBufIn;
	volatile DWORD  dwInLen;
	PBYTE pSrcBufOut;
	volatile PBYTE pBufOut;
	volatile DWORD  dwOutLen;
}IOCTLWTPARAMS, *PIOCTLWTPARAMS;

DWORD AsyncIOThread(LPVOID lpParameter);
}
#endif  /* __DEMODRVR_H__ */
