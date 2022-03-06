#include "stm32wlxx_hal.h"
#include "stm32wlxx_hal_i2c_pvt.h"

// Model for SHTC3

struct shtc3_model;

struct shtc3_model *shtc3_model_new();

void shtc3_powerdown(struct shtc3_model *);
void shtc3_powerup(struct shtc3_model *);
void shtc3_set_temperature(struct shtc3_model *,float temp_c);
void shtc3_set_humidity(struct shtc3_model *,float hum_percent);

extern struct i2c_device_ops shtc3_ops;



