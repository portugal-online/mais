#ifndef CHGDISCHG_H__
#define CHGDISCHG_H__

#include <stdbool.h>

void chgdischg_enable_charge(void);
void chgdischg_disable_charge(void);
void chgdischg_enable_discharge(void);
void chgdischg_disable_discharge(void);

bool chgdischg_charge_enabled(void);
bool chgdischg_discharge_enabled(void);

#endif
