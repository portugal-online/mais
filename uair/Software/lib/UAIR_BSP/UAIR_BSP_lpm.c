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

void UAIR_BSP_LPM_init(void)
{

#ifdef PSU_LOWNOISE_MODE_PIN
    GPIO_InitTypeDef gpio_init_structure = {0};

    gpio_init_structure.Pin = PSU_LOWNOISE_MODE_PIN;
    gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(PSU_LOWNOISE_MODE_GPIO_PORT, &gpio_init_structure);
    HAL_GPIO_WritePin(PSU_LOWNOISE_MODE_GPIO_PORT, PSU_LOWNOISE_MODE_PIN, PSU_LOWNOISE_DISABLE);

    UAIR_BSP_LPM_disable_lownoise_operation();
#endif
}

void UAIR_BSP_LPM_enable_lownoise_operation(void)
{
#if (defined PSU_LOWNOISE_MODE_PIN) && (defined PSU_USE_LOWNOISE)
    HAL_GPIO_WritePin(PSU_LOWNOISE_MODE_GPIO_PORT, PSU_LOWNOISE_MODE_PIN, PSU_LOWNOISE_ENABLE);
#endif
}

void UAIR_BSP_LPM_disable_lownoise_operation(void)
{
#if (defined PSU_LOWNOISE_MODE_PIN) && (defined PSU_USE_LOWNOISE)
    HAL_GPIO_WritePin(PSU_LOWNOISE_MODE_GPIO_PORT, PSU_LOWNOISE_MODE_PIN, PSU_LOWNOISE_DISABLE);
#endif
}
