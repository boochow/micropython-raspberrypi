#include <unistd.h>
#include "arm_exceptions.h"

// ldr pc, [pc, #24]
#define	JMP_PC_24	0xe59ff018


extern void _start(void);  // defined in start.s

static void __attribute__((naked)) hangup(void) {
  while(1) {
  }
}

vector_table_t exception_vector = { \
    .vector = { JMP_PC_24, JMP_PC_24, JMP_PC_24, JMP_PC_24, \
                JMP_PC_24, JMP_PC_24, JMP_PC_24, JMP_PC_24 },
    .reset = _start,
    .undef = undef_handler,
    .svc = svc_handler,
    .prefetch_abort = abort_handler,
    .data_abort = abort_handler,
    .hypervisor_trap = hangup,
    .irq = irq_handler,
    .fiq = fiq_handler
};

static void set_vbar(vector_table_t *base) {
    __asm volatile ("mcr p15, 0, %[base], c12, c0, 0"
                  :: [base] "r" (base));
}

void arm_exceptions_init() {
    set_vbar(&exception_vector);
}
