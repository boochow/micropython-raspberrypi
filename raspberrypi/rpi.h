#ifndef MICROPY_INCLUDED_RPI_RPI_H
#define MICROPY_INCLUDED_RPI_RPI_H

#include "bcm283x.h"
#include "bcm283x_it.h"
#include "bcm283x_gpio.h"
#include "bcm283x_systimer.h"

extern systimer_t *systimer;

volatile uint64_t systime(void);

void rpi_setup_exception_vectors(void);

extern void isr_irq_timer(void);

#endif // MICROPY_INCLUDED_RPI_RPI_H
