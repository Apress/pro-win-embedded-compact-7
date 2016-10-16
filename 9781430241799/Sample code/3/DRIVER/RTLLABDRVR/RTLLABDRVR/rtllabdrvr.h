// Rtllabdrvr.h
//
//

#ifndef __RTLLABDRVR_H__
#define __RTLLABDRVR_H__

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
#include "omap.h"
#include "sdk_gpio.h"
#include "gpio_ioctls.h"
#include "..\SDK\RtllabdrvrSDK.h"

#define DRVR_INVALID_HANDLE 0x0

BOOL RTL_Cancel(DWORD hOpenContext, HANDLE hAsyncHandle);
DWORD RTL_Init(LPCTSTR pContext, LPCVOID lpvBusContext);
BOOL RTL_PreDeinit(DWORD hDeviceContext);
BOOL RTL_Deinit(DWORD hDeviceContext);
DWORD RTL_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode);
BOOL RTL_PreClose(DWORD hOpenContext);
BOOL RTL_Close(DWORD hOpenContext);
void RTL_PowerUp(DWORD hDeviceContext);
void RTL_PowerDown(DWORD hDeviceContext);
BOOL RTL_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut, HANDLE hAsyncHandle);
DWORD RTL_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count, HANDLE hAsyncHandle);
DWORD RTL_Write(DWORD hOpenContext, LPCVOID pSourceBytes, DWORD NumberOfBytes, HANDLE hAsyncHandle);
DWORD RTL_Seek(DWORD hOpenContext, long Amount, DWORD Type);

typedef struct _RTL_DEVCONTXT_tag
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
	BOOL        bOpenEx;				// Exclusive open mode
	LONG       lOpenCount;		// number of active open handles 
	HANDLE        hDriverMan;			// Device manager handle.
	HANDLE        hIstEvent;			// interrupt event 
	HANDLE        hIst;				// IST handle 
	DWORD       dwIstPriority;		// IST priority 
	DWORD       dwIstQuantum;		// IST quantum 
	DWORD       dwInterfaceType;	// interface type 
	DWORD       dwBusNumber;		// bus number 
	Trigger	    TrigParam;
} RTL_DEVCONTXT, *PRTL_DEVCONTXT;


DWORD RTLInterruptServiceThread(LPVOID lpParameter);
DWORD CreateInterruptServiceThread(PRTL_DEVCONTXT pDevCntxt);
DWORD DestroyInterruptServiceThread(PRTL_DEVCONTXT pDevCntxt);

///////////////////////////////////////////////////////////////
//
// PDD function declarations
//
HANDLE pdd_Init(int nGPIOpinID, PRTL_DEVCONTXT pDevCntxt);
DWORD pdd_DeInit(PRTL_DEVCONTXT pDevCntxt);
}
#endif  /* __RTLLABDRVR_H__ */
