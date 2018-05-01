#ifndef MICROPY_INCLUDED_RPI_UART_QEMU_H
#define MICROPY_INCLUDED_RPI_UART_QEMU_H

#include <stdint.h>

void uart0_qemu_init();
void uart0_putc(char c);
uint32_t uart0_getc(void);
uint32_t uart0_rx_state(void);

#endif // MICROPY_INCLUDED_RPI_UART_QEMU_H
