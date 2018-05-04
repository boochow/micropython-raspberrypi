.global _start
.section .init

_start:
    // set SVC_MODE stack
    ldr sp, =_estack

    push {r0}
    // set IRQ_MODE stack
    ldr r0, =0x000000d2
    msr cpsr_c, r0
    ldr sp, =0x8000

    // set FIQ_MODE stack
    ldr r0, =0x000000d1
    msr cpsr_c, r0
    ldr sp, =0x4000

    // set mode to SVC_MODE
    ldr r0, =0x000000d3
    msr cpsr_c, r0

    /// enable unaligned access support
    mrc p15, 0, r0, c1, c0, 0
    orr r0, r0, #0x00400000
    mcr p15, 0, r0, c1, c0, 0
    
    // jump to main
    pop {r0}
    bl arm_main

halt:
    b halt
    
