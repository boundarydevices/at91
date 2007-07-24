#ifndef __MEMORY_H__
#define __MEMORY_H__ "$Id$"

/*
 * memory.h
 *
 * This header file declares a variety of memory allocation
 * and access routines:
 *
 *	malloc()
 *	memset()
 *	memcpy()
 *
 * Note that there is no 'free()' routine (yet). Restart the
 * unit to free memory...
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

extern void *memset( void *ptr, int value, unsigned long size );
extern void *memcpy( void *dest, void const *src, unsigned long size );

// returns next chunk of RAM, 4-byte aligned
extern void *malloc( unsigned numBytes );

// return non-zero if the pointer is valid
extern int validHeapAddr( void const *p );

unsigned strlen( char const *s );

// copy NULL-terminated string, return pointer to end
char *stpcpy( char *dest, char const *src );

// stringify, return pointer to end
char *toDecimal( char *dest, unsigned long value );

// stringify, return pointer to end
char *toHex( char *dest, unsigned long value );

// return non-zero if valid ram pointer
#define isValidRamPtr(p) (((unsigned)p>=0x200000)&&((unsigned)p<0x210000))

#endif

