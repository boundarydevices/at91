/*
 * Program usbFlash.cpp
 *
 * This program is a minimal usb peripheral program which supports 
 * only the FRAM application for use in testing the FRAM app without
 * re-flashing.
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
#include "sambaClone.h"
#include "messageRx.h"
#include "appSerial.h"
#include "appFRAM.h"
#include "appIds.h"

void init( void )
{
	int i ;
        
	enableUART(0,115200);
	enableUART(1,115200);

	usbll_init();
        messageRxInit();
	framInit();
	samba_init();

	setUsbCallback(USBAPP_FRAM,framUsbRx);

	while(1){
		setLED(i++ >> 10);
		
		usbll_poll();
	}
}
