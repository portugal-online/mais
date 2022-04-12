#ifndef LORAMAC_H__
#define LORAMAC_H__

#include "LoRaMacTypes.h"
// host-mode

typedef enum eActivationType
{
    ACTIVATION_TYPE_NONE = 0,
    ACTIVATION_TYPE_ABP = 1,
    ACTIVATION_TYPE_OTAA = 2,
}ActivationType_t;

typedef enum eLoRaMacRegion_t
{
    LORAMAC_REGION_AS923 = 0,
    LORAMAC_REGION_AU915,
    LORAMAC_REGION_CN470,
    LORAMAC_REGION_CN779,
    LORAMAC_REGION_EU433,
    LORAMAC_REGION_EU868,
    LORAMAC_REGION_KR920,
    LORAMAC_REGION_IN865,
    LORAMAC_REGION_US915,
    LORAMAC_REGION_RU864,
}LoRaMacRegion_t;


#define LORAMAC_CLASSB_ENABLED (1)

#define LORAMAC_HANDLER_SUCCESS 0

#endif