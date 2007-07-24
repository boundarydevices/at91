#ifndef __USBDESC_H__
#define __USBDESC_H__ "$Id$"

/*
 * usbDesc.h
 *
 * This header file declares the USB descriptor data structures
 * for SEFRAM applications.
 *
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#ifndef _USB_H
#include "usb.h"
#endif

extern struct S_usb_device_descriptor_s const sDeviceDescriptor ;
extern struct S_core_configuration_descriptor const sConfigurationDescriptor ;

// application can call this to override the "product" id.
extern void setUsbProduct( char const *appName );

#define NUMUSB_STRINGS 5
extern char const *usbStringDescriptors[NUMUSB_STRINGS];

#endif

