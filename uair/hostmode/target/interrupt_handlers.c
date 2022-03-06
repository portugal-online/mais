extern void unexpected_interrupt();

#define __WEAK __attribute__((weak))

__WEAK void Reset_Handler         () { unexpected_interrupt(); }
__WEAK void NMI_Handler           () { unexpected_interrupt(); }
__WEAK void HardFault_Handler     () { unexpected_interrupt(); }
__WEAK void MemManage_Handler     () { unexpected_interrupt(); }
__WEAK void BusFault_Handler      () { unexpected_interrupt(); }
__WEAK void UsageFault_Handler    () { unexpected_interrupt(); }
__WEAK void SVC_Handler           () { unexpected_interrupt(); }
__WEAK void DebugMon_Handler      () { unexpected_interrupt(); }
__WEAK void PendSV_Handler         () { unexpected_interrupt(); }
__WEAK void SysTick_Handler        () { unexpected_interrupt(); }
__WEAK void WWDG_IRQHandler        () { unexpected_interrupt(); }                /* Window Watchdog interrupt                          */
__WEAK void PVD_PVM_IRQHandler     () { unexpected_interrupt(); }                /* PVD and PVM interrupt through EXTI                 */
__WEAK void TAMP_STAMP_LSECSS_SSRU_IRQHandler (){ unexpected_interrupt(); }      /* RTC Tamper() RTC TimeStamp() LSECSS and RTC SSRU int.*/
__WEAK void RTC_WKUP_IRQHandler         () { unexpected_interrupt(); }           /* RTC wakeup interrupt through EXTI[19]              */
__WEAK void FLASH_IRQHandler            () { unexpected_interrupt(); }           /* Flash memory global interrupt and Flash memory ECC */
__WEAK void RCC_IRQHandler              () { unexpected_interrupt(); }           /* RCC global interrupt                               */
__WEAK void EXTI0_IRQHandler             () { unexpected_interrupt(); }          /* EXTI line 0 interrupt                              */
__WEAK void EXTI1_IRQHandler             () { unexpected_interrupt(); }          /* EXTI line 1 interrupt                              */
__WEAK void EXTI2_IRQHandler             () { unexpected_interrupt(); }          /* EXTI line 2 interrupt                              */
__WEAK void EXTI3_IRQHandler             () { unexpected_interrupt(); }          /* EXTI line 3 interrupt                              */
__WEAK void EXTI4_IRQHandler             () { unexpected_interrupt(); }          /* EXTI line 4 interrupt                              */
__WEAK void DMA1_Channel1_IRQHandler     () { unexpected_interrupt(); }          /* DMA1 channel 1 interrupt                           */
__WEAK void DMA1_Channel2_IRQHandler     () { unexpected_interrupt(); }          /* DMA1 channel 2 interrupt                           */
__WEAK void DMA1_Channel3_IRQHandler      () { unexpected_interrupt(); }         /* DMA1 channel 3 interrupt                           */
__WEAK void DMA1_Channel4_IRQHandler      () { unexpected_interrupt(); }         /* DMA1 channel 4 interrupt                           */
__WEAK void DMA1_Channel5_IRQHandler     () { unexpected_interrupt(); }          /* DMA1 channel 5 interrupt                           */
__WEAK void DMA1_Channel6_IRQHandler     () { unexpected_interrupt(); }          /* DMA1 channel 6 interrupt                           */
__WEAK void DMA1_Channel7_IRQHandler     () { unexpected_interrupt(); }          /* DMA1 channel 7 interrupt                           */
__WEAK void ADC_IRQHandler               () { unexpected_interrupt(); }          /* ADC interrupt                                      */
__WEAK void DAC_IRQHandler               () { unexpected_interrupt(); }          /* DAC interrupt                                      */
__WEAK void C2SEV_PWR_C2H_IRQHandler     () { unexpected_interrupt(); }          /* CPU M0+ SEV Interrupt                              */
__WEAK void COMP_IRQHandler              () { unexpected_interrupt(); }          /* COMP1 and COMP2 interrupt through EXTI             */
__WEAK void EXTI9_5_IRQHandler           () { unexpected_interrupt(); }          /* EXTI line 9_5 interrupt                            */
__WEAK void TIM1_BRK_IRQHandler          () { unexpected_interrupt(); }          /* Timer 1 break interrupt                            */
__WEAK void TIM1_UP_IRQHandler           () { unexpected_interrupt(); }          /* Timer 1 Update                                     */
__WEAK void TIM1_TRG_COM_IRQHandler      () { unexpected_interrupt(); }          /* Timer 1 trigger and communication                  */
__WEAK void TIM1_CC_IRQHandler           () { unexpected_interrupt(); }          /* Timer 1 capture compare interrupt                  */
__WEAK void TIM2_IRQHandler              () { unexpected_interrupt(); }          /* TIM2 global interrupt                              */
__WEAK void TIM16_IRQHandler             () { unexpected_interrupt(); }          /* Timer 16 global interrupt                          */
__WEAK void TIM17_IRQHandler             () { unexpected_interrupt(); }          /* Timer 17 global interrupt                          */
__WEAK void I2C1_EV_IRQHandler           () { unexpected_interrupt(); }          /* I2C1 event interrupt                               */
__WEAK void I2C1_ER_IRQHandler           () { unexpected_interrupt(); }          /* I2C1 event interrupt                               */
__WEAK void I2C2_EV_IRQHandler           () { unexpected_interrupt(); }          /* I2C2 error interrupt                               */
__WEAK void I2C2_ER_IRQHandler           () { unexpected_interrupt(); }          /* I2C2 error interrupt                               */
__WEAK void SPI1_IRQHandler              () { unexpected_interrupt(); }          /* SPI1 global interrupt                              */
__WEAK void SPI2_IRQHandler              () { unexpected_interrupt(); }          /* SPI2 global interrupt                              */
__WEAK void USART1_IRQHandler            () { unexpected_interrupt(); }          /* USART1 global interrupt                            */
__WEAK void USART2_IRQHandler            () { unexpected_interrupt(); }          /* USART2 global interrupt                            */
__WEAK void LPUART1_IRQHandler           () { unexpected_interrupt(); }          /* LPUART1 global interrupt                           */
__WEAK void LPTIM1_IRQHandler            () { unexpected_interrupt(); }          /* LPtimer 1 global interrupt                         */
__WEAK void LPTIM2_IRQHandler            () { unexpected_interrupt(); }          /* LPtimer 2 global interrupt                         */
__WEAK void EXTI15_10_IRQHandler         () { unexpected_interrupt(); }          /* EXTI line 15_10] interrupt through EXTI            */
__WEAK void RTC_Alarm_IRQHandler         () { unexpected_interrupt(); }          /* RTC Alarms A & B interrupt                         */
__WEAK void LPTIM3_IRQHandler            () { unexpected_interrupt(); }          /* LPtimer 3 global interrupt                         */
__WEAK void SUBGHZSPI_IRQHandler         () { unexpected_interrupt(); }          /* SUBGHZSPI global interrupt                         */
__WEAK void IPCC_C1_RX_IRQHandler        () { unexpected_interrupt(); }          /* IPCC CPU1 RX occupied interrupt                    */
__WEAK void IPCC_C1_TX_IRQHandler        () { unexpected_interrupt(); }          /* IPCC CPU1 RX free interrupt                        */
__WEAK void HSEM_IRQHandler              () { unexpected_interrupt(); }          /* Semaphore interrupt 0 to CPU1                      */
__WEAK void I2C3_EV_IRQHandler           () { unexpected_interrupt(); }          /* I2C3 event interrupt                               */
__WEAK void I2C3_ER_IRQHandler           () { unexpected_interrupt(); }          /* I2C3 error interrupt                               */
__WEAK void SUBGHZ_Radio_IRQHandler      () { unexpected_interrupt(); }          /* Radio IRQs RFBUSY interrupt through EXTI           */
__WEAK void AES_IRQHandler                () { unexpected_interrupt(); }         /* AES global interrupt                               */
__WEAK void RNG_IRQHandler                () { unexpected_interrupt(); }         /* RNG interrupt                                      */
__WEAK void PKA_IRQHandler               () { unexpected_interrupt(); }          /* PKA interrupt                                      */
__WEAK void DMA2_Channel1_IRQHandler     () { unexpected_interrupt(); }          /* DMA2 channel 1 interrupt                           */
__WEAK void DMA2_Channel2_IRQHandler     () { unexpected_interrupt(); }          /* DMA2 channel 2 interrupt                           */
__WEAK void DMA2_Channel3_IRQHandler     () { unexpected_interrupt(); }          /* DMA2 channel 3 interrupt                           */
__WEAK void DMA2_Channel4_IRQHandler     () { unexpected_interrupt(); }          /* DMA2 channel 4 interrupt                           */
__WEAK void DMA2_Channel5_IRQHandler     () { unexpected_interrupt(); }          /* DMA2 channel 5 interrupt                           */
__WEAK void DMA2_Channel6_IRQHandler     () { unexpected_interrupt(); }          /* DMA2 channel 6 interrupt                           */
__WEAK void DMA2_Channel7_IRQHandler     () { unexpected_interrupt(); }          /* DMA2 channel 7 interrupt                           */
__WEAK void DMAMUX1_OVR_IRQHandler() { unexpected_interrupt(); }                  /* DMAMUX overrun interrupt                           */
