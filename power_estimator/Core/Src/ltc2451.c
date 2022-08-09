#include "stm32l1xx_hal.h"

#define LTC_I2C_ADDR (0x28)

static uint8_t ltc_data[2];
extern I2C_HandleTypeDef hi2c1;
void (*completefun)(int);

void ltc2415_set_callback( void (*complete)(int value) )
{
    completefun = complete;
}

int ltc2415_read()
{
    HAL_StatusTypeDef status;

    status = HAL_I2C_Master_Receive_IT(&hi2c1, LTC_I2C_ADDR,
                                       &ltc_data[0], sizeof(ltc_data));
    return status == HAL_OK? 0:-1;
};


void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *handle)
{
    if (completefun) {
        completefun( ((uint32_t)ltc_data[0] << 8) + ltc_data[1] );
    }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *handle)
{
    if (completefun) {
        completefun( -1 );
    }
}
