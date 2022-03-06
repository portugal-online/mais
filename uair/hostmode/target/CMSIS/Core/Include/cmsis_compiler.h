#ifndef CMSIS_COMPILER_H__
#define CMSIS_COMPILER_H__

// Host-mode. (C) Alvaro Lopes

int __get_PRIMASK();
void __set_PRIMASK(int);
void __disable_irq();
void __enable_irq();

#define __WEAK __attribute((weak))

void __WFI();
void __NOP();

typedef int ptrdiff_t;

#endif
