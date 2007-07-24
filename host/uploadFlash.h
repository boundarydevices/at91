#ifndef __UPLOADFLASH_H__
#define __UPLOADFLASH_H__ "$Id$"

/*
 * uploadFlash.h
 *
 * This header file declares the uploadFlash() routine,
 * which is used to upload a flash image one page at a 
 * time using the FLASH application interface.
 * 
 * Note that the running image on the SEFRAM must support 
 * the flash application.
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

bool uploadFlash( int fdUpload, unsigned long addr, char const *data, unsigned length );


#endif

