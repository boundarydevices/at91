/*
 * Module serialRegs.c
 *
 * This module defines the serial register command
 * line as declared and described in serialRegs.h
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */


#include "serialRegs.h"
#include "usart.h"
#include "memory.h"
#include "delay.h"
#include "reboot.h"
#include "usb_ll.h"
#include "llInit.h"
#include "usbEndpoints.h"
#include "appIds.h"

#define TXBUFSIZE 128
#define TXBUFMASK (TXBUFSIZE-1)

static char txBuf[TXBUFSIZE+1];
static unsigned char txTake = 0 ;
static unsigned char txAdd = 0 ;

static void tx( char const *s ){
	while( *s ){
		txBuf[txAdd++] = *s++ ;
		txAdd &= TXBUFMASK ;
	}
}

static void txChar( char c ){
	txBuf[txAdd++] = c ;
	txAdd &= TXBUFMASK ;
}

static void txHex( unsigned long value ){
	char *out = txBuf+txAdd;
	char *next = toHex( out, value );
	txAdd = txAdd + (next-out);
	txAdd &= TXBUFMASK ;
}

static char const prompt[] = {
	"\r\n# "
};

#define RXBUFSIZE 128
#define RXBUFMASK (RXBUFSIZE-1)
static char rxBuf[RXBUFSIZE+1];
static unsigned char rxAdd = 0 ;

//
// Read hex from a command line, return pointer to next and *value
// if at least one hex digit found. Otherwise return NULL
// 
static char const *readHex( char const *s, unsigned long *value )
{
	char c ;
	int  foundHex = 0 ;
	*value = 0 ;

	while( 0 != (c=*s) ){
		s++ ;
		if( ('0' <= c) && ('9' >= c) ){
			*value <<= 4 ;
			*value += (c-'0');
			foundHex = 1 ;
		} else if( ('A' <= c) && ('F' >= c) ){
			*value <<= 4 ;
			*value += (c-'A'+10);
			foundHex = 1 ;
		} else if( ('a' <= c) && ('f' >= c) ){
			*value <<= 4 ;
			*value += (c-'a'+10);
			foundHex = 1 ;
		} else if( ('x' == c) || ('X' == c) ){
			// allow 0xHEX form
		}
		else if( (' ' == c) && (0 == foundHex) ){
			// skip leading whitespace
		}
		else 
			break ;
	}
	
	if( 0 == foundHex )
		s = 0 ;
	return s ;
}

static void showValue( unsigned long addr ){
	unsigned long value ;
	unsigned const lowBits = addr & 3 ;
	if( 0 == lowBits ){
		// longword
		memcpy( &value, (void *)addr, sizeof(value) );
	} else if( 0 == (lowBits&1) ){
		// short
		value = 0;
		memcpy( &value, (void *)addr, 2 );
	} else {
		// byte
		value = 0 ;
		memcpy( &value, (void *)addr, 1 );
	}
	txHex( addr ); tx( " == " ); txHex( value ); tx( "\r\n" );
}

extern void dumpEP0( void );
extern unsigned long savedCtx[4];

//
// spit out large transmits to the bulk endpoint
// 
static struct sg_t sg0[2] = {
	{ 0 }
,	{ 0 }
};

static struct sg_t sg1[2] = {
	{ 0 }
,	{ 0 }
};

static void flashTxComplete1
	( void *opaque,
          unsigned epNum,
	  struct sg_t *data )
{
   write( DEFUART, __FUNCTION__ ); write( DEFUART, "\r\n" );
}

static void flashTxComplete0
	( void *opaque,
          unsigned epNum,
	  struct sg_t *data )
{
	write( DEFUART, __FUNCTION__ ); write( DEFUART, "\r\n" );
	sg0[0].data   = (unsigned char *)&textStart ;
	sg0[0].length = &textEnd-&textStart ;
	sg0[0].offset = 0 ;
	usbll_transmit_app( USBEP_BULKIN, USBAPP_FLASH, sg0, flashTxComplete1, 0 );
}

static void sambaTxComplete
	( void *opaque,
	  unsigned epNum,
	  struct sg_t *data )
{
   write( DEFUART, __FUNCTION__ ); write( DEFUART, "\r\n" );
}

static void usbSpew( void ){
	write( DEFUART, "spew large amounts of data here\n" );
	sg0[0].data   = (unsigned char *)&textStart ;
	sg0[0].length = &textEnd-&textStart ;
	sg0[0].offset = 0 ;
	usbll_transmit_app( USBEP_BULKIN, USBAPP_FLASH, sg0, flashTxComplete0, 0 );
//					  12345678901234
	sg1[0].data   = (unsigned char *)"Hello, world\r\n" ;
	sg1[0].length = 14 ;
	sg1[0].offset = 0 ;
	usbll_transmit_app( USBEP_BULKIN, USBAPP_SERIAL, sg1, sambaTxComplete, 0 );
}

static void process(){
	rxBuf[rxAdd] = '\0' ;
	char const *rx = rxBuf ;
	
	unsigned long addr ;
	rx = readHex(rx,&addr);
	if( rx ){
		unsigned long value ;
		rx = readHex(rx,&value);
		if( rx ){
			showValue(addr);
			unsigned width ;
			switch( addr & 3 ){
				case 0: width = 4 ; break ;
				case 1: 
				case 3: width = 1 ; break ;
				case 2: width = 2 ; break ;
			}
			memcpy( (void *)addr, &value, width );
			showValue(addr);
		}
		else {
			showValue(addr);
		} // read request
	}
	else if( ('r' == rxBuf[0]) || ('R' == rxBuf[0]) )
		reboot();
	else if( ('s' == rxBuf[0]) || ('S' == rxBuf[0]) ){
		write( DEFUART, "wait 10\r\n" );
		delay(10);
		write( DEFUART, "delay done\r\n" );
	} else if( ('x' == rxBuf[0]) || ('X' == rxBuf[0]) ){
		unsigned i ;
		write( DEFUART, "saved context\r\n" );
		for( i = 0 ; i < 4 ; i++ ){
			writeChar( DEFUART, '[' ); writeHexChar(DEFUART,i); write(DEFUART, "] == 0x" ); writeHex(DEFUART,savedCtx[i]); write( DEFUART, "\r\n" );
		}
	} else if( ('p' == rxBuf[0]) || ('P' == rxBuf[0]) ){
		dumpEP(0);
	} else if( ('u' == rxBuf[0]) || ('U' == rxBuf[0]) ){
		usbSpew();
	} else if( '.' == rxBuf[0] ){
		dumpEP(USBEP_BULKIN);
	} else	
		tx( "No address (Usage is \"address value\" in hex)\r\n" );
	rxAdd = 0 ;
}

void serialRegsInit( void )
{
	tx( prompt );
}

void serialRegsPoll( void )
{
	StructUSART *usart = DEFUART ;
	
	unsigned short csr = usart->US_CSR ;
	while( ( 0 != (csr & US_TXRDY) )
	     &&
	     ( txAdd != txTake ) )
	{
		usart->US_THR = txBuf[txTake++];
		csr = usart->US_CSR ;
	}

	if( txAdd == txTake ){
		txAdd = txTake = 0 ;
	} // reset to beginning

	while( 0 != (csr & US_RXRDY ) ){
		unsigned char inChar = usart->US_RHR ;
		if( '\r' == inChar ){
			// process data here
			rxBuf[rxAdd] = 0 ;
			process();
			tx( prompt );
		} else if( '\b' == inChar ){
			if( rxAdd >= 1 ){
				--rxAdd ;
				tx( "\b \b" );
			}
		} else if( ('U'-'@') == inChar ){
			if( rxAdd >= 1 ){
				rxAdd = 0 ;
				tx( "^U\r\n" );
			}
		} else {
			txChar(inChar);
			rxBuf[rxAdd++] = inChar ;
                        rxAdd &= RXBUFMASK ;
		}

                csr = usart->US_CSR ;
      }
}


