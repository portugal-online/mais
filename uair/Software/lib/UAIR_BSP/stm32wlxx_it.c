/**
  ******************************************************************************
  * @file    stm32wlxx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include "UAIR_BSP.h"
#include "stm32wlxx_it.h"

extern SUBGHZ_HandleTypeDef hsubghz;
extern UART_HandleTypeDef UAIR_BSP_debug_usart;
extern DMA_HandleTypeDef UAIR_BSP_debug_hdma_tx;
#ifdef UAIR_UART_RX_DMA
extern DMA_HandleTypeDef UAIR_BSP_debug_hdma_rx;
#endif
extern LPTIM_HandleTypeDef UAIR_BSP_lptim;
extern RTC_HandleTypeDef UAIR_BSP_rtc;

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/**
  * @brief This function handles RTC Tamper, RTC TimeStamp, LSECSS and RTC SSRU Interrupts.
  */
void TAMP_STAMP_LSECSS_SSRU_IRQHandler(void)
{
  HAL_RTCEx_SSRUIRQHandler(&UAIR_BSP_rtc);
}

/**
  * @brief This function handles EXTI Line 0 Interrupt.
  */
void EXTI0_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

/**
  * @brief This function handles EXTI Line 1 Interrupt.
  */
void EXTI1_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

/**
  * @brief This function handles EXTI Line 3 Interrupt.
  */
void EXTI3_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&UAIR_BSP_debug_usart);
}

void DMA1_Channel5_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&UAIR_BSP_debug_hdma_tx);
}

#ifdef UAIR_UART_RX_DMA
void DMA1_Channel4_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&UAIR_BSP_debug_hdma_rx);
}
#endif
/**
  * @brief This function handles RTC Alarms (A and B) Interrupt.
  */
void RTC_Alarm_IRQHandler(void)
{
  HAL_RTC_AlarmIRQHandler(&UAIR_BSP_rtc);
}

/**
  * @brief This function handles SUBGHZ Radio Interrupt.
  */
void SUBGHZ_Radio_IRQHandler(void)
{
  HAL_SUBGHZ_IRQHandler(&hsubghz);
}

void LPTIM1_IRQHandler(void)
{
    HAL_LPTIM_IRQHandler(&UAIR_BSP_lptim);
}

extern void Default_Handler(void);

#ifdef HAVE_LTO

#define DEFAULT_IRQ(x) void x ( void ) { Default_Handler(); }

DEFAULT_IRQ(WWDG_IRQHandler);
DEFAULT_IRQ(PVD_PVM_IRQHandler);
DEFAULT_IRQ(RTC_WKUP_IRQHandler);
DEFAULT_IRQ(FLASH_IRQHandler);
DEFAULT_IRQ(RCC_IRQHandler);
DEFAULT_IRQ(EXTI2_IRQHandler);
DEFAULT_IRQ(EXTI4_IRQHandler);
DEFAULT_IRQ(DMA1_Channel1_IRQHandler);
DEFAULT_IRQ(DMA1_Channel2_IRQHandler);
DEFAULT_IRQ(DMA1_Channel3_IRQHandler);
DEFAULT_IRQ(DMA1_Channel6_IRQHandler);
DEFAULT_IRQ(DMA1_Channel7_IRQHandler);
DEFAULT_IRQ(ADC_IRQHandler);
DEFAULT_IRQ(DAC_IRQHandler);
DEFAULT_IRQ(C2SEV_PWR_C2H_IRQHandler);
DEFAULT_IRQ(COMP_IRQHandler);
DEFAULT_IRQ(EXTI9_5_IRQHandler);
DEFAULT_IRQ(TIM1_BRK_IRQHandler);
DEFAULT_IRQ(TIM1_UP_IRQHandler);
DEFAULT_IRQ(TIM1_TRG_COM_IRQHandler);
DEFAULT_IRQ(TIM1_CC_IRQHandler);
DEFAULT_IRQ(TIM2_IRQHandler);
DEFAULT_IRQ(TIM16_IRQHandler);
DEFAULT_IRQ(TIM17_IRQHandler);
DEFAULT_IRQ(I2C1_EV_IRQHandler);
DEFAULT_IRQ(I2C1_ER_IRQHandler);
DEFAULT_IRQ(I2C2_EV_IRQHandler);
DEFAULT_IRQ(I2C2_ER_IRQHandler);
DEFAULT_IRQ(SPI1_IRQHandler);
DEFAULT_IRQ(SPI2_IRQHandler);
DEFAULT_IRQ(USART1_IRQHandler);
DEFAULT_IRQ(LPUART1_IRQHandler);
DEFAULT_IRQ(LPTIM2_IRQHandler);
DEFAULT_IRQ(EXTI15_10_IRQHandler);
DEFAULT_IRQ(LPTIM3_IRQHandler);
DEFAULT_IRQ(SUBGHZSPI_IRQHandler);
DEFAULT_IRQ(IPCC_C1_RX_IRQHandler);
DEFAULT_IRQ(IPCC_C1_TX_IRQHandler);
DEFAULT_IRQ(HSEM_IRQHandler);
DEFAULT_IRQ(I2C3_EV_IRQHandler);
DEFAULT_IRQ(I2C3_ER_IRQHandler);
DEFAULT_IRQ(AES_IRQHandler);
DEFAULT_IRQ(RNG_IRQHandler);
DEFAULT_IRQ(PKA_IRQHandler);
DEFAULT_IRQ(DMA2_Channel1_IRQHandler);
DEFAULT_IRQ(DMA2_Channel2_IRQHandler);
DEFAULT_IRQ(DMA2_Channel3_IRQHandler);
DEFAULT_IRQ(DMA2_Channel4_IRQHandler);
DEFAULT_IRQ(DMA2_Channel5_IRQHandler);
DEFAULT_IRQ(DMA2_Channel6_IRQHandler);
DEFAULT_IRQ(DMA2_Channel7_IRQHandler);
DEFAULT_IRQ(DMAMUX1_OVR_IRQHandler);

#endif




/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
