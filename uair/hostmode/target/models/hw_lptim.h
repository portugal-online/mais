#ifndef HW_LPTIM_H__
#define HW_LPTIM_H__

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

void lptim_engine_init(uint32_t divider);
void lptim_engine_deinit(void);
void lptim_engine_enable(void);
void lptim_engine_start_it(uint32_t Period);
void lptim_engine_stop_it(void);

#ifdef __cplusplus
}
#endif

#endif
