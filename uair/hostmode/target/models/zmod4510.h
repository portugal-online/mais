#include "stm32wlxx_hal.h"
#include "stm32wlxx_hal_i2c_pvt.h"

// Model for ZMOD4510

struct zmod4510_model;

struct zmod4510_model *zmod4510_model_new();

void zmod4510_powerdown(struct zmod4510_model *);
void zmod4510_powerup(struct zmod4510_model *);

extern struct i2c_device_ops zmod4510_ops;



