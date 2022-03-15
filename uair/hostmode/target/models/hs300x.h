#include "stm32wlxx_hal.h"
#include "stm32wlxx_hal_i2c_pvt.h"

// Model for HS300x

struct hs300x_model;

struct hs300x_model *hs300x_model_new();
void hs300x_powerdown(struct hs300x_model *);
void hs300x_powerup(struct hs300x_model *);

void hs300x_set_temperature(struct hs300x_model *,float temp_c);
void hs300x_set_humidity(struct hs300x_model *,float hum_percent);
/* Set callback to be called when sampling is performed */
void hs300x_set_sampling_callback(struct hs300x_model *, void (*callback)(void *user, struct hs300x_model*), void*user);

extern struct i2c_device_ops hs300x_ops;



