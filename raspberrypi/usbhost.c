#include <stdio.h>
#include <string.h>
#include "usbd/usbd.h"

#include "py/runtime.h"
#include "mphalport.h"
#include "bcm283x_mailbox.h"
#include "vc_property.h"
#include "usbhost.h"

/* USB host controller driver static variables */
// memory block for these variables are allocated using
// m_malloc() so we must keep them in MP object
// to avoid them from deallocating by gc

extern void *Core;
extern void *Host;
extern void *Power;
extern void **Devices;
extern void *databuffer;
extern void **keyboards;
extern void **mice;
extern u32 RootHubDeviceNumber;


extern hcd_globals_t *hcd_globals;
static u32 usb_initialised = 0;

void* MemoryAllocate(u32 length);
void PowerOffUsb();

void rpi_usb_host_init(void) {
    if (usb_initialised == 0) {
        MP_STATE_PORT(hcd_globals) = MemoryAllocate(sizeof(hcd_globals_t));
        UsbInitialise();
        MP_STATE_PORT(hcd_globals)->core = Core;
        MP_STATE_PORT(hcd_globals)->host = Host;
        MP_STATE_PORT(hcd_globals)->power = Power;
        for (int i = 0; i < 32; i++) {
            MP_STATE_PORT(hcd_globals)->devices[i] = &Devices[i];
        }
        MP_STATE_PORT(hcd_globals)->databuffer = databuffer;
        for (int i = 0; i < 4; i++) {
            MP_STATE_PORT(hcd_globals)->keyboards[i] = &keyboards[i];
            MP_STATE_PORT(hcd_globals)->mice[i] = &mice[i];
        }

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
        databuffer = NULL;
        RootHubDeviceNumber = 0;
        PowerOffUsb();
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

#define MB_SET_POWER_STATE (0x00028001)
#define POWER_USBHCD       (3)

static Result power_usb(bool onoff){
	u32 result;
    __attribute__((aligned(16))) u32 msg[] = {
        0x20,               // message length is 32 bytes
        0,                  // this is a request
        MB_SET_POWER_STATE, // set power state tag
        8,                  // the value length is 8 bytes
        0,                  // this is a request
        POWER_USBHCD,       // device id
        3,                  // state value;bit0 = on, bit1=wait
        0,                  // end tag
    };

    if (onoff) {
        msg[6] = 3;
    } else {
        msg[6] = 2;
    }
    mailbox_write(MB_CH_PROP_ARM, (uint32_t) BUSADDR(msg) >> 4);
    mailbox_read(MB_CH_PROP_ARM);

    if ((msg[1] == MB_PROP_SUCCESS) && (msg[4] & MB_PROP_SUCCESS)) {
        result = OK;
    } else {
        result = ErrorDevice;
    }
    return result;
}

Result PowerOnUsb() {
    return power_usb(true);
}

void PowerOffUsb() {
    power_usb(false);
}

void MicroDelay(u32 delay) {
    mp_hal_delay_us(delay);
}
