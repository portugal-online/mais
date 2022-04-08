#ifndef CMSIS_COMPILER_H__
#define CMSIS_COMPILER_H__

// Host-mode. (C) Alvaro Lopes

#include <stddef.h>
#include "hlog.h"

extern void __disable_irq_impl();
extern void __enable_irq_impl();
extern void __set_PRIMASK_impl();

#define __disable_irq()  \
do {                               \
    INTERRUPT_LOG("Disable interrupts");    \
    __disable_irq_impl();          \
} while (0)

#define __enable_irq()  \
do {                               \
    INTERRUPT_LOG("Enable interrupts");    \
    __enable_irq_impl();           \
} while (0)


int __get_PRIMASK();
#define __set_PRIMASK(x) \
do {                               \
    INTERRUPT_LOG("Restore PRIMASK %08x",x );    \
    __set_PRIMASK_impl(x); \
} while (0)

#define __WEAK __attribute((weak))

void __WFI();
void __NOP();

#endif
