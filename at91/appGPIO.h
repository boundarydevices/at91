#ifndef __APPGPIO_H__
#define __APPGPIO_H__ "$Id$"

/*
 * appGPIO.h
 *
 * This header file declares the GPIO application
 * callbacks.
 *
 * Refer to atmelUSB.pdf for a more detailed description.
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

extern void gpioInit( void );
extern void gpioUsbRx( void const *data,
		       unsigned    fragLength,
		       unsigned    bytesLeft ); // including fragment


#endif

