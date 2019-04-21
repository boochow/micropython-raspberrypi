#include "bcm283x_gpio.h"
#include "bcm283x_aux.h"
#include "bcm283x_it.h"
#include "rpi.h"
#include "mini-uart.h"
#include "lib/utils/interrupt_char.h"

// Set this to 0 for avoiding mini-UART interrupts.
#define USE_IRQ 1

mini_uart_t volatile *mini_uart = (mini_uart_t*) AUX_MU;

#define RX_CH        (mini_uart->IO)
#define TX_FULL      ((mini_uart->LSR & 0x20) == 0x0)
#define TX_CH(c)     (mini_uart->IO = (c))


#if USE_IRQ
// Receive interrupt only. Transmit interrput is not supported.

#define IRQ_MU_PENDING ((mini_uart->IIR & 1U) == 0)

#define RX_BUF_SIZE 0xfff

volatile unsigned char rbuf[RX_BUF_SIZE + 1];
volatile uint32_t rbuf_w;
volatile uint32_t rbuf_r;

#define RX_BUF_INIT   {rbuf_w = 0; rbuf_r = 0;}
#define RX_EMPTY     (rbuf_w == rbuf_r)
#define RX_FULL      (((rbuf_w + 1) & RX_BUF_SIZE) == rbuf_r)

#define RX_WRITE(c)     \
    rbuf[rbuf_w] = (c); \
    rbuf_w = (rbuf_w + 1) & RX_BUF_SIZE; \
    if RX_EMPTY { rbuf_r = (rbuf_r + 1) & RX_BUF_SIZE; }

#define RX_READ(c)    \
    c = rbuf[rbuf_r]; \
    rbuf_r = (rbuf_r + 1) & RX_BUF_SIZE;

static void mini_uart_irq_enable() {
    IRQ_ENABLE1 = IRQ_AUX;
}

static void mini_uart_irq_disable() {
    IRQ_DISABLE1 = IRQ_AUX;
}

void isr_irq_mini_uart(void) {
    while (IRQ_MU_PENDING) {
        if (mini_uart->IIR & MU_IIR_RX_AVAIL) {
            unsigned char c = (RX_CH & 0xff);
            if (c == mp_interrupt_char) {
                mp_keyboard_interrupt();
            } else {
                RX_WRITE(c);
            }
        }
    }
}

#else // USE_IRQ

#define RX_EMPTY       !(mini_uart->LSR & 1U)
#define RX_READ(c)     (c = RX_CH)

void isr_irq_mini_uart(void) {
    return;
}

#endif // USE_IRQ

void mini_uart_set_speed(uint32_t speed) {
    mini_uart->CNTL = 0;   // disable mini uart
    mini_uart->BAUD = ((rpi_freq_core() / speed) >> 3) - 1;
    mini_uart->CNTL = 3;   // enable mini uart
}

void mini_uart_init() {
    // set GPIO14, GPIO15, alternate function 5
    IOREG(GPFSEL1) = (GPF_ALT_5 << (3*4)) | (GPF_ALT_5 << (3*5));

    // UART basic settings
#if USE_IRQ    
    mini_uart_irq_disable();
#endif

    AUX_ENABLES |= AUX_FLAG_MU;
    mini_uart->IER = 0;    // disable interrupts
    mini_uart->CNTL = 0;   // disable mini uart

#if USE_IRQ    
    mini_uart->IER = MU_IER_RX_AVAIL; // enable receive interrupt
#else
    mini_uart->IER = 0;    // disable receive/transmit interrupts
#endif

    mini_uart->IIR = 0xC6; // enable FIFO(0xC0), clear FIFO(0x06)
    mini_uart->MCR = 0;    // set RTS to High

    // data and speed (mini uart is always parity none, 1 start bit 1 stop bit)
    mini_uart->LCR = 3;    // 8 bits; Note: BCM2835 manual p14 is incorrect
    mini_uart->BAUD = ((rpi_freq_core() / 115200) >> 3) - 1;
    //    mini_uart->BAUD = 270; // 1115200 bps @ core_freq=250
    //    mini_uart->BAUD = 434; // 1115200 bps @ core_freq=400

    // enable transmit and receive
#if USE_IRQ    
    RX_BUF_INIT;
    mini_uart_irq_enable();
#endif

    mini_uart->CNTL = 3;
};

void mini_uart_putc(char c) {
    while(TX_FULL) {
    }
    TX_CH(c);
}

uint32_t mini_uart_getc(void) {
    uint32_t c;

    while(RX_EMPTY) {
    }
    RX_READ(c);
    return c & 0xffU;
}

uint32_t mini_uart_rx_state(void) {
    return !RX_EMPTY;
}
