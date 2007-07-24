#ifndef __APPIDS_H__
#define __APPIDS_H__ "$Id$"

/*
 * appIds.h
 *
 * This header file declares the application identifier
 * constants used by both the Host and Device side drivers
 * for the Atmel USB interface. 
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

#define USBAPP_SAMBA		0
#define USBAPP_ETHERNET		1
#define USBAPP_SERIAL		2
#define USBAPP_FRAM		3
#define USBAPP_GPIO		4
#define USBAPP_FLASH		5

#define NUM_USBAPPIDS		6

#define APPMASK_ALL   (1<<USBAPP_SAMBA) |          \
                      (1<<USBAPP_ETHERNET) |       \
                      (1<<USBAPP_SERIAL) |         \
                      (1<<USBAPP_FRAM) |           \
                      (1<<USBAPP_GPIO) |           \
                      (1<<USBAPP_FLASH)
#endif

