#ifndef MICROPY_INCLUDED_RPI_MINI_UART_H
#define MICROPY_INCLUDED_RPI_MINI_UART_H

#include <stdint.h>

void mini_uart_init();
void mini_uart_putc(char c);
uint32_t mini_uart_getc(void);
uint32_t mini_uart_rx_state(void);

void isr_irq_mini_uart(void);

#endif // MICROPY_INCLUDED_RPI_MINI_UART_H
