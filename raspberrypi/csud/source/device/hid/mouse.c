/******************************************************************************
*	device/mouse.c
*	 by Steve White 
*
*	A light weight implementation of the USB protocol stack fit for a simple
*	driver.
*
*	device/mouse.c contains code relating to USB hid mice. The 
*	driver maintains a list of the mice on the system, and allows the 
*	operating system to retrieve the status of each one separately. It is coded
*	a little awkwardly on purpose to make OS development more fun!
******************************************************************************/
#include <device/hid/hid.h>
#include <device/hid/mouse.h>
#include <device/hid/report.h>
#include <platform/platform.h>
#include <types.h>
#include <usbd/device.h>
#include <usbd/usbd.h>

#define MouseMaxMice 4

u32 mouseCount __attribute__((aligned(4))) = 0;
u32 mouseAddresses[MouseMaxMice] = { 0, 0, 0, 0 };
struct UsbDevice* mice[MouseMaxMice];

void MouseLoad() 
{
	LOG_DEBUG("CSUD: Mouse driver version 0.1\n"); 
	mouseCount = 0;
	for (u32 i = 0; i < MouseMaxMice; i++)
	{
		mouseAddresses[i] = 0;
		mice[i] = NULL;
	}
	HidUsageAttach[DesktopMouse] = MouseAttach;
}

u32 MouseIndex(u32 address) {
	if (address == 0) return 0xffffffff;

	for (u32 i = 0; i < mouseCount; i++)
		if (mouseAddresses[i] == address)
			return i;

	return 0xffffffff;
}

u32 MouseGetAddress(u32 index) {
	if (index > mouseCount) return 0;

	for (u32 i = 0; index >= 0 && i < MouseMaxMice; i++) {
		if (mouseAddresses[i] != 0)
			if (index-- == 0)
				return mouseAddresses[i];
	}

	return 0;
}

void MouseDetached(struct UsbDevice *device) {
	struct MouseDevice *data;
	
	data = (struct MouseDevice*)((struct HidDevice*)device->DriverData)->DriverData;
	if (data != NULL) {
		if (mouseAddresses[data->Index] == device->Number) {
			mouseAddresses[data->Index] = 0;
			mouseCount--;
			mice[data->Index] = NULL;
		}
	}
}

void MouseDeallocate(struct UsbDevice *device) {
	struct MouseDevice *data;
	
	data = (struct MouseDevice*)((struct HidDevice*)device->DriverData)->DriverData;
	if (data != NULL) {
		MemoryDeallocate(data);
		((struct HidDevice*)device->DriverData)->DriverData = NULL;
	}
	((struct HidDevice*)device->DriverData)->HidDeallocate = NULL;
	((struct HidDevice*)device->DriverData)->HidDetached = NULL;
}

Result MouseAttach(struct UsbDevice *device, u32 interface) {
	u32 mouseNumber;
	struct HidDevice *hidData;
	struct MouseDevice *data;
	struct HidParserResult *parse;

	if ((MouseMaxMice & 3) != 0) {
		LOG("MOUSE: Warning! MouseMaxMice not a multiple of 4. The driver wasn't built for this!\n");
	}
	if (mouseCount == MouseMaxMice) {
		LOGF("MOUSE: %s not connected. Too many mice connected (%d/%d). Change MouseMaxMice in device.mouse.c to allow more.\n", UsbGetDescription(device), mouseCount, MouseMaxMice);
		return ErrorIncompatible;
	}
	
	hidData = (struct HidDevice*)device->DriverData;
	if (hidData->Header.DeviceDriver != DeviceDriverHid) {
		LOGF("MOUSE: %s isn't a HID device. The mouse driver is built upon the HID driver.\n", UsbGetDescription(device));
		return ErrorIncompatible;
	}

	parse = hidData->ParserResult;
	if (parse->Application.Page != GenericDesktopControl ||
		parse->Application.Desktop != DesktopMouse) {
		LOGF("MOUSE: %s doesn't seem to be a mouse...\n", UsbGetDescription(device));
		return ErrorIncompatible;
	}
	if (parse->ReportCount < 1) {
		LOGF("MOUSE: %s doesn't have enough outputs to be a mouse.\n", UsbGetDescription(device));
		return ErrorIncompatible;
	}
	hidData->HidDetached = MouseDetached;
	hidData->HidDeallocate = MouseDeallocate;
	
	if ((hidData->DriverData = MemoryAllocate(sizeof(struct MouseDevice))) == NULL) {
		LOGF("MOUSE: Not enough memory to allocate mouse %s.\n", UsbGetDescription(device));
		return ErrorMemory;
	}
	data = (struct MouseDevice*)hidData->DriverData;
	data->Header.DeviceDriver = DeviceDriverMouse;
	data->Header.DataSize = sizeof(struct MouseDevice);
	data->Index = mouseNumber = 0xffffffff;
	for (u32 i = 0; i < MouseMaxMice; i++) {
		if (mouseAddresses[i] == 0) {
			data->Index = mouseNumber = i;
			mouseAddresses[i] = device->Number;
			mouseCount++;
			break;
		}
	}

	if (mouseNumber == 0xffffffff) {
		LOG("MOUSE: PANIC! Driver in inconsistent state! MouseCount is inaccurate.\n");
		MouseDeallocate(device);
		return ErrorGeneral;
	}

	mice[mouseNumber] = device;

	for (u32 i = 0; i < parse->ReportCount; i++) {
		if (parse->Report[i]->Type == Input 
//		    && data->MouseReport == NULL
		) {
			LOG_DEBUGF("MOUSE: Output report %d. %d fields.\n", i, parse->Report[i]->FieldCount);
			data->MouseReport = parse->Report[i];
		} 
	}

	data->mouseX = 0;
	data->mouseY = 0;
	data->wheel = 0;
	data->buttonState = 0;

	LOG_DEBUGF("MOUSE: New mouse assigned %d!\n", device->Number);

	return OK;
}

u32 MouseCount() {
	return mouseCount;
}

Result MousePoll(u32 mouseAddress) {
	u32 mouseNumber;
	Result result;
	struct MouseDevice *data;
	
	mouseNumber = MouseIndex(mouseAddress);	
	if (mouseNumber == 0xffffffff) return ErrorDisconnected;
	data = (struct MouseDevice*)((struct HidDevice*)mice[mouseNumber]->DriverData)->DriverData;
	if ((result = HidReadDevice(mice[mouseNumber], data->MouseReport->Index)) != OK) {
		if (result != ErrorDisconnected)
			LOGF("MOUSE: Could not get mouse report from %s.\n", UsbGetDescription(mice[mouseNumber]));
		return result;
	}

	struct HidParserReport *MouseReport = data->MouseReport;
	if (MouseReport->Type == Input) {
		// XXX: I'm sure I should be using HidGetFieldValue()
		// But this was so terribly easy.
		u8 *ReportBuffer = MouseReport->ReportBuffer;
		data->buttonState = (u8)ReportBuffer[0];
		data->mouseX += (s8)ReportBuffer[1];
		data->mouseY += (s8)ReportBuffer[2];
		data->wheel += (s8)ReportBuffer[3];
		if (data->mouseX < 0) {
			data->mouseX = 0;
		}
		if (data->mouseY < 0) {
			data->mouseY = 0;
		}
	}

	return OK;
}

s16 MouseGetPositionX(u32 mouseAddress) {
	u32 mouseNumber;
	struct MouseDevice *data;
	
	mouseNumber = MouseIndex(mouseAddress);
	if (mouseNumber == 0xffffffff) return 0;
	data = (struct MouseDevice*)((struct HidDevice*)mice[mouseNumber]->DriverData)->DriverData;
	return data->mouseX;
}

s16 MouseGetPositionY(u32 mouseAddress) {
	u32 mouseNumber;
	struct MouseDevice *data;
	
	mouseNumber = MouseIndex(mouseAddress);
	if (mouseNumber == 0xffffffff) return 0;
	data = (struct MouseDevice*)((struct HidDevice*)mice[mouseNumber]->DriverData)->DriverData;
	return data->mouseY;
}

s16 MouseGetWheel(u32 mouseAddress) {
	u32 mouseNumber;
	struct MouseDevice *data;
	
	mouseNumber = MouseIndex(mouseAddress);
	if (mouseNumber == 0xffffffff) return 0;
	data = (struct MouseDevice*)((struct HidDevice*)mice[mouseNumber]->DriverData)->DriverData;
	return data->wheel;
}

u32 MouseGetPosition(u32 mouseAddress) {
	u32 mouseNumber;
	struct MouseDevice *data;
	
	mouseNumber = MouseIndex(mouseAddress);
	if (mouseNumber == 0xffffffff) return 0;
	data = (struct MouseDevice*)((struct HidDevice*)mice[mouseNumber]->DriverData)->DriverData;
	return (data->mouseX << 16) | (data->mouseY & 0xFFFF);  /* x is high 16 bits; y is low 16 bits */
}

u8 MouseGetButtons(u32 mouseAddress) {
	u32 mouseNumber;
	struct MouseDevice *data;
	
	mouseNumber = MouseIndex(mouseAddress);
	if (mouseNumber == 0xffffffff) return 0;
	data = (struct MouseDevice*)((struct HidDevice*)mice[mouseNumber]->DriverData)->DriverData;
	return data->buttonState;
}

bool MouseGetButtonIsPressed(u32 mouseAddress, enum MouseDeviceButton button) {
	u32 mouseNumber;
	struct MouseDevice *data;
	
	mouseNumber = MouseIndex(mouseAddress);
	if (mouseNumber == 0xffffffff) return 0;
	data = (struct MouseDevice*)((struct HidDevice*)mice[mouseNumber]->DriverData)->DriverData;

	switch (button) {
		case MouseDeviceButtonLeft:
			return (data->buttonState & 0x01);
		case MouseDeviceButtonRight:
			return (data->buttonState & 0x02);
		case MouseDeviceButtonMiddle:
			return (data->buttonState & 0x04);
		case MouseDeviceButtonSide:
			return (data->buttonState & 0x08);
		case MouseDeviceButtonExtra:
			return (data->buttonState & 0x10);
	}
	return false;
}
