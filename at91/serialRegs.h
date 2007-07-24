#ifndef __SERIALREGS_H__
#define __SERIALREGS_H__ "$Id$"

/*
 * serialRegs.h
 *
 * This header file declares an initialization routine
 * and a poller that provide register/memory read-write
 * command line over the default serial port.
 *
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

extern void serialRegsInit( void );
extern void serialRegsPoll( void );

#endif

