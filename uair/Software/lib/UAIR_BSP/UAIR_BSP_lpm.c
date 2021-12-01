#include "UAIR_lpm.h"
#include "pvt/UAIR_BSP_i2c_p.h"
#include "UAIR_BSP_gpio.h"
#include "UAIR_tracer.h"

// Callback from LPM HAL
void UAIR_LPM_PostStopModeHook()
{
    HAL_clk_resume_clocks();

    // I2C2, I2C3 lose register contents upon STOP2 mode.
    BSP_error_t err = UAIR_BSP_I2C_exit_low_power_mode();

    TRACER_RESUME();

    UAIR_BSP_DP_Off(DEBUG_PIN2);

    if (err) {
        BSP_TRACE("While re-initialising I2C buses: error %d", err);
    }
}

void BSP_LPM_enter_low_power_mode()
{
    UAIR_BSP_I2C_enter_low_power_mode();

    UAIR_BSP_DP_On(DEBUG_PIN2);
    UAIR_LPM_EnterLowPower();
}
