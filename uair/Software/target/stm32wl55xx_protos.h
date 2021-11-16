#ifndef __STM32WL55XX_PROTOS_H__
#define __STM32WL55XX_PROTOS_H__

#ifndef STM32WL55xx
#error Invalid target
#endif

void WWDG_IRQHandler(void);                         /* Window Watchdog interrupt                          */
void PVD_PVM_IRQHandler(void);                      /* PVD and PVM interrupt through EXTI                 */
void TAMP_STAMP_LSECSS_SSRU_IRQHandler(void);       /* RTC Tamper, RTC TimeStamp, LSECSS and RTC SSRU int.*/
void RTC_WKUP_IRQHandler(void);                     /* RTC wakeup interrupt through EXTI[19]              */
void FLASH_IRQHandler(void);                        /* Flash memory global interrupt and Flash memory ECC */
void RCC_IRQHandler(void);                          /* RCC global interrupt                               */
void EXTI0_IRQHandler(void);                        /* EXTI line 0 interrupt                              */
void EXTI1_IRQHandler(void);                        /* EXTI line 1 interrupt                              */
void EXTI2_IRQHandler(void);                        /* EXTI line 2 interrupt                              */
void EXTI3_IRQHandler(void);                        /* EXTI line 3 interrupt                              */
void EXTI4_IRQHandler(void);                        /* EXTI line 4 interrupt                              */
void DMA1_Channel1_IRQHandler(void);                /* DMA1 channel 1 interrupt                           */
void DMA1_Channel2_IRQHandler(void);                /* DMA1 channel 2 interrupt                           */
void DMA1_Channel3_IRQHandler(void);                /* DMA1 channel 3 interrupt                           */
void DMA1_Channel4_IRQHandler(void);                /* DMA1 channel 4 interrupt                           */
void DMA1_Channel5_IRQHandler(void);                /* DMA1 channel 5 interrupt                           */
void DMA1_Channel6_IRQHandler(void);                /* DMA1 channel 6 interrupt                           */
void DMA1_Channel7_IRQHandler(void);                /* DMA1 channel 7 interrupt                           */
void ADC_IRQHandler(void);                          /* ADC interrupt                                      */
void DAC_IRQHandler(void);                          /* DAC interrupt                                      */
void C2SEV_PWR_C2H_IRQHandler(void);                /* CPU M0+ SEV Interrupt                              */
void COMP_IRQHandler(void);                         /* COMP1 and COMP2 interrupt through EXTI             */
void EXTI9_5_IRQHandler(void);                      /* EXTI line 9_5 interrupt                            */
void TIM1_BRK_IRQHandler(void);                     /* Timer 1 break interrupt                            */
void TIM1_UP_IRQHandler(void);                      /* Timer 1 Update                                     */
void TIM1_TRG_COM_IRQHandler(void);                 /* Timer 1 trigger and communication                  */
void TIM1_CC_IRQHandler(void);                      /* Timer 1 capture compare interrupt                  */
void TIM2_IRQHandler(void);                         /* TIM2 global interrupt                              */
void TIM16_IRQHandler(void);                        /* Timer 16 global interrupt                          */
void TIM17_IRQHandler(void);                        /* Timer 17 global interrupt                          */
void I2C1_EV_IRQHandler(void);                      /* I2C1 event interrupt                               */
void I2C1_ER_IRQHandler(void);                      /* I2C1 event interrupt                               */
void I2C2_EV_IRQHandler(void);                      /* I2C2 error interrupt                               */
void I2C2_ER_IRQHandler(void);                      /* I2C2 error interrupt                               */
void SPI1_IRQHandler(void);                         /* SPI1 global interrupt                              */
void SPI2_IRQHandler(void);                         /* SPI2 global interrupt                              */
void USART1_IRQHandler(void);                       /* USART1 global interrupt                            */
void USART2_IRQHandler(void);                       /* USART2 global interrupt                            */
void LPUART1_IRQHandler(void);                      /* LPUART1 global interrupt                           */
void LPTIM1_IRQHandler(void);                       /* LPtimer 1 global interrupt                         */
void LPTIM2_IRQHandler(void);                       /* LPtimer 2 global interrupt                         */
void EXTI15_10_IRQHandler(void);                    /* EXTI line 15_10] interrupt through EXTI            */
void RTC_Alarm_IRQHandler(void);                    /* RTC Alarms A & B interrupt                         */
void LPTIM3_IRQHandler(void);                       /* LPtimer 3 global interrupt                         */
void SUBGHZSPI_IRQHandler(void);                    /* SUBGHZSPI global interrupt                         */
void IPCC_C1_RX_IRQHandler(void);                   /* IPCC CPU1 RX occupied interrupt                    */
void IPCC_C1_TX_IRQHandler(void);                   /* IPCC CPU1 RX free interrupt                        */
void HSEM_IRQHandler(void);                         /* Semaphore interrupt 0 to CPU1                      */
void I2C3_EV_IRQHandler(void);                      /* I2C3 event interrupt                               */
void I2C3_ER_IRQHandler(void);                      /* I2C3 error interrupt                               */
void SUBGHZ_Radio_IRQHandler(void);                 /* Radio IRQs RFBUSY interrupt through EXTI           */
void AES_IRQHandler(void);                          /* AES global interrupt                               */
void RNG_IRQHandler(void);                          /* RNG interrupt                                      */
void PKA_IRQHandler(void);                          /* PKA interrupt                                      */
void DMA2_Channel1_IRQHandler(void);                /* DMA2 channel 1 interrupt                           */
void DMA2_Channel2_IRQHandler(void);                /* DMA2 channel 2 interrupt                           */
void DMA2_Channel3_IRQHandler(void);                /* DMA2 channel 3 interrupt                           */
void DMA2_Channel4_IRQHandler(void);                /* DMA2 channel 4 interrupt                           */
void DMA2_Channel5_IRQHandler(void);                /* DMA2 channel 5 interrupt                           */
void DMA2_Channel6_IRQHandler(void);                /* DMA2 channel 6 interrupt                           */
void DMA2_Channel7_IRQHandler(void);                /* DMA2 channel 7 interrupt                           */
void DMAMUX1_OVR_IRQHandler(void);                  /* DMAMUX overrun interrupt                           */

#endif

