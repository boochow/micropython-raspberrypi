#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "py/nlr.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/stackctrl.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "lib/utils/interrupt_char.h"
#include "lib/utils/pyexec.h"

#include "arm_exceptions.h"
#include "rpi.h"
#include "mphalport.h"
#include "usbhost.h"

void clear_bss(void) {
    extern void * _bss_start;
    extern void *  _bss_end;
    unsigned int *p;

    for(p = (unsigned int *)&_bss_start; p < (unsigned int *) &_bss_end; p++) {
        *p = 0;
    }
}

#define TAG_CMDLINE 0x54410009
char *arm_boot_tag_cmdline(const int32_t *ptr) {
    int32_t datalen = *ptr;
    int32_t tag = *(ptr + 1);

    if (ptr != 0) {
        while(tag) {
            if (tag == TAG_CMDLINE) {
                return (char *) (ptr + 2);
            } else {
                ptr += datalen;
                datalen = *ptr;
                tag = *(ptr + 1);
            }
        }
    }
    return NULL;
}

int arm_main(uint32_t r0, uint32_t id, const int32_t *atag) {
    bool use_qemu = false;
    
    extern char * _heap_end;
    extern char * _heap_start;
    extern char * _estack;

    clear_bss();
    mp_stack_set_top(&_estack);
    mp_stack_set_limit((char*)&_estack - (char*)&_heap_end - 1024);

    // check ARM boot tag and set up standard I/O
    if (atag && (strcmp("qemu", arm_boot_tag_cmdline(atag)) == 0)) {
        use_qemu = true;
    }

    if (use_qemu) {
        uart_init(UART_QEMU);
    } else {
        uart_init(MINI_UART);
        arm_irq_disable();
        arm_exceptions_init();
        arm_irq_enable();
    }

    // start MicroPython
    while (true) {
        gc_init (&_heap_start, &_heap_end );
        mp_init();
        mp_obj_list_init(mp_sys_path, 0);
        mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_)); // current dir (or base dir of the script)
        mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_lib));
        mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_));
        mp_obj_list_init(mp_sys_argv, 0);

#ifdef MICROPY_PY_USBHOST
        // USB host library initialization must be called after MicroPython's
        // initialization because USB host library allocates its memory blocks
        // using MemoryAllocate in usbhost.c which in turn calls m_alloc().
        if (!use_qemu) {
            rpi_usb_host_init();
        }
#endif
        for (;;) {
            if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
                if (pyexec_raw_repl() != 0) {
                    break;
                }
            } else {
                if (pyexec_friendly_repl() != 0) {
                    break;
                }
            }
        }
#ifdef MICROPY_PY_USBHOST
        if (!use_qemu) {
            rpi_usb_host_deinit();
        }
#endif
        mp_deinit();
        printf("PYB: soft reboot\n");
    }
    return 0;
}

mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    mp_raise_OSError(MP_ENOENT);
}

void nlr_jump_fail(void *val) {
    while (1);
}

void NORETURN __fatal_error(const char *msg) {
    while (1);
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("Assertion failed");
}
#endif
