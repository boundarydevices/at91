/*
 * Module appEthernet.c
 *
 * This module defines the ethernet application entry points 
 * as declared in appEthernet.h
 *
 * Operation consists mainly of the message-specific flows:
 *
 ***	GETMAC/SETMAC each require little state. The requests
 *	are simply processed and the results returned.
 *
 ***	TRANSMIT requests are initiated by the host, and this
 *	module keeps track of the next buffer to write. Responses 
 *	are made upon completion of the receipt of the USB message.
 *	The primary error response is 'no buffer available' (1).
 *	The code path is chosen upon the initial message and 
 * 	defined by the state variable 'usbRxState'.
 *
 ***	RECEIVE messages are initiated in response to changes in 
 * 	the receive status register, and again on completion of
 *	transmission of packets to the Host. No provision for flow
 *	control is made, so the host better keep up.
 *
 *      The etherRxState variable is used to keep track of whether
 *      we're currently transmitting a packet to the host (ETHERRX_USB)
 *      or not (ETHERRX_IDLE).
 *
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */


#include "appEthernet.h"
#include "appFRAM.h"
#include "usart.h"
#include "hardware.h"
#include "assert.h"
#include "memory.h"
#include "msgEthernet.h"
#include "usb_ll.h"
#include "usbEndpoints.h"
#include "appIds.h"
// #define DEBUG
#include "debug.h"

#define FORCE_10MBPS

#define USBRX_MSGHEADER 0
#define USBRX_RXPACKET  1
#define USBRX_EATPACKET	2

static unsigned usbRxState = USBRX_MSGHEADER ;

#define ETHERRX_IDLE 0
#define ETHERRX_USB  1

static unsigned etherRxState = ETHERRX_IDLE ;

#define AT91_EMAC_RX_BUF_SIZE  128
#define AT91_EMAC_RX_BUF_COUNT 48
#define AT91_EMAC_RX_MEMSIZE (AT91_EMAC_RX_BUF_COUNT*AT91_EMAC_RX_BUF_SIZE)

#define AT91_EMAC_TX_BUF_SIZE  1536
#define AT91_EMAC_TX_BUF_COUNT 4
#define AT91_EMAC_TX_MEMSIZE (AT91_EMAC_TX_BUF_SIZE*AT91_EMAC_TX_BUF_COUNT)

#define AT91_EMAC_MEMSIZE (AT91_EMAC_RX_MEMSIZE+AT91_EMAC_TX_MEMSIZE)

// Receive Buffer Descriptor
typedef struct rbd_s
{
   unsigned long addr;
   unsigned long sr;
} rbd_t;

// Receive Buffer
typedef struct rb_s 
{
   unsigned char rb[AT91_EMAC_RX_BUF_SIZE];
} rb_t;

// Transmit Buffer Descriptor
typedef struct tbd_s
{
   unsigned long addr;
   unsigned long sr;
} tbd_t;

static unsigned char const defaultMac_[6] = {
   0x00, 0x00, 0xCA, 0xFE, 0xBE, 0xEF
};

typedef struct {
   msgEthernetResponse_t   getMacHeader_ ;
   unsigned char           mac_[sizeof(defaultMac_)];
   struct sg_t             getMac_sg_[2]; // header plus trailer

   msgEthernetResponse_t   setMacHeader_ ;
   struct sg_t             setMac_sg_[2]; // header plus trailer

   rbd_t                   rxDesc[AT91_EMAC_RX_BUF_COUNT];
   rb_t                   *rxBuf[AT91_EMAC_RX_BUF_COUNT];
   msgEthernetResponse_t   rxHeader_ ;
   unsigned 	           curRx_ ; // next fragment received
   unsigned                endRx_ ; // last fragment sent to host
   struct sg_t             rxsg_[4]; // max is header plus two buffers plus trailer

   msgEthernetResponse_t   txHeader_ ;
   tbd_t                   txDesc[AT91_EMAC_TX_BUF_COUNT];
   unsigned char          *txBuf[AT91_EMAC_TX_BUF_COUNT];
   unsigned                curTx_ ; // next fragment in txDesc and txBuf
   unsigned                offs_ ; // offset within fragment (when in message)
   struct sg_t             txsg_[2]; // header plus trailer
} at91_emac_bufs_t ;

#define PHY_ADDR 0

static at91_emac_bufs_t *bufs = (void *)0 ;

unsigned short readPhyReg( int reg ){
   int i ;

   EMAC->EMAC_MAN = AT91_EMAC_MAN_SOF 
                  | AT91_EMAC_MAN_RD 
                  | AT91_EMAC_MAN_CODE 
                  | AT91_EMAC_MAN_PHYA(PHY_ADDR) 
                  | AT91_EMAC_MAN_REGA(reg);
   for( i = 0 ; i < 10000 ; i++ )
   {
      volatile int val = EMAC->EMAC_NSR ;
      if( 0 == (val & AT91_EMAC_NSR_MDIO_MASK) )
         break ;
   }

   return EMAC->EMAC_NCR ;
}

static void writePhyReg( int reg, unsigned short value )
{
   int i ;
   EMAC->EMAC_MAN = AT91_EMAC_MAN_SOF 
                  | AT91_EMAC_MAN_WR 
                  | AT91_EMAC_MAN_CODE 
                  | AT91_EMAC_MAN_PHYA(PHY_ADDR) 
                  | AT91_EMAC_MAN_REGA(reg)
                  | AT91_EMAC_MAN_DATA(value);
   for( i = 0 ; i < 10000 ; i++ )
   {
      volatile int val = EMAC->EMAC_NSR ;
      if( 0 == (val & AT91_EMAC_NSR_MDIO_MASK) )
         break ;
   }
}

static void setMac( unsigned char const *mac )
{
   unsigned long i ; // use longword writes
   memcpy( &i, bufs->mac_, sizeof(i) );
   EMAC->EMAC_SA1B = i ;
   i = 0 ;
   memcpy( &i, bufs->mac_+sizeof(i), 2 );
   EMAC->EMAC_SA1T = i ;

   DEBUGMSG( "mac " ); DEBUGHEX( EMAC->EMAC_SA1B );
   DEBUGMSG( "/" ); DEBUGHEX( EMAC->EMAC_SA1T ); DEBUGMSG( "\r\n" ); 
}

static void dumpTx( void )
{
   int i ;
   DEBUGMSG( "tx (" ); DEBUGHEXCHAR( bufs->curTx_ );  DEBUGMSG( "\r\n" );
   for( i = 0 ; i < AT91_EMAC_TX_BUF_COUNT ; i++ ){
      DEBUGHEXCHAR( i ); DEBUGMSG( ":" ); DEBUGHEX( (unsigned)bufs->txBuf[i] ); DEBUGMSG( "\r\n" );
      DEBUGMSG( "    :" ); DEBUGHEX( bufs->txDesc[i].sr ); DEBUGMSG( "\r\n" );
      DEBUGMSG( "    :" ); DEBUGHEX( bufs->txDesc[i].addr ); DEBUGMSG( "\r\n" );
   }
}
static void dumpRx( void )
{
   unsigned i ;
   for( i = 0 ; i < AT91_EMAC_RX_BUF_COUNT ; i++ ){
      DEBUGHEXCHAR( i ); DEBUGMSG( "(rx):" ); DEBUGHEX( bufs->rxDesc[i].addr ); DEBUGMSG( "\r\n" );
      DEBUGMSG( "    :" ); DEBUGHEX( bufs->rxDesc[i].sr ); DEBUGMSG( "\r\n" );
      DEBUGMSG( "    :" ); DEBUGHEX( (unsigned)bufs->rxBuf[i] ); DEBUGMSG( "\r\n" );
   }
}

void ethernetInit( void )
{
   int i ;
   void *bufferMem ;
   unsigned char *nextMem = 0 ;
   unsigned flags ;

   EMAC->EMAC_NCR = AT91_EMAC_NCR_MPE ; // Allow management access

#ifdef FORCE_10MBPS
   writePhyReg( PHY_BMCR, 0 ); // don't auto-negotiate, 10Mbps, half-duplex
#endif

   bufs = (at91_emac_bufs_t *)malloc(sizeof(at91_emac_bufs_t));
   memset( bufs, 0, sizeof(*bufs));
   
   bufs->getMacHeader_.msgType_ = MSGETHERNET_GETMAC ;
   bufs->getMacHeader_.result_ = 0 ;
   if( !readFRAM( 0, bufs->mac_, sizeof(bufs->mac_) ) ){
	   DEBUGMSG( "Error reading mac address from FRAM\r\n" );
           memcpy( bufs->mac_, defaultMac_, sizeof( bufs->mac_) );
   }
   
   bufs->getMac_sg_[0].data = (unsigned char *)&bufs->getMacHeader_ ;
   bufs->getMac_sg_[0].length = sizeof(bufs->getMacHeader_)+sizeof(bufs->mac_);
   bufs->getMac_sg_[0].offset = 0 ;
   memset( &bufs->getMac_sg_[1], 0, sizeof(bufs->getMac_sg_[1]) );

   bufs->setMacHeader_.msgType_ = MSGETHERNET_SETMAC ;
   bufs->setMac_sg_[0].data = (unsigned char *)&bufs->setMacHeader_ ;
   bufs->setMac_sg_[0].length = sizeof(bufs->setMacHeader_);
   bufs->setMac_sg_[0].offset = 0 ;
   memset( &bufs->setMac_sg_[1], 0, sizeof(bufs->setMac_sg_[1]) );

   bufs->rxHeader_.msgType_ = MSGETHERNET_RECEIVE ;
   bufs->txHeader_.msgType_ = MSGETHERNET_TRANSMIT ;

   bufferMem = malloc(AT91_EMAC_RX_MEMSIZE+AT91_EMAC_TX_MEMSIZE);

   nextMem = (unsigned char *)bufferMem ;
   for( i = 0 ; i < AT91_EMAC_RX_BUF_COUNT ; i++ ){
      unsigned addr = (unsigned)nextMem & AT91_EMAC_RBD_ADDR_MASK ;
      bufs->rxDesc[i].addr = addr ;
      bufs->rxDesc[i].sr   = 0 ;
      bufs->rxBuf[i] = (rb_t *)addr ;
      memset(nextMem, 0x0f, sizeof(rb_t));
      nextMem += sizeof(rb_t);
   }
   dumpRx();

   assert( nextMem == (unsigned char *)bufferMem + AT91_EMAC_RX_MEMSIZE );
   bufs->rxDesc[AT91_EMAC_RX_BUF_COUNT-1].addr |= AT91_EMAC_RBD_ADDR_WRAP ;
   EMAC->EMAC_RBQP = (unsigned)bufs->rxDesc ;
   
   for( i = 0 ; i < AT91_EMAC_TX_BUF_COUNT ; i++ ){
      unsigned addr = (unsigned)nextMem ;
      bufs->txDesc[i].addr = addr ;
      bufs->txDesc[i].sr   = AT91_EMAC_TBD_SR_USED ;
      bufs->txBuf[i] = nextMem ;
      memset( nextMem, 0x0e, AT91_EMAC_TX_BUF_SIZE );
DEBUGMSG( "txBuf 0x" ); DEBUGHEXCHAR( i ); DEBUGMSG( "=" ); DEBUGHEX( (unsigned)nextMem ); DEBUGMSG( "\r\n" );
      nextMem += AT91_EMAC_TX_BUF_SIZE ;
   }
   assert( nextMem == (unsigned char *)bufferMem + AT91_EMAC_MEMSIZE );
   bufs->curRx_ = 0 ;
   bufs->curTx_ = 0 ;

   dumpTx();

   bufs->txDesc[AT91_EMAC_TX_BUF_COUNT-1].sr |= AT91_EMAC_TBD_SR_WRAP ;

   EMAC->EMAC_USRIO = 2 ; // enable transceiver ;

   // use longword writes to set mac
   setMac( bufs->mac_ );

//   EMAC->EMAC_IER = 0x3fff ;
   EMAC->EMAC_IDR = 0x3fff ;

   EMAC->EMAC_TBQP = (unsigned)bufs->txDesc ;
   flags = AT91_EMAC_NCFG_SPD_100Mbps 
           | AT91_EMAC_NCFG_FD  
           | AT91_EMAC_NCFG_MTI 
           | AT91_EMAC_NCFG_UNI 
           | AT91_EMAC_NCFG_BIG 
	   | AT91_EMAC_NCFG_RLCE ;
   DEBUGMSG( "NCFG == 0x" ); DEBUGHEX( flags ); DEBUGMSG( "\r\n" );
   EMAC->EMAC_NCFG = flags ;
   
   flags = AT91_EMAC_NCR_RE 
           | AT91_EMAC_NCR_TX 
	   | AT91_EMAC_NCR_MPE ;
   DEBUGMSG( "NCR == 0x" ); DEBUGHEX( flags ); DEBUGMSG( "\r\n" );
   EMAC->EMAC_NCR = flags ;
}

static void sendPacket( void )
{
   unsigned const tx = bufs->curTx_ ;
   tbd_t *const tbd = bufs->txDesc+tx ;
   unsigned const sr = (tbd->sr | AT91_EMAC_TBD_SR_EOF) & (~AT91_EMAC_TBD_SR_USED);

   if( 122223 == tx )
      dumpTx();

   tbd->sr = sr ;
   EMAC->EMAC_NCR |= AT91_EMAC_NCR_TSTART ;
   bufs->curTx_ = (tx+1)&(AT91_EMAC_TX_BUF_COUNT-1);
   if( 122223 == tx )
      dumpTx();
}

void ethernetUsbRx
	( void const *data,
	  unsigned    fragLength,
	  unsigned    bytesLeft )  // including fragment
{
   DEBUGMSG( __FUNCTION__ ); DEBUGMSG( ":" ); DEBUGHEX( fragLength ); 
	DEBUGMSG( " of " );DEBUGHEX( bytesLeft ); 
	DEBUGMSG( " in state " );DEBUGHEX( usbRxState ); 
	DEBUGMSG( "\r\n" );

   if( USBRX_MSGHEADER == usbRxState ){
      msgEthernetRequest_t const *hdr = (msgEthernetRequest_t *)data ;
      DEBUGMSG( "msgType 0x" ); DEBUGHEX( hdr->msgType_ ); DEBUGMSG( "\r\n" );
      switch( hdr->msgType_ ){
         case MSGETHERNET_SETMAC: {
            DEBUGMSG( "SETMAC 0x" ); DEBUGHEX( hdr->msgType_ ); DEBUGMSG( "\r\n" );
            if( fragLength == sizeof(hdr->msgType_)+sizeof(bufs->mac_) ){
               memcpy(bufs->mac_, hdr->data_, sizeof(bufs->mac_) );
	       if( !writeFRAM(0,bufs->mac_,sizeof(bufs->mac_)) ){
		       DEBUGMSG( "Error saving mac address to FRAM\r\n" );
	       }
               setMac( bufs->mac_ );
               bufs->setMacHeader_.result_ = 0 ;
            }
            else {
               bufs->setMacHeader_.result_ = 1 ;
               DEBUGMSG( "Invalid SETMAC length\r\n" ); DEBUGHEX( fragLength ); DEBUGMSG( "\r\n" );
            }
            usbll_transmit_app(USBEP_BULKIN, USBAPP_ETHERNET, bufs->setMac_sg_, 0, 0 );
            break ;
         }
         case MSGETHERNET_GETMAC: {
            unsigned long i = EMAC->EMAC_SA1B ;
            memcpy( bufs->mac_, &i, sizeof(i) );
            i = EMAC->EMAC_SA1T ;
            memcpy( bufs->mac_+sizeof(i), &i, 2 );

            usbll_transmit_app(USBEP_BULKIN, USBAPP_ETHERNET, bufs->getMac_sg_, 0, 0 );
            DEBUGMSG( "etherMsg 0x" ); DEBUGHEX( hdr->msgType_ ); DEBUGMSG( "\r\n" );
            break ;
         }

         case MSGETHERNET_RECEIVE: {
            DEBUGMSG( "?? etherMsg RCV 0x" ); DEBUGHEX( hdr->msgType_ ); DEBUGMSG( "\r\n" );
            break ;
         }

         case MSGETHERNET_TRANSMIT: {
            tbd_t *tbd = bufs->txDesc+bufs->curTx_ ;
            unsigned char *txBuf = bufs->txBuf[bufs->curTx_];
            unsigned packetLength = bytesLeft - sizeof(*hdr) + sizeof(hdr->data_);
            unsigned length = fragLength - sizeof(*hdr) + sizeof(hdr->data_);
            unsigned sr = AT91_EMAC_TBD_SR_USED | length ; // save length for later

// DEBUGMSG( "txBuf[1]:" ); DEBUGHEX( (unsigned)bufs->txBuf[1] ); DEBUGMSG( "\r\n" );
            assert( AT91_EMAC_TX_BUF_COUNT > bufs->curTx_ );
            if( ( 0 == (tbd->sr & AT91_EMAC_TBD_SR_USED) )
                ||
                ( packetLength > AT91_EMAC_TX_BUF_SIZE ) ) {
               DEBUGMSG( "etherTx overflow\r\n" );
               if( bytesLeft > fragLength )
                  usbRxState = USBRX_EATPACKET ;
               break ;
            } // no room at the inn

            if( AT91_EMAC_TX_BUF_COUNT-1 == bufs->curTx_ ){
               DEBUGMSG( "---> wrap 0x" ); DEBUGHEXCHAR( bufs->curTx_ ); DEBUGMSG( "\r\n" );
               sr |= AT91_EMAC_TBD_SR_WRAP ;
            }
            tbd->sr = sr ;

            if( !validHeapAddr( txBuf ) ){
               DEBUGMSG( "Invalid txBuffer 0x" ); DEBUGHEX( bufs->curTx_ ); DEBUGMSG( ":" ); DEBUGHEX( (unsigned)txBuf ); DEBUGMSG( "\r\n" );
               DEBUGMSG( "descriptors at 0x" ); DEBUGHEX( (unsigned)bufs->txDesc ); DEBUGMSG( "\r\n" );
               DEBUGMSG( "buffers at 0x" ); DEBUGHEX( (unsigned)bufs->txBuf ); DEBUGMSG( "\r\n" );

               dumpTx();
               DEBUGMSG( "txQP: " ); DEBUGHEX( EMAC->EMAC_TBQP ); DEBUGMSG( "\r\n" );
               DEBUGMSG( "rxQP: " ); DEBUGHEX( EMAC->EMAC_RBQP ); DEBUGMSG( "\r\n" );

	       dumpRx();
               txBuf = bufs->txBuf[bufs->curTx_] = (unsigned char *)bufs->txDesc[bufs->curTx_].addr ;
            }

            memcpy( txBuf, hdr->data_, length );
            if( fragLength < bytesLeft ){
DEBUGMSG( "need more\r\n" );
               usbRxState = USBRX_RXPACKET ;
            }
            else {
// DEBUGMSG( "sending packet. sr == " ); DEBUGHEX( tbd->sr ); DEBUGMSG( "\r\n" );
               sendPacket();
// DEBUGMSG( "txBuf[1](3):" ); DEBUGHEX( (unsigned)bufs->txBuf[1] ); DEBUGMSG( "\r\n" );
            }

            break ;
         } // transmit
      } // switch( msgType )
   } else if( USBRX_RXPACKET == usbRxState ){
      tbd_t *tbd = bufs->txDesc+bufs->curTx_ ;
      unsigned sr = tbd->sr ;
      unsigned length = sr & AT91_EMAC_TBD_SR_LEN_MASK ;

      assert( AT91_EMAC_TX_BUF_COUNT > bufs->curTx_ );
      assert( 0 != (sr & AT91_EMAC_TBD_SR_USED) );

      if( length + fragLength <= AT91_EMAC_TX_BUF_SIZE ){
         unsigned char *txBuf = bufs->txBuf[bufs->curTx_] + length ;
         memcpy( txBuf, data, fragLength );
         length += fragLength ;
         sr = (sr & ~AT91_EMAC_TBD_SR_LEN_MASK) | length ;
         tbd->sr = sr ;
      }
      else
         DEBUGMSG( "etherPacketOverflow(2)\r\n" );

      if( fragLength == bytesLeft ){
         sendPacket();
         DEBUGMSG( "etherEndOfPacket(2)\r\n" );
         usbRxState = USBRX_MSGHEADER ;
      } // end of packet
   } else {
      if( fragLength == bytesLeft ){
         usbRxState = USBRX_MSGHEADER ;
      }
   } // eat packet
//  DEBUGMSG( "~" ); DEBUGMSG( __FUNCTION__ ); DEBUGMSG( "\r\n" ); 
}

#if 0
static void dumpPacket( unsigned startIdx, unsigned endIdx )
{
   do {
#ifdef DEBUG
      unsigned char const *buf = bufs->rxBuf[startIdx]->rb ;
#endif
      rbd_t *rbd = bufs->rxDesc+startIdx ;
      unsigned len = rbd->sr & AT91_EMAC_RBD_SR_LEN_MASK ;
      if( len <= AT91_EMAC_RX_BUF_SIZE ){
         int i ;
         DEBUGMSG( "frag @" ); DEBUGHEX( (unsigned long)buf ); 
            DEBUGCHAR( '/' ); DEBUGHEX( rbd->addr ); DEBUGMSG( "\r\n" );
         DEBUGMSG( "len 0x" ); DEBUGHEXCHAR(len); DEBUGMSG( ":\r\n  " );
         for( i = 0 ; i < len ; i++ ){
            DEBUGHEXCHAR( buf[i] ); DEBUGCHAR( ' ' );
            if( 15 == (i & 15) )
               DEBUGMSG( "\r\n  " );
         }
         DEBUGMSG( "\r\n" );
      }
      else {
         DEBUGMSG( "Invalid length 0x" ); DEBUGHEX( len ); DEBUGMSG( "\r\n" );
      }
      
      if( startIdx != endIdx )
         startIdx = (startIdx + 1)%AT91_EMAC_RX_BUF_COUNT ;
   } while( startIdx != endIdx );
}
#endif

static void readRxPacket( void );

static void etherRxComplete
	( void *opaque,
          unsigned epNum,
	  struct sg_t *data )
{
   DEBUGMSG( __FUNCTION__ ); DEBUGMSG( "\r\n" );
   do {
      rbd_t *rbd = bufs->rxDesc+bufs->curRx_ ;
      rbd->sr = 0 ;
      rbd->addr &= ~AT91_EMAC_RBD_ADDR_OWNER_SW ;

      int done = (bufs->curRx_ == bufs->endRx_ );
      bufs->curRx_ = (bufs->curRx_+1) % AT91_EMAC_RX_BUF_COUNT ;

      if( done )
         break ;
   } while( 1 );

   readRxPacket();
}

static void readRxPacket( void ){
   assert( bufs->curRx_ < AT91_EMAC_RX_BUF_COUNT );
   do {
      rbd_t *rbd = bufs->rxDesc+bufs->curRx_ ;
//      DEBUGMSG( "readRx: " ); DEBUGHEXCHAR( bufs->curRx_ ); DEBUGMSG( "\r\n" );
// DEBUGMSG( "txBuf[1]:" ); DEBUGHEX( (unsigned)bufs->txBuf[1] ); DEBUGMSG( "\r\n" );
      if( rbd->addr & AT91_EMAC_RBD_ADDR_OWNER_SW ){
         if( rbd->sr & AT91_EMAC_RBD_SR_SOF ){
            int i, start, end, foundEnd ;
            unsigned endLen = 0 ;
            start = end = bufs->curRx_ ;
            foundEnd = 0 ;

            DEBUGMSG( "start " ); DEBUGHEXCHAR( start ); DEBUGMSG( ".." ); 
            for( i = 0 ; i < AT91_EMAC_RX_BUF_COUNT ; i++ ){
               unsigned long sr = bufs->rxDesc[end].sr ;
	       unsigned long addr ;
               endLen = sr & AT91_EMAC_RBD_SR_LEN_MASK ;
               if( 0 == (addr = (bufs->rxDesc[end].addr & AT91_EMAC_RBD_ADDR_OWNER_SW)) ){
                  DEBUGMSG( "??? EMAC" ); DEBUGHEXCHAR( end ); DEBUGMSG( ":" ); DEBUGHEX( addr ); DEBUGMSG( "\r\n" );
                  break ;
               } else if( sr & AT91_EMAC_RBD_SR_EOF ){
/*
                  if( (end != start) && (0 != (sr & AT91_EMAC_RBD_SR_EOF)) ){
                     DEBUGMSG( "double-start\r\n" );
                     while( start != end ){
                        bufs->rxDesc[start].addr &= ~AT91_EMAC_RBD_ADDR_OWNER_SW ;
                        start = (start + 1) % AT91_EMAC_RX_BUF_COUNT ;
                     }
                  }
*/                  
                  DEBUGMSG( "end.." ); DEBUGHEXCHAR( end ); DEBUGMSG( ":" ); DEBUGHEX( sr ); DEBUGMSG( "\r\n" );
                  foundEnd = 1 ;
                  break ;
               }
               else {
                  DEBUGMSG( "mid.." ); DEBUGHEXCHAR( end ); DEBUGMSG( ":" ); DEBUGHEX( sr ); DEBUGMSG( "\r\n" );
                  end = (end+1)%AT91_EMAC_RX_BUF_COUNT ; // middle fragment
               }
            } // find end-of-packet or loop trying

            if( foundEnd ){
               bufs->rxHeader_.result_ = 0 ;
               bufs->rxsg_[0].data = (unsigned char *)&bufs->rxHeader_ ;
               bufs->rxsg_[0].length = sizeof(bufs->rxHeader_);
               bufs->rxsg_[0].offset = 0 ;

               bufs->rxsg_[1].data = bufs->rxBuf[start]->rb ;
               bufs->rxsg_[1].offset = 0 ;

               bufs->endRx_ = end ;
               if( end < start ){
                  bufs->rxsg_[1].length = ((AT91_EMAC_RX_BUF_COUNT-start)*AT91_EMAC_RX_BUF_SIZE);
                  bufs->rxsg_[2].data = bufs->rxBuf[0]->rb ;
                  bufs->rxsg_[2].offset = 0 ;
                  bufs->rxsg_[2].length = (end*AT91_EMAC_RX_BUF_SIZE) + endLen ;
                  memset( bufs->rxsg_+3, 0, sizeof(bufs->rxsg_[3]) );
		  DEBUGMSG( "wrapLen 0x" ); DEBUGHEX( bufs->rxsg_[1].length + bufs->rxsg_[2].length ); DEBUGMSG( "\r\n" );
               } else {
                  bufs->rxsg_[1].length = ((end-start)*AT91_EMAC_RX_BUF_SIZE) + endLen ;
                  memset( bufs->rxsg_+2, 0, sizeof(bufs->rxsg_[2]) );
		  DEBUGMSG( "rxLen 0x" ); DEBUGHEX( bufs->rxsg_[1].length ); DEBUGMSG( "\r\n" );
               } // two segments will do
               etherRxState = ETHERRX_USB ;
               usbll_transmit_app( USBEP_BULKIN, USBAPP_ETHERNET, bufs->rxsg_, etherRxComplete, 0 );
               return ;
            }
            else {
               DEBUGMSG( "no end\r\n" );
               break ;
            }
         }
         else {
            DEBUGMSG( "not start\r\n" );
            rbd->addr &= ~AT91_EMAC_RBD_ADDR_OWNER_SW ;
	    rbd->sr = 0 ;
            bufs->curRx_ = (bufs->curRx_ + 1) % AT91_EMAC_RX_BUF_COUNT ;
         }
      }
      else {
         DEBUGMSG( "... EMAC\r\n" );
         break ;
      }
   } while( 1 );

   etherRxState = ETHERRX_IDLE ;
}

static unsigned prevTxStat = 0xaa5555aa ;

void ethernetPoll( void )
{
   unsigned stat ;
   
   stat = EMAC->EMAC_RSR ;
   if( stat ){
      DEBUGMSG( "ethRx: " ); DEBUGHEX( stat );  DEBUGMSG( "\r\n" );
      EMAC->EMAC_RSR = stat ;
   }
   
   if( ( stat & AT91_EMAC_RSR_REC ) 
       && 
       ( ETHERRX_IDLE == etherRxState ) )
   {
      readRxPacket();
   } // send packet to host

   stat = EMAC->EMAC_TSR ;
   if( stat != prevTxStat ){
      DEBUGMSG( "ethTx: " ); DEBUGHEX( stat );  DEBUGMSG( "\r\n" );
      prevTxStat = stat ;
      if( stat & AT91_EMAC_TSR_COMP )
         EMAC->EMAC_TSR = stat ; // clear
   }
}

