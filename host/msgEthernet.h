#ifndef __MSGETHERNET_H__
#define __MSGETHERNET_H__ "$Id$"

/*
 * msgEthernet.h
 *
 * This header file declares the application-specific
 * message constants and structures for the ethernet
 * application.
 *
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#define MSGETHERNET_SETMAC	0
#define MSGETHERNET_GETMAC	1
#define MSGETHERNET_RECEIVE	2
#define MSGETHERNET_TRANSMIT	3
#define MSGETHERNET_TXSPACE	4

typedef struct {
   unsigned short msgType_ ;      // MSGETHERNET_X
   unsigned char  data_[2];       // message-specific data goes here...
} msgEthernetRequest_t ;

typedef struct {
	unsigned char msgType_ ;
	unsigned char result_ ; // 0 == success
	// message-specific data goes here if needed (GETMAC, RECEIVE)
} msgEthernetResponse_t ;

#endif

