#ifndef MICROPY_INCLUDED_RPI_EXCEPTIONS_H
#define MICROPY_INCLUDED_RPI_EXCEPTIONS_H

typedef void (*exception_hander_t)(void);

typedef struct __attribute__((aligned(32))) _vector_table_t {
    const unsigned int vector[8]; // all elements shoud be JMP_PC_24
    exception_hander_t reset;
    exception_hander_t undef;
    exception_hander_t svc;
    exception_hander_t prefetch_abort;
    exception_hander_t data_abort;
    exception_hander_t hypervisor_trap;
    exception_hander_t irq;
    exception_hander_t fiq;
} vector_table_t;

extern vector_table_t exception_vector;

void arm_exceptions_init();

__attribute__(( always_inline )) static inline void arm_irq_enable() {
  __asm volatile("mrs r0, cpsr \n"
                 "bic r0, r0, #0x80 \n"
                 "msr cpsr_c, r0 \n");
}

__attribute__(( always_inline )) static inline void arm_irq_disable(){
  __asm volatile("mrs r0, cpsr \n"
                 "orr r0, r0, #0x80 \n"
                 "msr cpsr_c, r0 \n");
}

extern void __attribute__((interrupt("UNDEF"))) undef_handler(void);
extern void __attribute__((interrupt("SWI"))) svc_handler(void);
extern void __attribute__((interrupt("ABORT"))) abort_handler(void);
extern void __attribute__((interrupt("IRQ"))) irq_handler(void);
extern void __attribute__((interrupt("FIQ"))) fiq_handler(void);

#endif // MICROPY_INCLUDED_RPI_EXCEPTIONS_H
