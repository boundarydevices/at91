#ifndef __FLASH_H__
#define __FLASH_H__ "$Id$"

/*
 * flash.h
 *
 * This header file declares constants and data structures
 * used to interface with the embedded flash controller of 
 * AT91SAM7X devices.
 *
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#define EFC_COMMAND_WRITEPAGE          0x01
#define EFC_COMMAND_SETLOCKBIT         0x02
#define EFC_COMMAND_WRITEPAGEANDLOCK   0x03
#define EFC_COMMAND_CLEARLOCKBIT       0x04
#define EFC_COMMAND_ERASEALL           0x08
#define EFC_COMMAND_SETNVM             0x0B
#define EFC_COMMAND_CLEARNVM           0x0D
#define EFC_COMMAND_SETSECURITY        0x0F

// These routines return zero to indicate success,
// non-zero to indicate an error.
//
// They each expect addresses in the range of 
// [FLASH_BASE..FLASH_BASE+FLASH_SIZE-1]
//

// only works on aligned full pages
extern int flashProg( unsigned addr, unsigned length, void const *data );
extern int flashVerify( unsigned addr, unsigned length, void const *data );

#endif

