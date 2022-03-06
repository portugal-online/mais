#ifndef HAL_TYPES_H__
#define HAL_TYPES_H__

#include <inttypes.h>

typedef int32_t HAL_StatusTypeDef;
typedef int HAL_LockTypeDef;

typedef enum  {
    DISABLE = 0,
    ENABLE = !DISABLE
} FunctionalState;


#define HAL_OK (0)

#define __IO /* */

#endif
