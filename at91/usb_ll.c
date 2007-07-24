/*
 * Module usb_ll.c
 *
 * This module defines the low-level interfaces to the
 * USB device port on the Atmel device as declared in 
 * usb_ll.h and described in atmelUSB.pdf
 *
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#include "usb_ll.h"
#include "hardware.h"
#include "appIds.h"
#include "usart.h"
#include "assert.h"
#include "memory.h"
#include "message.h"
// #define DEBUG
#include "debug.h"
#include "reboot.h"
#include "delay.h"
#include "usb.h"
#include "usbPulldown.h"
#include "usbEndpoints.h"

struct transmitRequest_t {
	unsigned 		  flags ;	// NEEDAPPID and appid in low bits
        struct sg_t              *txStart ;
        struct sg_t              *txNext ;
        usbll_tx_callback_t 	  callback ;
	void			 *opaque ;
};

#define NEEDAPPID	0x80000
#define NEEDNULLTX	0x40000
#define APPID_MASK 7
#define MAXTX_ENTRIES 16

// transmitter states
#define EPTX_IDLE 0
#define EPTX_WAITCOMP 1

struct endpointDetails_t {
	usbll_rx_callback_t	   rx_callback ;
        unsigned char             *rx_data ;
	void *			   rx_opaque ;
	unsigned char		   pingPong ;
	struct transmitRequest_t   txQueue[MAXTX_ENTRIES+1]; // circular buffer to prevent need for heap
	unsigned		   txAdd ;
	unsigned		   txTake ;
	unsigned char              txState ;
};

static struct endpointDetails_t connections[UDP_MAXENDPOINTS];
static usbll_setup_callback_t setupCallback = 0 ;
void  *setupOpaque = 0 ;

void usbll_init( void )
{
        memset( connections, 0, sizeof(connections) );
}

void usbll_onRxData
	( unsigned epNum,
	  usbll_rx_callback_t callback,
	  unsigned char *rxBuffer,
	  void *opaque )
{
	struct endpointDetails_t *ep = &connections[epNum];
	ep->rx_callback = callback ;
	ep->rx_data = rxBuffer ;
	ep->rx_opaque = opaque ;
}

static unsigned fillOutput(unsigned epNum, struct transmitRequest_t *tx)
{
	if( !isValidRamPtr(tx) ){
		write( DEFUART, "Invalid tx ptr: 0x" ); writeHex( DEFUART, (unsigned long)tx ); write( DEFUART, "\r\n" );
		reboot();
	}
	assert( tx->txNext ); // or how did we get here?

	//
	// 2 cases here:
	//	start of new message - check flags and send app id if necessary
	//	continuing in same message - send data
	//
	// The second is performed in either case
	//
	unsigned max = endpointMaxSize[epNum];
	if( ( tx->txStart == tx->txNext )
	    &&
	    ( 0 == tx->txStart->offset )
	    &&
	    ( tx->flags & NEEDAPPID ) ){
		unsigned i ;
		struct appHeader_t header ;
		tx->flags &= ~NEEDAPPID ;
		unsigned char const *next = (unsigned char *)&header ;
		header.bd = '\xbd' ;
		header.appId = tx->flags & APPID_MASK ;
		header.length = sg_len(tx->txStart);
                unsigned char volatile *const fifo = (unsigned char *)&UDP->UDP_FDR[epNum];
		for( i = 0 ; i < sizeof(header); i++ ){
			*fifo = *next++ ;
		} // send header
		max -= sizeof(header);
		DEBUGMSG( "appId 0x" ); DEBUGHEXCHAR( header.appId ); DEBUGMSG( ", len 0x" ); DEBUGHEX(header.length); DEBUGMSG("\r\n" );
	}

	unsigned total = 0 ;
	while( ( 0 < max ) && !SG_DONE(tx->txNext) ){
		struct sg_t *sg = tx->txNext ;
		unsigned char *data = sg->data + sg->offset ;
		unsigned i ;
		unsigned len = SG_LEFT(sg);
		if(len>max)
			len = max ;
                unsigned char volatile *const fifo = (unsigned char *)&UDP->UDP_FDR[epNum];
		for( i = 0 ; i < len ; i++ )
			*fifo = *data++ ;
		sg->offset += len ;
		max -= len ;
                total += len ;
		if( SG_DONE(sg) ){
			tx->txNext++ ;
			if(SG_END(tx->txNext))
				break ;
		} // more space for data
	}

	return total ;
}

static void startTransmit(unsigned epNum, struct transmitRequest_t *tx)
{
	unsigned total = fillOutput(epNum,tx);
	if( 0 == ( UDP->UDP_CSR[epNum] & AT91C_UDP_TXPKTRDY ) ){
                UDP_SETEPFLAGS(UDP->UDP_CSR+epNum, AT91C_UDP_TXPKTRDY );
		connections[epNum].txState = EPTX_WAITCOMP ;
		if( USBEP_BULKIN == epNum )
			total += fillOutput(epNum,tx);
if( 0 )// != epNum )
{
	NODEBUGMSG( "extra 0x" ); NODEBUGHEX( total ); NODEBUGMSG( "\r\n" );
}
	}
	else {
		write( DEFUART, "PKTRDY already set for ep 0x" ); writeHexChar( DEFUART, epNum ); write( DEFUART, "\r\n" );
	}

	DEBUGMSG( "~txPacket: 0x" ); DEBUGHEX(tx->txNext->length-tx->txNext->offset); DEBUGMSG( "\r\n" );
}

static void continueTransmit(unsigned epNum, struct transmitRequest_t *tx)
{
	unsigned total ;
	if( USBEP_BULKIN != epNum )
		total = fillOutput(epNum,tx);
	else
		total = 0 ;

	if( 0 == ( UDP->UDP_CSR[epNum] & AT91C_UDP_TXPKTRDY ) ){
                UDP_SETEPFLAGS(UDP->UDP_CSR+epNum, AT91C_UDP_TXPKTRDY );
		connections[epNum].txState = EPTX_WAITCOMP ;
                if( USBEP_BULKIN == epNum )
			total = fillOutput(epNum,tx);
if( 0 )// != epNum )
{
	NODEBUGMSG( "extra 0x" ); NODEBUGHEX( total ); NODEBUGMSG( "\r\n" );
}
	}
	else {
		write( DEFUART, "PKTRDY already set for ep 0x" ); writeHexChar( DEFUART, epNum ); write( DEFUART, "\r\n" );
	}

	DEBUGMSG( "~txPacket: 0x" ); DEBUGHEX(tx->txNext->length-tx->txNext->offset); DEBUGMSG( "\r\n" );
}

void usbll_transmit
	( unsigned epNum,
	  struct sg_t *data,
	  usbll_tx_callback_t callback,
	  void *opaque )
{
        struct endpointDetails_t *const ep = &connections[epNum];
	int first = ( ep->txAdd == ep->txTake );
	unsigned next = ep->txAdd + 1 ;
	if( next > MAXTX_ENTRIES )
		next = 0 ;
	if( next != ep->txTake ){
                struct transmitRequest_t *tx = ep->txQueue + ep->txAdd ;
		tx->flags = 0 ;
                tx->txStart = tx->txNext = data ;
		tx->callback = callback ;
		tx->opaque   = opaque ;
		ep->txAdd = next ;
		sg_zero_offs(data);
		if( USBEP_BULKIN == epNum ){
                        tx->flags |= NEEDNULLTX ;
			unsigned const len = sg_len(tx->txStart);
			if( 0 == (len&63) ){
				
			}
		}
		if( first )
			startTransmit(epNum, tx);
	}
	else
                DEBUGMSG( "Transmit queue overflow\r\n" );
}


/* prepends an application header */
void usbll_transmit_app
	( unsigned epNum,
	  unsigned char appId,
	  struct sg_t *data,
	  usbll_tx_callback_t callback,
	  void *opaque )
{
        struct endpointDetails_t *const ep = &connections[epNum];
	int first = ( ep->txAdd == ep->txTake );
	unsigned next = ep->txAdd + 1 ;

	assert( 0 != data ); // tx logic doesn't handle this (header doesn't get sent)
//        assert( 0 != data->length );

	if( next > MAXTX_ENTRIES )
		next = 0 ;
	if( next != ep->txTake ){
                struct transmitRequest_t *tx = ep->txQueue + ep->txAdd ;
		tx->flags = NEEDAPPID | (appId&APPID_MASK);
		tx->txStart = tx->txNext = data ;
		tx->callback = callback ;
		tx->opaque   = opaque ;
		ep->txAdd = next ;
                sg_zero_offs(data);
		if( USBEP_BULKIN == epNum ){
				tx->flags |= NEEDNULLTX ;
			unsigned const len = sg_len(tx->txStart);
			if( 0 == (len&63) ){
			}
		}
		if( first )
			startTransmit(epNum, tx);
		else {
			DEBUGMSG( "second transmit 0x" ); DEBUGHEX(sg_len(tx->txStart)); DEBUGMSG( " bytes\r\n" );
		}
	}
	else
                DEBUGMSG( "Transmit queue overflow\r\n" );
}

static void dump_sg(struct sg_t *sg )
{
	write( DEFUART, "data " ); writeHex( DEFUART, (unsigned)sg->data );
	write( DEFUART, ", len 0x" ); writeHex( DEFUART, sg->length );
        write( DEFUART, ", offs 0x" ); writeHex( DEFUART, sg->offset );
        write( DEFUART, "\r\n" );
}

void dumpEP(unsigned epNum){
	struct endpointDetails_t const *ep = connections+epNum ;
	write( DEFUART, "------> ep details\r\n" );
	write( DEFUART, "cb 0x" ); writeHex( DEFUART, (unsigned)ep->rx_callback ); write( DEFUART, "\r\n" );
	write( DEFUART, "dat 0x" ); writeHex( DEFUART, (unsigned)ep->rx_data ); write( DEFUART, "\r\n" );
	write( DEFUART, "rxop 0x" ); writeHex( DEFUART, (unsigned)ep->rx_opaque ); write( DEFUART, "\r\n" );
	write( DEFUART, "ping 0x" ); writeHexChar( DEFUART, ep->pingPong ); write( DEFUART, "\r\n" );
	write( DEFUART, "txAdd 0x" ); writeHexChar( DEFUART, ep->txAdd ); write( DEFUART, "\r\n" );
	write( DEFUART, "txTake 0x" ); writeHexChar( DEFUART, ep->txTake ); write( DEFUART, "\r\n" );
	write( DEFUART, "txState 0x" ); writeHexChar( DEFUART, ep->txState ); write( DEFUART, "\r\n" );

	unsigned i = ep->txTake ;
	while( i != ep->txAdd ){
		write( DEFUART, "[" ); writeHexChar( DEFUART, i ); write( DEFUART, "] == \n   " );
		struct transmitRequest_t const *tx = ep->txQueue+i ;
		write( DEFUART, "flags 0x" ); writeHexChar( DEFUART, tx->flags ); write( DEFUART, "\r\n   " );
		write( DEFUART, "start 0x" ); writeHex( DEFUART, (unsigned)tx->txStart ); write( DEFUART, "\r\n   " );
		if( tx->txStart ){ dump_sg( tx->txStart ); }
		write( DEFUART, "next 0x" ); writeHex( DEFUART, (unsigned)tx->txNext ); write( DEFUART, "\r\n   " );
		if( tx->txNext && ( tx->txNext != tx->txStart ) ){ dump_sg( tx->txStart ); }
		write( DEFUART, "cb 0x" ); writeHex( DEFUART, (unsigned)tx->callback ); write( DEFUART, "\r\n   " );
		write( DEFUART, "op 0x" ); writeHex( DEFUART, (unsigned)tx->opaque ); write( DEFUART, "\r\n" );
		
		i++ ;
		if( i >= MAXTX_ENTRIES ){
			i = 0 ;
		}
	}
}

void usbll_halt( unsigned bEndpoint )
{
	UDP_SETEPFLAGS(UDP->UDP_CSR + bEndpoint, AT91C_UDP_FORCESTALL);
}

void usbll_resume( unsigned bEndpoint )
{
	UDP_CLEAREPFLAGS(UDP->UDP_CSR + bEndpoint, AT91C_UDP_FORCESTALL);
	
	// Reset Endpoint Fifos, beware this is a 2 steps operation
	UDP->UDP_RSTEP |= 1 << bEndpoint;
	UDP->UDP_RSTEP &= ~(1 << bEndpoint);
}

void usbll_onSetup
	( usbll_setup_callback_t callback,
	  void *opaque )
{
	setupOpaque = opaque ;
	setupCallback = callback ;
}

static unsigned prevISR = 0 ;

static void completeTransmit(unsigned epNum)
{
	struct endpointDetails_t *const ep = &connections[epNum];
	if( ep->txState != EPTX_IDLE ){
		while(ep->txTake != ep->txAdd){
			struct transmitRequest_t *tx = ep->txQueue + ep->txTake ;
			ep->txTake++ ;
			if( ep->txTake > MAXTX_ENTRIES )
				ep->txTake = 0 ;
                        usbll_tx_callback_t callback = tx->callback ;
			if( callback )
				callback( tx->opaque, epNum, tx->txStart );
			memset( tx, 0, sizeof(*tx) ); // kill pointers
		}
		ep->txState = EPTX_IDLE ;
	}
}

static void abortTransmitter(unsigned epNum)
{
	struct endpointDetails_t *const ep = &connections[epNum];
	while(ep->txTake != ep->txAdd){
		struct transmitRequest_t *tx = ep->txQueue + ep->txTake ;
		ep->txTake++ ;
		if( ep->txTake > MAXTX_ENTRIES )
			ep->txTake = 0 ;
		usbll_tx_callback_t callback = tx->callback ;
		if( callback )
			callback( tx->opaque, epNum, tx->txStart );
		memset( tx, 0, sizeof(*tx) ); // kill pointers
	}
	ep->txState = EPTX_IDLE ;
	UDP_CLEAREPFLAGS(UDP->UDP_CSR+epNum, AT91C_UDP_TXCOMP|AT91C_UDP_TXPKTRDY);
}

static void abortTransmitters()
{
        unsigned epNum ;
	for( epNum = 0 ; epNum < UDP_MAXENDPOINTS ; epNum++ ){
		abortTransmitter(epNum);
	}
}

static void transmitComplete( unsigned char epNum )
{
if( 0 )// != epNum )
	NODEBUGMSG( "TXCOMP\r\n" );
	struct endpointDetails_t *const ep = &connections[epNum];
	struct transmitRequest_t *tx = (ep->txTake != ep->txAdd)  ? ep->txQueue + ep->txTake : 0 ;
	struct transmitRequest_t *firstTx = tx ;
	int completed = 0 ;

	ep->txState = EPTX_IDLE ;

	if( tx && tx->txNext && SG_END(tx->txNext) ){
		if( tx->flags & NEEDNULLTX ){
			DEBUGMSG( "NULL tx\r\n" );
                        tx->flags &= ~NEEDNULLTX ;
		}
		else {
			if( tx->callback )
				tx->callback( tx->opaque, epNum, tx->txStart );
			memset( tx, 0, sizeof(*tx) ); // kill pointers
			ep->txTake++ ;
			if( ep->txTake > MAXTX_ENTRIES )
				ep->txTake = 0 ;
		}
	} // done with previous request

	tx = (ep->txTake != ep->txAdd) ? ep->txQueue + ep->txTake : 0 ;
	if( 0 ){ // tx
		DEBUGMSG( "multi-tx 0x" ); DEBUGHEX( sg_len(tx->txStart) ); DEBUGMSG( " bytes\r\n" );
		dump_sg( tx->txStart );
	}

	if( tx ){
		if( firstTx == tx )
			continueTransmit(epNum,tx);
		else {
                        UDP_CLEAREPFLAGS(UDP->UDP_CSR+epNum, AT91C_UDP_TXCOMP);
			DEBUGMSG( "nextTx 0x" ); DEBUGHEX( sg_len(tx->txStart) ); DEBUGMSG("\r\n");
			startTransmit(epNum,tx);
                        completed = 1 ;
		}
	}
	else {
		assert(ep->txTake == ep->txAdd);
	}

	if( !completed )
		UDP_CLEAREPFLAGS(UDP->UDP_CSR+epNum, AT91C_UDP_TXCOMP);
}

static void rxData(unsigned char epNum, unsigned rxMask)
{
	DEBUGMSG( "rxMask " ); DEBUGHEX( rxMask ); DEBUGMSG( "\r\n" );
	unsigned readyCount = (0 != (rxMask & AT91C_UDP_RX_DATA_BK0) );
	readyCount += ( 0 != (rxMask & AT91C_UDP_RX_DATA_BK1) );
	
	struct endpointDetails_t *const ep = &connections[epNum];
	if( 2 == readyCount ){
		rxMask = ep->pingPong ^ 0x42 ;
		if( 0 == rxMask ){
			UDP_CLEAREPFLAGS( UDP->UDP_CSR+epNum, rxMask );
			DEBUGMSG( "double rx on first go-round\r\n" );
			return ;
		}
	}
	ep->pingPong = rxMask ; // keep track of last
	
	unsigned short rxLength = ( UDP->UDP_CSR[epNum] >> 16 ) & 0x7FF ;
	if( ep->rx_data ){
		assert( endpointMaxSize[epNum] >= rxLength );
		unsigned i ;
		unsigned char volatile *const fifo = (unsigned char *)&UDP->UDP_FDR[epNum];
		for( i = 0 ; i < rxLength ; i++ ){
			ep->rx_data[i] = *fifo ;
		}
		assert( ep->rx_callback );
		ep->rx_callback( ep->rx_opaque, epNum, ep->rx_data, rxLength );
	} // read if somebody's listening
	
	UDP_CLEAREPFLAGS( UDP->UDP_CSR+epNum, rxMask );
}

unsigned long savedCtx[] = {
	0		// CSR0
,	0		// ISR
,	0		// GLBSTATE
,	0		// FADDR
};

/* called by the main application to poll for events */
void usbll_poll( void )
{
	int epNum ;
	unsigned gstate ;
	unsigned isr = UDP->UDP_ISR ;
	if( isr != prevISR ){
		prevISR = isr ;
	}
	
	if( isr & AT91C_UDP_EPINT0 ){
		UDP->UDP_ICR = AT91C_UDP_EPINT0 ;
	}

	if( isr & AT91C_UDP_WAKEUP ){
		DEBUGMSG( "UDP_WAKEUP\r\n" );
		UDP->UDP_ICR = AT91C_UDP_WAKEUP ;
	}

	if( isr & AT91C_UDP_RXRSM ){
		DEBUGMSG( "UDP_RXRSM\r\n" );
		UDP->UDP_ICR = AT91C_UDP_RXRSM ;
	}

	if( isr & AT91C_UDP_EXTRSM ){
		DEBUGMSG( "UDP_EXTRSM\r\n" );
		UDP->UDP_ICR = AT91C_UDP_EXTRSM ;
	}

	if( isr & AT91C_UDP_SOFINT ){
//		DEBUGMSG( "UDP_SOFINT\r\n" );
		UDP->UDP_ICR = AT91C_UDP_SOFINT ;
	}

	if( isr & AT91C_UDP_ENDBUSRES ){
//		write( DEFUART, "GLBSTATE == " ); writeHex( DEFUART, UDP->UDP_GLBSTATE ); write( DEFUART, "\r\n" );
		UDP->UDP_FADDR = AT91C_UDP_FEN ;
		UDP_CLEAREPFLAGS(&UDP->UDP_GLBSTATE,AT91C_UDP_CONFG|AT91C_UDP_FADDEN);
		UDP->UDP_RSTEP = 0xffffffff ;
		UDP->UDP_RSTEP = 0 ;
		UDP_SETEPFLAGS(UDP->UDP_CSR + 0, AT91C_UDP_EPEDS);
                UDP_CLEAREPFLAGS(UDP->UDP_CSR + 0, AT91C_UDP_DIR);
		UDP->UDP_ICR = AT91C_UDP_ENDBUSRES ;
		abortTransmitters();
//		write( DEFUART, "UDP_ENDBUSRES 0x" );writeHex( DEFUART, UDP->UDP_GLBSTATE ); write( DEFUART, "\r\n" );
	}

	gstate = UDP->UDP_GLBSTATE; 

	for( epNum = 0 ; epNum < UDP_MAXENDPOINTS ; epNum++ ){
		unsigned int dStatus = UDP->UDP_CSR[epNum];

		if( dStatus & AT91C_UDP_TXCOMP ){
			transmitComplete(epNum);
		} // transmit completed

		if( dStatus & AT91C_UDP_RXSETUP ){
			unsigned char setupBuf[16];
			unsigned short rxLength = ( dStatus >> 16 ) & 0x7FF ;
			if( sizeof(struct S_usb_request) <= rxLength ){
                                struct S_usb_request const *rxSetup = (struct S_usb_request const *)setupBuf ;
                                rxLength = sizeof(*rxSetup);
				unsigned i ;
                                unsigned char volatile *const fifo = (unsigned char *)&UDP->UDP_FDR[epNum];
				for( i = 0 ; i < rxLength ; i++ ){
					setupBuf[i] = *fifo ;
				}
				if( rxSetup->bmRequestType & 0x80) {
					UDP_SETEPFLAGS(UDP->UDP_CSR + epNum, AT91C_UDP_DIR);
				}

				UDP_CLEAREPFLAGS(UDP->UDP_CSR + epNum, AT91C_UDP_RXSETUP);

				if( 0 != (UDP->UDP_CSR[epNum] & (AT91C_UDP_TXPKTRDY|AT91C_UDP_TXCOMP|AT91C_UDP_STALLSENT)) ){
					write( DEFUART, "rxSetup: txPending\r\n" );
				}

				if( setupCallback ){
					completeTransmit(epNum);
					setupCallback( setupOpaque, setupBuf, rxLength );
				}
			}
			else {
				write( DEFUART, "rxSetup too small 0x" ); writeHexChar( DEFUART, rxLength ); write( DEFUART, "\r\n" );
				UDP_CLEAREPFLAGS(UDP->UDP_CSR + epNum, AT91C_UDP_RXSETUP);
			}
		}
		
		if( dStatus & AT91C_UDP_STALLSENT ){
			UDP_CLEAREPFLAGS(UDP->UDP_CSR + epNum, AT91C_UDP_STALLSENT);
		}

		unsigned int rxMask = dStatus & (AT91C_UDP_RX_DATA_BK0|AT91C_UDP_RX_DATA_BK1);
		if( rxMask ){
			rxData(epNum,rxMask);
		} // received something
	}
}


void clearEpFlags(unsigned volatile *reg, unsigned flags )
{
	// clear timer
        while( 0 != RTT->RTT_VALUE )
		RTT->RTT_MR = RTT_MR_RTTRST ;

	*reg &= ~flags ;
        while( ( 0 == RTT->RTT_VALUE ) && (*reg & flags) )
		;
	if( *reg & flags ){
		write( DEFUART, "Error clearing flags 0x" ); writeHex( DEFUART, flags ); 
			write( DEFUART, " from reg " ); writeHex( DEFUART, (unsigned)reg ); 
			write( DEFUART, "\r\n" );
		reboot();
	}
}

void setEpFlags(unsigned volatile *reg, unsigned flags )
{
	// clear timer
        while( 0 != RTT->RTT_VALUE )
		RTT->RTT_MR = RTT_MR_RTTRST ;

	*reg |= flags ;
        while( ( 0 == RTT->RTT_VALUE ) && (0 == (*reg & flags)) )
		*reg |= flags ;
	if( 0 == (*reg & flags) ){
		write( DEFUART, "Error setting flags 0x" ); writeHex( DEFUART, flags ); 
			write( DEFUART, " from reg " ); writeHex( DEFUART, (unsigned)reg ); 
			write( DEFUART, "  value 0x" ); writeHex( DEFUART, *reg ); 
			write( DEFUART, "\r\n" );
		reboot();
	}
}

