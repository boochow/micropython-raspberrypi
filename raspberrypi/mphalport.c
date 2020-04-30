#include <stdio.h>

#include <stdint.h>
#include "py/mpconfig.h"
#include "py/obj.h"
#include "py/stream.h"
#include "rpi.h"
#include "mphalport.h"
#include "mini-uart.h"
#include "uart.h"
#include "extmod/misc.h"

#ifdef MICROPY_HW_USBHOST
#include "usbkbd.h"
#endif

// Time and delay

void mp_hal_delay_ms(mp_uint_t ms) {
    uint64_t end_time;

    end_time = systime() + ms * 1000;
    while(systime() < end_time) {
        extern void mp_handle_pending(bool raise_exc);
        mp_handle_pending(true);
    }
  
    return;
}

void mp_hal_delay_us(mp_uint_t us) {
    uint64_t end_time;

    end_time = systime() + us;
    while(systime() < end_time);
  
    return;
}

mp_uint_t mp_hal_ticks_cpu(void) {
    return systime();
}

mp_uint_t mp_hal_ticks_us(void) {
    return systime();
}

mp_uint_t mp_hal_ticks_ms(void) {
    return systime() / 1000;
}


// standard I/O for REPL

static void (*_uart_putc)(char c);
static uint32_t (*_uart_getc)(void);
static uint32_t (*_uart_rx_state)(void);

void uart_init(std_io_t interface) {
    switch(interface) {
    case UART_QEMU:
        uart0_qemu_init();
        _uart_putc = &uart0_putc;
        _uart_getc = &uart0_getc;
        _uart_rx_state = &uart0_rx_state;
        break;
    default:
        mini_uart_init();
        _uart_putc = &mini_uart_putc;
        _uart_getc = &mini_uart_getc;
        _uart_rx_state = &mini_uart_rx_state;
        break;
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

uint32_t uart_rx_state(void) {
    return _uart_rx_state();
}

#ifdef MICROPY_HW_USBHOST
static usbkbd_t usbkbd;

void usbkbd_setup() {
    usbkbd_init(&usbkbd);
}

#endif
// Receive single character
int mp_hal_stdin_rx_chr(void) {
    for(;;) {
#ifdef MICROPY_HW_USBHOST
        int result;
        if ((result = usbkbd_getc(&usbkbd)) >= 0) {
            return result;
        }
#endif
        if (uart_rx_state()) {
            return uart_getc();
        }
        extern void mp_handle_pending(bool raise_exc);
        mp_handle_pending(true);
    }
}

// Send string of given length
void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {
    uart_write(str,len);
#ifdef MICROPY_PY_OS_DUPTERM
    mp_uos_dupterm_tx_strn(str, len);
#endif
}

/*This is a dummy function (needed because "lib/utils/sys_stdio_mphal: 
Add support to poll sys.stdin and sys.stdou" was added to 1.12 in
to lib/utils/sys_stdio_mphal.c by commit b7da67cdaaf32317cfc9a3940bd58f2aab4976c9
previous build for raspi (1.11) did not have this addition */
uintptr_t mp_hal_stdio_poll(uintptr_t poll_flags) {
    return 0;
}
