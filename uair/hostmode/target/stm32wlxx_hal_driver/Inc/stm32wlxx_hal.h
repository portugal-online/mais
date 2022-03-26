#ifndef STM32WLXX_HAL_H__
#define STM32WLXX_HAL_H__

#include "stm32wlxx_hal_def.h"

#include "stm32wlxx_hal_dma.h"
#include "stm32wlxx_hal_dma_ex.h"
#include "stm32wlxx_hal_usart.h"
#include "stm32wlxx_hal_uart.h"
#include "stm32wlxx_hal_i2c.h"
#include "stm32wlxx_hal_adc.h"
#include "stm32wlxx_hal_adc_ex.h"
#include "stm32wlxx_hal_iwdg.h"
#include "stm32wlxx_hal_rcc.h"
#include "stm32wlxx_hal_rtc.h"
#include "stm32wlxx_hal_gpio.h"
#include "stm32wlxx_hal_lptim.h"
#include "stm32wlxx_hal_spi.h"
#include "stm32wlxx_hal_exti.h"
#include "stm32wlxx_hal_tim.h"
#include "stm32wlxx_hal_rng.h"
#include "stm32wlxx_hal_pwr.h"
#include "stm32wlxx_hal_subghz.h"
#include "stm32wlxx_ll_system.h"

#include <stdlib.h>

// Host-mode

#define HAL_MAX_DELAY (1000000)


#define HAL_OK (0)
#define HAL_ERROR (1)
#define UNUSED(x) ((void)(x))

// NVIC
void HAL_NVIC_DisableIRQ(int);
void HAL_NVIC_EnableIRQ(int);
void HAL_NVIC_SetPriority(int,int,int);

uint32_t HAL_GetTick(void);
void HAL_IncTick(void);
void HAL_SuspendTick(void);
void HAL_ResumeTick(void);

void HAL_Delay(uint32_t);
uint32_t HAL_GetHalVersion(void);
uint32_t HAL_GetREVID(void);
uint32_t HAL_GetDEVID(void);
uint32_t HAL_GetUIDw0(void);
uint32_t HAL_GetUIDw1(void);
uint32_t HAL_GetUIDw2(void);

void HAL_DBGMCU_EnableDBGSleepMode(void);
void HAL_DBGMCU_EnableDBGStopMode(void);
void HAL_DBGMCU_EnableDBGStandbyMode(void);

void HAL_DBGMCU_DisableDBGSleepMode(void);
void HAL_DBGMCU_DisableDBGStopMode(void);
void HAL_DBGMCU_DisableDBGStandbyMode(void);

//#define DMA1_Channel5_IRQn 0

#define ASSERT(x...) do { \
    if (!(x)) { \
    fprintf(stderr, "line %d: assertion " #x " failed\n", __LINE__); \
    abort(); \
    } \
} while (0);

#endif
