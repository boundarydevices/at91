#ifndef __USBPULLDOWN_H__
#define __USBPULLDOWN_H__ "$Id$"

/*
 * usbPulldown.h
 *
 * This header file declares routines to control
 * the USB pull-down line on the SEFRAM board.
 *
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

//
// disable, then re-enable USB to force the Host to reconnect
//
extern void usbWink();

//
// disable USB
//
extern void usbDisable();

//
// enable USB
//
extern void usbEnable();

#endif

