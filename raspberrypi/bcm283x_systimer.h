#ifndef MICROPY_INCLUDED_RPI_BCM283X_SYSTIMER_H
#define MICROPY_INCLUDED_RPI_BCM283X_SYSTIMER_H

#define SYSTIMER (IO_BASE + 0x3000)

typedef volatile struct systimer_t {
    uint32_t CS;
    uint32_t CLO;
    uint32_t CHI;
    uint32_t C[4];
} systimer_t;

#endif // MICROPY_INCLUDED_RPI_BCM283X_SYSTIMER_H
