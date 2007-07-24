/*
 * Module usbPulldown.c
 *
 * This module defines the routines that control
 * the USB pull-down line.
 *
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#include "usbPulldown.h"
#include "hardware.h"

void usbDisable()
{
	PIOB->PIO_CODR = PIOB_USB_PULLUP ; // drive it low
	PIOB->PIO_OER = PIOB_USB_PULLUP ;  // set as output
	PIOB->PIO_PER = PIOB_USB_PULLUP ; // Let GPIO control it
}

//
// enable USB
//
void usbEnable()
{
	PIOB->PIO_SODR = PIOB_USB_PULLUP ; // drive it high
	PIOB->PIO_ODR = PIOB_USB_PULLUP ; // Now an input
	PIOB->PIO_PER = PIOB_USB_PULLUP ; // Let GPIO control it
}

void usbWink()
{
	usbDisable();
	usbEnable();
}

