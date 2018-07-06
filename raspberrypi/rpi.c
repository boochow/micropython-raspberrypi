#include <stdint.h>
#include <stdio.h>

#include "arm_exceptions.h"
#include "bcm283x.h"
#include "bcm283x_it.h"
#include "bcm283x_gpio.h"
#include "bcm283x_aux.h"
#include "bcm283x_mailbox.h"
#include "vc_property.h"
#include "rpi.h"

static uint32_t freq_cpu  = 700000000;
static uint32_t freq_core = 250000000;

// System Timers

systimer_t *systimer = (systimer_t *) SYSTIMER;

volatile uint64_t systime(void) {
    uint64_t t;
    uint32_t chi;
    uint32_t clo;

    chi = systimer->CHI;
    clo = systimer->CLO;
    if (chi != systimer->CHI) {
        chi = systimer->CHI;
        clo = systimer->CLO;
    }
    t = chi;
    t = t << 32;
    t += clo;
    return t;
}

// IRQ handler

#define IRQ_SYSTIMERS (IRQ_SYSTIMER(0) | \
                       IRQ_SYSTIMER(1) | \
                       IRQ_SYSTIMER(2) | \
                       IRQ_SYSTIMER(3))

void __attribute__((interrupt("IRQ"))) irq_handler(void) {
    if (IRQ_PEND1 & IRQ_SYSTIMERS) {
        isr_irq_timer();
    }
    if (IRQ_PEND1 & IRQ_AUX) {
        if (AUX_IRQ & AUX_FLAG_MU) {
            isr_irq_mini_uart();
        }
    }
}

// Clock speed
// Todo: use VC mailbox interface to get real clock speed

uint32_t rpi_freq_core() {
    return freq_core;
}

uint32_t rpi_freq_cpu() {
    return freq_cpu;
}

// Initialize Raspberry Pi peripherals

#define MB_GET_CLK_RATE (0x00030002)
#define CLKID_ARM       (3)
#define CLKID_CORE      (4)
enum MB_BUFF { BUFSIZE=0, MB_REQ_RES, \
               TAG0, VALBUFSIZE0, REQ_RES0, VALUE00, VALUE01,\
               TAG1, VALBUFSIZE1, REQ_RES1, VALUE10, VALUE11,\
               ENDTAG };

static void get_clock_value() {
    __attribute__((aligned(16))) uint32_t clkinfo[ENDTAG + 1];
    clkinfo[BUFSIZE] = 4 * (ENDTAG + 1);
    clkinfo[MB_REQ_RES] = MB_PROP_REQUEST;

    clkinfo[TAG0] = MB_GET_CLK_RATE;
    clkinfo[VALBUFSIZE0] = 8;
    clkinfo[REQ_RES0] = 0;
    clkinfo[VALUE00] = CLKID_ARM;
    clkinfo[VALUE01] = 0;

    clkinfo[TAG1] = MB_GET_CLK_RATE;
    clkinfo[VALBUFSIZE1] = 8;
    clkinfo[REQ_RES1] = 0;
    clkinfo[VALUE10] = CLKID_CORE;
    clkinfo[VALUE11] = 0;

    clkinfo[ENDTAG] = 0;

    mailbox_write(MB_CH_PROP_ARM, (uint32_t) BUSADDR(clkinfo) >> 4);
    mailbox_read(MB_CH_PROP_ARM);

    if (clkinfo[MB_REQ_RES] == MB_PROP_SUCCESS) {
        if (clkinfo[REQ_RES0] & MB_PROP_SUCCESS) {
            freq_cpu = clkinfo[VALUE01];
        }
        if (clkinfo[REQ_RES1] & MB_PROP_SUCCESS) {
            freq_core = clkinfo[VALUE11];
        }
    }
}

void rpi_init() {
    get_clock_value();
// any initialization 
}

