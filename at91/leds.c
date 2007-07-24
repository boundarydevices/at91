/*
 * Module leds.cpp
 *
 * This module defines the setLED() routine as
 * declared in leds.h
 *
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#include "leds.h"
#include "hardware.h"

static unsigned const swap_[] = {
	0x00
,	0x08
,	0x04
,	0x0c
,	0x02
,	0x0a
,	0x06
,	0x0e
,	0x01
,	0x09
,	0x05
,	0x0d
,	0x03
,	0x0b
,	0x07
,	0x0f
};

void setLED( unsigned value )
{
	unsigned bits = swap_[value&15];
	bits <<= PIOB_LEDSHIFT ;
	bits &= PIOB_LEDMASK ;
	PIOB->PIO_CODR = bits ; // light 'em up
	bits = (~bits)&PIOB_LEDMASK ;
	PIOB->PIO_SODR = bits ; // shut 'em down
}
