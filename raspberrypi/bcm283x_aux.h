#ifndef MICROPY_INCLUDED_RPI_BCM283X_AUX_H
#define MICROPY_INCLUDED_RPI_BCM283X_AUX_H

#include "bcm283x.h"

#define AUX_BASE    (0x215000 + IO_BASE)
#define AUX_REG(X)  ((X) + AUX_BASE)

#define AUX_IRQ     AUX_REG(0x00)
#define AUX_ENABLES AUX_REG(0x04)
#define AUX_MU      AUX_REG(0x40)
#define AUX_SPI1    AUX_REG(0x80)
#define AUX_SPI2    AUX_REG(0xC0)

#define AUX_IRQ_MU_PENDING   (1)
#define AUX_IRQ_SPI1_PENDING (1 << 1)
#define AUX_IRQ_SPI2_PENDING (1 << 2)

#define AUX_ENABLE_MU   (1)
#define AUX_ENABLE_SPI1 (1 << 1)
#define AUX_ENABLE_SPI2 (1 << 2)

typedef struct mini_uart_t {
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

typedef struct aux_spi_t {
    uint32_t CNTL0;
    uint32_t CNTL1;
    uint32_t STAT;
    uint32_t IO;
    uint32_t PEEK;
} aux_spi_t;

#endif // MICROPY_INCLUDED_RPI_BCM283X_AUX_H
