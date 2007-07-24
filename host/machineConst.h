#ifndef __MACHINECONST_H__
#define __MACHINECONST_H__ "$Id$"

/*
 * machineConst.h
 *
 * This header file declares some constants specific to 
 * the SEFRAM board.
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#define PAGE_SIZE 256 
#define FLASH_BASE 0x00100000 
#define FLASH_PAGE_SIZE 256   	// 256K 
#define NUM_FLASH_PAGES 1024
#define FLASH_SIZE (FLASH_PAGE_SIZE*NUM_FLASH_PAGES)

#define SRAM_START 0x00200000
#define SRAM_SIZE  0x00010000

#endif

