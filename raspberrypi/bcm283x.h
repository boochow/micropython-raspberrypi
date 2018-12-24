#ifndef MICROPY_INCLUDED_RPI_BCM283X_H
#define MICROPY_INCLUDED_RPI_BCM283X_H

#include <stdint.h>

#ifdef RPI1
#define IO_BASE   0x20000000U
#else
#define IO_BASE   0x3F000000U
#endif

#define IOREG(X)  (*(volatile uint32_t *) (X))

#endif // MICROPY_INCLUDED_RPI_BCM283X_H
