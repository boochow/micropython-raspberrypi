#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init();

void uart_putc(char c);
void uart_write ( const char* str, uint32_t len );
uint32_t uart_getc(void);

#endif
