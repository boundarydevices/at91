#ifndef __MSGFRAM_H__
#define __MSGFRAM_H__ "$Id$"

/*
 * msgFRAM.h
 *
 * This module defines the application-layer message
 * headers used for the FRAM application.
 *
 * Note that the same message header is used for both
 * received and sent messages, but that the transmit
 * (device to host) messages are divided into pieces
 * if the returned message is longer than a USB 
 * to prevent the need for a single chunk of RAM and 
 * the need for multi-segment USB messages (the messaging
 * layer only supports transmission of a single chunk
 * of data).
 *
 * As described in atmelUSB.pdf, the returned mess
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#define MSGFRAM_READ    0
#define MSGFRAM_WRITE   1

typedef struct {
   unsigned short msgType_ ;      // MSGFRAM_X
   unsigned short offs_ ;         // offset of read or write
   unsigned short length_ ;       // number of bytes to read or write
   
   // data goes here
} msgFramHeader_t ;

#endif

