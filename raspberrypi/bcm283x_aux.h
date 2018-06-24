#ifndef MICROPY_INCLUDED_RPI_BCM283X_AUX_H
#define MICROPY_INCLUDED_RPI_BCM283X_AUX_H

#include "bcm283x.h"

#define AUX_BASE    (0x215000 + IO_BASE)
#define AUX(X)      ((X) + AUX_BASE)

#define AUX_IRQ        IOREG(AUX(0x00))
#define AUX_ENABLES    IOREG(AUX(0x04))

#define AUX_FLAG_MU    (1)
#define AUX_FLAG_SPI1  (1 << 1)
#define AUX_FLAG_SPI2  (1 << 2)

// top address of mini_uart_t
#define AUX_MU  AUX(0x40)

typedef struct _mini_uart_t {
    uint32_t IO;
    uint32_t IER;
    uint32_t IIR;
    uint32_t LCR;
    uint32_t MCR;
    uint32_t LSR;
    uint32_t MSR;
    uint32_t SCRATCH;
    uint32_t CNTL;
    uint32_t STAT;
    uint32_t BAUD;
} mini_uart_t;

// The description of IER in the manual is incorrect.
// Following definitions are from 16550's IER
#define MU_IER_RX_AVAIL  (1)
#define MU_IER_TX_EMPTY  (1 << 1)
#define MU_IER_LSR_CHGD  (1 << 2)

#define MU_IIR_NOTHING   (0)
#define MU_IIR_TX_EMPTY  (1 << 1)
#define MU_IIR_RX_AVAIL  (1 << 2)


// aux spi not yet tested

// top addresses of aux_spi_t
#define AUX_SPI1    AUX(0x80)
#define AUX_SPI2    AUX(0xC0)

typedef struct _aux_spi_t {
    uint32_t CNTL0;
    uint32_t CNTL1;
    uint32_t STAT;
    uint32_t IO;
    uint32_t PEEK;
} aux_spi_t;

#endif // MICROPY_INCLUDED_RPI_BCM283X_AUX_H
