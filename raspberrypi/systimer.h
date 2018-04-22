#ifndef MICROPY_INCLUDED_RPI_SYSTIMER_H
#define MICROPY_INCLUDED_RPI_SYSTIMER_H

typedef volatile struct systimer_t {
    uint32_t CS;
    uint32_t CLO;
    uint32_t CHI;
    uint32_t C[4];
} systimer_t;

extern systimer_t *systimer;

volatile uint64_t systime(void);

#endif // MICROPY_INCLUDED_RPI_SYSTIMER_H
