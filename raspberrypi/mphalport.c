#include <stdio.h>
#ifdef MICROPY_HW_USBHOST
#include "usbd/usbd.h"
#include "device/hid/keyboard.h"
#endif

#include <stdint.h>
#include "py/mpconfig.h"
#include "py/obj.h"
#include "rpi.h"
#include "mphalport.h"
#include "mini-uart.h"
#include "uart.h"
#include "extmod/misc.h"

// Time and delay

void mp_hal_delay_ms(mp_uint_t ms) {
    uint64_t end_time;

    end_time = systime() + ms * 1000;
    while(systime() < end_time) {
        extern void mp_handle_pending(void);
        mp_handle_pending();
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
static unsigned int kbd_addr;
static unsigned int keys[6];
unsigned char keymap[2][104] = {
    {
        0x0, 0x1, 0x2, 0x3, 'a', 'b', 'c', 'd',
        'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
        'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
        'u', 'v', 'w', 'x', 'y', 'z', '1', '2',
        '3', '4', '5', '6', '7', '8', '9', '0',
        0xd, 0x1b, 0x8, 0x9, ' ', '-', '=', '[',
        ']', '\\', 0x0, ';', '\'', '`', ',', '.',
        '/', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x12, 0x0, 0x0, 0x7f, 0x0, 0x0, 0x1c,
        0x1d, 0x1f, 0x1e, 0x0, '/', '*', '-', '+',
        0xd, '1', '2', '3', '4', '5', '6', '7',
        '8', '9', '0', '.', '\\', 0x0, 0x0, '=',
    },
    {
        0x0, 0x0, 0x0, 0x0, 'A', 'B', 'C', 'D',
        'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
        'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
        'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@',
        '#', '$', '%', '^', '&', '*', '(', ')',
        0xa, 0x1b, '\b', '\t', ' ', '_', '+', '{',
        '}', '|', '~', ':', '"', '~', '<', '>',
        '?', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, '/', '*', '-', '+',
        0xa, '1', '2', '3', '4', '5', '6', '7',
        '8', '9', '0', '.', '|', 0x0, 0x0, '=',
    }
};

unsigned char keycode2char(int k, unsigned char shift) {
    if (k > 103) {
        return 0;
    } else {
        return keymap[(shift == 0) ? 0 : 1][k];
    }
}

// usb_keyboard_getc() returns -1 if no input
int usb_keyboard_getc() {
    unsigned int key;
    struct KeyboardModifiers mod;
    int result = -1;
    
    if (kbd_addr == 0) {
        // Is there a keyboard ?
        UsbCheckForChange();
        if (KeyboardCount() > 0) {
            kbd_addr = KeyboardGetAddress(0);
        }
    }

    if (kbd_addr != 0) {
        for(int i = 0; i < 6; i++) {
            // Read and print each keycode of pressed keys
            key = KeyboardGetKeyDown(kbd_addr, i);
            if (key != keys[0] && key != keys[1] && key != keys[2] && \
                key != keys[3] && key != keys[4] && key != keys[5] && key) {
                mod = KeyboardGetModifiers(kbd_addr);
                if (mod.RightControl | mod.LeftControl) {
                    unsigned char c = (unsigned char) keycode2char(key, 1);
                    switch(c) {
                    case 'A': result = 1; break;
                    case 'B': result = 2; break;
                    case 'C': result = 3; break;
                    case 'D': result = 4; break;
                    case 'E': result = 5; break;
                    case 'F': result = 6; break;
                    case 'K': result = 11; break;
                    case 'N': result = 14; break;
                    case 'P': result = 16; break;
                    case 'U': result = 21; break;
                    default: result = 0;
                    }
                } else {
                    result = keycode2char(key, mod.RightShift | mod.LeftShift);
                    // convert arrow keys to EMACS binding controls
                    switch(result) {
                    case 0x1d: result = 2; break;
                    case 0x1c: result = 6; break;
                    case 0x1f: result = 14; break;
                    case 0x1e: result = 16; break;
                    default: break;
                    }
                }
            }
            keys[i] = key;
        }

        if (KeyboardPoll(kbd_addr) != 0) {
            kbd_addr = 0;
        }
    }
    return result;
}
#endif

// Receive single character
int mp_hal_stdin_rx_chr(void) {
    for(;;) {
#ifdef MICROPY_HW_USBHOST
        int result;
        if ((result = usb_keyboard_getc()) >= 0) {
            return result;
        }
#endif
        if (uart_rx_state()) {
            return uart_getc();
        }
        extern void mp_handle_pending(void);
        mp_handle_pending();
    }
}

// Send string of given length
void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {
    uart_write(str,len);
#ifdef MICROPY_PY_OS_DUPTERM
    mp_uos_dupterm_tx_strn(str, len);
#endif
}
