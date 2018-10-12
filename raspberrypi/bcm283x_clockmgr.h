#ifndef MICROPY_INCLUDED_RPI_BCM283X_CLOCKMGR_H
#define MICROPY_INCLUDED_RPI_BCM283X_CLOCKMGR_H

#define CM_GP0     (IO_BASE + 0x101070)
#define CM_GP1     (IO_BASE + 0x101078)
#define CM_GP2     (IO_BASE + 0x101080)
#define CM_PCM     (IO_BASE + 0x101098)
#define CM_PWM     (IO_BASE + 0x1010a0)

typedef volatile struct _clockmgr_t {
    uint32_t CTL;
    uint32_t DIV;
} clockmgr_t;


#define CM_PASSWORD       (0x5a000000)

#define CM_CTL_MASH_MASK  (3U<<9)
#define CM_CTL_MASH_IDIV  (0U<<9)
#define CM_CTL_MASH_1STG  (1U<<9)
#define CM_CTL_MASH_2STG  (2U<<9)
#define CM_CTL_MASH_3STG  (3U<<9)

#define CM_CTL_FLIP       (1U<<8)
#define CM_CTL_BUSY       (1U<<7)
#define CM_CTL_KILL       (1U<<5)
#define CM_CTL_ENAB       (1U<<4)

#define CM_CTL_SRC_MASK   (0xfU)
#define CM_CTL_SRC_GND    (0U)
#define CM_CTL_SRC_OSC    (1U)
#define CM_CTL_SRC_PLLA   (4U)
#define CM_CTL_SRC_PLLC   (5U)
#define CM_CTL_SRC_PLLD   (6U)
#define CM_CTL_SRC_HDMI   (7U)

#define CM_DIV_MASK       (0x00ffffffU)

__attribute__(( always_inline )) inline void clockmgr_set_ctl(clockmgr_t *clk, uint32_t ctl) {
    clk->CTL = CM_PASSWORD | (ctl & 0xffffff);
}

__attribute__(( always_inline )) inline void clockmgr_set_div(clockmgr_t *clk, uint32_t divi, uint32_t divf) {
    clk->DIV = CM_PASSWORD | (divi & 0xfff) << 12 | (divf & 0xfff);
}

void clockmgr_pause(clockmgr_t *clk);

void clockmgr_config_ctl(clockmgr_t *clk, int32_t flags);

void clockmgr_config_div(clockmgr_t *clk, int32_t divi, int32_t divf);

#endif // MICROPY_INCLUDED_RPI_BCM283X_CLOCKMGR_H
