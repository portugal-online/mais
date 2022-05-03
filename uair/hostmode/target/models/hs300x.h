#ifndef MODEL_HS300X_H__
#define MODEL_HS300X_H__

#include "stm32wlxx_hal.h"
#include "stm32wlxx_hal_i2c_pvt.h"

#ifdef __cplusplus
extern "C" {
#endif

// Model for HS300x

struct hs300x_model;
typedef i2c_status_t (*hs300x_i2c_master_transmit_hook_t)(struct hs300x_model *, void *userdata, const uint8_t *pData, uint16_t Size);
typedef i2c_status_t (*hs300x_i2c_master_receive_hook_t)(struct hs300x_model *, void *userdata, uint8_t *pData, uint16_t Size);


struct hs300x_model *hs300x_model_new();
void hs300x_powerdown(struct hs300x_model *);
void hs300x_powerup(struct hs300x_model *);

void hs300x_set_temperature(struct hs300x_model *,float temp_c);
void hs300x_set_humidity(struct hs300x_model *,float hum_percent);
/* Set callback to be called when sampling is performed */
void hs300x_set_sampling_callback(struct hs300x_model *, void (*callback)(void *user, struct hs300x_model*), void*user);

void hs300x_set_receive_hook(struct hs300x_model *m, hs300x_i2c_master_receive_hook_t hook, void*user);
void hs300x_set_transmit_hook(struct hs300x_model *m, hs300x_i2c_master_transmit_hook_t hook, void*user);


extern struct i2c_device_ops hs300x_ops;

#ifdef __cplusplus
}
#endif

#endif
