#ifndef APP_H__
#define APP_H__

#include <stdbool.h>

void app_init(void);
void app_run(void);

void app_enable_reporting(void);
void app_disable_reporting(void);
bool app_reporting_enabled(void);

#endif
