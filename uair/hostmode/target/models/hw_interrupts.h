#ifndef HW_INTERRUPTS_H__
#define HW_INTERRUPTS_H__


#ifdef __cplusplus
extern "C" {
#endif

void init_interrupts(void);
void deinit_interrupts(void);
void raise_interrupt(int line);

#ifdef __cplusplus
}
#endif

#endif
