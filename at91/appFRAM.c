/*
 * Module appFRAM.c
 *
 * This module defines the fram application 
 * entry points as declared in appFRAM.h
 *
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */


#include "appFRAM.h"
#include "usart.h"
#include "hardware.h"
#include "fram.h"
#include "msgFRAM.h"
#include "message.h"
#include "memory.h"
#include "usbEndpoints.h"
#include "appIds.h"
#include "assert.h"
// #define DEBUG
#include "debug.h"
#include "usb_ll.h"

enum state_t {
   INIT     = 0,     // expect message header
   MOREDATA = 1,     // read remainder of command
   RESPONSE = 2
};

static enum state_t state = INIT ;

static msgFramHeader_t *response_ = 0 ;
static struct sg_t sg[2] = {
   { .data = 0,
     .length = 0,
     .offset = 0 }
,  { .data = 0,
     .length = 0,
     .offset = 0 }
};

#define MAX_RESPONSE_DATA (USBMAX_BULK_DATA-sizeof(struct appHeader_t)-sizeof(msgFramHeader_t))
static unsigned bytesExpected = 0 ;
static unsigned bytesReceived = 0 ;

void framInit( void )
{
   unsigned responseBytes ;
   SPI0->SP_CR = SP_SWRST ;
   SPI0->SP_MR = 0x06000011 ;     /* DelayBCS 6, PCS 0, Master, Mode Fault Disable, no loopback */
   SPI0->SP_CSR[0] = 0x0A0AFF02 ; /* DLYBCT=1, DLYBS=1, SCBR=0x01, BITS=8, CSAAT=0, SPI mode 0 (CPHA=0,CPOL=0) */
   SPI0->SP_CR = SP_SPIEN ;
   SPI0DMA->pdmaPTCR = 0x0202 ;   // disable transmit and receive periph dma
   
   responseBytes = sizeof(msgFramHeader_t)+MAX_RESPONSE_DATA;
   response_ = (msgFramHeader_t *)malloc(responseBytes);
   memset( response_, 0xaa, responseBytes );
   sg->data = (unsigned char *)response_ ;
}

//
// issue "Write Enable" command, return non-zero if successful
//
static int writeEnable( void )
{
   int rval = 0 ;
   if( SPI0->SP_SR & SP_ENDTX ){
      int sr ;
      unsigned long cmd = FRAM_OPCODE_WREN ;
      unsigned long response = 0 ;

      SPI0DMA->pdmaRPR = (unsigned long)&response ;
      SPI0DMA->pdmaRCR = sizeof(response);
      SPI0DMA->pdmaTPR = (unsigned long)&cmd ;
      SPI0DMA->pdmaTCR = sizeof(cmd);
      SPI0DMA->pdmaPTCR = 0x0101 ;   // enable transmit and receive periph dma
      while( 0 == ( (sr = SPI0->SP_SR) & SP_ENDRX ) )
         ;

      DEBUGMSG( "writeEnab resp 0x" ); DEBUGHEX( response ); DEBUGMSG( "\r\n" );
      rval = 1 ;
   }
   else 
      DEBUGMSG( "writeEnable tx not empty\r\n" );
   
   SPI0DMA->pdmaPTCR = 0x0202 ;   // disable transmit and receive periph dma
   
   return rval ;
}

static unsigned char readwrite_[USBMAX_BULK_DATA];

/*
 * write data, return non-zero for success
 */
int writeFRAM
   ( unsigned offs,
     unsigned char const *data,
     unsigned length )
{
   int rval = writeEnable();
   if( rval && ( 0 != (SPI0->SP_SR & SP_ENDTX ) ) ){
      int sr ;
      unsigned long cmd = FRAM_OPCODE_WRITE | (offs&0xFF00) | ((offs&0xff)<<16);

DEBUGMSG( "writing " ); DEBUGHEX( length ); DEBUGMSG( " bytes at 0x" ); DEBUGHEX(offs); DEBUGMSG("\r\n");

memset( readwrite_, 0xdd, sizeof(readwrite_));
      memcpy(readwrite_, &cmd, 3 );
      memcpy(readwrite_+3, data, length );

      SPI0DMA->pdmaRPR = (unsigned long)(readwrite_);
      SPI0DMA->pdmaRCR = length+3 ;
      SPI0DMA->pdmaTPR = (unsigned long)readwrite_ ;
      SPI0DMA->pdmaTCR = length+3 ;
      SPI0DMA->pdmaPTCR = 0x0101 ;   // enable transmit and receive periph dma

      while( 0 == ( (sr = SPI0->SP_SR) & SP_ENDRX ) )
         ;

      rval = 1 ;
      DEBUGMSG( "wrote data. 1st byte 0x" ); DEBUGHEXCHAR( data[0] ); DEBUGMSG( "\r\n" );
   }
   else 
      DEBUGMSG( "writeData tx not empty\r\n" );

   SPI0DMA->pdmaPTCR = 0x0202 ;   // disable transmit and receive periph dma
   
   return rval ;
}

/*
 * read data, return non-zero for success
 */
int readFRAM
   ( unsigned offs,
     unsigned char *data,
     unsigned length )
{
   DEBUGMSG( __FUNCTION__ ); DEBUGHEX( offs ); DEBUGCHAR('/'); DEBUGHEX(length); DEBUGMSG("\r\n");
   memset( readwrite_, 0xcc, sizeof(readwrite_));
   memset( data, 0x55, length );
   int rval = 0 ;
   if( SPI0->SP_SR & SP_ENDTX ){
      int sr ;
      unsigned long cmd = FRAM_OPCODE_READ | (offs&0xFF00) | ((offs&0xff)<<16);

      memcpy(readwrite_,&cmd, sizeof(cmd));

      SPI0DMA->pdmaRPR = (unsigned long)(readwrite_+sizeof(cmd));
      SPI0DMA->pdmaRCR = length+sizeof(cmd);
      SPI0DMA->pdmaTPR = (unsigned long)readwrite_ ;
      SPI0DMA->pdmaTCR = length+sizeof(cmd);
      SPI0DMA->pdmaPTCR = 0x0101 ;   // enable transmit and receive periph dma

      while( 0 == ( (sr = SPI0->SP_SR) & SP_ENDRX ) )
         ;

      DEBUGMSG( "readData resp 0x" ); DEBUGHEX( *((unsigned long *)(readwrite_+sizeof(cmd))) ); DEBUGMSG( "\r\n" );
      DEBUGMSG( ".." ); DEBUGHEX( *((unsigned long *)(readwrite_+sizeof(cmd)+4)) ); DEBUGMSG( "\r\n" );
      memcpy( data, readwrite_+sizeof(cmd)+3, length);
      rval = 1 ;
   }
   else 
      DEBUGMSG( "readData tx not empty\r\n" );
   
   SPI0DMA->pdmaPTCR = 0x0202 ;   // disable transmit and receive periph dma
   
   DEBUGMSG( __FUNCTION__ ); DEBUGHEX( rval ); DEBUGMSG("\r\n");
   return rval ;
}

static void framUsbResponseComplete
   ( void *opaque,
     unsigned epNum,
     struct sg_t *data )
{
   if( RESPONSE == state ){
      DEBUGMSG( "sent response for req 0x" ); DEBUGHEX( response_->msgType_ ); DEBUGMSG( "\r\n" );
      state = INIT ;
      if( 12 == response_->msgType_ )
         debug = 1 ;
   } else if( MOREDATA == state ){
      unsigned numToRead = bytesExpected-bytesReceived ;
      assert( bytesReceived < bytesExpected );
      response_->offs_ += response_->length_ ;
      response_->length_ = 0 ;

      numToRead = numToRead > MAX_RESPONSE_DATA ? MAX_RESPONSE_DATA : numToRead ;
      if( readFRAM( response_->offs_, (unsigned char *)(response_+1), numToRead ) ){
         unsigned respBytes = sizeof(*response_)+numToRead ;
         response_->length_ = numToRead ;
         bytesReceived += numToRead ;
         DEBUGMSG( "sending 0x" ); DEBUGHEX( respBytes ); DEBUGMSG( " bytes of response\r\n" );
         DEBUGMSG( "first char 0x" ); DEBUGHEXCHAR( *((unsigned char*)(response_+1)) ); DEBUGMSG( "\r\n" );
         sg->length = respBytes ;
         usbll_transmit_app( USBEP_BULKIN, USBAPP_FRAM, sg, framUsbResponseComplete, 0 );
         if( bytesExpected == bytesReceived ){
            state = RESPONSE ;
         } // done reading data
         else {
            DEBUGMSG( "rx0 " ); DEBUGHEX( bytesReceived ); DEBUGMSG( " of " ); DEBUGHEX( bytesExpected ); DEBUGMSG( "\r\n" );
         }
      } // read success
      else {
         state = RESPONSE ;
         sg->length = sizeof(*response_);
         usbll_transmit_app( USBEP_BULKIN, USBAPP_FRAM, sg, framUsbResponseComplete, 0 );
      } // read errror
   }
   else {
      DEBUGMSG( "usbtxComplete in invalid state: " ); DEBUGHEX( state ); DEBUGMSG( "\r\n" );
      state = INIT ;
   }
}


void framUsbRx
   ( void const *data,
     unsigned    fragLength,
     unsigned    bytesLeft )  // including fragment
{
   DEBUGMSG( __FUNCTION__ ); DEBUGMSG( ":" ); DEBUGHEX( fragLength ); 
   DEBUGMSG( ":" ); DEBUGHEX( bytesLeft ); DEBUGMSG( "\r\n" );
   switch( state ){
      case INIT:
      {
         if( sizeof(msgFramHeader_t) <= fragLength ){
            msgFramHeader_t const *hdr = (msgFramHeader_t const *)data ;
            response_->msgType_ = hdr->msgType_ ;
            response_->offs_ = hdr->offs_ ;
            response_->length_ = 0 ; // filled in on completion

            switch( hdr->msgType_ ){
               case MSGFRAM_WRITE:
               {
                  assert(hdr->offs_+hdr->length_ <= (FRAM_MEM_SIZE-ETHERMAC_BYTES));

                  bytesExpected = bytesLeft-sizeof(*hdr);
                  bytesReceived = fragLength-sizeof(*hdr);

                  if( writeFRAM( ETHERMAC_BYTES+hdr->offs_, (unsigned char *)(hdr+1), bytesReceived ) ){
                     if( bytesExpected == bytesReceived ){
                        if( hdr->length_ != (fragLength-sizeof(*hdr)) ){
                           DEBUGMSG( "fragLength mismatch: " );
                           DEBUGHEX( fragLength ); 
                           DEBUGMSG( ":" ); DEBUGHEX( hdr->length_ );
      //          assert(hdr->length_ == (fragLength-sizeof(*hdr)));
                        }
                        response_->length_ = hdr->length_ ;
                        state = RESPONSE ;
                        sg->length = sizeof(*response_);
                        usbll_transmit_app( USBEP_BULKIN, USBAPP_FRAM, sg, framUsbResponseComplete, 0 );
                        DEBUGMSG( "wrote " ); DEBUGHEX( bytesReceived ); DEBUGMSG( " of " ); DEBUGHEX( bytesExpected ); DEBUGMSG( "\r\n" );
                     } // done writing data
                     else {
                        DEBUGMSG( "wx0 " ); DEBUGHEX( bytesReceived ); DEBUGMSG( " of " ); DEBUGHEX( bytesExpected ); DEBUGMSG( "\r\n" );
                        state = MOREDATA ;
                     }
                  } // write successful
                  else {
                     state = RESPONSE ;
                     sg->length = sizeof(*response_);
                     usbll_transmit_app( USBEP_BULKIN, USBAPP_FRAM, sg, framUsbResponseComplete, 0 );
                  } // write error

                  break ;
               } // WRITE
               case MSGFRAM_READ:
               {
                  assert(hdr->offs_+hdr->length_ <= (FRAM_MEM_SIZE-ETHERMAC_BYTES));

                  assert(sizeof(*hdr) == fragLength);
                  assert(bytesLeft==fragLength);

                  bytesExpected = hdr->length_ ;
                  bytesReceived = bytesExpected > MAX_RESPONSE_DATA ? MAX_RESPONSE_DATA : bytesExpected ;

                  if( readFRAM( ETHERMAC_BYTES+hdr->offs_, (unsigned char *)(response_+1), bytesReceived ) ){
                     unsigned respBytes = sizeof(*response_)+bytesReceived ;
                     response_->length_ = bytesReceived ;
                     DEBUGMSG( "sending 0x" ); DEBUGHEX( respBytes ); DEBUGMSG( " bytes of response\r\n" );
                     DEBUGMSG( "first char 0x" ); DEBUGHEXCHAR( *((unsigned char*)(response_+1)) ); DEBUGMSG( "\r\n" );
                     sg->length = respBytes ;
                     usbll_transmit_app( USBEP_BULKIN, USBAPP_FRAM, sg, framUsbResponseComplete, 0 );
                     if( bytesExpected == bytesReceived ){
                        state = RESPONSE ;
                     } // done reading data
                     else {
                        DEBUGMSG( "rx0 " ); DEBUGHEX( bytesReceived ); DEBUGMSG( " of " ); DEBUGHEX( bytesExpected ); DEBUGMSG( "\r\n" );
                        state = MOREDATA ;
                     }
                  } // read success
                  else {
                     state = RESPONSE ;
                     sg->length = sizeof(*response_);
                     usbll_transmit_app( USBEP_BULKIN, USBAPP_FRAM, sg, framUsbResponseComplete, 0 );
                  } // read errror

                  break ;
               } // READ
               default: {
                     state = RESPONSE ;
                     DEBUGMSG( "unknown FRAM msg: " ); DEBUGHEX( hdr->msgType_ ); DEBUGMSG( "\r\n" );
                     sg->length = sizeof(*response_);
                     usbll_transmit_app( USBEP_BULKIN, USBAPP_FRAM, sg, framUsbResponseComplete, 0 );
               }
            }
         }
         else
            DEBUGMSG( "short FRAM msg\r\n" );
         break ;
      } // INIT (command) state

      case MOREDATA:
      {
         if( MSGFRAM_WRITE == response_->msgType_ ){
            unsigned expected = bytesExpected-bytesReceived ;
            if( expected >= fragLength ){
               if( writeFRAM( response_->offs_+bytesReceived, data, fragLength ) ){
                  bytesReceived += fragLength ;
DEBUGMSG( "wrote " ); DEBUGHEX( bytesReceived ); DEBUGMSG( " of " ); DEBUGHEX( bytesExpected ); DEBUGMSG( " bytes\r\n" );
                  if( bytesExpected == bytesReceived ){
                     response_->length_ = bytesExpected ;
                     state = RESPONSE ;
DEBUGMSG( "transmitting response: 0x" ); 
   DEBUGHEX( sizeof(*response_) ); DEBUGMSG( ", 0x" ); 
   DEBUGHEX( response_->length_ ); DEBUGMSG( "\r\n" ); 
                     sg->length = sizeof(*response_);
                     usbll_transmit_app( USBEP_BULKIN, USBAPP_FRAM, sg, framUsbResponseComplete, 0 );
                  } // done writing data
                  else {
                     DEBUGMSG( "wx1 " ); DEBUGHEX( bytesReceived ); DEBUGMSG( " of " ); DEBUGHEX( bytesExpected ); DEBUGMSG( "\r\n" );
                  } // still more
               } // write successful
               else {
                  DEBUGMSG( "FRAM write failure after " ); DEBUGHEX( bytesReceived ); DEBUGMSG( " of " ); DEBUGHEX( bytesExpected ); DEBUGMSG( "\r\n" );
                  state = RESPONSE ;
                  sg->length = sizeof(*response_);
                  usbll_transmit_app( USBEP_BULKIN, USBAPP_FRAM, sg, framUsbResponseComplete, 0 );
               } // write error
            }
            else {
               DEBUGMSG( "FRAM write overflow: expecting " );
                  DEBUGHEX( expected ); DEBUGMSG( ", got " );
                  DEBUGHEX( fragLength ); DEBUGMSG( "\r\n" );
            }
         } // in FRAM_WRITE
         else {
            DEBUGMSG( "follow-up fragment during msgType 0x" ); DEBUGHEXCHAR( response_->msgType_ ); DEBUGMSG( "\r\n" );
         }
         break ;
      } // MOREDATA 
      case RESPONSE:
      {
         DEBUGMSG( "waiting for FRAM response\r\n" );
         break ;
      }
   }
}

