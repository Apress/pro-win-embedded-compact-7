// Noloaddrvr.h
//
//

#ifndef __NOLOADDRVR_H__
#define __NOLOADDRVR_H__

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

#define DRVR_INVALID_HANDLE 0x0

BOOL NOL_Cancel(DWORD hOpenContext, HANDLE hAsyncHandle);
DWORD NOL_Init(LPCTSTR pContext, LPCVOID lpvBusContext);
BOOL NOL_PreDeinit(DWORD hDeviceContext);
BOOL NOL_Deinit(DWORD hDeviceContext);
DWORD NOL_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode);
BOOL NOL_PreClose(DWORD hOpenContext);
BOOL NOL_Close(DWORD hOpenContext);
void NOL_PowerUp(DWORD hDeviceContext);
void NOL_PowerDown(DWORD hDeviceContext);
BOOL NOL_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut, HANDLE hAsyncHandle);
DWORD NOL_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count, HANDLE hAsyncHandle);
DWORD NOL_Write(DWORD hOpenContext, LPCVOID pSourceBytes, DWORD NumberOfBytes, HANDLE hAsyncHandle);
DWORD NOL_Seek(DWORD hOpenContext, long Amount, DWORD Type);

typedef struct _NOL_DEVCONTXT_tag
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
	DWORD       dwInterfaceType;	// interface type 
	DWORD       dwBusNumber;		// bus number 
} NOL_DEVCONTXT, *PNOL_DEVCONTXT;


}
#endif  /* __NOLOADDRVR_H__ */
