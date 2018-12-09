.global _start
.section .init

_start:
    // return to supervisor mode
    mrs r0, cpsr
    bic r0, r0, #0x1f
    orr r0, r0, #0x13
    msr spsr_cxsf, r0
    add r0, pc, #4
    msr ELR_hyp, r0
    eret

	// stop core 1-3
//	mrc p15, #0, r1, c0, c0, #5
//	and r1, r1, #3
//	cmp r1, #2
//	bne halt
	
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
    ldr sp, =_estack


//    mrc p15, 0, r0, c1, c0, 0
    /// enable unaligned access support
//    orr r0, r0, #0x00400000
    /// enable data cache
//    orr r0, r0, #0x00000004
    /// enable instruction cache
//    orr r0, r0, #0x00001000
    /// enable branch prediction
//    orr r0, r0, #0x00000800
//    mcr p15, 0, r0, c1, c0, 0

    // jump to main
    ldr r0, =0x00000000
    bl arm_main

halt:
    b halt
    
