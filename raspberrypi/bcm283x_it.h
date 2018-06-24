#ifndef MICROPY_INCLUDED_RPI_BCM283X_IT_H
#define MICROPY_INCLUDED_RPI_BCM283X_IT_H

#include "bcm283x.h"

#define IRQ_PEND_BASIC    IOREG(IO_BASE + 0xB200)
#define IRQ_PEND1         IOREG(IO_BASE + 0xB204)
#define IRQ_PEND2         IOREG(IO_BASE + 0xB208)
#define IRQ_FIQ_CONTROL   IOREG(IO_BASE + 0xB20C)
#define IRQ_ENABLE1       IOREG(IO_BASE + 0xB210)
#define IRQ_ENABLE2       IOREG(IO_BASE + 0xB214)
#define IRQ_ENABLE_BASIC  IOREG(IO_BASE + 0xB218)
#define IRQ_DISABLE1      IOREG(IO_BASE + 0xB21C)
#define IRQ_DISABLE2      IOREG(IO_BASE + 0xB220)
#define IRQ_DISABLE_BASIC IOREG(IO_BASE + 0xB224)

// IRQ_PEND_BASIC bits
#define IRQ_ARM_TIMER    (1)
#define IRQ_MAILBOX      (1 << 1)
#define IRQ_DOORBELL0    (1 << 2)
#define IRQ_DOORBELL1    (1 << 3)
    
// IRQ_PEND1 bits
#define IRQ_SYSTIMER(X)  (1 << (X))
#define IRQ_AUX          (1 << 29)

// IRQ_PEND2 bits
#define IRQ_GPIO(X)      (1 << (17 + X))
#define IRQ_I2C          (1 << 21)
#define IRQ_SPI          (1 << 22)
#define IRQ_PCM          (1 << 23)
#define IRQ_UART         (1 << 25)
    
#endif // MICROPY_INCLUDED_RPI_BCM283X_IT_H
