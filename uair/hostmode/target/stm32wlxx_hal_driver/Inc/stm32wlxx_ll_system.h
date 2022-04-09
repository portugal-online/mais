#ifndef LL_SYSTEM_H__
#define LL_SYSTEM_H__

#include <inttypes.h>

// Host-mode

static inline uint32_t LL_FLASH_GetUDN()
{
    return 0xDEADBEEF;
}

static inline uint32_t LL_FLASH_GetDeviceID()
{
    return 0xAE;
}

static inline uint32_t LL_FLASH_GetSTCompanyID()
{
    return 0x0080E1;
}

#endif

