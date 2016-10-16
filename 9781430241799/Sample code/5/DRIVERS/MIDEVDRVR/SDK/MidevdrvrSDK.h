
#ifndef __MIDEVDRVRSDK_H__
#define __MIDEVDRVRSDK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <winioctl.h>

//
// Device IOControl codes definitions
//
#define	IOCTLDEVTYPE_DEVICE_MIDEVDRVR	2048

///////////////////////////////////////////////////////////////
//
// Specific driver IOCTL codes 
//
//
// IOCTL_MIDEVDRVR_FOO1
//
#define IOCTL_MIDEVDRVR_FOO1\
	CTL_CODE(IOCTLDEVTYPE_DEVICE_MIDEVDRVR, 0x100, METHOD_BUFFERED, FILE_ANY_ACCESS)

}
#endif  /* __MIDEVDRVRSDK_H__ */
