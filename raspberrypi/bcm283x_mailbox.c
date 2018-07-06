#include <stdint.h>
#include "bcm283x_mailbox.h"

void mailbox_write(uint8_t chan, uint32_t msg) {
    while ((MAILBOX1_STATUS & MAIL_FULL) != 0) {
    }
    MAILBOX1_FIFO = (msg << 4) | chan;
}

uint32_t mailbox_read(uint8_t chan) {
    uint32_t data;
    do {
        while (MAILBOX0_STATUS & MAIL_EMPTY) {
        }
    } while (((data = MAILBOX0_FIFO) & 0xfU) != chan);
    return data >> 4;
}

