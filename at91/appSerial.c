/*
 * Module appSerial.c
 *
 * This module defines the serial application 
 * entry points as declared in appSerial.h
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */


#include "appSerial.h"
#include "msgSerial.h"
#include "usart.h"
#include "usb_ll.h"
#include "usbEndpoints.h"
#include "appIds.h"
#include "assert.h"
#include "memory.h"
#include "leds.h"
// #define DEBUG
#include "debug.h"

#define RXBUFSIZE 64

//
// flags bitfields
//
#define RXQUEUED 1

typedef struct {
   unsigned short length_ ;
   unsigned short pad_ ;

   //
   // [[ msgSerialHeader_t ]]
   //
   unsigned char  uart_ ;         // 0, 1
   unsigned char  msgType_ ;      // MSGSERIAL_X
   unsigned char  data_[RXBUFSIZE];
   struct sg_t    sg_[2];
} rxBuf_t ;

//
// receives are double-buffered buffer+length pairs so one can be in transit 
// over USB while the other can be filled.
//
// UART0 occupies the first two slots in this array, followed by UART1
//
static rxBuf_t *rxBuf_[2*NUM_USARTS];

static unsigned const numRxBufs_ = sizeof(rxBuf_)/sizeof(rxBuf_[0]);

// these bytes define which of the two buffers data is being added to (!=0).
static unsigned char rxBufOdd_[NUM_USARTS];

// is something queued to the host?
static unsigned char rxQueued_[NUM_USARTS];

typedef struct {
   unsigned char add_ ;
   unsigned char take_ ;
   unsigned char data_[256];
} txBuf_t ;

//
// transmits are single-buffered and simply copy the 
// data into a circular transmit queue.
//

static txBuf_t *txBuf_[NUM_USARTS] = { 
   0, 0
};

void serialInit( void )
{
   unsigned char *buffers = (unsigned char *)malloc(numRxBufs_*sizeof(rxBuf_t));

   unsigned i ;
   for( i = 0 ; i < numRxBufs_ ; i++ ){
      rxBuf_t *buf = (rxBuf_t *)buffers ;
      memset( buf, 0, sizeof(*buf) );

      rxBuf_[i] = buf ;
DEBUGMSG( "serialRx[" ); DEBUGHEXCHAR( i ); 
    DEBUGMSG( "] == " ); DEBUGHEX( (unsigned)rxBuf_[i] ); DEBUGMSG( "\r\n" );
      buf->length_  = 0 ;
      buf->uart_    = (i/2);
      buf->msgType_ = MSGSERIAL_DATA ;
      buffers += sizeof(rxBuf_t);
      buf->sg_[0].data = &buf->uart_ ;
/*
      DEBUGMSG( "serialRx[" ); DEBUGHEXCHAR( i ); 
            DEBUGMSG( "] == " ); DEBUGHEX( (unsigned)rxBuf_[i] ); DEBUGMSG( "\r\n" );
*/            
   }

   buffers = (unsigned char *)malloc(NUM_USARTS*sizeof(txBuf_t));
   memset( buffers, 0xee, NUM_USARTS*sizeof(txBuf_t));
   for( i = 0 ; i < NUM_USARTS ; i++ ){
      txBuf_t *buf = txBuf_[i] = (txBuf_t *)buffers ;
      buf->add_ = buf->take_ = 0 ;
      buffers += sizeof(txBuf_t);
/*
      DEBUGMSG( "serialTx[" ); DEBUGHEXCHAR( i ); 
            DEBUGMSG( "] == " ); DEBUGHEX( (unsigned)buf ); DEBUGMSG( "\r\n" );
*/            
      rxBufOdd_[i] = rxQueued_[i] = 0 ;
   }
}

void serialUsbRx
	( void const *data,
	  unsigned    fragLength,
	  unsigned    bytesLeft )  // including fragment
{
//   DEBUGMSG( __FUNCTION__ ); DEBUGMSG( ":" ); DEBUGHEX( fragLength ); DEBUGMSG( "\r\n" );
   if( 2 <= fragLength ){
      msgSerialHeader_t const *msg = (msgSerialHeader_t const *)data ;
/*            
      DEBUGMSG( "uart 0x" ); DEBUGHEXCHAR( msg->uart_ ); 
      DEBUGMSG( "\r\n"
                      "msgType 0x" ); DEBUGHEXCHAR( msg->msgType_ ); 
      DEBUGMSG( "\r\n" );
*/      
      if( NUM_USARTS > msg->uart_ ){
         txBuf_t *txb = txBuf_[msg->uart_];
         unsigned char const *bytes = msg->data_ ;
         fragLength -= 2 ;
         while( 0 < fragLength-- ){
            txb->data_[txb->add_++] = *bytes++ ;
         }
      
         DEBUGMSG( "add 0x" ); DEBUGHEXCHAR( txb->add_ );
         DEBUGMSG( "\r\n"
                         "take 0x" ); DEBUGHEXCHAR( txb->take_ );
         DEBUGMSG( "\r\n" );
      }
      else {
         DEBUGMSG( __FUNCTION__ ); DEBUGMSG( ": invalid uart 0x" ); DEBUGHEXCHAR( msg->uart_ ); DEBUGMSG( "\r\n" );
      }
   }
   else {
      DEBUGMSG( __FUNCTION__ ); DEBUGMSG( ": invalid msg size 0x" ); DEBUGHEX( fragLength ); DEBUGMSG( "\r\n" );
   }
}

//
// called when received data has been sent to the host
//
static void serialUsbTxComplete
	( void *opaque,
          unsigned epNum,
	  struct sg_t *data )
{
   unsigned uart = (unsigned)opaque ;
   unsigned rxIndex = (2*uart)+rxBufOdd_[uart];
   rxBuf_t *next_rx = rxBuf_[rxIndex];
   rxBuf_t *prev_rx = rxBuf_[rxIndex^1];

   assert( NUM_USARTS > uart );
   assert( USBEP_BULKIN == epNum );

   prev_rx->length_ = 0 ;

   if( 0 != next_rx->length_ ){
      rxBufOdd_[uart] ^= 1 ;
      next_rx->sg_[0].length = next_rx->length_+sizeof(next_rx->uart_)+sizeof(next_rx->msgType_);
      usbll_transmit_app( USBEP_BULKIN, USBAPP_SERIAL, next_rx->sg_, serialUsbTxComplete, (void *)uart );
   } // more to send
   else
      rxQueued_[uart] = 0 ;
}

void serialPoll( void )
{
   unsigned i ;
   for( i = 0 ; i < NUM_USARTS ; i++ ){
      StructUSART *usart = usarts[i];
      txBuf_t *txb = txBuf_[i];
      
      unsigned short csr = usart->US_CSR ;
      while( ( 0 != (csr & US_TXRDY) )
             &&
             ( txb->add_ != txb->take_ ) )
      {
         usart->US_THR = txb->data_[txb->take_++];
         csr = usart->US_CSR ;
      }

      while( 0 != (csr & US_RXRDY ) ){
         unsigned char inChar = usart->US_RHR ;

         rxBuf_t *rx = rxBuf_[(2*i)+rxBufOdd_[i]];
         if(rx->length_ < RXBUFSIZE){
            rx->data_[rx->length_++] = inChar ;
// DEBUGHEXCHAR( inChar );
         }
         else
            DEBUGMSG( "rx overflow\r\n" );
         
         if( !rxQueued_[i] ){
            rxQueued_[i] = 1 ;
            rxBufOdd_[i] ^= 1 ;
// DEBUGMSG( "send " ); DEBUGHEX( rx->length_ ); DEBUGMSG( " bytes: " ); DEBUGHEXCHAR( rx->data_[0] ); DEBUGMSG( "\r\n" );
            rx->sg_[0].length = rx->length_+sizeof(rx->uart_)+sizeof(rx->msgType_);
            usbll_transmit_app( USBEP_BULKIN, USBAPP_SERIAL, rx->sg_, serialUsbTxComplete, (void *)i );
//	    rx = rxBuf_[(2*i)+rxBufOdd_[i]];
//DEBUGMSG( "next " ); DEBUGHEX( rx->length_ ); DEBUGMSG( " bytes: " ); DEBUGHEXCHAR( rx->data_[0] ); DEBUGMSG( "\r\n" );
         }

         csr = usart->US_CSR ;
      }
   }
}

