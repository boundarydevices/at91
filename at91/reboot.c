/*
 * Module reboot.c
 *
 * This module defines the reboot() routine as declared
 * in reboot.h
 *
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#include "hardware.h"
#include "reboot.h"
#define DEBUG
#include "debug.h"
#include "usbPulldown.h"
#include "delay.h"

void reboot( void ){
	DEBUGMSG( "rebooting...\r\n" );
	
	/* drive usb_reset (PB20) as an output/low */
	usbWink();

	/* Now reboot */
	RSTC->RSTC_MR = 0xA5000F00 ;
	RSTC->RSTC_CR = 0xA500000D ;
	DEBUGMSG( "wait for reboot...\r\n" );

	while(1){
		/* just in case */
		PIOB->PIO_OER = PIOB_USB_PULLUP ; // set as output
		PIOB->PIO_CODR = PIOB_USB_PULLUP ; // drive it low
		PIOB->PIO_PER = PIOB_USB_PULLUP ; // Now let GPIO control it
	}
}

