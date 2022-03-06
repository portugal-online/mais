#ifndef STM32WLXX_HAL_I2C_PVT_H__
#define STM32WLXX_HAL_I2C_PVT_H__

#include "stm32wlxx_hal_dma.h"
#include "stm32wlxx_hal_i2c.h"

struct i2c_device_ops
{
    int (*master_transmit)(void *,const uint8_t *pData, uint16_t Size);
    int (*master_receive)(void *,uint8_t *pData, uint16_t Size);
    int (*master_mem_write)(void *,uint16_t memaddress, uint8_t memaddrsize, const uint8_t *pData, uint16_t Size);
    int (*master_mem_read)(void *,uint16_t memaddress, uint8_t memaddrsize, uint8_t *pData, uint16_t Size);
};

void i2c_register_device(I2C_TypeDef *bus, uint8_t device, const struct i2c_device_ops*ops, void *user);

#endif
