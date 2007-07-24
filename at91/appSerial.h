#ifndef __APPSERIAL_H__
#define __APPSERIAL_H__ "$Id$"

/*
 * appSerial.h
 *
 * This header file declares the serial-port application
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

extern void serialInit( void );
extern void serialUsbRx( void const *data,
		         unsigned    fragLength,
		         unsigned    bytesLeft ); // including fragment
extern void serialPoll( void );

#endif

