#include "stm32wlxx_hal.h"
#include "stm32wlxx_hal_i2c_pvt.h"

#ifdef __cplusplus
extern "C" {
#endif


// Model for SHTC3

#define SHTC3_CMD_SLEEP 0xB098
#define SHTC3_CMD_WAKEUP 0x3517
#define SHTC3_CMD_READID 0xEFC8

#define SHTC3_CMD_NORMAL_READTFIRST 0x7866
#define SHTC3_CMD_NORMAL_READHFIRST 0x58E0
#define SHTC3_CMD_LOWPOWER_READTFIRST 0x609C
#define SHTC3_CMD_LOWPOWER_READHFIRST 0x401A
#define SHTC3_CMD_SOFTRESET 0x805D

#define SHTC3_CMD_CAPTURE_SERIAL 0xC595
#define SHTC3_CMD_READ_SERIAL 0xC7F7

#define CRC8_POLYNOMIAL 0x31
#define CRC8_INIT 0xFF

struct shtc3_model;

struct shtc3_model *shtc3_model_new();

void shtc3_powerdown(struct shtc3_model *);
void shtc3_powerup(struct shtc3_model *);
void shtc3_set_temperature(struct shtc3_model *,float temp_c);
void shtc3_set_humidity(struct shtc3_model *,float hum_percent);
void shtc3_set_sampling_callback(struct shtc3_model *, void (*callback)(void *user, struct shtc3_model*), void*user);

void shtc3_set_command_handler( struct shtc3_model *model, i2c_status_t (*handler)(uint16_t command, const uint8_t *data, uint16_t len) );

extern struct i2c_device_ops shtc3_ops;


#ifdef __cplusplus
}
#endif

