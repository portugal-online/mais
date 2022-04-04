#include "HS300X.h"
#include "GNSE_tracer.h"
#include "UAIR_rtc.h" // Naming TBD


#define HS300X_I2C_ADDRESS (0x44)
#define HS300X_I2C_TIMEOUT (200)

#define HS300X_REG_HSR_READ (0x06)
#define HS300X_REG_HSR_WRITE (0x46)
#define HS300X_REG_TSR_READ (0x11)
#define HS300X_REG_TSR_WRITE (0x51)
#define HS300X_REG_SENSOR_ID0 (0x1E)
#define HS300X_REG_SENSOR_ID1 (0x1F)

#define HS300X_ACCURACY_MASK (0x0C00)

#define HS300X_TIME_WAKEUP_US     100ULL    /* 0.1ms */
#define HS300X_TIME_RES_8BIT_US   550ULL    /* 0.55ms */
#define HS300X_TIME_RES_10BIT_US  1310ULL   /* 1.31ms */
#define HS300X_TIME_RES_12BIT_US  4500ULL   /* 4.50ms */
#define HS300X_TIME_RES_14BIT_US  16900ULL  /* 16.90ms */


static HAL_StatusTypeDef HS300X_write_register(HS300X_t *hs, uint8_t reg, uint16_t value, bool wait);

typedef enum {
    HS300X_STATUS_VALID,
    HS300X_STATUS_STALE,
    HS300X_STATUS_INVALID
} HS300X_status_t;

static HAL_StatusTypeDef HS300X_read_register_contents(HS300X_t *hs, uint16_t *value);

static HAL_StatusTypeDef HS300X_enter_program_mode(HS300X_t *hs)
{
    int r = HS300X_write_register(hs, 0xA0, 0x0000, false);
    HAL_Delay(1);
    return r;
}

static HAL_StatusTypeDef HS300X_leave_program_mode(HS300X_t *hs)
{
    int r = HS300X_write_register(hs, 0x80, 0x0000, false);
    HAL_Delay(1);
    return r;
}

HAL_StatusTypeDef HS300X_read_register(HS300X_t *hs, uint8_t reg, uint16_t *value)
{
    HS300X_write_register(hs, reg, 0x0000, false);
    HAL_Delay(1); // 120us
    return HS300X_read_register_contents(hs, value);
}

static HAL_StatusTypeDef HS300X_read_register_contents(HS300X_t *hs, uint16_t *value)
{
    uint8_t buf[3];

    HAL_StatusTypeDef r = HAL_I2C_Master_Receive(hs->bus,
                                                 (uint16_t)(hs->address << 1),
                                                 buf, sizeof(buf), hs->i2c_timeout);
    if (r!=HAL_OK)
        return r;

    if (buf[0] != 0x81) {
        // No success.
        APP_PRINTF("HS300x: read reg returned status 0x%02x (data %02x %02x)\r\n",
                   buf[0], buf[1], buf[2]);
        return -1;
    }
    *value = (((uint16_t)buf[1])<<8)  | (uint16_t)buf[2];

    return r;
}

static HAL_StatusTypeDef HS300X_write_register(HS300X_t *hs, uint8_t reg, uint16_t value, bool wait)
{
    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = value>>8;
    buf[2] = value;

    HAL_StatusTypeDef r = HAL_I2C_Master_Transmit(hs->bus,
                                                  (uint16_t)(hs->address << 1),
                                                  buf, sizeof(buf), hs->i2c_timeout);
    if (r == HAL_OK)
    {
        if (wait) {
            HAL_Delay(14); // 14ms as per datasheet
        }
    }
    return r;
}

HAL_StatusTypeDef HS300X_probe(HS300X_t *hs,
                               HS300X_accuracy_t hum_accuracy,
                               HS300X_accuracy_t temp_accuracy)
{
    uint16_t devid[2];
    uint16_t hum_setting;
    uint16_t hum_value;
    uint16_t temp_setting;
    uint16_t temp_value;
    HAL_StatusTypeDef err, leaveerr;

    do {
        err = HS300X_enter_program_mode(hs);
        if (err!=HAL_OK)
            break;

        err = HS300X_read_register(hs, HS300X_REG_SENSOR_ID0, &devid[0]);

        if (err!=HAL_OK)
            break;

        err = HS300X_read_register(hs, HS300X_REG_SENSOR_ID1, &devid[1]);

        if (err!=HAL_OK)
            break;

        err = HS300X_read_register(hs, HS300X_REG_HSR_READ, &hum_setting);
        if (err!=HAL_OK)
            break;

        APP_PRINTF("HS300X hum setting 0x%04x\r\n", hum_setting);

        hum_value =  (((uint16_t)(hum_accuracy & HS300X_ACCURACY_14BIT)) << 10) & HS300X_ACCURACY_MASK;

        if (hum_accuracy!=HS300X_ACCURACY_NONE && ((hum_setting&HS300X_ACCURACY_MASK)!=hum_value)) {
            // Bits 11:10
            hum_setting &= ~HS300X_ACCURACY_MASK;
            hum_setting |= ((uint16_t)hum_accuracy << 10) & HS300X_ACCURACY_MASK;

            err = HS300X_write_register(hs, HS300X_REG_HSR_WRITE, hum_setting, true);
            if (err!=HAL_OK)
                break;
        }

        err = HS300X_read_register(hs, HS300X_REG_TSR_READ, &temp_setting);

        if (err!=HAL_OK)
            break;

        temp_value =  (((uint16_t)(temp_accuracy & HS300X_ACCURACY_14BIT)) << 10) & HS300X_ACCURACY_MASK;

        APP_PRINTF("HS300X temp setting 0x%04x\r\n", temp_setting);

        if (temp_accuracy!=HS300X_ACCURACY_NONE && ((temp_setting&HS300X_ACCURACY_MASK)!=temp_value)) {
            // Bits 11:10
            temp_setting &= ~HS300X_ACCURACY_MASK;
            temp_setting |= hum_value;
            err = HS300X_write_register(hs, HS300X_REG_TSR_WRITE, temp_setting, true);
            if (err!=HAL_OK)
                break;
        }

#ifndef HS300X_NO_CHECK_TIMING
        hs->temp_acc = temp_accuracy;
        hs->hum_acc = hum_accuracy;
#endif
        hs->serial = ((devid[0])<<16) | devid[1];
    } while (0);

    // We need to leave program mode even with errors above. So we use a new return value.
    leaveerr = HS300X_leave_program_mode(hs);

    if (err==HAL_OK) {
        err = leaveerr; // This is to ensure we report eventual leave program mode error
    }

    return err;
}

HAL_StatusTypeDef HS300X_start_measurement(HS300X_t *hs)
{
    HAL_StatusTypeDef r = HAL_I2C_Master_Transmit(hs->bus,
                                                  (uint16_t)(hs->address << 1),
                                                  NULL, 0, hs->i2c_timeout);

#ifndef HS300X_NO_CHECK_TIMING
    uint16_t msec;
    uint32_t sec = UAIR_RTC_GetTime(&msec);
    hs->meas_start = ((uint64_t)sec * 1000ULL) + (uint64_t)msec;
#endif

    return r;
}

static inline uint64_t HS300X_time_for_measurement(const HS300X_accuracy_t acc)
{
    uint64_t time;

    switch (acc) {
    case HS300X_ACCURACY_8BIT:
        time = HS300X_TIME_RES_8BIT_US;
        break;
    case HS300X_ACCURACY_10BIT:
        time = HS300X_TIME_RES_10BIT_US;
        break;
    case HS300X_ACCURACY_12BIT:
        time = HS300X_TIME_RES_12BIT_US;
        break;
    case HS300X_ACCURACY_14BIT: /* Fall-through */
    default:
        time = HS300X_TIME_RES_14BIT_US;
        break;
    }
    return time;
}

HAL_StatusTypeDef HS300X_read_measurement(HS300X_t *hs, int32_t *temp_millicentigrade, int32_t *hum_millipercent, int *stale)
{
    uint8_t buf[4];
    uint32_t sens_temp;
    uint32_t sens_hum;

#ifndef HS300X_NO_CHECK_TIMING
    uint16_t msec;
    uint32_t sec = UAIR_RTC_GetTime(&msec);
    uint64_t now = ((uint64_t)sec * 1000ULL) + (uint64_t)msec;
    uint64_t delta = (now - hs->meas_start)*1000; // in 1us increments

    uint64_t required_time = HS300X_TIME_WAKEUP_US;
    required_time += HS300X_time_for_measurement(hs->temp_acc);
    required_time += HS300X_time_for_measurement(hs->hum_acc);

    if (delta<=required_time) {
        BSP_TRACE("HS300X: short read interval detected. Required time %llu, delta time %llu", required_time, delta);
        BSP_TRACE("RTC now %llu prev. %llu", now, hs->meas_start);
        return HAL_ERROR;
    }

#endif

    HAL_StatusTypeDef r = HAL_I2C_Master_Receive(hs->bus,
                                                 (uint16_t)(hs->address << 1),
                                                 buf, sizeof(buf), hs->i2c_timeout);
    if (r == HAL_OK) {
        sens_hum = (((uint32_t)buf[0])<<8 ) | (uint32_t)buf[1];
        sens_temp =(((uint32_t)buf[2])<<8 ) | (uint32_t)buf[3];
        // Unmask
        if ((sens_hum & 0xC000) != 0x0000) {
            *stale = 1;
        } else {
            *stale = 0;
        }
        sens_hum &= ~(0xC000); // Mask upper 2 bits
        sens_temp &= ~(0x0003);  // Mask lower 2 bits

        BSP_TRACE("Raw sensor data: hum=0x%04x temp=0x%04x",
                  sens_hum,
                  sens_temp);

        *temp_millicentigrade = (((sens_temp>>2)*165000)/16383) - 40000;
        *hum_millipercent =  ((sens_hum)*100000)/16383;
        BSP_TRACE("Calculated %d %d", *temp_millicentigrade, *hum_millipercent);

    } else {
        BSP_TRACE("Cannot receive data from sensor");
    }

    return r;
}

int HS300X_init(HS300X_t *hs, HAL_I2C_bus_t bus)
{
    hs->bus = bus;
    hs->address = HS300X_I2C_ADDRESS;
    hs->i2c_timeout = HS300X_I2C_TIMEOUT;

    return 0;
}

uint32_t HS300X_get_probed_serial(HS300X_t *hs)
{
    return hs->serial;
}

unsigned HS300X_time_for_measurement_us(const HS300X_accuracy_t temp_acc, const HS300X_accuracy_t hum_acc)
{
    uint64_t required_time = HS300X_TIME_WAKEUP_US;
    required_time += HS300X_time_for_measurement(temp_acc);
    required_time += HS300X_time_for_measurement(hum_acc);
    return (unsigned)required_time;
}

