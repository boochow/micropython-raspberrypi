/******************************************************************************
*	configuration.c
*	 by Alex Chadwick
*
*	A light weight implementation of the USB protocol stack fit for a simple
*	driver.
*
*	configuration.c contains code to load all components. In order to
*	allow the actual source files to be completely independent, one file must
*	exist which depends upon all of them, to perform static initialisation.
*	Each separate 'library' provides a Load method, which ConfigurationLoad
*	simply invoeks all of.
******************************************************************************/

#include <usbd/device.h>
#include <usbd/descriptors.h>

// Add load methods for new modules wrapped in ifdefs here:
void UsbLoad();
void PlatformLoad();
#ifdef LIB_ARM_V6
void Arm6Load();
#endif
#ifdef LIB_BCM2835
void Bcm2835Load();
#endif
#ifdef LIB_DWC
void DwcLoad();
#endif
#ifdef LIB_HID
void HidLoad();
#endif
#ifdef LIB_HUB
void HubLoad();
#endif
#ifdef LIB_KBD
void KbdLoad();
#endif
#ifdef LIB_MOUSE
void MouseLoad();
#endif

void ConfigurationLoad() {
	// Call each libraries load method here. Watch out for the order, load more
	// generic libraries first.
	PlatformLoad();
#ifdef LIB_ARM_V6
	Arm6Load();
#endif
#ifdef LIB_BCM2835
	Bcm2835Load();
#endif
	UsbLoad();
#ifdef LIB_DWC
	DwcLoad();
#endif
#ifdef LIB_HID
	HidLoad();
#endif
#ifdef LIB_HUB
	HubLoad();
#endif
#ifdef LIB_KBD
	KbdLoad();
#endif
#ifdef LIB_MOUSE
	MouseLoad();
#endif
}