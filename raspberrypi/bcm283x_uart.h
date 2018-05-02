#ifndef MICROPY_INCLUDED_RPI_BCM283X_UART_H
#define MICROPY_INCLUDED_RPI_BCM283X_UART_H

#include "bcm283x.h"

#define UART_BASE (0x201000 + IO_BASE)

typedef struct _uart_t {
    uint32_t DR;
    uint32_t RSRECR;
    uint8_t reserved1[0x10];
    const uint32_t FR;
    uint8_t reserved2[0x4];
    uint32_t ILPR;
    uint32_t IBRD;
    uint32_t FBRD;
    uint32_t LCRH;
    uint32_t CR;
    uint32_t IFLS;
    uint32_t IMSC;
    const uint32_t RIS;
    const uint32_t MIS;
    uint32_t ICR;
    uint32_t DMACR;
} uart_t;

#endif // MICROPY_INCLUDED_RPI_BCM283X_UART_H
