
#ifndef __RTLLABDRVRSDK_H__
#define __RTLLABDRVRSDK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <winioctl.h>

//
// Device IOControl codes definitions
//
#define	IOCTLDEVTYPE_DEVICE_RTLLABDRVR	2048

///////////////////////////////////////////////////////////////
//
// Specific driver IOCTL codes 
//
//
// IOCTL_RTLLABDRVR_GET_SYNCEVENT
//
#define IOCTL_RTLLABDRVR_GET_SYNCEVENT\
	CTL_CODE(IOCTLDEVTYPE_DEVICE_RTLLABDRVR, 0x100, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// IOCTL_RTLLABDRVR_GET_INTTIME
//
#define IOCTL_RTLLABDRVR_GET_INTTIME\
	CTL_CODE(IOCTLDEVTYPE_DEVICE_RTLLABDRVR, 0x101, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _Trigger_tag
{
	HANDLE hEvent;
	HANDLE hGPIO;
}Trigger, *pTrigger;

typedef struct _SYNCEVENTNAME_tag
{
	TCHAR strEventName[9];
	DWORD dwLentgth;
}SYNCEVENTNAME, *PSYNCEVENTNAME;
}
#endif  /* __RTLLABDRVRSDK_H__ */
