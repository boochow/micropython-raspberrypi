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

	// jump to main
	pop {r0}
	bl main

	/// enable unaligned support
    push {r0, r1}
    mrc p15, 0, r0, c1, c0, 0
    ldr r1, =0x00400000     /// set bit 22 (U-bit)
    orr r0, r1
    mcr p15, 0, r0, c1, c0, 0
	pop {r0, r1}

    bl main

halt:
    b halt
    
