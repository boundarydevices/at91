#ifndef __LEDS_H__
#define __LEDS_H__ "$Id$"

/*
 * leds.h
 *
 * This header file declares the setLED() routine, to 
 * set numeric values to the LED outputs on the AT91SAM7X-EK
 * eval board.
 * 
 * Note that only four LEDs are available, so only the low
 * 4-bits are displayed.
 *
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

void setLED(unsigned value);

#endif

