/*
 * Program fram.c
 *
 * This program allows interactive access to a 
 * RamTron FM25L512 SPI FRAM device connected to 
 * SPI0 of an Atmel AT91SAM7x cpu
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#include "init.h"
#include "hardware.h"
#include "leds.h"
#include "usart.h"
#include "fram.h"
#include "assert.h"
// #define DEBUG
#include "debug.h"

typedef enum {
   cmd       = 0,
   addr      = 1,
   readdata  = 2,
   writedata = 3,
   writeline = 4
} state_e ;

//
// read status register, return non-zero if successful
//
static int readStatusRegister( unsigned char *value )
{
   int rval = 0 ;
   if( SPI0->SP_SR & SP_ENDTX ){
      int i ;
      int sr ;
      unsigned long cmd = FRAM_OPCODE_RDSR ;
      unsigned long response = 0 ;
      
      SPI0DMA->pdmaRPR = (unsigned long)&response ;
      SPI0DMA->pdmaRCR = sizeof(response);
      SPI0DMA->pdmaTPR = (unsigned long)&cmd ;
      SPI0DMA->pdmaTCR = sizeof(cmd);
      SPI0DMA->pdmaPTCR = 0x0101 ;   // enable transmit and receive periph dma
      SPI0->SP_IER = 0x00000030 ; // enable ints
      while( 0 == ( (sr = SPI0->SP_SR) & SP_ENDRX ) ){
         DEBUGMSG( "txsr 0x" ); DEBUGHEX( sr ); DEBUGMSG( "\r\n" );
      }

      *value = (response >> 8);
      rval = 1 ;
   }
   else 
      DEBUGMSG( "readsr tx not empty\r\n" );
   
   SPI0DMA->pdmaPTCR = 0x0202 ;   // disable transmit and receive periph dma
   
   return rval ;
}

//
// read status register, return non-zero if successful
//
static int readData( unsigned short addr, unsigned char *value )
{
   int rval = 0 ;
   if( SPI0->SP_SR & SP_ENDTX ){
      int i ;
      int sr ;
      unsigned long cmd = FRAM_OPCODE_READ | (addr<<8);
      unsigned long response = 0 ;
      
      SPI0DMA->pdmaRPR = (unsigned long)&response ;
      SPI0DMA->pdmaRCR = sizeof(response);
      SPI0DMA->pdmaTPR = (unsigned long)&cmd ;
      SPI0DMA->pdmaTCR = sizeof(cmd);
      SPI0DMA->pdmaPTCR = 0x0101 ;   // enable transmit and receive periph dma
      while( 0 == ( (sr = SPI0->SP_SR) & SP_ENDRX ) )
         ;

      DEBUGMSG( "readData resp 0x" ); DEBUGHEX( response ); DEBUGMSG( "\r\n" );
      *value = (response >> 24);
      rval = 1 ;
   }
   else 
      DEBUGMSG( "readData tx not empty\r\n" );
   
   SPI0DMA->pdmaPTCR = 0x0202 ;   // disable transmit and receive periph dma
   
   return rval ;
}

//
// read status register, return non-zero if successful
//
static int writeEnable( void )
{
   int rval = 0 ;
   if( SPI0->SP_SR & SP_ENDTX ){
      int i ;
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

//
// read status register, return non-zero if successful
//
static int writeData( unsigned short addr, unsigned char value )
{
   int rval = 0 ;
   if( SPI0->SP_SR & SP_ENDTX ){
      int i ;
      int sr ;
      unsigned long response ;
      unsigned char cmd[] = {
         FRAM_OPCODE_WREN
      ,  FRAM_OPCODE_WRITE
      ,  addr&0xff
      ,  (addr>>8)
      ,  value
      };

      SPI0DMA->pdmaRPR = (unsigned long)&response ;
      SPI0DMA->pdmaRCR = sizeof(response);
      SPI0DMA->pdmaTPR = (unsigned long)cmd ;
      SPI0DMA->pdmaTCR = sizeof(cmd);
      SPI0DMA->pdmaPTCR = 0x0101 ;   // enable transmit and receive periph dma
      while( (SP_ENDRX|SP_ENDTX) != (sr = (SPI0->SP_SR & (SP_ENDRX|SP_ENDTX))) )
         ;

      DEBUGMSG( "address 0x" ); DEBUGHEX((unsigned)cmd );DEBUGMSG( "\r\n" );
//      DEBUGMSG( "writeData resp 0x" ); DEBUGHEX( response ); DEBUGMSG( "\r\n" );
      rval = 1 ;
   }
   else 
      DEBUGMSG( "writeData tx not empty\r\n" );
   
   SPI0DMA->pdmaPTCR = 0x0202 ;   // disable transmit and receive periph dma
   
   return rval ;
}

//
// read status register, return non-zero if successful
//
static int writeDataMulti
   ( unsigned short       addr, 
     unsigned char const *value,
     unsigned             length )
{
   int rval = 0 ;
   if( SPI0->SP_SR & SP_ENDTX ){
      int i ;
      int sr ;
      unsigned long cmd = FRAM_OPCODE_WREN | FRAM_OPCODE_WRITE<<8 | (addr<<16);
      unsigned long response = 0 ;

      SPI0DMA->pdmaRPR = (unsigned long)&response ;
      SPI0DMA->pdmaRCR = sizeof(response);
      SPI0DMA->pdmaTPR = (unsigned long)&cmd ;
      SPI0DMA->pdmaTCR = sizeof(cmd);
      SPI0DMA->pdmaTNPR = (unsigned)value ;
      SPI0DMA->pdmaTNCR = length ;
      SPI0DMA->pdmaPTCR = 0x0101 ;   // enable transmit and receive periph dma

      while( (SP_ENDRX|SP_ENDTX) != (sr = (SPI0->SP_SR & (SP_ENDRX|SP_ENDTX))) ){
         DEBUGMSG( "sr: " ); DEBUGHEX( sr ); DEBUGMSG( "\r\n" ); 
      }

      DEBUGMSG( "writeData resp 0x" ); DEBUGHEX( response ); DEBUGMSG( "\r\n" );
      rval = 1 ;
   }
   else 
      DEBUGMSG( "writeData tx not empty\r\n" );
   
   SPI0DMA->pdmaPTCR = 0x0202 ;   // disable transmit and receive periph dma
   
   return rval ;
}

//
// read status register, return non-zero if successful
//
static int readData2( unsigned short addr, unsigned char *value, unsigned length )
{
   int rval = 0 ;
   if( 0 != ((unsigned long)value & 3 ) ){
      DEBUGMSG( "readData unaligned\r\n" );
      return 0 ;
   }

   if( 0 == (SPI0->SP_SR & SP_ENDTX) ){
      DEBUGMSG( "last transmit not complete\r\n" );
      return 0 ;
   }

   PIOA->PIO_CODR = SPI0_NPCS0_MASK ; // assert chip-select
   
   { // local scope for transmit
      int i = 0 ;
      int sr ;
      unsigned long cmd = FRAM_OPCODE_READ | (addr<<8);

      SPI0DMA->pdmaTPR = (unsigned long)&cmd ;
      SPI0DMA->pdmaTCR = sizeof(cmd);
      SPI0->SP_CR = SP_SPIEN ;
      SPI0DMA->pdmaPTCR = 0x0102 ;   // enable transmit periph DMA
      SPI0->SP_IER = SP_ENDTX ;
      while( ( 0 == ((sr = SPI0->SP_SR) & SP_ENDTX) )
             &&
             ( i++ < 100 ) ){
         DEBUGMSG( "txsr 0x" ); DEBUGHEX( sr ); DEBUGMSG( "\r\n" );
      }

      SPI0DMA->pdmaPTCR = 0x0202 ;   // disable transmit periph DMA
      DEBUGMSG( "readData cmd complete\r\n" );
   }
   
   { // local scope for receive
      int sr ;
      int i = 0 ;

      memset( value, 0, length );
      SPI0DMA->pdmaTPR = (unsigned long)value ;
      SPI0DMA->pdmaTCR = length ;
      SPI0DMA->pdmaRPR = (unsigned long)value ;
      SPI0DMA->pdmaRCR = length ;
      SPI0DMA->pdmaPTCR = 0x0101 ;   // enable transmit and receive periph dma
      SPI0->SP_IER = SP_ENDRX ;
      while( ( 0 == ((sr = SPI0->SP_SR) & SP_ENDRX) )
             &&
             ( i++ < 100 ) ){
         DEBUGMSG( "rxsr 0x" ); DEBUGHEX( sr ); DEBUGMSG( "\r\n" );
      }

      DEBUGMSG( "readData resp <" ); 
      for( i = 0 ; i < length ; i++ ){
         DEBUGHEXCHAR( value[i] );
      }
      DEBUGMSG( ">\r\n" );

      SPI0DMA->pdmaPTCR = 0x0202 ;   // disable receive periph dma
      rval = 1 ;
   }

   PIOA->PIO_CODR = SPI0_NPCS0_MASK ; // de-assert chip-select
   
   return rval ;
}

void init( void )
{
	int i, j ;
        char cmdChar = '\0' ;
        state_e state = cmd ;
        unsigned long address = 0 ;
        char lineBuf[80];
        char *nextLineIn ;
        char *const lineEnd = lineBuf+sizeof(lineBuf);

	setLED(1);
	
	// initialize com ports
	for( i = 0 ; i < NUM_USARTS ; i++ ){
		enableUART(i,115200);
	}

	setLED(2);

        SPI0->SP_CR = SP_SWRST ;
        SPI0->SP_MR = 0x06000011 ;     /* DelayBCS 6, PCS 0, Master, Mode Fault Disable, no loopback */
        SPI0->SP_CSR[0] = 0x0A0AFF02 ; /* DLYBCT=1, DLYBS=1, SCBR=0x01, BITS=8, CSAAT=0, SPI mode 0 (CPHA=0,CPOL=0) */
        SPI0->SP_CR = SP_SPIEN ;
        SPI0DMA->pdmaPTCR = 0x0202 ;   // disable transmit and receive periph dma

        DEBUGMSG( ">> " );
//        writeEnable();

        j = 3 ;
	while(1){
           char c ;
           if( read( DEFUART, &c ) ){
/*
DEBUGMSG( "GPIOA SR: " ); DEBUGHEX( PIOA->PIO_PSR ); DEBUGMSG( "\r\n" );
DEBUGMSG( "GPIOA IFSR: " ); DEBUGHEX( PIOA->PIO_IFSR ); DEBUGMSG( "\r\n" );
DEBUGMSG( "PMC_PCSR: " ); DEBUGHEX( PMC->PMC_PCSR ); DEBUGMSG( "\r\n" );
*/
              setLED(j++);
              switch( state ){
                 case cmd: {
                    c &= ~'\x20' ; // make it UPPER

                    if( ( 'R' == c ) || ( 'W' == c ) || ( 'L' == c ) ){
                       state = addr ;
                       cmdChar = c ;
                       writeChar( DEFUART, c );
                       address = 0 ;
                    }
                    else {
                       DEBUGMSG( "Invalid command: use 'R', 'W', or 'L'\r\n"
                                       ">> " );
                       state = cmd ;
                    }
                    break ;
                 }
                 case addr: {
                    if( ( '0' <= c ) && ( '9' >= c ) ){
                       address *= 16 ;
                       address += (c-'0');
                       writeChar( DEFUART, c );
                    } else {
                       c &= ~'\x20' ; // make it UPPER
                       if( ('A' <= c ) && ( 'F' >= c ) ){
                          address *= 16 ;
                          address += (c-'A')+10;
                          writeChar( DEFUART, c );
                       }
                       else if( ( '\r' == c ) || ( '\n' == c ) ){
                          DEBUGMSG( "address 0x" ); DEBUGHEX( address ); DEBUGMSG( "\r\n" );
                          if('R' == cmdChar){
                             state = readdata ;
                             DEBUGMSG( "\r\nreading data from address 0x" ); DEBUGHEX( address );
                             DEBUGMSG( "\r\nhit <ESC> to cancel...\r\n" );
                          } else if( 'W' == cmdChar ){
                             state = writedata ;
                             DEBUGMSG( "\r\nwriting data to address 0x" ); DEBUGHEX( address );
                             DEBUGMSG( "\r\nhit <ESC> to cancel...\r\n" );
                          } else if( 'L' == cmdChar ){
                             state = writeline ;
                             DEBUGMSG( "\r\nwriting line to address 0x" ); DEBUGHEX( address );
                             DEBUGMSG( "\r\nhit <ESC> to cancel...\r\n" );
                             nextLineIn = lineBuf ;
                          } 
                       }
                       else {
                          DEBUGMSG( "Invalid address\r\n"
                                          ">> " );
                          state = cmd ;
                       }
                    }
                    break ;
                 }
                 case readdata: {
                    if( '\x1b' != c ){
/*
                       unsigned char rdsr = 0x5a ;
                       if( 0 != readStatusRegister( (unsigned char *)&rdsr) ){
                          DEBUGMSG( "rdsr == 0x" ); DEBUGHEXCHAR( rdsr ); DEBUGMSG( "\r\n" );
*/                          
                          unsigned long value = 0xa5a5a5a5 ;
                          if( 0 != readData2( address, &value, 4 ) ){
                             DEBUGMSG( "[" ); DEBUGHEX( address ); 
                                 DEBUGMSG( "] == " );
                                 DEBUGHEXCHAR( value );
                                 DEBUGMSG( "\r\n" );
                             address++ ;
                          }
//                       }
                    }
                    else {
                       DEBUGMSG( "\r\n>> " );
                       state = cmd ;
                    }
                    break ;
                 }
                 case writedata: {
                    if( '\x1b' != c ){
                       writeEnable();
                       if( 0 != writeData( address, c ) ){
                          DEBUGMSG( "[" ); DEBUGHEX( address++ ); 
                              DEBUGMSG( "] == " );
                              DEBUGHEXCHAR( c ); 
                              DEBUGMSG( "\r\n" );
                       }
                    }
                    else {
                       DEBUGMSG( "\r\n>> " );
                       state = cmd ;
                    }
                    break ;
                 }
                 case writeline: {
                    if( '\r' == c ){
                        if( nextLineIn > lineBuf ){
                           unsigned length = nextLineIn-lineBuf ;
                           if( writeDataMulti( address, lineBuf, length ) ){
                              DEBUGMSG( "wrote " ); DEBUGHEX( length ); DEBUGMSG( " bytes\r\n" );
                              address += length ;
                           }
                        }
                        nextLineIn = lineBuf ;
                    } else if( '\x1b' == c ){
                        DEBUGMSG( "Invalid address\r\n"
                                        ">> " );
                        state = cmd ;
                    } else if( nextLineIn < lineEnd ){
                       *nextLineIn++ = c ;
                    }
                 }
              }
           } // read something
	}
}
