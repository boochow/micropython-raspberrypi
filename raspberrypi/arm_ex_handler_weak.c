void __attribute__((weak, interrupt("UNDEF"))) undef_handler(void) {
}

void __attribute__((weak, interrupt("SWI"))) svc_handler(void) {
}

void __attribute__((weak, interrupt("ABORT"))) abort_handler(void) {
}

void __attribute__((weak, interrupt("IRQ"))) irq_handler(void) {
}

void __attribute__((weak, interrupt("IRQ"))) fiq_handler(void) {
}

