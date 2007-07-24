/*
 * Program usbFlash.cpp
 *
 * This program is a minimal usb peripheral
 * program which supports only the flash 
 * application. It initializes the UARTs as
 * well, but only so that error messages can
 * be displayed.
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#include "init.h"
#include "hardware.h"
#include "leds.h"
#include "usart.h"
#include "usb_ll.h"
#include "controlRx.h"
#include "sambaClone.h"
#include "messageRx.h"
#include "appFlash.h"
#include "appIds.h"
#include "usbPulldown.h"
#include "usb.h"

char const * const appName_ = __FILE__ " " __DATE__ " " __TIME__ ;

static const char usbProd[] = {
    USB_STRING_DESCRIPTOR_SIZE(9),
    USB_STRING_DESCRIPTOR,
    USB_UNICODE('U'),
    USB_UNICODE('S'),
    USB_UNICODE('B'),
    USB_UNICODE(' '),
    USB_UNICODE('F'),
    USB_UNICODE('l'),
    USB_UNICODE('a'),
    USB_UNICODE('s'),
    USB_UNICODE('h'),
};

void init( void )
{
        setUsbProduct(usbProd);
	
	usbWink();

	usbll_init();
	controlRxInit();
        messageRxInit();
	flashInit();
	samba_init();

	setUsbCallback(USBAPP_FLASH,flashUsbRx);

	while(1){
		usbll_poll();
	}
}
