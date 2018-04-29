#include <stdio.h>
#include <string.h>
#include "usbd/usbd.h"

#include "py/runtime.h"
#include "mphalport.h"
#include "usbhost.h"

/* USB host controller driver static variables */
// memory block for these variables are allocated using
// m_malloc() so we must keep them in MP object
// to avoid them from deallocating by gc

extern void *Core;
extern void *Host;
extern void *Power;
extern void *Devices;

typedef struct _hcd_globals_t {
    void *core;
    void *host;
    void *power;
    void *devices;
} hcd_globals_t;

void rpi_usb_host_init(void) {
    UsbInitialise();
    MP_STATE_PORT(hcd_globals) = m_new(hcd_globals_t, 1);
    MP_STATE_PORT(hcd_globals)->core = Core;
    MP_STATE_PORT(hcd_globals)->host = Host;
    MP_STATE_PORT(hcd_globals)->power = Power;
    MP_STATE_PORT(hcd_globals)->devices = Devices;
}

void rpi_usb_host_process(void) {
    UsbCheckForChange();
}

void rpi_usb_host_deinit(void) {
    //    HcdStop();
    //    HcdDeinitialise();
    m_del(hcd_globals_t, MP_STATE_PORT(hcd_globals), 1);
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
	volatile u32* mailbox;
	u32 result;

	mailbox = (u32*)0x2000B880;
	while (mailbox[6] & 0x80000000);
	mailbox[8] = 0x80;
	do {
		while (mailbox[6] & 0x40000000);		
	} while (((result = mailbox[0]) & 0xf) != 0);
	return result == 0x80 ? OK : ErrorDevice;
}

void PowerOffUsb() {
}

void MicroDelay(u32 delay) {
    mp_hal_delay_us(delay);
}

