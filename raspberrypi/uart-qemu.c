#include <unistd.h>
#include "py/mpconfig.h"
#include "uart-qemu.h"

#if 0

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

#define IS_RX_RDY (!(uart->FR & (1 << 4)))
#define RX_CH     (uart->DR)
#define IS_TX_RDY (!(uart->FR & (1 << 5)))
#define TX_CH(c)  (uart->DR = (c))

volatile struct uart_t *uart = (volatile struct uart_t *)0x101f1000;

void uart_init() {
    uart->CR = 0;
    // set spped to 115200 bps
    uart->IBRD = 1;
    uart->FBRD = 40;
    // parity none 8bits FIFO enable
    uart->LCRH = 0x70;

    uart->CR = 0x0301;
};

#else

#include "gpio_registers.h"

// Mini UART registers
#define AUX_IRQ     0x20215000
#define AUX_ENABLES 0x20215004

// Mini UART data

typedef struct uart_t {
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
} uart_t;

#define IS_RX_RDY (uart->LSR & 1U)
#define RX_CH     (uart->IO)
#define IS_TX_RDY ((uart->LSR & 0x60) == 0x60)
#define TX_CH(c)  (uart->IO = (c))

struct uart_t volatile *uart = (struct uart_t volatile *) 0x20215040;

void uart_init() {
    // set GPIO14, GPIO15 to pull down, alternate function 0
    IOREG(GPFSEL1) = (GPF_ALT_5 << (3*4)) | (GPF_ALT_5 << (3*5));

    // UART basic settings
    IOREG(AUX_ENABLES) = 1;
    uart->CNTL = 0;   // disable mini uart

    uart->IER = 0;    // disable receive/transmit interrupts
    uart->IIR = 0xC6; // enable FIFO(0xC0), clear FIFO(0x06)
    uart->MCR = 0;    // set RTS to High

    // data and speed (mini uart is always parity none, 1 start bit 1 stop bit)
    uart->LCR = 3;    // 8 bits
    uart->BAUD = 270; // 1115200 bps

    // enable transmit and receive
    uart->CNTL = 3;

};

#endif

void uart_putc(char c) {
    while(!IS_TX_RDY) {
    }
    TX_CH(c);
}

void uart_write (const char* str, uint32_t len) {
    for (uint32_t i = 0; i < len; i++ ) {
        uart_putc(*str++);
    }
}

uint32_t uart_getc(void) {
    uint32_t c;
  
    while (!IS_RX_RDY) {
    }
    c = RX_CH;
    return c & 0xffU;
}

// Receive single character
int mp_hal_stdin_rx_chr(void) {
    return uart_getc();
}

// Send string of given length
void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {
    uart_write(str,len);
}
