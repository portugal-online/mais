#ifndef STM32WLXX_HAL_RCC_H__
#define STM32WLXX_HAL_RCC_H__

#include "hal_types.h"
#include "stm32wlxx_hal_flash.h"

// Host-mode
typedef struct {
    uint32_t PeriphClockSelection;
    union {
        uint32_t Lptim1ClockSelection;
        uint32_t RTCClockSelection;
        uint32_t Usart2ClockSelection;
        uint32_t I2c1ClockSelection;
        uint32_t I2c2ClockSelection;
        uint32_t I2c3ClockSelection;
        uint32_t I2s2ClockSelection;
        uint32_t RngClockSelection;
    };
} RCC_PeriphCLKInitTypeDef;

typedef struct
{
  uint32_t PLLState;   /*!< The new state of the PLL.
                            This parameter must be a value of @ref RCC_PLL_Config                                 */

  uint32_t PLLSource;  /*!< RCC_PLLSource: PLL entry clock source.
                            This parameter must be a value of @ref RCC_PLL_Clock_Source                           */

  uint32_t PLLM;       /*!< PLLM: Division factor for PLL VCO input clock.
                            This parameter must be a value of @ref RCC_PLLM_Clock_Divider                         */

  uint32_t PLLN;       /*!< PLLN: Multiplication factor for PLL VCO output clock.
                            This parameter must be a number between Min_Data = 6 and Max_Data = 127                */

  uint32_t PLLP;       /*!< PLLP: Division factor for ADC clock.
                            This parameter must be a value of @ref RCC_PLLP_Clock_Divider                         */

  uint32_t PLLQ;       /*!< PLLQ: Division factor for I2S2 and RNG clock.
                            This parameter must be a value of @ref RCC_PLLQ_Clock_Divider                         */

  uint32_t PLLR;       /*!< PLLR: Division for the main system clock.
                            User has to set the PLLR parameter correctly to not exceed max frequency 48 MHZ.
                            This parameter must be a value of @ref RCC_PLLR_Clock_Divider                         */

} RCC_PLLInitTypeDef;

/**
  * @brief  RCC Internal/External Oscillator (HSE, HSI, MSI, LSE and LSI) configuration structure definition
  */
typedef struct
{
  uint32_t OscillatorType;       /*!< The oscillators to be configured.
                                      This parameter can be a combination of @ref RCC_Oscillator_Type             */

  uint32_t HSEState;             /*!< The new state of the HSE.
                                      This parameter can be a value of @ref RCC_HSE_Config                        */

  uint32_t HSEDiv;               /*!< The division factor of the HSE.
                                      This parameter can be a value of @ref RCC_HSE_Div                           */

  uint32_t LSEState;             /*!< The new state of the LSE.
                                      This parameter can be a value of @ref RCC_LSE_Config                        */

  uint32_t HSIState;             /*!< The new state of the HSI.
                                      This parameter can be a value of @ref RCC_HSI_Config                        */

  uint32_t HSICalibrationValue;  /*!< The calibration trimming value (default is @ref RCC_HSICALIBRATION_DEFAULT).
                                      This parameter must be a number between Min_Data = 0x00 and Max_Data = 0x7F */

  uint32_t LSIState;             /*!< The new state of the LSI.
                                      This parameter can be a value of @ref RCC_LSI_Config                        */

  uint32_t LSIDiv;               /*!< The division factor of the LSI.
                                      This parameter can be a value of @ref RCC_LSI_Div                           */

  uint32_t MSIState;             /*!< The new state of the MSI.
                                      This parameter can be a value of @ref RCC_MSI_Config */

  uint32_t MSICalibrationValue;  /*!< The calibration trimming value (default is @ref RCC_MSICALIBRATION_DEFAULT).
                                      This parameter must be a number between Min_Data = 0x00 and Max_Data = 0xFF */

  uint32_t MSIClockRange;        /*!< The MSI frequency range.
                                      This parameter can be a value of @ref RCC_MSI_Clock_Range                   */

  RCC_PLLInitTypeDef PLL;        /*!< Main PLL structure parameters                                               */

} RCC_OscInitTypeDef;

/**
  * @brief  RCC System, AHB and APB buses clock configuration structure definition
  */
typedef struct
{
  uint32_t ClockType;             /*!< The clock to be configured.
                                       This parameter can be a combination of @ref RCC_System_Clock_Type          */

  uint32_t SYSCLKSource;          /*!< The clock source used as system clock (SYSCLK).
                                       This parameter can be a value of @ref RCC_System_Clock_Source              */

  uint32_t AHBCLKDivider;         /*!< The AHBx clock (HCLK1) divider. This clock is derived from the system clock (SYSCLK).
                                       This parameter can be a value of @ref RCC_AHBx_Clock_Source                */

  uint32_t APB1CLKDivider;        /*!< The APB1 clock (PCLK1) divider. This clock is derived from the AHB clock (HCLK).
                                       This parameter can be a value of @ref RCC_APBx_Clock_Source                */

  uint32_t APB2CLKDivider;        /*!< The APB2 clock (PCLK2) divider. This clock is derived from the AHB clock (HCLK).
                                       This parameter can be a value of @ref RCC_APBx_Clock_Source                */

#if defined(DUAL_CORE)
  uint32_t AHBCLK2Divider;        /*!< The AHB clock (HCLK2) divider. This clock is derived from the system clock (SYSCLK).
                                       This parameter can be a value of @ref RCC_AHBx_Clock_Source                */

#endif /* DUAL_CORE */
  uint32_t AHBCLK3Divider;        /*!< The AHB shared clock (HCLK3) divider. This clock is derived from the system clock (SYSCLK).
                                       This parameter can be a value of @ref RCC_AHBx_Clock_Source                */

} RCC_ClkInitTypeDef;



#define RCC_FLAG_PINRST (1<<0)
#define RCC_FLAG_BORRST (1<<1)
#define RCC_FLAG_SFTRST (1<<2)
#define RCC_FLAG_IWDGRST (1<<3)
#define RCC_FLAG_WWDGRST (1<<4)
#define RCC_FLAG_LPWRRST (1<<5)

#define RCC_STOP_WAKEUPCLOCK_MSI (1)

#define RCC_LPTIM1CLKSOURCE_PCLK1 (0)
#define RCC_PERIPHCLK_LPTIM1 (0)
#define RCC_PERIPHCLK_RTC (0)
#define RCC_PERIPHCLK_I2C1 (0)
#define RCC_PERIPHCLK_USART2 (0)
#define RCC_PERIPHCLK_I2C2 (0)
#define RCC_PERIPHCLK_I2C3 (0)
#define RCC_PERIPHCLK_I2S2 (0)
#define RCC_PERIPHCLK_RNG (0)

#define RCC_RTCCLKSOURCE_LSE (0)
#define RCC_USART2CLKSOURCE_SYSCLK (0)
#define RCC_I2C1CLKSOURCE_SYSCLK (0)
#define RCC_I2C2CLKSOURCE_SYSCLK (0)
#define RCC_I2C3CLKSOURCE_SYSCLK (0)
#define RCC_I2S2CLKSOURCE_HSI (0)
#define RCC_RNGCLKSOURCE_LSE (0)

#define RCC_SYSCLKSOURCE_MSI (0)

#define RCC_SYSCLK_DIV1 (1)
#define RCC_HCLK_DIV1 (1)
#define RCC_LSI_DIV128 (1)

#define RCC_OSCILLATORTYPE_MSI (1<<0)
#define RCC_OSCILLATORTYPE_LSE (1<<1)
#define RCC_OSCILLATORTYPE_HSI (1<<2)
#define RCC_OSCILLATORTYPE_LSI (1<<3)

#define RCC_MSIRANGE_9 (9)
#define RCC_MSIRANGE_5 (5)

#define RCC_HSICALIBRATION_DEFAULT 0
#define RCC_MSICALIBRATION_DEFAULT 0
#define RCC_LSICALIBRATION_DEFAULT 0

#define RCC_PLL_NONE (0)

uint32_t __HAL_RCC_GET_FLAG(uint32_t flag);
void __HAL_RCC_CLEAR_RESET_FLAGS(void);

#define RCC_HSE_OFF (0)
#define RCC_MSI_ON (1<<0)
#define RCC_LSE_ON (1<<1)
#define RCC_HSE_ON (1<<2)
#define RCC_LSI_ON (1<<3)

#define RCC_LSEDRIVE_LOW (0)

#define RCC_CLOCKTYPE_HCLK3 (1<<3)
#define RCC_CLOCKTYPE_HCLK2 (1<<2)
#define RCC_CLOCKTYPE_HCLK (1<<0)
#define RCC_CLOCKTYPE_SYSCLK (1<<4)
#define RCC_CLOCKTYPE_PCLK1 (1<<5)
#define RCC_CLOCKTYPE_PCLK2 (1<<6)


void __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(uint32_t);
void __HAL_RCC_HSE_CONFIG(uint32_t);

void __HAL_RCC_I2C1_FORCE_RESET();
void __HAL_RCC_I2C1_RELEASE_RESET();
void __HAL_RCC_I2C1_CLK_ENABLE();
void __HAL_RCC_I2C1_CLK_DISABLE();

void __HAL_RCC_I2C2_FORCE_RESET();
void __HAL_RCC_I2C2_RELEASE_RESET();
void __HAL_RCC_I2C2_CLK_ENABLE();
void __HAL_RCC_I2C2_CLK_DISABLE();
void __HAL_RCC_I2C3_FORCE_RESET();
void __HAL_RCC_I2C3_RELEASE_RESET();
void __HAL_RCC_I2C3_CLK_ENABLE();
void __HAL_RCC_I2C3_CLK_DISABLE();

void __HAL_RCC_USART2_FORCE_RESET();
void __HAL_RCC_USART2_RELEASE_RESET();
void __HAL_RCC_USART1_CLK_ENABLE();
void __HAL_RCC_USART1_CLK_DISABLE();
void __HAL_RCC_USART2_CLK_ENABLE();
void __HAL_RCC_USART2_CLK_DISABLE();

void __HAL_RCC_GPIOA_CLK_ENABLE();
void __HAL_RCC_GPIOB_CLK_ENABLE();
void __HAL_RCC_GPIOC_CLK_ENABLE();

void __HAL_RCC_GPIOA_CLK_DISABLE();
void __HAL_RCC_GPIOB_CLK_DISABLE();
void __HAL_RCC_GPIOC_CLK_DISABLE();

void __HAL_RCC_LPTIM1_CLK_ENABLE();
void __HAL_RCC_LPTIM1_CLK_DISABLE();

void __HAL_RCC_ADC_CLK_ENABLE();
void __HAL_RCC_ADC_CLK_DISABLE();

void __HAL_RCC_RTC_CLK_ENABLE();
void __HAL_RCC_RTC_CLK_DISABLE();

void __HAL_RCC_RTCAPB_CLK_ENABLE();
void __HAL_RCC_RTCAPB_CLK_DISABLE();

void __HAL_RCC_DMAMUX1_CLK_ENABLE();
void __HAL_RCC_DMAMUX1_CLK_DISABLE();

void __HAL_RCC_SUBGHZSPI_CLK_ENABLE();
void __HAL_RCC_SUBGHZSPI_CLK_DISABLE();

void __HAL_RCC_RTC_ENABLE();
void __HAL_RCC_RTC_DISABLE();

void __HAL_RCC_SPI2_FORCE_RESET();
void __HAL_RCC_SPI2_RELEASE_RESET();

void __HAL_RCC_SPI2_CLK_ENABLE();
void __HAL_RCC_SPI2_CLK_DISABLE();

void __HAL_RCC_DMA1_CLK_ENABLE();
void __HAL_RCC_DMA1_CLK_DISABLE();

void __HAL_RCC_RNG_CLK_ENABLE();
void __HAL_RCC_RNG_CLK_DISABLE();

int LL_RCC_HSI_IsReady();
int LL_RCC_LSI_IsReady();
void LL_RCC_LSE_SetDriveCapability(uint32_t);

HAL_StatusTypeDef HAL_RCCEx_PeriphCLKConfig(const RCC_PeriphCLKInitTypeDef*);
HAL_StatusTypeDef HAL_RCC_OscConfig(const RCC_OscInitTypeDef*);
HAL_StatusTypeDef HAL_RCC_ClockConfig(const RCC_ClkInitTypeDef*, uint32_t latency);


#endif
