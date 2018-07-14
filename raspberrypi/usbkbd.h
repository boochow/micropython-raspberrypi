#ifndef MICROPY_INCLUDED_RPI_USBKBD_H
#define MICROPY_INCLUDED_RPI_USBKBD_H

#ifdef MICROPY_HW_USBHOST

typedef unsigned char (*f_keycode2char_t)(int k, unsigned char shift);

unsigned char keycode2char_us(int k, unsigned char shift);
unsigned char keycode2char_jp(int k, unsigned char shift);

int usbkbd_getc();
void usbkbd_set_keycodefunc(f_keycode2char_t f);
f_keycode2char_t usbkbd_get_keycodefunc();

#endif

#endif // MICROPY_INCLUDED_RPI_USBKBD_H
