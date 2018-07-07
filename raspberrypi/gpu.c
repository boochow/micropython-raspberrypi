#include <stdio.h>
#include <stdint.h>

#include "bcm283x_mailbox.h"
#include "gpu.h"

void rpi_fb_init(fb_info_t *fb_info) {
    fb_info->buf_addr = 0;
    fb_info->buf_size = 0;
    fb_info->rowbytes = 0;
    while(fb_info->buf_addr == 0) {
        mailbox_write(MB_CH_FRAMEBUF, (uint32_t) BUSADDR(fb_info) >> 4);
        mailbox_read(MB_CH_FRAMEBUF);
    }
}

