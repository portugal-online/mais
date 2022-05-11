#ifndef MODEL_VM3111_H__
#define MODEL_VM3111_H__

#include "stm32wlxx_hal.h"
#include "stm32wlxx_hal_i2c_pvt.h"

// Model for VM3011

#ifdef __cplusplus
extern "C" {
#endif

struct vm3011_model;

struct vm3011_model *vm3011_model_new();

void vm3011_powerdown(struct vm3011_model *);
void vm3011_powerup(struct vm3011_model *);
void vm3011_set_gain(struct vm3011_model *, uint8_t gain);
void vm3011_set_read_callback(struct vm3011_model *, void (*callback)(void *user, struct vm3011_model*), void*user);

extern struct i2c_device_ops vm3011_ops;


#ifdef __cplusplus
}
#endif

#endif
