/*
 * Program usbPeriph.cpp
 *
 * This program is the first application for the SEFRAM
 * board. It is designed to be run from Flash (to allow
 * most RAM to be used for buffers) and provide access to
 * the Serial ports, Ethernet, FRAM, and GPIO over the USB
 * slave interface.
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#include "init.h"
#include "hardware.h"
#include "usart.h"
#include "usb_ll.h"
#include "sambaClone.h"
#include "messageRx.h"
#include "appSerial.h"
#include "appEthernet.h"
#include "appIds.h"
// #define DEBUG
#include "debug.h"
#include "appSerial.h"
#include "appEthernet.h"
#include "appFlash.h"
#include "appFRAM.h"
#include "appGPIO.h"
#include "controlRx.h"
#include "usbPulldown.h"
#include "usb.h"
#include "usbDesc.h"

char const * const appName_ = __FILE__ " " __DATE__ " " __TIME__ ;

static const char usbProd[] = {
    USB_STRING_DESCRIPTOR_SIZE(10),
    USB_STRING_DESCRIPTOR,
    USB_UNICODE('U'),
    USB_UNICODE('S'),
    USB_UNICODE('B'),
    USB_UNICODE(' '),
    USB_UNICODE('P'),
    USB_UNICODE('e'),
    USB_UNICODE('r'),
    USB_UNICODE('i'),
    USB_UNICODE('p'),
    USB_UNICODE('h'),
};

void init( void )
{
        setUsbProduct(usbProd);

	usbWink();
	usbll_init();

	controlRxInit();
        messageRxInit();
	framInit(); // must come before ethernet
	ethernetInit();
	flashInit();
	gpioInit();
	serialInit();

	setUsbCallback(USBAPP_ETHERNET,ethernetUsbRx);
	setUsbCallback(USBAPP_SERIAL,serialUsbRx);
	setUsbCallback(USBAPP_FRAM,framUsbRx);
	setUsbCallback(USBAPP_GPIO,gpioUsbRx);
	setUsbCallback(USBAPP_FLASH,flashUsbRx);

	samba_init();

	while(1){
		if( debug ){ 
			DEBUGMSG("before usb\r\n");
		}
		usbll_poll();
		if( debug ){ 
			DEBUGMSG("before serial\r\n");
		}
                serialPoll();
		if( debug ){ 
			DEBUGMSG("before ethernet\r\n");
		}
                ethernetPoll();
		if( debug ){ 
			DEBUGMSG("loop 0x"); DEBUGHEX(i); DEBUGMSG("\r\n");
		}
	}
}
