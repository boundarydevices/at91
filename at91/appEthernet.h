#ifndef __APPETHERNET_H__
#define __APPETHERNET_H__ "$Id$"

/*
 * appEthernet.h
 *
 * This header file declares the Ethernet application
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

extern void ethernetInit( void );

extern void ethernetUsbRx( void const *data,
		           unsigned    fragLength,
			   unsigned    bytesLeft ); // including fragment

extern void ethernetPoll( void );

#endif

