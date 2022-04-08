#include "stm32wlxx_hal.h"
#include "stm32wlxx_hal_i2c_pvt.h"
#include <stdio.h>
#include <stdlib.h>

void i2c_set_error_mode( I2C_TypeDef *bus, uint8_t device, i2c_error_mode_t error_mode, uint32_t error_code)
{
    struct i2c_device *d = &bus->i2c_devices[(device>>1)&0x7F];
    d->error_mode = error_mode;
    d->error_code = error_code;

}

static struct i2c_device *find_i2c_device(I2C_TypeDef *i2c, uint16_t address)
{
    struct i2c_device *d = NULL;

    if (i2c != NULL) {
        d = &i2c->i2c_devices[(address>>1)&0x7F];
    }

    if ((!d) || (!d->ops)) {
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
    d->error_mode = I2C_NORMAL;
}

HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c)
{
    hi2c->Instance->mode = hi2c->Mode;//HAL_I2C_MODE_MASTER
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_DeInit(I2C_HandleTypeDef *hi2c)
{
    return HAL_OK;
}

HAL_StatusTypeDef i2c_precheck_error_mode(I2C_HandleTypeDef *hi2c, struct i2c_device *dev)
{
    HAL_StatusTypeDef r;

    if (NULL==dev)
    {
        hi2c->ErrorCode = HAL_I2C_ERROR_AF;
        r = HAL_ERROR;
    }
    else
    {
        switch (dev->error_mode)
        {

        case I2C_BUSY:
            r = HAL_BUSY;
            break;
        case I2C_FAIL_PRETX:
            hi2c->ErrorCode = dev->error_code;
            r = HAL_ERROR;
            break;
        default:
            r = HAL_OK;
            break;
        }
    }
    return r;
}

HAL_StatusTypeDef i2c_postcheck_error_mode(HAL_StatusTypeDef status, I2C_HandleTypeDef *hi2c, struct i2c_device *dev)
{
    if (status == HAL_OK) {
        switch (dev->error_mode) {
        case I2C_FAIL_POSTTX:
            hi2c->ErrorCode = dev->error_code;
            status = HAL_ERROR;
            break;
        default:
            break;
        }
    }
    return status;
}

HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size,
                                          uint32_t Timeout)
{
    HAL_StatusTypeDef r;

    struct i2c_device *dev = find_i2c_device(hi2c->Instance, DevAddress);

    r = i2c_precheck_error_mode(hi2c, dev);

    if (r == HAL_OK)
    {
        r = dev->ops->master_transmit(dev->data, pData, Size);
        r = i2c_postcheck_error_mode(r, hi2c, dev);
    }
    return r;

}

HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size,
                                         uint32_t Timeout)
{
    HAL_StatusTypeDef r;

    struct i2c_device *dev = find_i2c_device(hi2c->Instance, DevAddress);
    r = i2c_precheck_error_mode(hi2c, dev);

    if (r == HAL_OK)
    {
        r = dev->ops->master_receive(dev->data, pData, Size);
        r = i2c_postcheck_error_mode(r, hi2c, dev);
    }
    return r;
}

HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress,
                                    uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    HAL_StatusTypeDef r;

    struct i2c_device *dev = find_i2c_device(hi2c->Instance, DevAddress);
    r = i2c_precheck_error_mode(hi2c, dev);
    if (r == HAL_OK)
    {
        r = dev->ops->master_mem_write(dev->data, MemAddress, MemAddSize, pData, Size);
        r = i2c_postcheck_error_mode(r, hi2c, dev);
    }
    return r;
}

HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress,
                                   uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    HAL_StatusTypeDef r;
    struct i2c_device *dev = find_i2c_device(hi2c->Instance, DevAddress);

    r = i2c_precheck_error_mode(hi2c, dev);

    if (r == HAL_OK)
    {
        r = dev->ops->master_mem_read(dev->data, MemAddress, MemAddSize, pData, Size);
        r = i2c_postcheck_error_mode(r, hi2c, dev);
    }
    return r;
}


HAL_StatusTypeDef HAL_I2CEx_ConfigAnalogFilter(I2C_HandleTypeDef *hi2c, uint32_t AnalogFilter)
{
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2CEx_ConfigDigitalFilter(I2C_HandleTypeDef *hi2c, uint32_t DigitalFilter)
{
    return HAL_OK;
}


