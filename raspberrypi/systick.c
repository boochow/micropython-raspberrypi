#include <unistd.h>
#include "py/mpconfig.h"
#include "bcm283x.h"
#include "systick.h"

typedef struct systimer_t {
    uint32_t CS;
    uint32_t CLO;
    uint32_t CHI;
    uint32_t C0;
    uint32_t C1;
    uint32_t C2;
    uint32_t C3;
} systimer_t;

volatile systimer_t *systimer = (volatile systimer_t *)(IO_BASE + 0x3000);

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

