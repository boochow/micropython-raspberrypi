#ifndef MICROPY_INCLUDED_RPI_USBKBD_H
#define MICROPY_INCLUDED_RPI_USBKBD_H

#ifdef MICROPY_HW_USBHOST

typedef unsigned char (*f_keycode2char_t)(int k, unsigned char shift);

typedef struct _usbkbd_t {
    unsigned int kbd_addr;
    unsigned int keys[6];
    f_keycode2char_t keycode2char;
} usbkbd_t;

unsigned char keycode2char_us(int k, unsigned char shift);
unsigned char keycode2char_jp(int k, unsigned char shift);

void usbkbd_init(usbkbd_t *kbd);
int usbkbd_getc(usbkbd_t *kbd);

#endif

#endif // MICROPY_INCLUDED_RPI_USBKBD_H
