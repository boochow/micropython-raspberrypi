#include <stdint.h>
#include "bcm283x_mailbox.h"

void mailbox_write(uint8_t chan, uint32_t msg) {
    if ((msg & 0xfU) == 0) {
        while ((MAILBOX_STATUS & MAIL_FULL) != 0) {
        }
        MAILBOX_WRITE = msg | chan;
    }
}

uint32_t mailbox_read(uint8_t chan) {
    uint32_t data;
    do {
        while (MAILBOX_STATUS & MAIL_EMPTY) {
        }
    } while (((data = MAILBOX_READ) & 0xfU) != chan);
    return data >> 4;
}

