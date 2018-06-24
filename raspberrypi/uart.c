#include <stdint.h>
#include <stdbool.h>
#include "py/mpconfig.h"
#include "bcm283x_uart.h"
#include "uart.h"

#define IS_RX_RDY (!(uart0->FR & (1 << 4)))
#define RX_CH     (uart0->DR)
#define IS_TX_RDY (!(uart0->FR & (1 << 5)))
#define TX_CH(c)  (uart0->DR = (c))

volatile uart_t *uart0;

static void uart0_init_with_regs_addr(const uint32_t addr) {
    uart0 = (volatile uart_t *) addr;
    
    uart0->CR = 0;
    // set spped to 115200 bps
    uart0->IBRD = 1;
    uart0->FBRD = 40;
    // parity none 8bits FIFO enable
    uart0->LCRH = 0x70;

    uart0->CR = 0x0301;
};

void uart0_qemu_init() {
    uart0_init_with_regs_addr(0x101F1000);
}

void uart0_putc(char c) {
    while(!IS_TX_RDY) {
    }
    TX_CH(c);
}

uint32_t uart0_getc(void) {
    uint32_t c;
  
    while (!IS_RX_RDY) {
    }
    c = RX_CH;
    return c & 0xffU;
}

uint32_t uart0_rx_state(void) {
    return IS_RX_RDY;
}


