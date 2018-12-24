#ifndef MICROPY_INCLUDED_RPI_BCM283X_MAILBOX_H
#define MICROPY_INCLUDED_RPI_BCM283X_MAILBOX_H

#include "bcm283x.h"

#define MAILBOX0_FIFO   IOREG(IO_BASE + 0xB880)
#define MAILBOX0_POLL   IOREG(IO_BASE + 0xB890)
#define MAILBOX0_SENDER IOREG(IO_BASE + 0xB894)
#define MAILBOX0_STATUS IOREG(IO_BASE + 0xB898)
#define MAILBOX0_CONFIG IOREG(IO_BASE + 0xB89C)
#define MAILBOX1_FIFO   IOREG(IO_BASE + 0xB8A0)
#define MAILBOX1_POLL   IOREG(IO_BASE + 0xB8B0)
#define MAILBOX1_SENDER IOREG(IO_BASE + 0xB8B4)
#define MAILBOX1_STATUS IOREG(IO_BASE + 0xB8B8)
#define MAILBOX1_CONFIG IOREG(IO_BASE + 0xB8BC)

#define MAIL_FULL      0x80000000U
#define MAIL_EMPTY     0x40000000U

#define MB_CH_POWER    0
#define MB_CH_FRAMEBUF 1
#define MB_CH_VUART    2
#define MB_CH_VCHIQ    3
#define MB_CH_LED      4
#define MB_CH_BUTTON   5
#define MB_CH_TOUCH    6
#define MB_CH_RESERVED 7
#define MB_CH_PROP_ARM 8
#define MB_CH_PROP_VC  9

#ifdef RPI1
#define BUSADDR(a)      (0x40000000 | (unsigned int) a)
#else
#define BUSADDR(a)      (0xC0000000 | (unsigned int) a)
#endif

#define PHYSICALADDR(a) (0x3fffffff & (unsigned int) a)

void mailbox_write(uint8_t chan, uint32_t msg);
uint32_t mailbox_read(uint8_t chan);

#endif // MICROPY_INCLUDED_RPI_BCM283X_MAILBOX_H
