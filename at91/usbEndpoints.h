#ifndef __USBENDPOINTS_H__
#define __USBENDPOINTS_H__ "$Id$"

/*
 * usbEndpoints.h
 *
 * This header file declares the USB endpoint constants.
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */


#define USBEP_CONTROL	0
#define USBEP_BULKOUT	1
#define USBEP_BULKIN	2
#define USBEP_INTERRUPT 3

#define USBMAX_CONTROL_DATA   8
#define USBMAX_BULK_DATA      64

#endif

