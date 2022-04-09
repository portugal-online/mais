#include "cmsis_compiler.h"
#ifndef __APPLE__
	#include <sys/signal.h>
#else
	#include <signal.h>
#endif
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>
#include "stm32wl55xx_protos.h"
#include "stm32wlxx_hal_def.h"
#include <stdbool.h>
#include <assert.h>
#include <stdatomic.h>

void __NOP()
{
}

void __WFI()
{
    /*int signal;
    sigset_t s;
    sigemptyset(&s);
    sigaddset(&s, SIGUSR1 );
    sigwait(&s, &signal);
    */
    pause();
}

typedef void (*interrupt_handler)(void);

void Reset_Handler();
void NMI_Handler();
void HardFault_Handler();
void MemManage_Handler();
void BusFault_Handler();
void UsageFault_Handler();
void SVC_Handler();
void DebugMon_Handler();
void PendSV_Handler();
void SysTick_Handler();

static const interrupt_handler interrupt_handlers[] =
{
    0,
    Reset_Handler         ,
    NMI_Handler           ,
    HardFault_Handler     ,
    MemManage_Handler     ,
    BusFault_Handler      ,
    UsageFault_Handler    ,
    0                     ,
    0                     ,
    0                     ,
    0                     ,
    SVC_Handler           ,
    DebugMon_Handler      ,
    0                      ,
    PendSV_Handler         ,
    SysTick_Handler        ,
    WWDG_IRQHandler        ,                 /* Window Watchdog interrupt                          16 */
    PVD_PVM_IRQHandler     ,                 /* PVD and PVM interrupt through EXTI                 */
    TAMP_STAMP_LSECSS_SSRU_IRQHandler ,      /* RTC Tamper, RTC TimeStamp, LSECSS and RTC SSRU int.*/
    RTC_WKUP_IRQHandler         ,            /* RTC wakeup interrupt through EXTI[19]              */
    FLASH_IRQHandler            ,            /* Flash memory global interrupt and Flash memory ECC */
    RCC_IRQHandler              ,            /* RCC global interrupt                               */
    EXTI0_IRQHandler             ,           /* EXTI line 0 interrupt                              */
    EXTI1_IRQHandler             ,           /* EXTI line 1 interrupt                              */
    EXTI2_IRQHandler             ,           /* EXTI line 2 interrupt                              */
    EXTI3_IRQHandler             ,           /* EXTI line 3 interrupt                              */
    EXTI4_IRQHandler             ,           /* EXTI line 4 interrupt                              */
    DMA1_Channel1_IRQHandler     ,           /* DMA1 channel 1 interrupt                           */
    DMA1_Channel2_IRQHandler     ,           /* DMA1 channel 2 interrupt                           */
    DMA1_Channel3_IRQHandler      ,          /* DMA1 channel 3 interrupt                           */
    DMA1_Channel4_IRQHandler      ,          /* DMA1 channel 4 interrupt                           */
    DMA1_Channel5_IRQHandler     ,           /* DMA1 channel 5 interrupt                           */
    DMA1_Channel6_IRQHandler     ,           /* DMA1 channel 6 interrupt                           32 */
    DMA1_Channel7_IRQHandler     ,           /* DMA1 channel 7 interrupt                           */
    ADC_IRQHandler               ,           /* ADC interrupt                                      */
    DAC_IRQHandler               ,           /* DAC interrupt                                      */
    C2SEV_PWR_C2H_IRQHandler     ,           /* CPU M0+ SEV Interrupt                              */
    COMP_IRQHandler              ,           /* COMP1 and COMP2 interrupt through EXTI             */
    EXTI9_5_IRQHandler           ,           /* EXTI line 9_5 interrupt                            */
    TIM1_BRK_IRQHandler          ,           /* Timer 1 break interrupt                            */
    TIM1_UP_IRQHandler           ,           /* Timer 1 Update                                     */
    TIM1_TRG_COM_IRQHandler      ,           /* Timer 1 trigger and communication                  */
    TIM1_CC_IRQHandler           ,           /* Timer 1 capture compare interrupt                  */
    TIM2_IRQHandler              ,           /* TIM2 global interrupt                              */
    TIM16_IRQHandler             ,           /* Timer 16 global interrupt                          */
    TIM17_IRQHandler             ,           /* Timer 17 global interrupt                          */
    I2C1_EV_IRQHandler           ,           /* I2C1 event interrupt                               */
    I2C1_ER_IRQHandler           ,           /* I2C1 event interrupt                               */
    I2C2_EV_IRQHandler           ,           /* I2C2 error interrupt                               48 */
    I2C2_ER_IRQHandler           ,           /* I2C2 error interrupt                               */
    SPI1_IRQHandler              ,           /* SPI1 global interrupt                              */
    SPI2_IRQHandler              ,           /* SPI2 global interrupt                              */
    USART1_IRQHandler            ,           /* USART1 global interrupt                            */
    USART2_IRQHandler            ,           /* USART2 global interrupt                            */
    LPUART1_IRQHandler           ,           /* LPUART1 global interrupt                           */
    LPTIM1_IRQHandler            ,           /* LPtimer 1 global interrupt                         */
    LPTIM2_IRQHandler            ,           /* LPtimer 2 global interrupt                         */
    EXTI15_10_IRQHandler         ,           /* EXTI line 15_10] interrupt through EXTI            */
    RTC_Alarm_IRQHandler         ,           /* RTC Alarms A & B interrupt                         */
    LPTIM3_IRQHandler            ,           /* LPtimer 3 global interrupt                         */
    SUBGHZSPI_IRQHandler         ,           /* SUBGHZSPI global interrupt                         */
    IPCC_C1_RX_IRQHandler        ,           /* IPCC CPU1 RX occupied interrupt                    */
    IPCC_C1_TX_IRQHandler        ,           /* IPCC CPU1 RX free interrupt                        */
    HSEM_IRQHandler              ,           /* Semaphore interrupt 0 to CPU1                      */
    I2C3_EV_IRQHandler           ,           /* I2C3 event interrupt                               64 */
    I2C3_ER_IRQHandler           ,           /* I2C3 error interrupt                               */
    SUBGHZ_Radio_IRQHandler      ,           /* Radio IRQs RFBUSY interrupt through EXTI           */
    AES_IRQHandler                ,          /* AES global interrupt                               */
    RNG_IRQHandler                ,          /* RNG interrupt                                      */
    PKA_IRQHandler               ,           /* PKA interrupt                                      */
    DMA2_Channel1_IRQHandler     ,           /* DMA2 channel 1 interrupt                           */
    DMA2_Channel2_IRQHandler     ,           /* DMA2 channel 2 interrupt                           */
    DMA2_Channel3_IRQHandler     ,           /* DMA2 channel 3 interrupt                           */
    DMA2_Channel4_IRQHandler     ,           /* DMA2 channel 4 interrupt                           */
    DMA2_Channel5_IRQHandler     ,           /* DMA2 channel 5 interrupt                           */
    DMA2_Channel6_IRQHandler     ,           /* DMA2 channel 6 interrupt                           */
    DMA2_Channel7_IRQHandler     ,           /* DMA2 channel 7 interrupt                           */
    DMAMUX1_OVR_IRQHandler                  /* DMAMUX overrun interrupt                           */
};

void interrupt(int line)
{
    interrupt_handler h  = interrupt_handlers[line];
    h();
}

void unexpected_interrupt()
{
    HERROR("\n**********************************************************\n"
    "*\n"
    "*\n"
    "* Unexpected interrupt\n"
    "*\n"
    "*\n"
    "**********************************************************\n");

    exit(-1);
}
