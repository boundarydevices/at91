#ifndef __APPMSGDIRECT_H__
#define __APPMSGDIRECT_H__ "$Id$"

/*
 * appMsgDirect.h
 *
 * This header file declares the data structures and routines
 * used to communicate over a single USB device to an Atmel
 * USB device as described in atmelUSB.pdf
 *
 * Note that this is just a placeholder until the kernel drivers
 * are ready to go.
 *
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#include "appIds.h"

struct appHeader_t {
	unsigned char  bd ;         // \xbd
	unsigned char  appId ;      // see appIds.h
	unsigned short length ;     // num bytes to follow
        
        // ... app message headers go here
};

inline void setAppHeader( appHeader_t &hdr, unsigned char appId, unsigned short length )
{
	hdr.bd = '\xbd' ;
	hdr.appId = appId ;
	hdr.length = length ;
}

inline bool validAppHeader( appHeader_t const &hdr, unsigned maxLength=0 ){
	return ('\xbd' == hdr.bd) 
		&& (NUM_USBAPPIDS > hdr.appId)
		&& ((0==maxLength)
		    ||
		    (maxLength >= hdr.length));
}

#endif

