#include "bcm283x_gpio.h"
#include "bcm283x_aux.h"
#include "mini-uart.h"

#define IS_RX_RDY_MINI (mini_uart->LSR & 1U)
#define RX_CH_MINI     (mini_uart->IO)
#define IS_TX_RDY_MINI ((mini_uart->LSR & 0x60) == 0x60)
#define TX_CH_MINI(c)  (mini_uart->IO = (c))

struct mini_uart_t volatile *mini_uart = (struct mini_uart_t*) AUX_MU;

void mini_uart_init() {
    // set GPIO14, GPIO15 to pull down, alternate function 0
    IOREG(GPFSEL1) = (GPF_ALT_5 << (3*4)) | (GPF_ALT_5 << (3*5));

    // UART basic settings
    IOREG(AUX_ENABLES) |= AUX_ENABLE_MU;
    mini_uart->CNTL = 0;   // disable mini uart

    mini_uart->IER = 0;    // disable receive/transmit interrupts
    mini_uart->IIR = 0xC6; // enable FIFO(0xC0), clear FIFO(0x06)
    mini_uart->MCR = 0;    // set RTS to High

    // data and speed (mini uart is always parity none, 1 start bit 1 stop bit)
    mini_uart->LCR = 3;    // 8 bits
    mini_uart->BAUD = 270; // 1115200 bps

    // enable transmit and receive
    mini_uart->CNTL = 3;
};

void mini_uart_putc(char c) {
    while(!IS_TX_RDY_MINI) {
    }
    TX_CH_MINI(c);
}

uint32_t mini_uart_getc(void) {
    uint32_t c;
  
    while (!IS_RX_RDY_MINI) {
    }
    c = RX_CH_MINI;
    return c & 0xffU;
}

uint32_t mini_uart_rx_state(void) {
    return IS_RX_RDY_MINI;
}
