#include <stdint.h>
#include <stdbool.h>
//#include <stdio.h>
//#include <string.h>
//#include <unistd.h>
#include "py/mpconfig.h"
#include "bcm283x.h"
#include "uart-qemu.h"

/// common UART support
typedef struct uart_t {
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

#define IS_RX_RDY (!(uart0->FR & (1 << 4)))
#define RX_CH     (uart0->DR)
#define IS_TX_RDY (!(uart0->FR & (1 << 5)))
#define TX_CH(c)  (uart0->DR = (c))

/// QEMU UART register address
volatile struct uart_t *uart0;

void uart0_init() {
    uart0 = (volatile struct uart_t *) 0x101f1000;
    
    uart0->CR = 0;
    // set spped to 115200 bps
    uart0->IBRD = 1;
    uart0->FBRD = 40;
    // parity none 8bits FIFO enable
    uart0->LCRH = 0x70;

    uart0->CR = 0x0301;
};

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
/*
#include "gpio_registers.h"

// Mini UART registers
#define AUX_IRQ     (0x215000 + IO_BASE)
#define AUX_ENABLES (0x215004 + IO_BASE)

// Mini UART data

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

#define IS_RX_RDY_MINI (mini_uart->LSR & 1U)
#define RX_CH_MINI     (mini_uart->IO)
#define IS_TX_RDY_MINI ((mini_uart->LSR & 0x60) == 0x60)
#define TX_CH_MINI(c)  (mini_uart->IO = (c))

struct mini_uart_t volatile *mini_uart = (struct mini_uart_t volatile *) (0x215040 + IO_BASE);

void mini_uart_init() {
    // set GPIO14, GPIO15 to pull down, alternate function 0
    IOREG(GPIO(GPFSEL1)) = (GPF_ALT_5 << (3*4)) | (GPF_ALT_5 << (3*5));

    // UART basic settings
    IOREG(AUX_ENABLES) = 1;
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
*/
static void (*_uart_putc)(char c);
static uint32_t (*_uart_getc)(void);

void uart_init(bool mini_uart) {
    if (mini_uart) {
        /*        mini_uart_init();
        _uart_putc = &mini_uart_putc;
        _uart_getc = &mini_uart_getc;*/
    } else {
        uart0_init();
        _uart_putc = &uart0_putc;
        _uart_getc = &uart0_getc;
    }
}

void uart_putc(char c) {
    _uart_putc(c);
}

uint32_t uart_getc(void) {
    return _uart_getc();
}

void uart_write (const char* str, uint32_t len) {
    for (uint32_t i = 0; i < len; i++ ) {
        uart_putc(*str++);
    }
}

