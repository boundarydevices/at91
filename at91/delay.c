/*
 * Module delay.c
 *
 * This module defines ...
 *
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */


#include "delay.h"
#include "hardware.h"
#include "usart.h"

void delay( unsigned secs )
{
        while( 0 != RTT->RTT_VALUE )
		RTT->RTT_MR = RTT_MR_RTTRST ;

	while( secs > RTT->RTT_VALUE )
		;
}
