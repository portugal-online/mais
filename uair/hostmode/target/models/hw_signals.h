#ifndef HW_SIGNALS_H__
#define HW_SIGNALS_H__

#ifdef __cplusplus
extern "C" {
#endif

void hw_setup_signals(void (*handler)(int));
void hw_activate_signals(void);

#ifdef __cplusplus
}
#endif

#endif