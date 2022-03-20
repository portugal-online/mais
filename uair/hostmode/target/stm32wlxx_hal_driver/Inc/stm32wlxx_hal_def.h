#ifndef STM32WLXX_HAL_DEF_H__
#define STM32WLXX_HAL_DEF_H__

#include "hal_types.h"
#include "cpu_registers.h"
#include <stdio.h>

#ifndef MIN
#define MIN( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#endif

typedef struct {
    uint32_t dummy;
} RTC_TypeDef;

extern RTC_TypeDef _rtc;
#define RTC (&_rtc)

typedef struct {
    uint32_t dummy;
} IWDG_TypeDef;

extern IWDG_TypeDef _iwdg;
#define IWDG (&_iwdg)


struct gpio_handler_ops
{
    void (*init)(void *);
    int (*read)(void *);
    void (*write)(void *, int);
};

struct gpio_def
{
    uint32_t Mode;
    struct gpio_handler_ops ops;
    void *data;
};

typedef struct
{
    struct gpio_def def[16];
} GPIO_TypeDef;

extern GPIO_TypeDef _gpioa;
extern GPIO_TypeDef _gpiob;
extern GPIO_TypeDef _gpioc;

#define GPIOA ((GPIO_TypeDef*)&_gpioa)
#define GPIOB ((GPIO_TypeDef*)&_gpiob)
#define GPIOC ((GPIO_TypeDef*)&_gpioc)

struct i2c_device
{
    const struct i2c_device_ops *ops;
    void *data;
};

struct i2c_device_ops;

typedef struct
{
    uint32_t CR1;
    uint32_t CR2;
    uint32_t OAR1;
    uint32_t OAR2;
    uint32_t TIMINGR;
    uint32_t TIMEOUTR;


    struct i2c_device i2c_devices[256];

} I2C_TypeDef;

extern I2C_TypeDef _I2C1;
extern I2C_TypeDef _I2C2;
extern I2C_TypeDef _I2C3;

#define I2C1 ((I2C_TypeDef*)&_I2C1)
#define I2C2 ((I2C_TypeDef*)&_I2C2)
#define I2C3 ((I2C_TypeDef*)&_I2C3)

typedef struct
{
    uint32_t dummy;
} LPTIM_TypeDef;

extern LPTIM_TypeDef _LPTIM1;
extern LPTIM_TypeDef _LPTIM2;
extern LPTIM_TypeDef _LPTIM3;

#define LPTIM1 ((LPTIM_TypeDef*)&_LPTIM1)
#define LPTIM2 ((LPTIM_TypeDef*)&_LPTIM2)
#define LPTIM3 ((LPTIM_TypeDef*)&_LPTIM3)

typedef struct
{
    uint32_t dummy;
} TIM_TypeDef;

extern TIM_TypeDef _TIM1;
#define TIM1 ((TIM_TypeDef*)&_TIM1)

typedef struct
{
    uint32_t dummy;
} ADC_TypeDef;

extern ADC_TypeDef _ADC;
#define ADC (&_ADC)

typedef struct
{
    uint32_t dummy;
} SPI_TypeDef;

extern SPI_TypeDef _SPI1;
extern SPI_TypeDef _SPI2;

#define SPI1 (&_SPI1)
#define SPI2 (&_SPI2)

typedef struct
{
    uint32_t dummy;
} USART_TypeDef;

extern USART_TypeDef _usart1;
extern USART_TypeDef _usart2;
#define USART1 (&_usart1)
#define USART2 (&_usart2)

typedef struct {
    uint32_t dummy;
} RNG_TypeDef;
extern RNG_TypeDef _rng1;
#define RNG (&_rng1)

typedef struct {
    uint32_t dummy;
} DMA_TypeDef;

typedef struct {
    uint32_t dummy;
} DMA_Channel_TypeDef;

typedef struct {
    uint32_t dummy;

} DMAMUX_Channel_TypeDef;

typedef struct {
    uint32_t dummy;
} DMAMUX_ChannelStatus_TypeDef;

typedef struct {
    uint32_t dummy;
} DMAMUX_RequestGen_TypeDef;

typedef struct {
    uint32_t dummy;
} DMAMUX_RequestGenStatus_TypeDef;



extern DMA_TypeDef _dma1;
#define DMA1 (&_dma1)

extern DMA_Channel_TypeDef _dma1chan5;
extern DMA_Channel_TypeDef _dma1chan4;
extern DMA_Channel_TypeDef _dma1chan1;

#define DMA1_Channel1 (&_dma1chan1)
#define DMA1_Channel4 (&_dma1chan4)
#define DMA1_Channel5 (&_dma1chan5)

#define __HAL_LINKDMA(__HANDLE__, __PPP_DMA_FIELD__, __DMA_HANDLE__) /* */

void LL_EXTI_EnableIT_0_31(uint32_t);
void LL_EXTI_EnableIT_32_63(uint32_t);

#define LL_EXTI_LINE_27 (27)
#define LL_EXTI_LINE_46 (46)

#include <string.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOGSTREAM stdout

#define HERROR(x...) do {  \
    fprintf(LOGSTREAM, "\033[31;1m%s: ", __FUNCTION__); \
    fprintf(LOGSTREAM, x); \
    fprintf(LOGSTREAM, ", at %s line %d\033[0m\n", __FILENAME__, __LINE__); \
    } while (0) \

#define HWARN(x...) do {  \
    fprintf(LOGSTREAM, "\033[33;1m%s: ", __FUNCTION__); \
    fprintf(LOGSTREAM, x); \
    fprintf(LOGSTREAM, ", at %s line %d\033[0m\n", __FILENAME__, __LINE__); \
    } while (0) \

#define HLOG(x...) do {  \
    fprintf(LOGSTREAM, "\033[36;1m%s: ", __FUNCTION__); \
    fprintf(LOGSTREAM, x); \
    fprintf(LOGSTREAM, ", at %s line %d\033[0m\n", __FILENAME__, __LINE__); \
    } while (0) \


#endif
