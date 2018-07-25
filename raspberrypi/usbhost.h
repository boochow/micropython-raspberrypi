#ifndef MICROPY_INCLUDED_RPI_USBHOST_H
#define MICROPY_INCLUDED_RPI_USBHOST_H

typedef struct _hcd_globals_t {
    void *core;
    void *host;
    void *power;
    void *devices[32];
    void *databuffer;
    void *keyboards[4];
    void *mice[4];
} hcd_globals_t;

void rpi_usb_host_init(void);
void rpi_usb_host_process(void);
void rpi_usb_host_deinit(void);

#endif // MICROPY_INCLUDED_RPI_USBHOST_H
