#include "stm32wlxx_hal.h"
#include "stm32wlxx_hal_i2c_pvt.h"
#include <stdio.h>
#include <stdlib.h>

static struct i2c_device *find_i2c_device(I2C_TypeDef *i2c, uint16_t address)
{
    struct i2c_device *d = &i2c->i2c_devices[(address>>1)&0x7F];
    if (!d->ops) {
        HERROR("Attempting to access unknown I2C device at address %d (0x%02x)", address, address);
        abort();
    }
    return d;
}

void i2c_register_device(I2C_TypeDef *bus, uint8_t device, const struct i2c_device_ops*ops, void *user)
{
    struct i2c_device *d = &bus->i2c_devices[device&0x7F];
    d->ops = ops;
    d->data = user;
}

HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c)
{
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_DeInit(I2C_HandleTypeDef *hi2c)
{
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size,
                                          uint32_t Timeout)
{
    struct i2c_device *dev = find_i2c_device(hi2c->Instance, DevAddress);
    return dev->ops->master_transmit(dev->data, pData, Size);
}

HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size,
                                         uint32_t Timeout)
{
    struct i2c_device *dev = find_i2c_device(hi2c->Instance, DevAddress);
    return dev->ops->master_receive(dev->data, pData, Size);
}

HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress,
                                    uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    struct i2c_device *dev = find_i2c_device(hi2c->Instance, DevAddress);
    return dev->ops->master_mem_write(dev->data, MemAddress, MemAddSize, pData, Size);
}

HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress,
                                   uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    struct i2c_device *dev = find_i2c_device(hi2c->Instance, DevAddress);
    return dev->ops->master_mem_read(dev->data, MemAddress, MemAddSize, pData, Size);
}


HAL_StatusTypeDef HAL_I2CEx_ConfigAnalogFilter(I2C_HandleTypeDef *hi2c, uint32_t AnalogFilter)
{
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2CEx_ConfigDigitalFilter(I2C_HandleTypeDef *hi2c, uint32_t DigitalFilter)
{
    return HAL_OK;
}


