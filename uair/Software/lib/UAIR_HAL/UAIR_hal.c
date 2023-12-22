/** Copyright © 2021 The Things Industries B.V.
 *  Copyright © 2021 MAIS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file UAIR_hal.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V., (c) 2021 MAIS Project
 *
 */

#include "UAIR_hal.h"
#include "UAIR_lpm.h"
#include "BSP.h"

static RCC_OscInitTypeDef RCC_OscInitStruct = {0};
static uint8_t hperfcnt = 0;

/**
  * @brief Initialises all common UAIR hardware
  * @param UAIR_HAL_Ctx_t gnse_inits: Interfaces to configure
  * @return none
  */

#if 0
void UAIR_HAL_Init(UAIR_HAL_Ctx_t gnse_inits)
{
    if (gnse_inits.external_sensors_init)
    {
        UAIR_BSP_Ext_Sensor_I2C3_Init();
    }

    if (gnse_inits.leds_init)
    {
        UAIR_BSP_LED_Init(LED_BLUE);
        UAIR_BSP_LED_Init(LED_RED);
        UAIR_BSP_LED_Init(LED_GREEN);
    }
}

/**
  * @brief Deinitialises all common UAIR hardware
  * @param UAIR_HAL_Ctx_t gnse_deinits: Interfaces to deconfigure
  * @return none
  */
void UAIR_HAL_DeInit(UAIR_HAL_Ctx_t gnse_deinits)
{
    if (gnse_deinits.external_sensors_init)
    {
        UAIR_BSP_Ext_Sensor_I2C3_DeInit();
    }
    if (gnse_deinits.leds_init)
    {
        UAIR_BSP_LED_DeInit(LED_BLUE);
        UAIR_BSP_LED_DeInit(LED_RED);
        UAIR_BSP_LED_DeInit(LED_GREEN);
    }
}
#endif

static bool is_lowpower = false;

bool UAIR_HAL_is_lowpower(void)
{
    return is_lowpower;
}

#ifndef MSI_RANGE_LOWPOWER
#error  MSI_RANGE_LOWPOWER undefined
#endif

UAIR_HAL_op_result_t UAIR_HAL_SysClk_Init(bool lowpower)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    is_lowpower = lowpower;
    if (!lowpower)
        hperfcnt = 1; // Increase perf. counter so not to go back to lowpower

    /** Configure the main internal regulator output voltage
     */
    if (lowpower) {
        __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
    } else {
        __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    }
    /** Initializes the CPU, AHB and APB busses clocks
     */

    // For SPI we need to enable HSI. TBD!. Alvie

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_MSI | RCC_OSCILLATORTYPE_LSI;
    //| RCC_OSCILLATORTYPE_HSI;

    RCC_OscInitStruct.LSEState = RCC_LSE_ON; // For RTC
    RCC_OscInitStruct.LSIState = RCC_LSI_ON; // For IWDG
    RCC_OscInitStruct.MSIState = RCC_MSI_ON; // System main oscillator
    RCC_OscInitStruct.LSIDiv = RCC_LSI_DIV128; // 250Hz

    //RCC_OscInitStruct.HSIState = RCC_HSI_ON; // For SPI/I2S

    HAL_PWR_EnableBkUpAccess();
    LL_RCC_LSE_SetDriveCapability(RCC_LSEDRIVE_LOW);
    HAL_PWR_DisableBkUpAccess();

    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;

    /*
     0000: Range 0 around 100 kHz
     0001: Range 1 around 200 kHz
     0010: Range 2 around 400 kHz
     0011: Range 3 around 800 kHz
     0100: Range 4 around 1 MHz
     0101: Range 5 around 2 MHz
     0110: Range 6 around 4 MHz (reset value)
     0111: Range 7 around 8 MHz
     1000: Range 8 around 16 MHz
     1001: Range 9 around 24 MHz
     1010: Range 10 around 32 MHz
     1011: Range 11 around 48 MHz
     Others: not allowed (hardware write protection)
     */
    if (lowpower) {
        RCC_OscInitStruct.MSIClockRange = MSI_RANGE_LOWPOWER;  // 2Mhz
    } else {
        RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_9;  // 24Mhz
    }

    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        return UAIR_HAL_OP_FAIL;
    }
    /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
     */

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3|RCC_CLOCKTYPE_HCLK2
        |RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
        |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;

    if (lowpower) {
        if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
        {
            return UAIR_HAL_OP_FAIL;
        }
    } else {
        if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
        {
            return UAIR_HAL_OP_FAIL;
        }
    }

#if 1
    /* Wait till LSI is ready */
    while(LL_RCC_LSI_IsReady() != 1)
    {
    }
#endif
    // Enter LPRun mode?
    if (lowpower) {
        //LL_PWR_SetFlashPowerModeLPRun(LL_PWR_FLASH_LPRUN_MODE_POWER_DOWN);
        HAL_PWREx_EnableLowPowerRunMode();
    }
    return UAIR_HAL_OP_SUCCESS;
}

__WEAK void UAIR_HAL_Error_Handler(void)
{
    UAIR_LPM_EnterLowPower();
    while (1)
    {
        // TBD: reset
    }
}

void HAL_delay_us(unsigned us)
{
    BSP_delay_us(us);
}


UAIR_HAL_op_result_t UAIR_HAL_request_high_performance(void)
{
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    HAL_PWREx_DisableLowPowerRunMode();

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_9;  // 24Mhz

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        HAL_FATAL();
    }

    is_lowpower = false;
    hperfcnt ++;
    return UAIR_HAL_OP_SUCCESS;
}

void UAIR_HAL_release_high_performance(void)
{
    if (hperfcnt>0) {
        hperfcnt--;
    }
    if (hperfcnt==0) {
        __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
        RCC_OscInitStruct.MSIState = RCC_MSI_ON;
        RCC_OscInitStruct.MSIClockRange = MSI_RANGE_LOWPOWER;  // 2/4Mhz

        if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        {
            HAL_FATAL();
        }
        HAL_PWREx_EnableLowPowerRunMode();

        is_lowpower = true;
    }
}
void  __attribute__((noreturn)) HAL_FATAL(void)
{
    BSP_FATAL();
}
