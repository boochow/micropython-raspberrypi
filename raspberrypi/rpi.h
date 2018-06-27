#ifndef MICROPY_INCLUDED_RPI_RPI_H
#define MICROPY_INCLUDED_RPI_RPI_H

#include <stdint.h>
#include "bcm283x_systimer.h"

extern systimer_t *systimer;

volatile uint64_t systime(void);

extern void isr_irq_timer(void);
extern void isr_irq_mini_uart(void);

void rpi_init();

uint32_t rpi_freq_core();
uint32_t rpi_freq_cpu();

#endif // MICROPY_INCLUDED_RPI_RPI_H
