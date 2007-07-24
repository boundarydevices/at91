#ifndef __MSGFLASH_H__
#define __MSGFLASH_H__ "$Id$"

/*
 * msgFlash.h
 *
 * This header file declares the message format for
 * the Atmel Flash application.
 *
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#define MSGFLASH_PROGRAM     0
#define MSGFLASH_ADLER       1

typedef struct {
   unsigned      msgType_ ;      // MSGFLASH_X
   unsigned      offs_ ;         // flash address offset [0..128K-1]
   unsigned      length_ ;
   // data here if programming
} msgFlashRequest_t ;

typedef struct {
   unsigned msgType_ ;
   unsigned rval_ ;              // response code (0==success) or adler value
} msgFlashResponse_t ;

#endif

