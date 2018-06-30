#include <stdio.h>
#include <string.h>
#include "usbd/usbd.h"

#include "py/runtime.h"
#include "mphalport.h"
#include "bcm283x_mailbox.h"
#include "usbhost.h"

/* USB host controller driver static variables */
// memory block for these variables are allocated using
// m_malloc() so we must keep them in MP object
// to avoid them from deallocating by gc

extern void *Core;
extern void *Host;
extern void *Power;
extern void *Devices;
extern void *databuffer;
extern void *keyboards;
extern void *mice;

extern hcd_globals_t *hcd_globals;
static u32 usb_initialised = 0;

void* MemoryAllocate(u32 length);

void rpi_usb_host_init(void) {
    if (usb_initialised == 0) {
        MP_STATE_PORT(hcd_globals) = MemoryAllocate(sizeof(hcd_globals_t));
        UsbInitialise();
        MP_STATE_PORT(hcd_globals)->core = Core;
        MP_STATE_PORT(hcd_globals)->host = Host;
        MP_STATE_PORT(hcd_globals)->power = Power;
        MP_STATE_PORT(hcd_globals)->devices = Devices;
        MP_STATE_PORT(hcd_globals)->databuffer = databuffer;
        MP_STATE_PORT(hcd_globals)->keyboards = keyboards;
        MP_STATE_PORT(hcd_globals)->mice = mice;

        usb_initialised = 1;
    }
}

void rpi_usb_host_process(void) {
    UsbCheckForChange();
}

void rpi_usb_host_deinit(void) {
    if (usb_initialised != 0) {
        extern Result HcdStop();
        HcdStop();
        extern Result HcdDeinitialise();
        HcdDeinitialise();
        m_del(hcd_globals_t, MP_STATE_PORT(hcd_globals), 1);
        
        usb_initialised = 0;
    }
}

/* platform dependent functions called from functions in libcsud.a */
// defined in csud/include/platform/platform.h

void LogPrint(const char* message, uint32_t messageLength) {
    printf(message);
}

void* MemoryReserve(u32 length, void* physicalAddress) {
    return physicalAddress;
}

void* MemoryAllocate(u32 length) {
    return m_new(uint8_t, length);
}

void MemoryDeallocate(void* address) {
    m_free(address);
}

void MemoryCopy(void* destination, void* source, u32 length) {
    memcpy(destination, source, length);
}

Result PowerOnUsb() {
	u32 result;
    mailbox_write(MB_CH_POWER, 0x8);
    result = mailbox_read(MB_CH_POWER);
    return  (result == 8) ? OK : ErrorDevice;
}

void PowerOffUsb() {
}

void MicroDelay(u32 delay) {
    mp_hal_delay_us(delay);
}
