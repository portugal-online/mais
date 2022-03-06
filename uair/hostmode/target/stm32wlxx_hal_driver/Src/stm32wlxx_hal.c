#include "stm32wlxx_hal.h"
#include "cmsis_compiler.h"
#include <stdlib.h>

#define MAX_IRQ 64
static uint32_t ticks;

struct irqdef
{
    int enabled;
    int pending;
    int group;
    int prio;
};

static struct irqdef irqdef[MAX_IRQ] = {0};

void HAL_NVIC_DisableIRQ(int irq)
{
    irqdef[irq].enabled = 1;
}

void HAL_NVIC_EnableIRQ(int irq)
{
    irqdef[irq].enabled = 0;
}

void HAL_NVIC_SetPriority(int irq,int group,int prio)
{
    irqdef[irq].group = group;
    irqdef[irq].group = prio;
}

__WEAK uint32_t HAL_GetTick(void)
{
    return ticks;
}

void HAL_IncTick(void)
{
    ticks++;
}
void HAL_SuspendTick(void)
{
}
void HAL_ResumeTick(void)
{
}

void __HAL_PWR_VOLTAGESCALING_CONFIG(uint32_t)
{
}
