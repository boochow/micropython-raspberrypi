#include <stdint.h>
#include <stdio.h>

#include "arm_exceptions.h"
#include "rpi.h"

// System Timers

systimer_t *systimer = (systimer_t *) SYSTIMER;

volatile uint64_t systime(void) {
    uint64_t t;
    uint32_t chi;
    uint32_t clo;

    chi = systimer->CHI;
    clo = systimer->CLO;
    if (chi != systimer->CHI) {
        chi = systimer->CHI;
        clo = systimer->CLO;
    }
    t = chi;
    t = t << 32;
    t += clo;
    return t;
}

// IRQ handler

#define IRQ_SYSTIMERS (IRQ_SYSTIMER(0) | IRQ_SYSTIMER(1) | IRQ_SYSTIMER(2) | IRQ_SYSTIMER(3))

void __attribute__((interrupt("IRQ"))) irq_handler(void) {
    if (IRQ_PEND1 & IRQ_SYSTIMERS) {
        isr_irq_timer();
    }
}

