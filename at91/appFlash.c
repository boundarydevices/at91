/*
 * Module appFlash.c
 *
 * This module defines the flash application 
 * entry points as declared in appFlash.h
 *
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */


#include "appFlash.h"
#include "usart.h"
#include "msgFlash.h"
#include "flash.h"
#include "usbEndpoints.h"
#include "appIds.h"
#include "adler32.h"
#include "assert.h"
#include "memory.h"
#include "usb_ll.h"
// #define DEBUG
#include "debug.h"

enum state_t {
   INIT     = 0,		// expect message header
   MOREDATA = 1, 		// read remainder of command
   RESPONSE = 2
};

static enum state_t state = INIT ;

void flashInit( void )
{
}

static unsigned flashOffs_ = 0 ;
static unsigned char flashData_[FLASH_PAGE_SIZE];
static msgFlashResponse_t response_ ;
#define RESPONSEBYTES ((unsigned char *)&response_)
static struct sg_t sg[2] = {
	{ .data = (unsigned char *)&response_,
	  .length = sizeof(response_),
	  .offset = 0 }
,	{ .data = 0,
	  .length = 0,
	  .offset = 0 }
};
static unsigned bytesExpected = 0 ;
static unsigned bytesReceived = 0 ;

unsigned long notAdler32(unsigned long adler, const unsigned char *buf, unsigned len)
{
	while( len-- ){
		adler += *buf++ ;
	}
	return adler ;
}

static void flashUsbResponseComplete
	( void *opaque,
          unsigned epNum,
	  struct sg_t *data )
{
   DEBUGMSG( __FUNCTION__ ); DEBUGMSG( ":" ); 
	DEBUGHEX( state ); DEBUGMSG( "\r\n" );
   state = INIT ;
}

void flashUsbRx
	( void const *data,
	  unsigned    fragLength,
	  unsigned    bytesLeft )  // including fragment
{
   DEBUGMSG( __FUNCTION__ ); DEBUGMSG( ":" ); DEBUGHEX( fragLength ); 
	DEBUGMSG( ":" ); DEBUGHEX( bytesLeft ); DEBUGMSG( "\r\n" );
   switch( state ){
      case INIT:
      {
         if( sizeof(msgFlashRequest_t) <= fragLength ){
            msgFlashRequest_t const *msg = (msgFlashRequest_t const *)data ;
            response_.msgType_ = msg->msgType_ ;
   
            switch( msg->msgType_ ){
               case MSGFLASH_PROGRAM:
               {
                  assert(msg->length_ <= FLASH_PAGE_SIZE);
                  bytesExpected = bytesLeft-sizeof(*msg);
                  bytesReceived = fragLength-sizeof(*msg);
                  if( bytesExpected == bytesReceived ){
                     assert(msg->length_ == (fragLength-sizeof(*msg)));
                     int rval = flashProg( FLASH_BASE+msg->offs_, msg->length_, msg+1 );
                     response_.rval_ = rval ;
                     state = RESPONSE ;
                     usbll_transmit_app( USBEP_BULKIN, USBAPP_FLASH, sg, flashUsbResponseComplete, 0 );
                  }
                  else {
//                     DEBUGMSG( "rx0 " ); DEBUGHEX( bytesReceived ); DEBUGMSG( " of " ); DEBUGHEX( bytesExpected ); DEBUGMSG( "\r\n" );
                     flashOffs_ = msg->offs_ ;
                     memcpy(flashData_, msg+1, bytesReceived);
                     state = MOREDATA ;
                  }
   
                  break ;
               }
               case MSGFLASH_ADLER:
               {
                  int rval = notAdler32( 0, (unsigned char *)(FLASH_BASE+msg->offs_), msg->length_ );
                  response_.rval_ = rval ;
                  usbll_transmit_app( USBEP_BULKIN, USBAPP_FLASH, sg, flashUsbResponseComplete, 0 );
                  DEBUGMSG( "adler " ); DEBUGHEX( msg->offs_ ); 
                     DEBUGMSG( "/" ); DEBUGHEX( msg->length_ ); 
                     DEBUGMSG( " == 0x" ); DEBUGHEX( response_.rval_ ); DEBUGMSG( "\r\n" );
                  state = RESPONSE ;
                  break ;
               }
            }
         }
         else
            DEBUGMSG( "short flash msg\r\n" );
         break ;
      }
      case MOREDATA:
      {
         assert(fragLength <= bytesExpected-bytesReceived);
         memcpy(flashData_+bytesReceived, data, fragLength);
         bytesReceived += fragLength ;
//         DEBUGMSG( "rx1 " ); DEBUGHEX( bytesReceived ); DEBUGMSG( " of " ); DEBUGHEX( bytesExpected );  DEBUGMSG( "\r\n" );
         if( bytesReceived == bytesExpected ){
            response_.rval_ = flashProg( FLASH_BASE+flashOffs_, bytesReceived, flashData_ );
// DEBUGMSG( "writeFlash offs " ); DEBUGHEX( flashOffs_ ); DEBUGMSG( ", rval " ); DEBUGHEXCHAR( response_.rval_ ); DEBUGMSG( "\r\n" );
            state = RESPONSE ;
            usbll_transmit_app( USBEP_BULKIN, USBAPP_FLASH, sg, flashUsbResponseComplete, 0 );
         }
         break ;
      }
      case RESPONSE:
      {
         DEBUGMSG( "waiting for flash response\r\n" );
         break ;
      }
   }
}

