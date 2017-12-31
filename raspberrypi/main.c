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

#include "uart-qemu.h"
#include "gpio.h"

extern char heap_end, heap_start, stack_top;

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

int main(int argc, char **argv) {
    extern uint32_t _ebss;
    extern uint32_t _sdata;

    mp_stack_set_top(&stack_top);
    mp_stack_set_limit(&heap_end);

    uart_init();
        
    gc_init ( &heap_start, &heap_end );

    mp_init();

    do_str("for i in range(1):pass", MP_PARSE_FILE_INPUT);

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
    return 0;
}

void gc_collect(void) {
}

/*
void mp_hal_stdout_tx_strn_cooked(const char *str, size_t len) {
  while(len > 0) {
    print_ch(*str++);
    len--;
  }
}
*/

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
