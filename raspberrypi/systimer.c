#include <unistd.h>
#include "bcm283x.h"
#include "systimer.h"

systimer_t *systimer = (systimer_t *) (IO_BASE + 0x3000);

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
