#ifndef MICROPY_INCLUDED_RPI_VC_PROP_H
#define MICROPY_INCLUDED_RPI_VC_PROP_H

// mailbox request / response code
#define MB_PROP_REQUEST (0)
#define MB_PROP_SUCCESS (0x80000000)

// Subjects
#define PROP_FIRMWARE 0x00000000
#define PROP_HARDWARE 0x00010000
#define PROP_POWER    0x00020000
#define PROP_CLOCK    0x00030000
#define PROP_VOLTAGE  0x00030000
#define PROP_TEMP     0x00030000
#define PROP_GPU      0x00030000
#define PROP_FRAMEBUF 0x00040000
#define PROP_CMDLINE  0x00050000
#define PROP_DMACHAN  0x00060000

// Cursor Info (set only)
#define PROP_CRSRINFO 0x00008010
#define PROP_CRSRSTAT 0x00008011

// Commands
#define PROP_GET  0x00000000
#define PROP_TEST 0x00004000
#define PROP_SET  0x00008000

// Arguments
//   Firmware
#define PROP_VERSION  1

//   Hardware
#define PROP_REVISION 2
#define PROP_MACADDR  3
#define PROP_SERIAL   4
#define PROP_ARM_MEM  5
#define PROP_VC_MEM   6
#define PROP_ALLCLOCK 7

//   Power
#define PROP_STATE    1
#define PROP_TIMING   2

//   Clock
#define PROP_RATE     2
#define PROP_MAXRATE  4
#define PROP_MINRATE  7
#define PROP_TURBO    9

//   Voltage
#define PROP_CUR_V    3
#define PROP_MAX_V    5
#define PROP_MIN_V    8

//   GPU
#define PROP_CUR_TEMP 6
#define PROP_MAX_TEMP 10
#define PROP_ALLOCMEM 12
#define PROP_LOCKMEM  13
#define PROP_UNLOCKMEM 14
#define PROP_FREEMEM  15
#define PROP_DISPMANX 20
#define PROP_EDID     32

//   Frame buffer
#define PROP_BUFADDR  1
#define PROP_BLANK    2
#define PROP_PHYSSIZE 3
#define PROP_VIRTSIZE 4
#define PROP_BITDEPTH 5
#define PROP_PXLORDER 6
#define PROP_ALPHA    7
#define PROP_PITCH    8
#define PROP_OFFSET   9
#define PROP_OVERSCAN 10
#define PROP_PALETTE  11

int vc_property_length(unsigned int property);

#endif // MICROPY_INCLUDED_RPI_VC_PROP_H
