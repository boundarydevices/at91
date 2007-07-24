#ifndef __FRAM_H__
#define __FRAM_H__ "$Id$"

/*
 * fram.h
 *
 * This header file declares the constants and 
 * data structures used to communicate with the 
 * RamTron FM25L512 SPI FRAM device.
 *
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */


// opcodes
#define FRAM_OPCODE_WREN  0x06     // Set Write Enable Latch 
#define FRAM_OPCODE_WRDI  0x04     // Write Disable 
#define FRAM_OPCODE_RDSR  0x05     // Read Status Register 
#define FRAM_OPCODE_WRSR  0x01     // Write Status Register 
#define FRAM_OPCODE_READ  0x03     // Read Memory Data 
#define FRAM_OPCODE_WRITE 0x02     // Write Memory Data 

#define ETHERMAC_BYTES 6
#define FRAM_MEM_SIZE 32768

#endif

