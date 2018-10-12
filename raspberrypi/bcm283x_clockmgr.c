#include <stdint.h>
#include "bcm283x_clockmgr.h"

void clockmgr_pause(clockmgr_t *clk) {
    if (clk->CTL & CM_CTL_ENAB) {
        clockmgr_set_ctl(clk, (clk->CTL & ~CM_CTL_ENAB));
        while(clk->CTL & CM_CTL_BUSY) {};
    }
}

void clockmgr_config_ctl(clockmgr_t *clk, int32_t flags) {
    clockmgr_pause(clk);
    clk->CTL = CM_PASSWORD | flags;
}

void clockmgr_config_div(clockmgr_t *clk, int32_t divi, int32_t divf) {
    uint32_t save = clk->CTL;
    clockmgr_pause(clk);
    clockmgr_set_div(clk, divi, divf);
    if (save & CM_CTL_ENAB) {
        clockmgr_set_ctl(clk, save);
    }
}
