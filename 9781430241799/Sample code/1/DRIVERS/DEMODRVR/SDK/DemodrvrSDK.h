
#ifndef __DEMODRVRSDK_H__
#define __DEMODRVRSDK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <winioctl.h>

//
// Device IOControl codes definitions
//
#define	IOCTLDEVTYPE_DEVICE_DEMODRVR	2048

///////////////////////////////////////////////////////////////
//
// Specific driver IOCTL codes 
//
//
// IOCTL_DEMODRVR_ASYNC_WRITE
//
#define IOCTL_DEMODRVR_ASYNC_WRITE\
	CTL_CODE(IOCTLDEVTYPE_DEVICE_DEMODRVR, 0x100, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DEMODRVR_SET_COEFFS\
	CTL_CODE(IOCTLDEVTYPE_DEVICE_DEMODRVR, 0x101, METHOD_BUFFERED, FILE_ANY_ACCESS)
}
#endif  /* __DEMODRVRSDK_H__ */
