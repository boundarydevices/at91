#ifndef __CONTROLRX_H__
#define __CONTROLRX_H__ "$Id$"

/*
 * controlRx.h
 *
 * This header file declares the controlRxInit() routine,
 * which initializes the receive handler for the control
 * endpoint.
 *
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

void controlRxInit(void);

void controlRxCallback
	( void *opaque,
          unsigned epNum,
          unsigned char *data,
          unsigned length );

#endif

