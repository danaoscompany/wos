#ifndef IRQ_H
#define IRQ_H

#include "isr.h"

void init_irq();
void irq_set_handler(int irq, void (*handler)(struct regs_t* r));
void irq_remove_handler(int irq);

#endif
