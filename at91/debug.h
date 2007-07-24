#ifndef __DEBUG_H__
#define __DEBUG_H__ "$Id$"

/*
 * debug.h
 *
 * This header file declares the debug flag for use in 
 * tracing code execution.
 *
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

extern int volatile debug ;

#ifdef DEBUG
	#include "usart.h"
	#define DEBUGMSG( __s ) write( DEFUART, (__s) )
	#define DEBUGHEXCHAR( __c ) writeHexChar( DEFUART, (__c) )
	#define DEBUGHEX( __v ) writeHex( DEFUART, (__v) )
	#define DEBUGCHAR( __c ) writeChar( DEFUART, (__c) )
#else
	#define DEBUGMSG( __s )  
	#define DEBUGHEXCHAR( __c ) 
	#define DEBUGHEX( __v ) 
	#define DEBUGCHAR( __c ) 
#endif

#define NODEBUGMSG( __s ) write( DEFUART, (__s) )
#define NODEBUGHEXCHAR( __c ) writeHexChar( DEFUART, (__c) )
#define NODEBUGHEX( __v ) writeHex( DEFUART, (__v) )
#define NODEBUGCHAR( __c ) writeChar( DEFUART, (__c) )

#endif

