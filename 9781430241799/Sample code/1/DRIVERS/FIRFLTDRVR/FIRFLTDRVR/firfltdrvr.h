// Firfltdrvr.h
//
//

#ifndef __FIRFLTDRVR_H__
#define __FIRFLTDRVR_H__


#include <windows.h>
#include <types.h>
#include <Pkfuncs.h>
#include <ceddk.h>
#include <devload.h>
#include <drfilter.h>
#include <Ddkreg.h>
#include <errorrep.h>

class Filter : public DriverFilterBase
{
protected:
    // Value returned from xxx_Init()
    DWORD m_initReturn;
public:
    Filter(LPCTSTR lpcFilterRegistryPath, LPCTSTR lpcDriverRegistryPath, PDRIVER_FILTER pNextFilterParam);
    ~Filter();
    DWORD FilterInit(DWORD dwContext,LPVOID lpParam);
    BOOL FilterPreDeinit(DWORD dwContext);
    BOOL FilterDeinit(DWORD dwContext);
    DWORD FilterOpen(DWORD dwContext, DWORD AccessCode, DWORD ShareMode);
    BOOL FilterPreClose(DWORD dwOpenCont);
    BOOL FilterClose(DWORD dwOpenCont);
    BOOL FilterControl(DWORD dwOpenCont,DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
            PDWORD pdwActualOut, HANDLE hAsyncRef);
    void FilterPowerdn(DWORD dwConext);
    void FilterPowerup(DWORD dwConext);
    BOOL FilterCancelIo(DWORD dwOpenCont, HANDLE hAsyncRef);
    DWORD FilterRead(DWORD dwOpenCont, LPVOID pBuffer, DWORD Count, HANDLE hAsyncRef);
    DWORD FilterWrite(DWORD dwOpenCont, LPCVOID pSourceBytes, DWORD NumberOfBytes, HANDLE hAsyncRef);
    DWORD FilterSeek(DWORD dwOpenCont, long Amount, DWORD Type);
};

CRITICAL_SECTION  g_CriticalSection;

#ifndef DEBUG
#define DEBUG
#endif //DEBUG

#ifdef DEBUG

#include <dbgapi.h>

#define	DEBUGMASK(bit)		(1 << (bit))

#define	ZONE_INIT		DEBUGZONE(0)
#define	ZONE_OPEN		DEBUGZONE(1)
#define	ZONE_IOCTL		DEBUGZONE(2)
#define	ZONE_DEINIT		DEBUGZONE(3)
#define	ZONE_READ		DEBUGZONE(4)
#define	ZONE_WRITE		DEBUGZONE(5)
#define	ZONE_ERROR		DEBUGZONE(15)

#endif //DEBUG


#endif  /* __FIRFLTDRVR_H__ */
