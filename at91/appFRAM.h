#ifndef __APPFRAM_H__
#define __APPFRAM_H__ "$Id$"

/*
 * appFRAM.h
 *
 * This header file declares the FRAM application
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

extern void framInit( void );

extern void framUsbRx( void const *data,
		       unsigned    fragLength,
		       unsigned    bytesLeft ); // including fragment
/*
 * returns non-zero for success
 */
extern int readFRAM
   ( unsigned offs,
     unsigned char *data,
     unsigned length );

/*
 * returns non-zero for success
 */
extern int writeFRAM
   ( unsigned offs,
     unsigned char const *data,
     unsigned length );

#endif

