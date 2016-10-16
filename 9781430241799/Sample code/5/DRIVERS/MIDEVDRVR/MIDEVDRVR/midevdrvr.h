// Midevdrvr.h
//
//

#ifndef __MIDEVDRVR_H__
#define __MIDEVDRVR_H__

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
#include "..\SDK\MidevdrvrSDK.h"

#define DRVR_INVALID_HANDLE 0

BOOL MID_Cancel(DWORD hOpenContext, HANDLE hAsyncHandle);
DWORD MID_Init(LPCTSTR pContext, LPCVOID lpvBusContext);
BOOL MID_PreDeinit(DWORD hDeviceContext);
BOOL MID_Deinit(DWORD hDeviceContext);
DWORD MID_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode);
BOOL MID_PreClose(DWORD hOpenContext);
BOOL MID_Close(DWORD hOpenContext);
void MID_PowerUp(DWORD hDeviceContext);
void MID_PowerDown(DWORD hDeviceContext);
BOOL MID_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut, HANDLE hAsyncHandle);
DWORD MID_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count, HANDLE hAsyncHandle);
DWORD MID_Write(DWORD hOpenContext, LPCVOID pSourceBytes, DWORD NumberOfBytes, HANDLE hAsyncHandle);
DWORD MID_Seek(DWORD hOpenContext, long Amount, DWORD Type);

typedef struct _MID_DEVCONTXT_tag
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
} MID_DEVCONTXT, *PMID_DEVCONTXT;

typedef struct _INSTANCE_DATA_tag
{
	DWORD* pInstanceData;
	PMID_DEVCONTXT pDevContxt;
}INSTANCE_DATA, *PINSTANCE_DATA;

}
#endif  /* __MIDEVDRVR_H__ */
