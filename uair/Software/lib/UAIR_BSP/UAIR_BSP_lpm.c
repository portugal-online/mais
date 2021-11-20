#include "UAIR_lpm.h"
#include "pvt/UAIR_BSP_i2c_p.h"
#include "UAIR_BSP_gpio.h"
#include "UAIR_tracer.h"

// Callback from LPM HAL
void UAIR_LPM_PostStopModeHook()
{
    HAL_clk_resume_clocks();

    TRACER_RESUME();
    UAIR_BSP_DP_Off(DEBUG_PIN2);

}

void BSP_LPM_enter_low_power_mode()
{
    UAIR_BSP_DP_On(DEBUG_PIN2);
    UAIR_LPM_EnterLowPower();
}
