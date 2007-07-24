/*
 * Module messageRx.c
 *
 * This module defines the messageRxInit() routine
 * and internals of the messaging receive layer of
 * Atmel USB software.
 *
 * Refer to atmelUSB.pdf for a more detailed description.
 *
 *
 * Change History :
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#include "messageRx.h"
#include "sambaClone.h"
#include "assert.h"
#include "usart.h"
#include "usbEndpoints.h"
#include "usb_ll.h"
#include "appIds.h"
#include "message.h"
#include "memory.h"
// #define DEBUG
#include "debug.h"

typedef enum {
	startOfMessage,
	inSambaMsg,
	inAppMsg
} rxState ;

static rxState state = startOfMessage ;
static unsigned appId = USBAPP_SAMBA ;
static unsigned bytesLeft = 0 ;
static unsigned char bdMsgPre[sizeof(struct appHeader_t)];
static unsigned prevHeader = 0 ;

static appUsbRx_t appCallbacks_[NUM_USBAPPIDS] = {0};

static void rxCallback
	( void *opaque,
          unsigned epNum,
          unsigned char *data,
          unsigned length )
{
//	DEBUGMSG( "rxCallback\r\n" );
	assert( epNum == USBEP_BULKOUT );
	if( 0 == length ){
		if( inSambaMsg == state )
			state = startOfMessage ;
		return ;
	}

	if( startOfMessage == state ){
		assert( 0 == bytesLeft );
		if( 0 != prevHeader ){
                        struct appHeader_t header ;
			memcpy( &header, bdMsgPre, prevHeader );
			if( prevHeader+length >= sizeof(header) ){
				unsigned used = sizeof(header)-prevHeader ;
				length -= used ;
				data += used ;
				prevHeader = 0 ; // eat previous header
				state = inAppMsg ;
				bytesLeft = header.length ;
				appId = header.appId ;
				if( appId >= NUM_USBAPPIDS ){
					NODEBUGMSG( "Invalid appId in short msg 0x" ); NODEBUGHEX(appId); NODEBUGMSG("\r\n");
					state = startOfMessage ;
					length = 0 ;
				}
				DEBUGMSG( "rxApp: " ); DEBUGHEX( appId ); DEBUGMSG( ": " ); DEBUGHEX( bytesLeft ); DEBUGMSG( "\r\n" );
			}
			else {
				memcpy( bdMsgPre+prevHeader, data, length );
				prevHeader += length ;
			}
		}
		else if( BDAPP_TAG == *(unsigned char const *)data ){
			struct appHeader_t const *header = (struct appHeader_t const *)data ;
			if( sizeof(struct appHeader_t) <= length ){
				state = inAppMsg ;
				bytesLeft = header->length ;
				appId = header->appId ;
				assert( appId < NUM_USBAPPIDS );
				data = (unsigned char *)data + sizeof( struct appHeader_t );
				length -= sizeof( struct appHeader_t );
				DEBUGMSG( "rxApp: " ); DEBUGHEX( appId ); DEBUGMSG( ": " ); DEBUGHEX( bytesLeft ); DEBUGMSG( "\r\n" );
			}
			else {
				NODEBUGMSG( "short bdMsg 0x" ); NODEBUGHEX(length); NODEBUGMSG("\r\n");
				memcpy( bdMsgPre, data, length );
				prevHeader = length ;
			}
		}
		else {
			state = inSambaMsg ;
		}
	}

	if( inAppMsg == state ){
		// deliver fragment to application
                appUsbRx_t callback = appCallbacks_[appId];
		if( 0 != callback ){
DEBUGMSG( "appCallback: " ); DEBUGHEX( (unsigned long)callback ); DEBUGMSG( "\r\n" );
			callback( data, length, bytesLeft );
DEBUGMSG( "length 0x" ); DEBUGHEX( length ); DEBUGMSG( "\r\n" );
DEBUGMSG( "left 0x" ); DEBUGHEX( bytesLeft ); DEBUGMSG( "\r\n" );
		}
		
		assert( bytesLeft >= length );
		bytesLeft -= length ;
		if( 0 == bytesLeft )
			state = startOfMessage ;
	}
	else if( length ){
		sambaRx( data, length );
		if( length < endpointMaxSize[epNum] )
			state = startOfMessage ;
	} // samba message
}

static unsigned char bulkRxBuffer[64];

void messageRxInit( void )
{
   memset( appCallbacks_, 0, sizeof(appCallbacks_) );
   usbll_onRxData( USBEP_BULKOUT, rxCallback, bulkRxBuffer, bulkRxBuffer );
}

void setUsbCallback( unsigned appId, appUsbRx_t callback )
{
   if( appId < NUM_USBAPPIDS )
      appCallbacks_[appId] = callback ;
}

