#include <unistd.h>
#include "py/mpconfig.h"
#include "uart-qemu.h"

typedef struct uart_t {
  uint32_t DR;
  uint32_t RSR_ECR;
  uint8_t reserved1[0x10];
  const uint32_t FR;
  uint8_t reserved2[0x4];
  uint32_t LPR;
  uint32_t IBRD;
  uint32_t FBRD;
  uint32_t LCR_H;
  uint32_t CR;
  uint32_t IFLS;
  uint32_t IMSC;
  const uint32_t RIS;
  const uint32_t MIS;
  uint32_t ICR;
  uint32_t DMACR;
} uart_t;

uint32_t RXFE = 0x10;
uint32_t TXFF = 0x20;

struct uart_t volatile *uart = (struct uart_t volatile *)0x101f1000;

void uart_init() {
};

void print_buf(char *buf) {
  for(;*buf;++buf) {
    uart->DR = *buf;
  }
}

void print_ch(char ch) {
  uart->DR = ch;
}

void uart_write ( const char* str, uint32_t len ) {
    for ( int i=0; i<len; i++ ) {
        print_ch( str[i] );
    }
}

// Receive single character
int mp_hal_stdin_rx_chr(void) {
    unsigned int c = 0;
    while (uart->FR & (1 << 4));
    c = uart->DR;
    return c & 0xff;
}

// Send string of given length
void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {
  uart_write(str,len);
}
