#ifndef STM32WLXX_HAL_I2C_PVT_H__
#define STM32WLXX_HAL_I2C_PVT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32wlxx_hal_dma.h"
#include "stm32wlxx_hal_i2c.h"

typedef uint32_t i2c_status_t;

#define HAL_I2C_ERROR_NONE      (0x00000000U)    /*!< No error              */
#define HAL_I2C_ERROR_BERR      (0x00000001U)    /*!< BERR error            */
#define HAL_I2C_ERROR_ARLO      (0x00000002U)    /*!< ARLO error            */
#define HAL_I2C_ERROR_AF        (0x00000004U)    /*!< ACKF error            */
#define HAL_I2C_ERROR_OVR       (0x00000008U)    /*!< OVR error             */
#define HAL_I2C_ERROR_DMA       (0x00000010U)    /*!< DMA transfer error    */
#define HAL_I2C_ERROR_TIMEOUT   (0x00000020U)    /*!< Timeout error         */


struct i2c_device_ops
{
    i2c_status_t (*master_transmit)(void *,const uint8_t *pData, uint16_t Size);
    i2c_status_t (*master_receive)(void *,uint8_t *pData, uint16_t Size);
    i2c_status_t (*master_mem_write)(void *,uint16_t memaddress, uint8_t memaddrsize, const uint8_t *pData, uint16_t Size);
    i2c_status_t (*master_mem_read)(void *,uint16_t memaddress, uint8_t memaddrsize, uint8_t *pData, uint16_t Size);
};

void i2c_register_device(I2C_TypeDef *bus, uint8_t device, const struct i2c_device_ops*ops, void *user);
void i2c_set_error_mode( I2C_TypeDef *bus, uint8_t device, i2c_error_mode_t error_mode, uint32_t error_code);

#ifdef __cplusplus
}
#endif

#endif
