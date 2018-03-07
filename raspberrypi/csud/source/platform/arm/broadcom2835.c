/******************************************************************************
*	platform/arm/broadcom2835.c
*	 by Alex Chadwick
*
*	A light weight implementation of the USB protocol stack fit for a simple
*	driver.
*
*	platform/arm/broadcom2835.c contains code for the broadcom2835 chip, used 
*	in the Raspberry Pi. Compiled conditionally on LIB_BCM2835=1.
******************************************************************************/
#include <configuration.h>
#include <platform/platform.h>
#include <types.h>

void Bcm2835Load()
{
	LOG_DEBUG("CSUD: Broadcom2835 driver version 0.1.\n");
}

#ifndef TYPE_DRIVER

void MicroDelay(u32 delay) {
	volatile u64* timeStamp = (u64*)0x20003004;
	u64 stop = *timeStamp + delay;

	while (*timeStamp < stop) 
		__asm__("nop");
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

#endif