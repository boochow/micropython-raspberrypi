#ifndef MICROPY_INCLUDED_RPI_BCM283X_PWM_H
#define MICROPY_INCLUDED_RPI_BCM283X_PWM_H

#include "bcm283x.h"

#define PWM (IO_BASE + 0x20C000)

typedef volatile struct _pwm_t {
    uint32_t CTL;
    uint32_t STA;
    uint32_t DMAC;
    uint32_t undef1;
    uint32_t RNG1;
    uint32_t DAT1;
    uint32_t FIF1;
    uint32_t undef2;
    uint32_t RNG2;
    uint32_t DAT2;
} pwm_t;

#define CTL_MSEN2 (1<<15)
#define CTL_USEF2 (1<<13)
#define CTL_POLA2 (1<<12)
#define CTL_SBIT2 (1<<11)
#define CTL_RPTL2 (1<<10)
#define CTL_MODE2 (1<<9)
#define CTL_PWEN2 (1<<8)
#define CTL_MSEN1 (1<<7)
#define CTL_CLRF1 (1<<6)
#define CTL_USEF1 (1<<5)
#define CTL_POLA1 (1<<4)
#define CTL_SBIT1 (1<<3)
#define CTL_RPTL1 (1<<2)
#define CTL_MODE1 (1<<1)
#define CTL_PWEN1 (1)

#define STA_STA4  (1<<12)
#define STA_STA3  (1<<11)
#define STA_STA2  (1<<10)
#define STA_STA1  (1<<9)
#define STA_BERR  (1<<8)
#define STA_GAPO4 (1<<7)
#define STA_GAPO3 (1<<6)
#define STA_GAPO2 (1<<5)
#define STA_GAPO1 (1<<4)
#define STA_RERR1 (1<<3)
#define STA_WERR1 (1<<2)
#define STA_EMPT1 (1<<1)
#define STA_FULL1 (1)

#define DMAC_ENAB  (1<<31)
#define DMAC_PANIC (255<<8)
#define DMAC_DREQ  (255)

#endif // MICROPY_INCLUDED_RPI_BCM283X_PWM_H
