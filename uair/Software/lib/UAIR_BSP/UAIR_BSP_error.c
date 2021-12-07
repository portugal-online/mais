#include "UAIR_BSP_error.h"
#include "UAIR_BSP.h"

// GLOBAL: so we can use debugger
BSP_error_detail_t bsp_error_lasterror;

void BSP_error_push(BSP_error_detail_t error)
{
    bsp_error_lasterror = error;
}

BSP_error_detail_t BSP_error_get_last_error(void)
{
    return bsp_error_lasterror;
}
