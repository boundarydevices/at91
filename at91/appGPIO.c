/*
 * Module appGPIO.c
 *
 * This module defines the gpio application 
 * entry points as declared in appGPIO.h
 *
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */


#include "appGPIO.h"
#include "usart.h"
// #define DEBUG
#include "debug.h"

void gpioInit( void )
{
}

void gpioUsbRx
	( void const *data,
	  unsigned    fragLength,
	  unsigned    bytesLeft )  // including fragment
{
   DEBUGMSG( __FUNCTION__ ); DEBUGMSG( ":" ); DEBUGHEX( fragLength ); DEBUGMSG( "\r\n" );
}

