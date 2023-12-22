#ifndef STM32WLXX_HAL_DEF_H__
#define STM32WLXX_HAL_DEF_H__

#include "hal_types.h"
#include <inttypes.h>
#include "cpu_registers.h"
#include <stdio.h>
#include "hlog.h"
#include <stdbool.h>

#ifndef MIN
#define MIN( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#endif

typedef struct {
    uint32_t dummy;
} RTC_TypeDef;

extern RTC_TypeDef _rtc;
#define RTC (&_rtc)

typedef struct {
    uint8_t prescaler;
    volatile uint16_t counter;
    volatile uint16_t period;
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

typedef enum
{
    I2C_NORMAL,
    I2C_BUSY,
    I2C_FAIL_PRETX,
    I2C_FAIL_POSTTX
} i2c_error_mode_t;

struct i2c_device
{
    const struct i2c_device_ops *ops;
    void *data;
    i2c_error_mode_t error_mode;
    uint32_t error_code;
};

struct i2c_device_ops;

#undef CR1
#undef CR2
typedef struct
{
    uint32_t CR1;
    uint32_t CR2;
    uint32_t OAR1;
    uint32_t OAR2;
    uint32_t TIMINGR;
    uint32_t TIMEOUTR;
    uint32_t ISR;

    struct i2c_device i2c_devices[256];
    uint32_t mode;
    bool init;
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

typedef struct {
    uint32_t dummy;
} RNG_TypeDef;
extern RNG_TypeDef _rng1;
#define RNG (&_rng1)

typedef struct {
    uint32_t dummy;
} DMA_TypeDef;

typedef struct {
    uint16_t id;
    int16_t interrupt;
    void *parent;
    uint32_t Request;
    uint32_t Direction;
    uint32_t PeriphInc;
    uint32_t MemInc;
    uint32_t PeriphDataAlignment;
    uint32_t MemDataAlignment;
    uint32_t Mode;

    size_t Source;
    size_t Dest;
    unsigned Len;
    unsigned Offset;
    
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


extern DMA_Channel_TypeDef _dma1channels[8];

#define DMA1_Channel1 (&_dma1channels[1])
#define DMA1_Channel4 (&_dma1channels[4])
#define DMA1_Channel5 (&_dma1channels[5])



typedef struct
{
    uint32_t dummy;
    size_t virtual_read_address;
    FILE *filedes;
} USART_TypeDef;

extern USART_TypeDef _usart1;
extern USART_TypeDef _usart2;
#define USART1 (&_usart1)
#define USART2 (&_usart2)


#define __HAL_LINKDMA(__HANDLE__, __PPP_DMA_FIELD__, __DMA_HANDLE__) \
    do { (__HANDLE__) -> __PPP_DMA_FIELD__  = &__DMA_HANDLE__ ; \
    (&__DMA_HANDLE__)->Parent = __HANDLE__;  \
} while (0)


typedef struct
{
    uint32_t poly;
    uint32_t initial;
    uint32_t initialxor;
    uint32_t finalxor;

    uint32_t crcTable[256];

} CRC_TypeDef;

extern CRC_TypeDef _crc1;
#define CRC (&_crc1)


void LL_EXTI_EnableIT_0_31(uint32_t);
void LL_EXTI_EnableIT_32_63(uint32_t);

#define LL_EXTI_LINE_27 (27)
#define LL_EXTI_LINE_46 (46)


#endif
