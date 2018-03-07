/******************************************************************************
*	platform/arm/armv6.c
*	 by Alex Chadwick
*
*	A light weight implementation of the USB protocol stack fit for a simple
*	driver.
*
*	platform/arm/armv6.c contains code for arm version 6 processors. Compiled
*	conditionally on LIB_ARM_V6=1.
******************************************************************************/
#include <configuration.h>
#include <platform/platform.h>
#include <types.h>

void Arm6Load()
{
	LOG_DEBUG("CSUD: ARMv6 driver version 0.1.\n");
}

#ifndef TYPE_DRIVER
u64 __aeabi_uidivmod(u32 value, u32 divisor) {
	u64 answer = 0;

	for (u32 i = 0; i < 32; i++) {
		if ((divisor << (31 - i)) >> (31 - i) == divisor) {
			if (value >= divisor << (31 - i)) {
				value -= divisor << (31 - i);
				answer |= 1 << (31 - i);
				if (value == 0) break;
			} 
		}
	}

	answer |= (u64)value << 32;
	return answer;
};

u32 __aeabi_uidiv(u32 value, u32 divisor) {
	return __aeabi_uidivmod(value, divisor);
};
#endif