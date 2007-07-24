/*
 * Module usart.cpp
 *
 * This module defines the utility routines and
 * data structures declared in usart.h
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */


#include "usart.h"

StructUSART * const usarts[2] = {
	USART0
,	USART1
};

static unsigned long master_clock( void )
{
	/* 
	 *
	 *  PLL register 0x3c
	 *    0x10483F0E
	 *      1  USB divisor 1
	 *       048  Multiplier 0x048   - out clock = in clock*73
	 *          3f out 0, count 3f
	 *            0E  divisor 0x0e (14)
	 *
	 *          XIn == 18.432MHz == 18432000
	 *          PLLOut == (18432000*73)/14    == 96109714
	 *
	 * MCKR Master clock register 0x30
	 *	 0x00000007
	 *           0b0111
	 *		1 - prescale x 2
         *		 11 - source is PLL
	 *
	 *       96109714/2=48054857
	 */
        return 48054857 ;
}

/*
 * Power-on default is
 */
static unsigned short baud_to_cd( unsigned baud )
{
	return master_clock()/(16*baud);
}

void waitUartIdle(StructUSART *usart)
{
	while( 0 == (usart->US_CSR & 0x02) );
}

void writeChar( StructUSART *usart, unsigned char c )
{
	waitUartIdle(usart);
	usart->US_THR = c ;
}

void write( StructUSART *usart, char const *str )
{
	while( *str ){
		writeChar(usart, *str++ );
	}
}

void writeHexChar( StructUSART *usart, unsigned char c )
{
	char buf[3];
	unsigned long mask = 0xF0 ;
	unsigned shift = 4 ;
	char *nextOut = buf ;
	while( mask ){
		unsigned char v = ( ( c & mask ) >> shift ) & 0x0f ;
		if( 10 > v ){
			*nextOut++ = '0' + v ;
		}
		else
			*nextOut++ = 'A' + v - 10 ;

		mask >>= 4 ;
		shift -= 4 ;
	}
	*nextOut++ = '\0' ;
	write( usart, buf );
}

void writeHex( StructUSART *usart, unsigned long val )
{
	char buf[12];
	unsigned long mask = 0xF0000000 ;
	unsigned shift = 28 ;
	char *nextOut = buf ;
	while( mask ){
		unsigned char v = ( ( val & mask ) >> shift ) & 0x0f ;
		if( 10 > v ){
			*nextOut++ = '0' + v ;
		}
		else
			*nextOut++ = 'A' + v - 10 ;

		mask >>= 4 ;
		shift -= 4 ;
	}
	*nextOut++ = '\0' ;
	write( usart, buf );
}

int read( StructUSART *usart, char *inChar )
{
	int rx = ( usart->US_CSR & US_RXRDY );
	if( rx )
		*inChar = usart->US_RHR ;

	return rx ;
}

void enableUART( unsigned which, unsigned baud )
{
	StructUSART *const usart = usarts[which & 1];
	usart->US_CR = US_RSTRX | US_RSTTX | US_RXDIS | US_TXDIS ;

	//* Clear Transmit and Receive Counters
	usart->US_RCR = 0 ;
	usart->US_TCR = 0 ;
	
	//* Define the USART mode
	usart->US_MR = ( US_CHMODE_NORMAL + US_NBSTOP_1_5 + US_PAR_NO + US_CHRL_8 + US_CLKS_MCK );

	//* Define the baud rate divisor register
	usart->US_BRGR = baud_to_cd(baud);

	//* Write the Timeguard Register
	usart->US_TTGR = 0 ;

	//* Enable receiver and transmitter
	usart->US_CR = US_RXEN | US_TXEN ;
}

 
