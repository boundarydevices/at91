#ifndef __USB_LL_H__
#define __USB_LL_H__ "$Id$"

/*
 * usb_ll.h
 *
 * This header file declares the interface to the 
 * low-level, fragment-oriented interface to the 
 * Atmel USB Device Port.
 *
 * Refer to atmelUSB.pdf for a more complete description.
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#include "sg.h"

/*
 * Default handler ignores inbound data
 */
void usbll_init( void );

typedef void (*usbll_rx_callback_t)
	( void *opaque,
          unsigned epNum,
          unsigned char *data,
          unsigned length );

void usbll_onRxData
	( unsigned epNum,
	  usbll_rx_callback_t callback,
	  unsigned char *rxBuffer,
	  void *opaque );

typedef void (*usbll_setup_callback_t)
	( void *opaque,
	  unsigned char const *data,
	  unsigned length );

void usbll_onSetup
	( usbll_setup_callback_t callback,
	  void *opaque );

typedef void (*usbll_tx_callback_t)
	( void          *opaque,
          unsigned       epNum,
          struct sg_t   *data );

void usbll_transmit
	( unsigned epNum,
          struct sg_t *data,
	  usbll_tx_callback_t callback,
	  void *opaque );

/* prepends an application header */
void usbll_transmit_app
	( unsigned epNum,
	  unsigned char appId,
	  struct sg_t *data,
	  usbll_tx_callback_t callback,
	  void *opaque );

/* Halt an endpoint */
void usbll_halt( unsigned epNum );

/* Resume an endpoint */
void usbll_resume( unsigned epNum );

/* called by the main application to poll for events */
void usbll_poll( void );

#endif

														
