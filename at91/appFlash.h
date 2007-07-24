#ifndef __APPFLASH_H__
#define __APPFLASH_H__ "$Id$"

/*
 * appFlash.h
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

extern void flashInit( void );
extern void flashUsbRx( void const *data,
			unsigned    fragLength,
			unsigned    bytesLeft ); // including fragment

#endif

