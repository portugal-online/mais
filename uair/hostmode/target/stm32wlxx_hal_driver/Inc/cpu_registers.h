#ifndef CPU_REGISTERS_H__
#define CPU_REGISTERS_H__

#define DUAL_CORE (1)

#define LPTIM_CFGR_PRESC_0 (1<<0)
#define LPTIM_CFGR_PRESC_2 (1<<2)



#define RTC_CR_TAMPALRM_TYPE (1<<0)


#define RTC_ICSR_BIN_0 (1<<0)

#define RTC_ALRMASSR_MASKSS_Pos        0

#define RTC_CR_ALRAE (1<<0)


#define FLASH_CR_PER (1<<0)
#define FLASH_CR_PG  (1<<1)

#define USART_CR3_WUS_1 (1<<0)

#define USART_ISR_BUSY (1<<0)
#define USART_ISR_REACK (1<<1)

#define USART_CR1_RE (1<<0)
#define USART_CR1_TE (1<<0)

// IWDG
#define IWDG_PR_PR_0 (1<<0)
#define IWDG_PR_PR_1 (1<<1)
#define IWDG_PR_PR_2 (1<<2)

#define IWDG_WINR_WIN (0x0FFFU)

typedef enum
{
  RESET = 0,
  SET = !RESET
} FlagStatus, ITStatus;

typedef enum
{
/******  Cortex-M4 Processor Exceptions Numbers ****************************************************************/
  NonMaskableInt_IRQn                 = -14,    /*!< Non Maskable Interrupt                                            */
  HardFault_IRQn                      = -13,    /*!< Cortex-M4 Hard Fault Interrupt                                    */
  MemoryManagement_IRQn               = -12,    /*!< Cortex-M4 Memory Management Interrupt                             */
  BusFault_IRQn                       = -11,    /*!< Cortex-M4 Bus Fault Interrupt                                     */
  UsageFault_IRQn                     = -10,    /*!< Cortex-M4 Usage Fault Interrupt                                   */
  SVCall_IRQn                         = -5,     /*!< Cortex-M4 SV Call Interrupt                                       */
  DebugMonitor_IRQn                   = -4,     /*!< Cortex-M4 Debug Monitor Interrupt                                 */
  PendSV_IRQn                         = -2,     /*!< Cortex-M4 Pend SV Interrupt                                       */
  SysTick_IRQn                        = -1,     /*!< Cortex-M4 System Tick Interrupt                                   */

/*************  STM32WLxx specific Interrupt Numbers on M4 core ************************************************/
  WWDG_IRQn                           = 0,      /*!< Window WatchDog Interrupt                                         */
  PVD_PVM_IRQn                        = 1,      /*!< PVD and PVM detector                                              */
  TAMP_STAMP_LSECSS_SSRU_IRQn         = 2,      /*!< RTC Tamper, RTC TimeStamp, LSECSS and RTC SSRU Interrupts         */
  RTC_WKUP_IRQn                       = 3,      /*!< RTC Wakeup Interrupt                                              */
  FLASH_IRQn                          = 4,      /*!< FLASH (CFI)  global Interrupt                                     */
  RCC_IRQn                            = 5,      /*!< RCC Interrupt                                                     */
  EXTI0_IRQn                          = 6,      /*!< EXTI Line 0 Interrupt                                             */
  EXTI1_IRQn                          = 7,      /*!< EXTI Line 1 Interrupt                                             */
  EXTI2_IRQn                          = 8,      /*!< EXTI Line 2 Interrupt                                             */
  EXTI3_IRQn                          = 9,      /*!< EXTI Line 3 Interrupt                                             */
  EXTI4_IRQn                          = 10,     /*!< EXTI Line 4 Interrupt                                             */
  DMA1_Channel1_IRQn                  = 11,     /*!< DMA1 Channel 1 Interrupt                                          */
  DMA1_Channel2_IRQn                  = 12,     /*!< DMA1 Channel 2 Interrupt                                          */
  DMA1_Channel3_IRQn                  = 13,     /*!< DMA1 Channel 3 Interrupt                                          */
  DMA1_Channel4_IRQn                  = 14,     /*!< DMA1 Channel 4 Interrupt                                          */
  DMA1_Channel5_IRQn                  = 15,     /*!< DMA1 Channel 5 Interrupt                                          */
  DMA1_Channel6_IRQn                  = 16,     /*!< DMA1 Channel 6 Interrupt                                          */
  DMA1_Channel7_IRQn                  = 17,     /*!< DMA1 Channel 7 Interrupt                                          */
  ADC_IRQn                            = 18,     /*!< ADC Interrupt                                                     */
  DAC_IRQn                            = 19,     /*!< DAC Interrupt                                                     */
  C2SEV_PWR_C2H_IRQn                  = 20,     /*!< CPU2 SEV Interrupt                                                */
  COMP_IRQn                           = 21,     /*!< COMP1 and COMP2 Interrupts                                        */
  EXTI9_5_IRQn                        = 22,     /*!< EXTI Lines [9:5] Interrupt                                        */
  TIM1_BRK_IRQn                       = 23,     /*!< TIM1 Break Interrupt                                              */
  TIM1_UP_IRQn                        = 24,     /*!< TIM1 Update Interrupt                                             */
  TIM1_TRG_COM_IRQn                   = 25,     /*!< TIM1 Trigger and Communication Interrupts                         */
  TIM1_CC_IRQn                        = 26,     /*!< TIM1 Capture Compare Interrupt                                    */
  TIM2_IRQn                           = 27,     /*!< TIM2 Global Interrupt                                             */
  TIM16_IRQn                          = 28,     /*!< TIM16 Global Interrupt                                            */
  TIM17_IRQn                          = 29,     /*!< TIM17 Global Interrupt                                            */
  I2C1_EV_IRQn                        = 30,     /*!< I2C1 Event Interrupt                                              */
  I2C1_ER_IRQn                        = 31,     /*!< I2C1 Error Interrupt                                              */
  I2C2_EV_IRQn                        = 32,     /*!< I2C2 Event Interrupt                                              */
  I2C2_ER_IRQn                        = 33,     /*!< I2C2 Error Interrupt                                              */
  SPI1_IRQn                           = 34,     /*!< SPI1 Interrupt                                                    */
  SPI2_IRQn                           = 35,     /*!< SPI2 Interrupt                                                    */
  USART1_IRQn                         = 36,     /*!< USART1 Interrupt                                                  */
  USART2_IRQn                         = 37,     /*!< USART2 Interrupt                                                  */
  LPUART1_IRQn                        = 38,     /*!< LPUART1 Interrupt                                                 */
  LPTIM1_IRQn                         = 39,     /*!< LPTIM1 Global Interrupt                                           */
  LPTIM2_IRQn                         = 40,     /*!< LPTIM2 Global Interrupt                                           */
  EXTI15_10_IRQn                      = 41,     /*!< EXTI Lines [15:10] Interrupt                                      */
  RTC_Alarm_IRQn                      = 42,     /*!< RTC Alarms (A and B) Interrupt                                    */
  LPTIM3_IRQn                         = 43,     /*!< LPTIM3 Global Interrupt                                           */
  SUBGHZSPI_IRQn                      = 44,     /*!< SUBGHZSPI Interrupt                                               */
  IPCC_C1_RX_IRQn                     = 45,     /*!< IPCC RX Occupied Interrupt                                        */
  IPCC_C1_TX_IRQn                     = 46,     /*!< IPCC TX Free Interrupt                                            */
  HSEM_IRQn                           = 47,     /*!< HSEM Interrupt                                                    */
  I2C3_EV_IRQn                        = 48,     /*!< I2C3 Event Interrupt                                              */
  I2C3_ER_IRQn                        = 49,     /*!< I2C3 Error Interrupt                                              */
  SUBGHZ_Radio_IRQn                   = 50,     /*!< SUBGHZ Radio Interrupt                                            */
  AES_IRQn                            = 51,     /*!< AES Interrupt                                                     */
  RNG_IRQn                            = 52,     /*!< RNG Interrupt                                                     */
  PKA_IRQn                            = 53,     /*!< PKA Interrupt                                                     */
  DMA2_Channel1_IRQn                  = 54,     /*!< DMA2 Channel 1 Interrupt                                          */
  DMA2_Channel2_IRQn                  = 55,     /*!< DMA2 Channel 2 Interrupt                                          */
  DMA2_Channel3_IRQn                  = 56,     /*!< DMA2 Channel 3 Interrupt                                          */
  DMA2_Channel4_IRQn                  = 57,     /*!< DMA2 Channel 4 Interrupt                                          */
  DMA2_Channel5_IRQn                  = 58,     /*!< DMA2 Channel 5 Interrupt                                          */
  DMA2_Channel6_IRQn                  = 59,     /*!< DMA2 Channel 6 Interrupt                                          */
  DMA2_Channel7_IRQn                  = 60,     /*!< DMA2 Channel 7 Interrupt                                          */
  DMAMUX1_OVR_IRQn                    = 61      /*!< DMAMUX1 overrun Interrupt                                         */
} IRQn_Type;


#endif
