/*
 * Module assert.c
 *
 * This module defines the __assert_fail routine
 * as declared in assert.h
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#include "assert.h"
#include "usart.h"
#include "reboot.h"

void __assert_fail( char const *msg, 
 		   char const *file,
 		   int         line )
{
	write( DEFUART, "Assertion failed: " );
	write( DEFUART, msg );
	write( DEFUART, ":" );
	write( DEFUART, file );
	write( DEFUART, ": line " );
	writeHex( DEFUART, line );
        reboot();
}

