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
 * @file UAIR_BSP_gpio.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V., (c) 2021 MAIS Project
 *
 */

#include "UAIR_BSP.h"
#include "UAIR_BSP_gpio.h"
#include "pvt/UAIR_BSP_gpio_p.h"

typedef void (*UAIR_BSP_EXTI_LineCallback)(void);

EXTI_HandleTypeDef hpb_exti[BUTTONn];

static GPIO_TypeDef *LED_PORT[LEDn] = {LED1_GPIO_PORT, LED2_GPIO_PORT, LED3_GPIO_PORT};
static const uint16_t LED_PIN[LEDn] = {LED1_PIN, LED2_PIN, LED3_PIN};
static GPIO_TypeDef *BUTTON_PORT[BUTTONn] = {BUTTON_SW1_GPIO_PORT,BUTTON_SW2_GPIO_PORT};
static const uint16_t BUTTON_PIN[BUTTONn] = {BUTTON_SW1_PIN,BUTTON_SW2_PIN};
static const IRQn_Type BUTTON_IRQn[BUTTONn] = {BUTTON_SW1_EXTI_IRQn,BUTTON_SW2_EXTI_IRQn};


struct load_switch_control {
    GPIO_TypeDef *port;
    uint16_t pin;
    HAL_clk_clock_control_fun_t clock_control;
};


const struct load_switch_control load_switches_r1[] =
{
    /* Microphone */
    { .port = LOAD_SWITCH1_GPIO_PORT, .pin = LOAD_SWITCH1_PIN, .clock_control = LOAD_SWITCH1_GPIO_CLK_CONTROL },
    /* Ambient sensor */
    { .port = LOAD_SWITCH2_GPIO_PORT, .pin = LOAD_SWITCH2_PIN, .clock_control = LOAD_SWITCH2_GPIO_CLK_CONTROL },
    /* Internal sensors (in fact, I2C pullups) */
    { .port = LOAD_SWITCH3_GPIO_PORT, .pin = LOAD_SWITCH3_PIN,  .clock_control = LOAD_SWITCH3_GPIO_CLK_CONTROL }
};

const struct load_switch_control load_switches_r2[] =
{
    /* Microphone (PB2) */
    { .port = LOAD_SWITCH1_GPIO_PORT, .pin = LOAD_SWITCH1_PIN,  .clock_control = LOAD_SWITCH1_GPIO_CLK_CONTROL },
    /* Ambient sensor (PA6) */
    { .port = LOAD_SWITCH3_GPIO_PORT, .pin = LOAD_SWITCH3_PIN,  .clock_control = LOAD_SWITCH3_GPIO_CLK_CONTROL },
    /* Internal sensors (PC2) */
    { .port = LOAD_SWITCH4_GPIO_PORT, .pin = LOAD_SWITCH4_PIN,  .clock_control = LOAD_SWITCH4_GPIO_CLK_CONTROL }
};



static const HAL_GPIODef_t DEBUG_PIN[]= {
    { .port = DEBUG_PIN1_GPIO_PORT, .pin = DEBUG_PIN1_PIN, .af = 0, .clock_control = DEBUG_PIN1_GPIO_CLOCK_CONTROL },
    { .port = DEBUG_PIN2_GPIO_PORT, .pin = DEBUG_PIN2_PIN, .af = 0, .clock_control = DEBUG_PIN2_GPIO_CLOCK_CONTROL },
    { .port = DEBUG_PIN3_GPIO_PORT, .pin = DEBUG_PIN3_PIN, .af = 0, .clock_control = DEBUG_PIN3_GPIO_CLOCK_CONTROL }
};

static void BUTTON_SW1_EXTI_Callback(void);

/**
 * LED APIs
 */

/**
  * @brief  Configures LED GPIO.
  * @param  Led: LED to be configured.
  *         This parameter can be one of the following values:
  *            @arg LED1
  *            @arg LED2
  * @return UAIR_BSP status
  */
int32_t UAIR_BSP_LED_Init(Led_TypeDef Led)
{
    GPIO_InitTypeDef gpio_init_structure = {0};

    /* Enable the GPIO_LED Clock */
    LEDx_GPIO_CLK_ENABLE(Led);

    /* Configure the GPIO_LED pin */
    gpio_init_structure.Pin = LED_PIN[Led];
    gpio_init_structure.Mode = GPIO_MODE_ANALOG; // Save power.
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(LED_PORT[Led], &gpio_init_structure);

    return BSP_ERROR_NONE;
}

/**
  * @brief  DeInit LEDs.
  * @param  Led: LED to be de-init.
  *         This parameter can be one of the following values:
  *            @arg LED1
  *            @arg LED2
  * @note Led DeInit does not disable the GPIO clock nor disable the Mfx
  * @return UAIR_BSP status
  */
int32_t UAIR_BSP_LED_DeInit(Led_TypeDef Led)
{
    BSP_LED_off(Led);

    /* DeInit the GPIO_LED pin */
    HAL_GPIO_DeInit(LED_PORT[Led], LED_PIN[Led]);

    return BSP_ERROR_NONE;
}

/**
 * @brief  Turns selected LED On.
 * @param  Led: Specifies the Led to be set on.
 *         This parameter can be one of the following values:
 *            @arg LED1
 *            @arg LED2
 * @return UAIR_BSP status
 */
int32_t BSP_LED_on(Led_TypeDef Led)
{
    GPIO_InitTypeDef gpio_init_structure = {0};
    /* Configure the GPIO_LED pin */
    gpio_init_structure.Pin = LED_PIN[Led];
    gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP; // Save power.
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_SET);

    HAL_GPIO_Init(LED_PORT[Led], &gpio_init_structure);


    return BSP_ERROR_NONE;
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led: Specifies the Led to be set off.
  *         This parameter can be one of the following values:
  *            @arg LED1
  *            @arg LED2
  * @return UAIR_BSP status
  */
int32_t BSP_LED_off(Led_TypeDef Led)
{
    GPIO_InitTypeDef gpio_init_structure = {0};
    /* Configure the GPIO_LED pin */
    gpio_init_structure.Pin = LED_PIN[Led];
    gpio_init_structure.Mode = GPIO_MODE_ANALOG;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(LED_PORT[Led], &gpio_init_structure);

    return BSP_ERROR_NONE;
}

/**
  * @brief  Toggles the selected LED.
  * @param  Led: Specifies the Led to be toggled.
  *         This parameter can be one of the following values:
  *            @arg LED1
  *            @arg LED2
  * @return UAIR_BSP status
  */
int32_t BSP_LED_toggle(Led_TypeDef Led)
{
    HAL_GPIO_TogglePin(LED_PORT[Led], LED_PIN[Led]);

    return BSP_ERROR_NONE;
}

/**
  * @brief  Get the status of the selected LED.
  * @param  Led Specifies the Led to get its state.
  *         This parameter can be one of following parameters:
  *            @arg LED1
  *            @arg LED2
  * @return LED status
  */
int32_t BSP_LED_get_state(Led_TypeDef Led)
{
    return (int32_t)HAL_GPIO_ReadPin(LED_PORT[Led], LED_PIN[Led]);
}

/**
 * Push button (PB) APIs
 */

/**
  * @brief  Configures Button GPIO and EXTI Line.
  * @param  Button: Specifies the Button to be configured.
  *         This parameter can be one of following parameters:
  *           @arg BUTTON_SW1
  * @param  ButtonMode: Specifies Button mode.
  *   This parameter can be one of following parameters:
  *     @arg BUTTON_MODE_GPIO: Button will be used as simple IO
  *     @arg BUTTON_MODE_EXTI: Button will be connected to EXTI line with interrupt
  *                            generation capability
  * @return UAIR_BSP status
  */
int32_t UAIR_BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode)
{
    GPIO_InitTypeDef gpio_init_structure = {0};
    static UAIR_BSP_EXTI_LineCallback button_callback[BUTTONn] = {BUTTON_SW1_EXTI_Callback};
    static uint32_t button_interrupt_priority[BUTTONn] = {UAIR_BSP_BUTTON_SWx_IT_PRIORITY};
    static const uint32_t button_exti_line[BUTTONn] = {BUTTON_SW1_EXTI_LINE};

    /* Enable the BUTTON Clock */
    BUTTONx_GPIO_CLK_ENABLE(Button);

    gpio_init_structure.Pin = BUTTON_PIN[Button];
    gpio_init_structure.Pull = GPIO_PULLUP;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;

    if (ButtonMode == BUTTON_MODE_GPIO)
    {
        /* Configure Button pin as input */
        gpio_init_structure.Mode = GPIO_MODE_INPUT;
        HAL_GPIO_Init(BUTTON_PORT[Button], &gpio_init_structure);
    }
    else /* (ButtonMode == BUTTON_MODE_EXTI) */
    {
        /* Configure Button pin as input with External interrupt */
        gpio_init_structure.Mode = GPIO_MODE_IT_FALLING;

        HAL_GPIO_Init(BUTTON_PORT[Button], &gpio_init_structure);

        (void)HAL_EXTI_GetHandle(&hpb_exti[Button], button_exti_line[Button]);
        (void)HAL_EXTI_RegisterCallback(&hpb_exti[Button], HAL_EXTI_COMMON_CB_ID, button_callback[Button]);

        /* Enable and set Button EXTI Interrupt to the lowest priority */
        HAL_NVIC_SetPriority((BUTTON_IRQn[Button]), button_interrupt_priority[Button], 0x00);
        HAL_NVIC_EnableIRQ((BUTTON_IRQn[Button]));
    }

    return BSP_ERROR_NONE;
}

/**
  * @brief  Push Button DeInit.
  * @param  Button: Button to be configured
  *         This parameter can be one of following parameters:
  *           @arg BUTTON_SW1
  * @note PB DeInit does not disable the GPIO clock
  * @return UAIR_BSP status
  */
int32_t UAIR_BSP_PB_DeInit(Button_TypeDef Button)
{
    HAL_NVIC_DisableIRQ((BUTTON_IRQn[Button]));
    HAL_GPIO_DeInit(BUTTON_PORT[Button], BUTTON_PIN[Button]);

    return BSP_ERROR_NONE;
}

/**
  * @brief  Returns the selected Button state.
  * @param  Button: Specifies the Button to be checked.
  *         This parameter can be one of following parameters:
  *           @arg BUTTON_SW1
  * @return The Button GPIO pin value.
  */
int32_t UAIR_BSP_PB_GetState(Button_TypeDef Button)
{
    return (int32_t)HAL_GPIO_ReadPin(BUTTON_PORT[Button], BUTTON_PIN[Button]);
}

/**
  * @brief  This function handles Push-Button interrupt requests.
  * @param  Button Specifies the pin connected EXTI line
  * @return None
  */
void UAIR_BSP_PB_IRQHandler(Button_TypeDef Button)
{
    HAL_EXTI_IRQHandler(&hpb_exti[Button]);
}

/**
  * @brief  BSP Push Button callback
  * @param  Button: Specifies the Button to be checked.
  *         This parameter can be one of following parameters:
  *           @arg BUTTON_SW1
  * @return None.
  */
__WEAK void UAIR_BSP_PB_Callback(Button_TypeDef Button)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(Button);

    /* This function should be implemented by the user application.
     It is called into this driver when an event on Button is triggered. */
}

/**
  * @brief  Button SW1 EXTI line detection callback.
  * @return None
  */
static void BUTTON_SW1_EXTI_Callback(void)
{
    UAIR_BSP_PB_Callback(BUTTON_SW1);
}

/**
 * Load Switches (LS) control APIs
 */

static const struct load_switch_control *UAIR_LS_GetSwitch(Load_Switch_TypeDef sw)
{
    const struct load_switch_control *lsc;

    switch (BSP_get_board_version()) {
    case UAIR_NUCLEO_REV1:
        lsc = &load_switches_r1[sw];
        break;
    case UAIR_NUCLEO_REV2:
        lsc = &load_switches_r2[sw];
        break;
    default:
        lsc = NULL;
        break;
    }
    return lsc;
}

/**
  * @brief  Configures load switch GPIO.
  * @param  loadSwitch: Load Switch to be configured.
  *         This parameter can be one of the following values:
  *            @arg LOAD_SWITCH1
  *            @arg LOAD_SWITCH2
  * @return UAIR_BSP status
  */
int32_t UAIR_BSP_LS_Init(Load_Switch_TypeDef loadSwitch)
{
    const struct load_switch_control *lsc = UAIR_LS_GetSwitch(loadSwitch);

    if (NULL==lsc) {
        return BSP_ERROR_WRONG_PARAM;
    }

    
    GPIO_InitTypeDef gpio_init_structure = {0};

    /* Enable the GPIO_LOAD_SWITCH Clock */
    if (lsc->clock_control) {
        lsc->clock_control(1);
    }

    if (lsc->port != NULL) {
        BSP_TRACE("LS: initialising pin %d port %c",
                  __builtin_ctz(lsc->pin),
                  lsc->port==GPIOA?'A':lsc->port==GPIOB?'B':'C');
        if (lsc->clock_control)
            lsc->clock_control(1);

        /* Configure the GPIO_LOAD_SWITCH pin */
        gpio_init_structure.Pin = lsc->pin;
        gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
        gpio_init_structure.Pull = GPIO_NOPULL;
        gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;

        HAL_GPIO_Init(lsc->port, &gpio_init_structure);

        return UAIR_BSP_LS_Off(loadSwitch);
    }
    return BSP_ERROR_NONE;
}

/**
  * @brief  DeInit Load Switches.
  * @param  loadSwitch: Load Switch to be de-init.
  *         This parameter can be one of the following values:
  *            @arg LOAD_SWITCH1
  *            @arg LOAD_SWITCH2
  * @note Load Switch DeInit does not disable the GPIO clock nor disable the Mfx
  * @return UAIR_BSP status
  */
int32_t UAIR_BSP_LS_DeInit(Load_Switch_TypeDef loadSwitch)
{
    const struct load_switch_control *lsc = UAIR_LS_GetSwitch(loadSwitch);

    if (NULL==lsc) {
        return BSP_ERROR_WRONG_PARAM;
    }

    /* Turn off Load Switch */
    HAL_GPIO_WritePin(lsc->port, lsc->pin, GPIO_PIN_RESET);

    /* DeInit the GPIO_LOAD_SWITCH pin */
    HAL_GPIO_DeInit(lsc->port, lsc->pin);

    return BSP_ERROR_NONE;
}

/**
  * @brief  Turns selected Load Switch On.
  * @param  loadSwitch: Specifies the Load Switch to be set on.
  *         This parameter can be one of the following values:
  *            @arg LOAD_SWITCH1
  *            @arg LOAD_SWITCH2
  * @note   Turning on the load switch and powering the peripherals takes some time
            See SENSORS_LOAD_SWITCH_DELAY_MS and FLASH_LOAD_SWITCH_DELAY_MS
  * @return UAIR_BSP status
  */
int32_t UAIR_BSP_LS_On(Load_Switch_TypeDef loadSwitch)
{
    const struct load_switch_control *lsc = UAIR_LS_GetSwitch(loadSwitch);

    if (NULL==lsc) {
        return BSP_ERROR_WRONG_PARAM;
    }
    BSP_TRACE("LS ON: pin %d port %c",
              __builtin_ctz(lsc->pin),
              lsc->port==GPIOA?'A':lsc->port==GPIOB?'B':'C');
    HAL_GPIO_WritePin(lsc->port, lsc->pin, GPIO_PIN_SET);

    return BSP_ERROR_NONE;
}

/**
  * @brief  Turns selected Load Switch Off.
  * @param  loadSwitch: Specifies the Load Switch to be set off.
  *         This parameter can be one of the following values:
  *            @arg LOAD_SWITCH1
  *            @arg LOAD_SWITCH2
  * @note   The power off time of the load switch is 22 us
  *         (and peripherals have an additional power on time)
  * @return UAIR_BSP status
  */
int32_t UAIR_BSP_LS_Off(Load_Switch_TypeDef loadSwitch)
{
    const struct load_switch_control *lsc = UAIR_LS_GetSwitch(loadSwitch);

    if (NULL==lsc) {
        return BSP_ERROR_WRONG_PARAM;
    }

    HAL_GPIO_WritePin(lsc->port, lsc->pin, GPIO_PIN_RESET);

    return BSP_ERROR_NONE;
}

/**
  * @brief  Get the status of the selected Load Switch.
  * @param  loadSwitch Specifies the Load Switch to get its state.
  *         This parameter can be one of following parameters:
  *            @arg LOAD_SWITCH1
  *            @arg LOAD_SWITCH2
  * @return Load Switch status
  */
int32_t UAIR_BSP_LS_GetState(Load_Switch_TypeDef loadSwitch)
{
    const struct load_switch_control *lsc = UAIR_LS_GetSwitch(loadSwitch);

    if (NULL==lsc) {
        return BSP_ERROR_WRONG_PARAM;
    }

    return (int32_t)HAL_GPIO_ReadPin(lsc->port, lsc->pin);
}

#if 0
int32_t UAIR_BSP_I2C2_PULLUP_Init()
{
    GPIO_InitTypeDef gpio_init_structure = {0};

    I2C2_PULLUP_ENABLE_GPIO_CLK_ENABLE();

    /* Configure the GPIO_LOAD_SWITCH pin */
    gpio_init_structure.Pin = I2C2_PULLUP_ENABLE_PIN;
    gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(I2C2_PULLUP_ENABLE_PORT, &gpio_init_structure);
    HAL_GPIO_WritePin(I2C2_PULLUP_ENABLE_PORT, I2C2_PULLUP_ENABLE_PIN, GPIO_PIN_RESET);

    return BSP_ERROR_NONE;
}

int32_t UAIR_BSP_I2C2_PULLUP_On()
{
    HAL_GPIO_WritePin(I2C2_PULLUP_ENABLE_PORT, I2C2_PULLUP_ENABLE_PIN, GPIO_PIN_SET);
    return BSP_ERROR_NONE;
}

int32_t UAIR_BSP_I2C2_PULLUP_Off()
{
    HAL_GPIO_WritePin(I2C2_PULLUP_ENABLE_PORT, I2C2_PULLUP_ENABLE_PIN, GPIO_PIN_RESET);
    return BSP_ERROR_NONE;
}
#endif

int32_t UAIR_BSP_DP_Init(Debug_Pin_TypeDef Pin)
{
    if (DEBUG_PIN[Pin].clock_control)
        DEBUG_PIN[Pin].clock_control(1);

    HAL_GPIO_configure_output_pp(&DEBUG_PIN[Pin]);

    HAL_GPIO_set(&DEBUG_PIN[Pin], 0);

    return BSP_ERROR_NONE;
}

int32_t UAIR_BSP_DP_On(Debug_Pin_TypeDef Pin)
{
    HAL_GPIO_set(&DEBUG_PIN[Pin], 1);
    return BSP_ERROR_NONE;
}

int32_t UAIR_BSP_DP_Off(Debug_Pin_TypeDef Pin)
{
    HAL_GPIO_set(&DEBUG_PIN[Pin], 0);
    return BSP_ERROR_NONE;
}


int32_t UAIR_BSP_DP_Resume()
{
    DEBUG_PIN_GPIO_CLK_ENABLE();
    return BSP_ERROR_NONE;
}



