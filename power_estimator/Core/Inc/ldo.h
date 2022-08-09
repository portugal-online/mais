#ifndef VSEL_H__
#define VSEL_H__

#include <stdbool.h>

typedef enum {
    VSEL_2V2,
    VSEL_2V5,
    VSEL_3V7,
    VSEL_4V0
} ldo_voltage_t;

void ldo_enable(void);
void ldo_disable(void);
void ldo_set_voltage(ldo_voltage_t);
ldo_voltage_t ldo_get_voltage(void);
bool ldo_enabled(void);


#endif
