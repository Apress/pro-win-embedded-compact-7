
#ifndef __RTLABDRVRSDK_H__
#define __RTLABDRVRSDK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <winioctl.h>

//
// Device IOControl codes definitions
//
#define	IOCTLDEVTYPE_DEVICE_RTLABDRVR	2048

///////////////////////////////////////////////////////////////
//
// Specific driver IOCTL codes 
//
//
// IOCTL_RTLABDRVR_GET_SYNCEVENT
//
#define IOCTL_RTLABDRVR_GET_SYNCEVENT\
	CTL_CODE(IOCTLDEVTYPE_DEVICE_RTLABDRVR, 0x100, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_RTLABDRVR_GET_INTTIME\
	CTL_CODE(IOCTLDEVTYPE_DEVICE_RTLABDRVR, 0x101, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _Trigger_tag
{
	HANDLE hEvent;
	HANDLE hGPIO;
}Trigger, *pTrigger;

typedef struct _SYNCEVENTNAME_tag
{
	TCHAR strEventName[7];
	DWORD dwLentgth;
}SYNCEVENTNAME, *PSYNCEVENTNAME;
}
#endif  /* __RTLABDRVRSDK_H__ */
