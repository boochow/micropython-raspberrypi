#ifndef MICROPY_INCLUDED_RPI_GPU_H
#define MICROPY_INCLUDED_RPI_GPU_H

#include <stdint.h>
#include "bcm283x_mailbox.h"

typedef volatile struct                                          \
__attribute__((aligned(16))) _fb_info_t {
    uint32_t screen_w;   //write display width
    uint32_t screen_h;   //write display height
    uint32_t w;          //write framebuffer width
    uint32_t h;          //write framebuffer height
    uint32_t rowbytes;   //write 0 to get value
    uint32_t bpp;        //write bits per pixel
    uint32_t offset_x;   //write x offset of framebuffer
    uint32_t offset_y;   //write y offset of framebuffer
    uint32_t buf_addr;   //write 0 to get value
    uint32_t buf_size;   //write 0 to get value
} fb_info_t;

void rpi_fb_init(fb_info_t *fb_info);

#endif // MICROPY_INCLUDED_RPI_GPU_H
