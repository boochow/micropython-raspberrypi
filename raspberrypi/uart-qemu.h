#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init();

void uart_write ( const char* str, uint32_t len );
uint32_t uart_getc(void);

int mp_hal_stdin_rx_chr(void);
void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len);

#endif
