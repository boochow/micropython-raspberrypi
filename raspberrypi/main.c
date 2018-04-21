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
#include "uart-qemu.h"
#include "usbhost.h"

void do_str(const char *src, mp_parse_input_kind_t input_kind) {
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src, strlen(src), 0);
        qstr source_name = lex->source_name;
        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);
        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, MP_EMIT_OPT_NONE, true);
        mp_call_function_0(module_fun);
        nlr_pop();
    } else {
        // uncaught exception
        mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
    }
}

void clear_bss(void) {
    extern void * _bss_start;
    extern void *  _bss_end;
    unsigned int *p;

    for(p = (unsigned int *)&_bss_start; p < (unsigned int *) &_bss_end; p++) {
        *p = 0;
    }
}

#define TAG_CMDLINE 0x54410009
char *arm_boot_tag_cmdline(int32_t *ptr) {
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

int main(uint32_t r0, uint32_t id, const int32_t *atag) {
    extern char * _heap_end;
    extern char * _heap_start;
    extern char * _estack;
    //    register int R2 __asm("r2");
    //    int32_t r2_save;

    //    r2_save = R2;
    clear_bss();
    mp_stack_set_top(&_estack);
    mp_stack_set_limit((char*)&_estack - (char*)&_heap_end - 1024);

    // use UART if arm boot tag holds "qemu" in cmdline else use Mini-UART.
    if (atag && (strcmp("qemu", arm_boot_tag_cmdline(atag)) == 0)) {
        uart_init(false);
    } else {
        uart_init(true);
        arm_exceptions_init();
    }
         
    while (true) {
        gc_init (&_heap_start, &_heap_end );

	mp_init();

	do_str("for i in range(1):pass", MP_PARSE_FILE_INPUT);

#ifdef MICROPY_PY_USBHOST
    rpi_usb_host_init();
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
        mp_deinit();
        printf("PYB: soft reboot\n");
    }
    return 0;
}

void gc_collect(void) {
}

mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    mp_raise_OSError(MP_ENOENT);
}

mp_import_stat_t mp_import_stat(const char *path) {
    return MP_IMPORT_STAT_NO_EXIST;
}

mp_obj_t mp_builtin_open(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);

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
