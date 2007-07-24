#ifndef __ADLER32_H__
#define __ADLER32_H__ "$Id$"

/*
 * adler32.h
 *
 * This header file declares the adler32() routine, which is a 
 * fast form of cyclic redundancy check.
 *
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

extern unsigned long adler32(unsigned long adler, const unsigned char *buf, unsigned len);

#endif

