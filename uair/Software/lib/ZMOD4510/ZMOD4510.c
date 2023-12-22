#include "ZMOD4510.h"
#include <string.h>
#include "BSP.h"

#include "zmod4xxx_api.h"

#define ZMOD_DEFAULT_I2C_ADDRESS (0x33)
#define ZMOD_DEFAULT_I2C_TIMEOUT (500)

#define MAX_ZMOD_SENSORS 1

#undef DEBUG_ZMOD_I2C

/* Registry */
#ifdef ZMOD_EXTENDED_REGISTRY
static ZMOD4510_t *zmod_registry[MAX_ZMOD_SENSORS];
static uint8_t zmod_registry_idx = 0;

static int8_t ZMOD4510_register(ZMOD4510_t *zmod)
{
    int8_t ret = -1;
    if (zmod_registry_idx<MAX_ZMOD_SENSORS) {
        zmod_registry[zmod_registry_idx] = zmod;
        ret = zmod_registry_idx;
        zmod_registry_idx++;
    }
    return ret;
}

static ZMOD4510_t *ZMOD4510_registry_get(uint8_t index)
{
    if (zmod_registry_idx==0)
        return NULL;

    if (index>(zmod_registry_idx-1))
        return NULL;
    return zmod_registry[index];
}

#else
static ZMOD4510_t *current_zmod;
static int8_t ZMOD4510_register(ZMOD4510_t *zmod)
{
    current_zmod = zmod;
    return 0;
}
static ZMOD4510_t *ZMOD4510_registry_get(uint8_t index)
{
    return current_zmod;
}
#endif

static HAL_StatusTypeDef ZMOD4510_i2c_read(ZMOD4510_t *zmod, uint8_t startreg, uint8_t *data, uint16_t count)
{
    HAL_StatusTypeDef r = HAL_I2C_Mem_Read(zmod->bus,
                            (zmod->address<<1),
                            startreg,
                            I2C_MEMADD_SIZE_8BIT, data,
                            count,
                            zmod->i2c_timeout);
#ifdef DEBUG_ZMOD_I2C
    do {
        char tmp[64+32*3+1];
        char *p = &tmp[0];
        uint16_t i;

        p += sprintf(p, "ZMOD read %d reg 0x%02x: [",
                     (int)r,
                     startreg);
        for (i=0;i<count;i++) {
            if (i!=0) {
                *p++ = ' ';
                *p = '\0';
            }

            p += sprintf(p, "%02x", data[count]);
        }
        *p++ = ']';
        *p = '\0';
        BSP_TRACE("%s",tmp);
    } while (0);
#endif

    return r;
}

static HAL_StatusTypeDef ZMOD4510_i2c_write(ZMOD4510_t *zmod, uint8_t startreg, const uint8_t *data,
                                            uint16_t count)
{
    HAL_StatusTypeDef r = HAL_I2C_Mem_Write(zmod->bus,
                             (zmod->address<<1),
                             startreg,
                             I2C_MEMADD_SIZE_8BIT,
                             (uint8_t*)data, count,
                             zmod->i2c_timeout);

#ifdef DEBUG_ZMOD_I2C
    do {
        char tmp[64+32*3+1];
        char *p = &tmp[0];
        uint16_t i;

        p += sprintf(p, "ZMOD write %d reg 0x%02x: [",
                     (int)r,
                     startreg);
        for (i=0;i<count;i++) {
            if (i!=0) {
                *p++ = ' ';
                *p = '\0';
            }

            p += sprintf(p, "%02x", data[count]);
        }
        *p++ = ']';
        *p = '\0';
        BSP_TRACE(tmp);
    } while (0);
#endif

    return r;
}

static ZMOD4510_op_result_t ZMOD4510_Reset(ZMOD4510_t *zmod)
{
    if (zmod->reset_gpio) {
        BSP_TRACE("Resetting ZMOD");
        HAL_GPIO_set(zmod->reset_gpio,0);
        HAL_Delay(40);
        HAL_GPIO_set(zmod->reset_gpio,1);
        HAL_Delay(400);
        zmod->initialised = false;
        //zmod->sequencer_running = false;
    }
    return ZMOD4510_OP_SUCCESS;
}

static int8_t ZMOD4510_i2c_read_api_wrapper(uint8_t i2c_address, uint8_t reg_addr,
                                            uint8_t *data_buf, uint8_t len)
{
    ZMOD4510_t *self = ZMOD4510_registry_get(i2c_address);

    if (NULL==self) {
        BSP_TRACE("Invalid read to ZMOD4510 device @%d",i2c_address);
        return -1;
    }

    ZMOD4510_op_result_t r = ZMOD4510_i2c_read(self, reg_addr, data_buf, len);

    if (r == ZMOD4510_OP_SUCCESS)
        return 0;
    else
        return -1;
}

static int8_t ZMOD4510_i2c_write_api_wrapper(uint8_t i2c_address, uint8_t reg_addr,
                                             uint8_t *data_buf, uint8_t len)
{
    ZMOD4510_t *self = ZMOD4510_registry_get(i2c_address);
    if (NULL==self) {
        BSP_TRACE("Invalid read to ZMOD4510 device @%d",i2c_address);
        return -1;
    }

    ZMOD4510_op_result_t r = ZMOD4510_i2c_write(self, reg_addr, data_buf, len);

    if (r == ZMOD4510_OP_SUCCESS)
        return 0;
    else
        return -1;
}


static void ZMOD4510_delay_ms(uint32_t ms)
{
    // TBD
    HAL_delay_us(ms * 1000);
}

void ZMOD4510_deinit(ZMOD4510_t *zmod)
{
    zmod->initialised = false;
    //zmod->sequencer_running = false;
}

ZMOD4510_op_result_t ZMOD4510_Init(ZMOD4510_t *zmod, HAL_I2C_bus_t bus, HAL_GPIO_t reset_gpio)
{
    zmod->bus = bus;
    zmod->reset_gpio = reset_gpio;
    zmod->address = ZMOD_DEFAULT_I2C_ADDRESS;
    zmod->i2c_timeout = ZMOD_DEFAULT_I2C_TIMEOUT;
    zmod->initialised = false;
    //zmod->sequencer_running = false;

    if (zmod->reset_gpio) {
        HAL_GPIO_configure_output_od(reset_gpio);
    }

    zmod->dev.pid = ZMOD4510_PID;
#if OAQ_GEN==2
    zmod->dev.init_conf = &zmod_oaq_sensor_type_gen2[Z_INIT];
    zmod->dev.meas_conf = &zmod_oaq_sensor_type_gen2[Z_MEASURE];
#else
    zmod->dev.init_conf = &zmod_oaq_sensor_type[Z_INIT];
    zmod->dev.meas_conf = &zmod_oaq_sensor_type[Z_MEASURE];
#endif
    zmod->dev.prod_data = zmod->prod_data;

    zmod->dev.read  = ZMOD4510_i2c_read_api_wrapper;
    zmod->dev.write = ZMOD4510_i2c_write_api_wrapper;

    zmod->dev.delay_ms = ZMOD4510_delay_ms;

    int8_t pseudo_id = ZMOD4510_register(zmod);

    if (pseudo_id <0)
        return ZMOD4510_OP_INTERNAL_ERROR;

    zmod->dev.i2c_addr = pseudo_id;


    return ZMOD4510_OP_SUCCESS;
}

zmod4xxx_dev_t *ZMOD4510_get_dev(ZMOD4510_t *zmod)
{
    return &zmod->dev;
}

ZMOD4510_op_result_t ZMOD4510_Probe(ZMOD4510_t *zmod)
{
    uint8_t tracking_num[ZMOD4XXX_LEN_TRACKING];


    if (zmod->reset_gpio)
    {
        ZMOD4510_Reset(zmod);
    }

    int8_t api_ret = zmod4xxx_read_sensor_info(&zmod->dev);

    if (api_ret != 0)
    {
        BSP_TRACE("Cannot read sensor info");
        return ZMOD4510_OP_DEVICE_ERROR;
    }

    api_ret = zmod4xxx_read_tracking_number(&zmod->dev, tracking_num);
    if (api_ret != 0)
    {
        BSP_TRACE("Cannot read tracking number");
        return ZMOD4510_OP_DEVICE_ERROR;
    }

    // Dump information.
    {
        char info[128];
        char *ptr;
        int i;


        ptr = info;
        for (i=0;i<ZMOD4XXX_LEN_TRACKING; i++) {
            ptr += tiny_sprintf(ptr, "%02x ", tracking_num[i]);
        }

        BSP_TRACE("Sensor tracking number (%d): %s", ZMOD4XXX_LEN_TRACKING,
                  info);

        ptr = info;
        for (i=0;i< zmod->dev.meas_conf->prod_data_len; i++) {
            ptr += tiny_sprintf(ptr, "%02x ", zmod->dev.prod_data[i]);
        }
        BSP_TRACE("Sensor prod data (%d): %s",
                  zmod->dev.meas_conf->prod_data_len,
                  info);

        ptr = info;
        for (i=0;i< ZMOD4XXX_LEN_CONF; i++) {
            ptr += tiny_sprintf(ptr, "%02x ", zmod->dev.config[i]);
        }
        BSP_TRACE("Sensor config: %s",
                  info);
    }


    api_ret = zmod4xxx_prepare_sensor(&zmod->dev);

    if (api_ret != 0)
    {
        BSP_TRACE("Cannot prepare sensor");
        return ZMOD4510_OP_DEVICE_ERROR;
    }

    BSP_TRACE("Sensor MOX_LR 0x%04x MOX_ER 0x%04x", zmod->dev.mox_lr, zmod->dev.mox_er);

    zmod->initialised = true;

    return ZMOD4510_OP_SUCCESS;
}

ZMOD4510_op_result_t ZMOD4510_start_measurement(ZMOD4510_t *zmod)
{
    if (!zmod->initialised)
    {
        return ZMOD4510_OP_NOT_INITIALISED;
    }

    /*if (zmod->sequencer_running)
    {
        return ZMOD4510_OP_BUSY;
    } */

    zmod4xxx_err api_ret = zmod4xxx_check_meas_configuration(&zmod->dev);
    switch (api_ret)
    {
    case ERROR_CONFIG_MISSING:
        // Need reconfiguration
        zmod->initialised = false;
        BSP_TRACE("MEAS configuration lost!");

        ZMOD4510_op_result_t r = ZMOD4510_Probe(zmod);

        if (r==ZMOD4510_OP_SUCCESS)
            return ZMOD4510_OP_DEVICE_ERROR;

        return r;

    case ZMOD4XXX_OK:
        break;
    default:
        // I2C error
        BSP_TRACE("Error reading MEAS configuration");
        return ZMOD4510_OP_DEVICE_ERROR;
    }

    api_ret = zmod4xxx_start_measurement(&zmod->dev);

    if (api_ret == ZMOD4XXX_OK)
    {
        //zmod->sequencer_running = true;
        return ZMOD4510_OP_SUCCESS;
    }

    BSP_TRACE("error: api_ret %d", api_ret);

    return ZMOD4510_OP_DEVICE_ERROR;
}

ZMOD4510_op_result_t ZMOD4510_read_adc(ZMOD4510_t *zmod)
{
    if (!zmod->initialised)
    {
        return ZMOD4510_OP_NOT_INITIALISED;
    }

    int8_t api_ret = zmod4xxx_read_adc_result(&zmod->dev, zmod->adc_result);

    if (api_ret == 0)
    {
        return ZMOD4510_OP_SUCCESS;
    }

    return ZMOD4510_OP_DEVICE_ERROR;
}

uint8_t *ZMOD4510_get_adc(ZMOD4510_t *zmod)
{
    return zmod->adc_result;
}

ZMOD4510_op_result_t ZMOD4510_is_sequencer_completed(ZMOD4510_t *zmod)
{
    uint8_t zmod4xxx_status;

    if (!zmod->initialised)
    {
        return ZMOD4510_OP_NOT_INITIALISED;
    }

    int8_t api_ret = zmod4xxx_read_status(&zmod->dev, &zmod4xxx_status);


    if (api_ret != 0)
    {
        BSP_TRACE("Cannot read status");
        return ZMOD4510_OP_DEVICE_ERROR;
    }

    // Log status
    //BSP_TRACE("Status: %02x", zmod4xxx_status);

    if ((zmod4xxx_status & STATUS_SEQUENCER_RUNNING_MASK))
    {
        return ZMOD4510_OP_BUSY;
    }

    //zmod->sequencer_running = false;

    return ZMOD4510_OP_SUCCESS;
}


