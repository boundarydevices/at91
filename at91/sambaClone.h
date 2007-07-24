#ifndef __SAMBACLONE_H__
#define __SAMBACLONE_H__ "$Id$"

/*
 * sambaClone.h
 *
 * This header file declares the interface to the 
 * samba clone application software, which provides
 * a subset of the functionality provided by the Atmel
 * boot loader.
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

extern char const * const appName_ ; // each app should define this

void samba_init( void );

void sambaRx
	( unsigned char const *data,
	  unsigned length );

#endif

