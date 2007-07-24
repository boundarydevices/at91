/*
 * Program usbEnum.cpp
 *
 * This program is a minimal usb peripheral program to test
 * bus enumeration.
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
#include "serialRegs.h"
#include "controlRx.h"
#include "usbPulldown.h"
#include "usb.h"
#include "messageRx.h"
#include "usbDesc.h"
#include "sambaClone.h"

char const * const appName_ = __FILE__ " " __DATE__ " " __TIME__ ;

static const char usbProd[] = {
    USB_STRING_DESCRIPTOR_SIZE(8),
    USB_STRING_DESCRIPTOR,
    USB_UNICODE('U'),
    USB_UNICODE('S'),
    USB_UNICODE('B'),
    USB_UNICODE(' '),
    USB_UNICODE('E'),
    USB_UNICODE('n'),
    USB_UNICODE('u'),
    USB_UNICODE('m'),
};

void init( void )
{
	enableUART(0,115200);
	enableUART(1,115200);

        setUsbProduct(usbProd);
	
	usbll_init();
	controlRxInit();
	serialRegsInit();

	usbWink();

	while(1){
		usbll_poll();
                serialRegsPoll();
	}
}
