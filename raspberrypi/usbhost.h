#ifndef MICROPY_INCLUDED_RPI_USBHOST_H
#define MICROPY_INCLUDED_RPI_USBHOST_H

typedef struct _hcd_globals_t {
    void *core;
    void *host;
    void *power;
    void *devices;
    void *databuffer;
    void *keyboards;
    void *mice;
} hcd_globals_t;

void rpi_usb_host_init(void);
void rpi_usb_host_process(void);
void rpi_usb_host_deinit(void);

#endif // MICROPY_INCLUDED_RPI_USBHOST_H
