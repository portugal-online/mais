#include "ZMOD4510.h"
#include <string.h>

#define ZMOD_DEFAULT_I2C_ADDRESS (0x33)
#define ZMOD_DEFAULT_I2C_TIMEOUT (500)

struct ZMOD4510 {
    HAL_I2C_bus_t bus;
    uint8_t address;
    unsigned i2c_timeout;
    HAL_GPIO_t reset_gpio;
};

typedef struct ZMOD4510 ZMOD4510_t;

static HAL_StatusTypeDef ZMOD4510_i2c_read(ZMOD4510_t *zmod, uint8_t startreg, uint8_t *data, uint16_t count)
{
    return HAL_I2C_Mem_Read(zmod->bus,
                            (zmod->address<<1),
                            startreg,
                            I2C_MEMADD_SIZE_8BIT, data,
                            count,
                            zmod->i2c_timeout);

}

static HAL_StatusTypeDef ZMOD4510_i2c_write(ZMOD4510_t *zmod, uint8_t startreg, const uint8_t *data,
                                            uint16_t count)
{
    return HAL_I2C_Mem_Write(zmod->bus,
                             (zmod->address<<1),
                             startreg,
                             I2C_MEMADD_SIZE_8BIT,
                             (uint8_t*)data, count,
                             zmod->i2c_timeout);

}

static ZMOD4510_op_result_t ZMOD4510_Reset(ZMOD4510_t *zmod)
{
    if (zmod->reset_gpio) {
        HAL_GPIO_set(zmod->reset_gpio,0);
        HAL_Delay(20);
        HAL_GPIO_set(zmod->reset_gpio,1);
        HAL_Delay(20);
    }
    return ZMOD4510_OP_SUCCESS;
}


ZMOD4510_op_result_t ZMOD4510_Init(ZMOD4510_t *zmod, HAL_I2C_bus_t bus, HAL_GPIO_t reset_gpio)
{
    zmod->bus = bus;
    zmod->reset_gpio = reset_gpio;
    zmod->address = ZMOD_DEFAULT_I2C_ADDRESS;
    zmod->i2c_timeout = ZMOD_DEFAULT_I2C_TIMEOUT;

    if (zmod->reset_gpio) {
        HAL_GPIO_configure_output_od(reset_gpio);
    }

    return ZMOD4510_OP_SUCCESS;
}

ZMOD4510_op_result_t ZMOD4510_Probe(ZMOD4510_t *zmod)
{
    uint8_t wdata[4];
    uint8_t rdata[4];

    if (zmod->reset_gpio)
    {
        ZMOD4510_Reset(zmod);
    }

    // As per "datasheet", we can read/write registers in the range 0x88-
    /*
     The I2C slave device interface supports various bus speeds: Standard Mode (≤ 100kHz) and Fast Mode (≤ 400kHz).
     By default, the 7-bit slave address for the serial I2C data interface is set to 33 HEX . The implemented data transmission protocol is similar to the
     one used for conventional EEPROM devices. The register to read/write is selected by a register address pointer. This address pointer must be
     set during an I2C write operation. For read access a repeated START condition but no STOP condition should be sent. After transmission of a
     register, the address pointer is automatically incremented. A STOP condition ends the whole transmission. An increment from the address
     FF HEX rolls over to 00 HEX .
     To validate the general MCU-specific I2C read/write driver without the use of Renesas firmware, it is possible to write random values to registers
     0x88 to 0x8B and read them afterwards. After this register testing, erase the testing code and reset the device by disconnecting the power
     support; otherwise the device may not operate properly.
     */
    wdata[0] = 0xDE;
    wdata[1] = 0xAD;
    wdata[2] = 0xBE;
    wdata[3] = 0xEF;
    if (ZMOD4510_i2c_write(zmod, 0x88, wdata, sizeof(wdata))!=HAL_OK) {
        return ZMOD4510_OP_FAIL_NOACK;
    }
    memset(rdata,0,sizeof(rdata));
    if (ZMOD4510_i2c_read(zmod, 0x88, rdata, sizeof(rdata))!=HAL_OK) {
        return ZMOD4510_OP_FAIL_NOACK;
    }
    int i;
    for (i=0;i<sizeof(rdata);i++) {
        if (rdata[i]!=wdata[i]) {
            return ZMOD4510_OP_DEVICE_ERROR;
        }
    }
    return ZMOD4510_OP_SUCCESS;
}

