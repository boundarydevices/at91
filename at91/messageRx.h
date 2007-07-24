#ifndef __MESSAGERX_H__
#define __MESSAGERX_H__ "$Id$"

/*
 * messageRx.h
 *
 * This header file declares an initialization routine
 * for the messaging layer of the Atmel USB application
 * software.
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

void messageRxInit(void);

typedef void (*appUsbRx_t)
	( void const *data,
	  unsigned    fragLength,
	  unsigned    bytesLeft ); // bytes left in message including fragment

void setUsbCallback( unsigned appId, appUsbRx_t callback );

#endif

