#include "stm32wlxx_hal.h"
#include "stm32wlxx_hal_i2c_pvt.h"

// Model for VM3011

struct vm3011_model;

struct vm3011_model *vm3011_model_new();

void vm3011_powerdown(struct vm3011_model *);
void vm3011_powerup(struct vm3011_model *);
void vm3011_set_gain(struct vm3011_model *, uint8_t gain);

extern struct i2c_device_ops vm3011_ops;



