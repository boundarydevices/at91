#ifndef __ASSERT_H__
#define __ASSERT_H__ "$Id$"

/*
 * assert.h
 *
 * This header file declares the assert macro for 
 * the AT91 device code.
 *
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

extern void __assert_fail( char const *msg, 
			   char const *file,
			   int         line );

#define assert( expr ) \
  if( !(expr) ) __assert_fail( #expr, __FILE__, __LINE__ )

#endif

