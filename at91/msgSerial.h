#ifndef __MSGSERIAL_H__
#define __MSGSERIAL_H__ "$Id$"

/*
 * Module msgSerial.h
 *
 * This module defines the application-layer message
 * headers used for the serial port application.
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#define MSGSERIAL_DATA        0
#define MSGSERIAL_SETTINGS    1

typedef struct {
   unsigned char uart_ ;         // 0, 1
   unsigned char msgType_ ;      // MSGSERIAL_X
   unsigned char data_[2];       // message-specific
} msgSerialHeader_t ;

#endif
