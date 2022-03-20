#include "stm32wlxx_hal_rcc.h"

#define PERIPH_I2C1 (0)
#define PERIPH_I2C2 (1)
#define PERIPH_I2C3 (2)
#define PERIPH_USART1 (3)
#define PERIPH_USART2 (4)
#define PERIPH_GPIOA (5)
#define PERIPH_GPIOB (6)
#define PERIPH_GPIOC (7)
#define PERIPH_LPTIM1 (8)
#define PERIPH_ADC (9)
#define PERIPH_RTC (10)
#define PERIPH_RTCAPB (11)
#define PERIPH_DMAMUX1 (12)
#define PERIPH_DMA1 (13)
#define PERIPH_SPI1 (14)
#define PERIPH_SPI2 (15)
#define PERIPH_SUBGHZSPI (16)
#define PERIPH_RNG (17)

static uint32_t reset_state = 0;
static uint32_t clock_state = 0;
static uint32_t flags = RCC_FLAG_PINRST;

static void periph_clk_enable(uint8_t module) {
    clock_state |= (1<<module);
}
static void periph_clk_disable(uint8_t module) {
    clock_state &= ~(1<<module);
}
static void periph_reset_enable(uint8_t module) {
    reset_state |= (1<<module);
}
static void periph_reset_disable(uint8_t module) {
    reset_state &= ~(1<<module);
}

#define CLOCKFUN(module) \
    void __HAL_RCC_ ## module ## _CLK_ENABLE() { \
    periph_clk_enable(PERIPH_ ## module); } \
    void __HAL_RCC_ ## module ## _CLK_DISABLE() { \
    periph_clk_disable(PERIPH_ ## module); }

#define RESETFUN(module) \
    void __HAL_RCC_ ## module ## _FORCE_RESET() { \
    periph_reset_enable(PERIPH_ ## module); }  \
    void __HAL_RCC_ ## module ## _RELEASE_RESET() { \
    periph_reset_disable(PERIPH_ ## module); }

CLOCKFUN(I2C1);
RESETFUN(I2C1);
CLOCKFUN(I2C2);
RESETFUN(I2C2);
CLOCKFUN(I2C3);
RESETFUN(I2C3);

CLOCKFUN(USART1);
RESETFUN(USART1);
CLOCKFUN(USART2);
RESETFUN(USART2);

CLOCKFUN(GPIOA);
CLOCKFUN(GPIOB);
CLOCKFUN(GPIOC);

CLOCKFUN(LPTIM1);
CLOCKFUN(RTC);
CLOCKFUN(ADC);
CLOCKFUN(RTCAPB);
CLOCKFUN(DMAMUX1);
CLOCKFUN(SUBGHZSPI);
CLOCKFUN(SPI2);
RESETFUN(SPI2);
CLOCKFUN(DMA1);
CLOCKFUN(RNG);

uint32_t __HAL_RCC_GET_FLAG(uint32_t flag) { return flags & flag; };
void __HAL_RCC_CLEAR_RESET_FLAGS(void) { flags = 0; }

void __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(uint32_t config)
{
}
void __HAL_RCC_HSE_CONFIG(uint32_t config)
{
}

void __HAL_RCC_RTC_ENABLE();
void __HAL_RCC_RTC_DISABLE();

int LL_RCC_HSI_IsReady() {
    return 1;
}

int LL_RCC_LSI_IsReady() {
    return 1;
}

void LL_RCC_LSE_SetDriveCapability(uint32_t drive) {
}

HAL_StatusTypeDef HAL_RCCEx_PeriphCLKConfig(const RCC_PeriphCLKInitTypeDef *d)
{
    return HAL_OK;
}

HAL_StatusTypeDef HAL_RCC_OscConfig(const RCC_OscInitTypeDef *d)
{
    return HAL_OK;
}

HAL_StatusTypeDef HAL_RCC_ClockConfig(const RCC_ClkInitTypeDef *d, uint32_t latency)
{
    return HAL_OK;

}

