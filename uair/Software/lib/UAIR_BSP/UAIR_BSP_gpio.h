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
 * @file UAIR_BSP_gpio.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V., (c) 2021 MAIS Project
 *
 */

#ifndef UAIR_BSP_GPIO_H
#define UAIR_BSP_GPIO_H

#include "stm32wlxx_hal.h"
#include "UAIR_BSP_error.h"
#include "UAIR_BSP_conf.h"
#include "HAL_clk.h"

#ifdef __cplusplus
 extern "C" {
#endif

/**
 * HW aliases for the board components
 */
typedef enum
{
  LED1 = 0,
  LED2 = 1,
  LED3 = 2,
  /* Color LED aliases */
  LED_RED = LED3,
  LED_GREEN = LED2,
  LED_BLUE = LED1
} Led_TypeDef;

typedef enum
{
    BUTTON_SW1 = 0,
    BUTTON_SW2 = 1
} Button_TypeDef;

typedef enum
{
  BUTTON_MODE_GPIO = 0,
  BUTTON_MODE_EXTI = 1
} ButtonMode_TypeDef;

typedef enum
{
  LOAD_SWITCH1 = 0,
  LOAD_SWITCH2 = 1,
  LOAD_SWITCH3 = 2,
  /* Load Switch aliases */
  LOAD_SWITCH_MICROPHONE = LOAD_SWITCH1,
  LOAD_SWITCH_AIRQUALITYSENSOR = LOAD_SWITCH2,
  LOAD_SWITCH_INTERNAL = LOAD_SWITCH3,  // Only available in rev.2
} Load_Switch_TypeDef;

typedef enum
{
    DEBUG_PIN1,
    DEBUG_PIN2,
    DEBUG_PIN3,
    DEBUG_PIN4,
    DEBUG_PIN5
} Debug_Pin_TypeDef;

/**
 * HAL defines
 * Configure below for any pin change
 */

#define LEDn 3

#define LED1_PIN GPIO_PIN_15
#define LED1_GPIO_PORT GPIOB
#define LED1_GPIO_CLK_CONTROL HAL_clk_GPIOB_clock_control

#define LED2_PIN GPIO_PIN_9
#define LED2_GPIO_PORT GPIOB
#define LED2_GPIO_CLK_CONTROL HAL_clk_GPIOB_clock_control

#define LED3_PIN GPIO_PIN_11
#define LED3_GPIO_PORT GPIOB
#define LED3_GPIO_CLK_CONTROL HAL_clk_GPIOB_clock_control

#define LEDx_GPIO_CLK_ENABLE(__INDEX__) \
  do                                    \
  {                                     \
    if ((__INDEX__) == LED1)            \
      LED1_GPIO_CLK_CONTROL(1);          \
    else if ((__INDEX__) == LED2)       \
      LED2_GPIO_CLK_CONTROL(1);           \
    else if ((__INDEX__) == LED3)       \
      LED3_GPIO_CLK_CONTROL(1);           \
  } while (0)

#define LEDx_GPIO_CLK_DISABLE(__INDEX__) \
  do                                     \
  {                                      \
    if ((__INDEX__) == LED1)             \
      LED1_GPIO_CLK_CONTROL(0);           \
    else if ((__INDEX__) == LED2)        \
      LED2_GPIO_CLK_CONTROL(0);           \
    else if ((__INDEX__) == LED3)        \
      LED3_GPIO_CLK_CONTROL(0);           \
  } while (0)

#define BUTTONn 2

#define BUTTON_SW1_PIN GPIO_PIN_6
#define BUTTON_SW1_GPIO_PORT GPIOC
#define BUTTON_SW1_GPIO_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()
#define BUTTON_SW1_GPIO_CLK_DISABLE() __HAL_RCC_GPIOC_CLK_DISABLE()
#define BUTTON_SW1_EXTI_LINE EXTI_LINE_6
#ifdef CORE_CM0PLUS
#define BUTTON_SW1_EXTI_IRQn EXTI4_0_IRQn
#else
#define BUTTON_SW1_EXTI_IRQn EXTI4_IRQn
#endif

#define BUTTON_SW2_PIN GPIO_PIN_1
#define BUTTON_SW2_GPIO_PORT GPIOA
#define BUTTON_SW2_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define BUTTON_SW2_GPIO_CLK_DISABLE() __HAL_RCC_GPIOA_CLK_DISABLE()
#define BUTTON_SW2_EXTI_LINE EXTI_LINE_1
#ifdef CORE_CM0PLUS
#define BUTTON_SW2_EXTI_IRQn EXTI1_0_IRQn
#else
#define BUTTON_SW2_EXTI_IRQn EXTI1_IRQn
#endif

#define LOAD_SWITCHn 4

#define LOAD_SWITCH1_PIN GPIO_PIN_2    /* MIC_EN */
#define LOAD_SWITCH1_GPIO_PORT GPIOB
#define LOAD_SWITCH1_GPIO_CLK_CONTROL HAL_clk_GPIOB_clock_control

#define LOAD_SWITCH2_PIN GPIO_PIN_15 /* SENS_EN, version R1 */
#define LOAD_SWITCH2_GPIO_PORT GPIOA
#define LOAD_SWITCH2_GPIO_CLK_CONTROL HAL_clk_GPIOA_clock_control

#define LOAD_SWITCH3_PIN GPIO_PIN_6  /* SENS_EN, version R2 */
#define LOAD_SWITCH3_GPIO_PORT GPIOA
#define LOAD_SWITCH3_GPIO_CLK_CONTROL HAL_clk_GPIOA_clock_control


#define LOAD_SWITCH4_PIN GPIO_PIN_2  /* INTSENS_EN, version R2 only */
#define LOAD_SWITCH4_GPIO_PORT GPIOC
#define LOAD_SWITCH4_GPIO_CLK_CONTROL HAL_clk_GPIOC_clock_control


//#define LOAD_SWITCH5_PIN GPIO_PIN_6  /* I2C_PULL, version R1 only */
//#define LOAD_SWITCH5_GPIO_PORT GPIOA
//#define LOAD_SWITCH5_GPIO_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()
//#define LOAD_SWITCH5_GPIO_CLK_DISABLE() __HAL_RCC_GPIOC_CLK_DISABLE()

#define LOAD_SWITCH2_DISCHG_PIN       GPIO_PIN_8 /* Only for R1 */
#define LOAD_SWITCH2_DISCHG_GPIO_PORT GPIOA
#define LOAD_SWITCH2_DISCHG_CLK_CONTROL HAL_clk_GPIOA_clock_control



#define LOAD_SWITCH_MICROPHONE 0

#define LOAD_SWITCH_SENS_R1 1

#define LOAD_SWITCH_SENS_R2 2

#define LOAD_SWITCH_I2C_PULL_R1 2
#define LOAD_SWITCH_INTSENS 3


#define BUTTONx_GPIO_CLK_ENABLE(__INDEX__) \
    do { \
    if ((__INDEX__)==BUTTON_SW1) { \
       BUTTON_SW1_GPIO_CLK_ENABLE();\
    } else if ((__INDEX__)==BUTTON_SW2) { \
       BUTTON_SW2_GPIO_CLK_ENABLE();\
    }; } while (0)

// Debug pins
#define DEBUG_PIN1_GPIO_PORT GPIOA
#define DEBUG_PIN1_PIN  GPIO_PIN_7
#define DEBUG_PIN1_GPIO_CLOCK_CONTROL HAL_clk_GPIOA_clock_control
#define DEBUG_PIN2_GPIO_PORT GPIOA
#define DEBUG_PIN2_PIN  GPIO_PIN_8
#define DEBUG_PIN2_GPIO_CLOCK_CONTROL HAL_clk_GPIOA_clock_control

#define DEBUG_PIN3_GPIO_PORT GPIOB
#define DEBUG_PIN3_PIN  GPIO_PIN_14
#define DEBUG_PIN3_GPIO_CLOCK_CONTROL HAL_clk_GPIOB_clock_control
/*
#define DEBUG_PIN4_PIN  GPIO_PIN_7
#define DEBUG_PIN5_PIN  GPIO_PIN_4
  */
#define DEBUG_PIN_PORT GPIOA
#define DEBUG_PIN_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOA_CLK_ENABLE()

#define AMBIENT_SENS_RESETN_PIN         GPIO_PIN_13
#define AMBIENT_SENS_RESETN_GPIO_PORT   GPIOC
#define AMBIENT_SENS_RESETN_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOC_CLK_ENABLE()

#define AMBIENT_SENS_INTN_PIN           GPIO_PIN_3
#define AMBIENT_SENS_INTN_GPIO_PORT     GPIOB
#define AMBIENT_SENS_INTN_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()



/**
 * BSP GPIO APIs
 */

int32_t BSP_LED_on(Led_TypeDef Led);
int32_t BSP_LED_off(Led_TypeDef Led);
int32_t BSP_LED_toggle(Led_TypeDef Led);
int32_t BSP_LED_get_state(Led_TypeDef Led);

int32_t UAIR_BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode);
int32_t UAIR_BSP_PB_DeInit(Button_TypeDef Button);
int32_t UAIR_BSP_PB_GetState(Button_TypeDef Button);
void UAIR_BSP_PB_Callback(Button_TypeDef Button);
void UAIR_BSP_PB_IRQHandler(Button_TypeDef Button);

int32_t UAIR_BSP_LS_Init(Load_Switch_TypeDef loadSwitch);
int32_t UAIR_BSP_LS_DeInit(Load_Switch_TypeDef loadSwitch);
int32_t UAIR_BSP_LS_SWITCH_DeInit(Load_Switch_TypeDef loadSwitch);
int32_t UAIR_BSP_LS_On(Load_Switch_TypeDef loadSwitch);
int32_t UAIR_BSP_LS_Off(Load_Switch_TypeDef loadSwitch);
int32_t UAIR_BSP_LS_Toggle(Load_Switch_TypeDef loadSwitch);
int32_t UAIR_BSP_LS_GetState(Load_Switch_TypeDef loadSwitch);

int32_t UAIR_BSP_I2C2_PULLUP_Init(void);
int32_t UAIR_BSP_I2C2_PULLUP_On(void);
int32_t UAIR_BSP_I2C2_PULLUP_Off(void);

int32_t UAIR_BSP_DP_Init(Debug_Pin_TypeDef Pin);
int32_t UAIR_BSP_DP_On(Debug_Pin_TypeDef Pin);
int32_t UAIR_BSP_DP_Off(Debug_Pin_TypeDef Pin);
int32_t UAIR_BSP_DP_Resume(void);

#ifdef __cplusplus
}
#endif

#endif
