#ifndef UART_H
#define UART_H

//#include "types.h"
#include <stdint.h>

void uart_init();
void print_buf(char *buf);
void print_ch(char ch);
uint32_t print_hex(uint32_t p);
void volatile* print_ptr(void *p);
void uart_write ( const char* str, uint32_t len );
void uart_echo();

extern struct uart_t volatile *uart;

#endif
