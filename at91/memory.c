/*
 * Module memory.c
 *
 * This module defines the memory access routines declared
 * in memory.h
 *
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */


#include "memory.h"
#include "assert.h"
#include "llInit.h"
#include "usart.h"
// #include "leds.h"
// #define DEBUG
#include "debug.h"

void *memset( void *ptr, int value, unsigned long size )
{
	unsigned char *next = (unsigned char *)ptr ;
	assert( 0 != ptr );
	while( size-- ){
		*next++ = (unsigned char)value ;
	}
	return ptr ;
}

void *memcpy( void *dest, void const *src, unsigned long size )
{
	unsigned char *nextIn = (unsigned char *)src ;
	unsigned char *nextOut = (unsigned char *)dest ;
	while( size-- ){
		*nextOut++ = *nextIn++ ;
	}
	return dest ;
}

static unsigned nextAddr = (unsigned)&bssEnd ;

void *malloc( unsigned numBytes )
{
   void *rval = (void *)nextAddr ;
   int avail = (unsigned)&rval - nextAddr ;
   
   DEBUGMSG( "malloc 0x" );
   DEBUGHEX(numBytes); 
   DEBUGMSG( ", next=" );
   DEBUGHEX( (unsigned long)nextAddr );
   DEBUGMSG( "\r\n" );

/*
*/

   if( 0 < avail ){
      unsigned needed = ((numBytes+3)/4)*4 ;
      if( needed <= avail ){
         // round up to nearest 4-byte boundary
         nextAddr += ((numBytes+3)/4)*4 ;
         
         avail = (unsigned)&rval - nextAddr ;
         DEBUGMSG( "avail=" );
         DEBUGHEX( avail );
         DEBUGMSG( "\r\n" );
   
         return rval ;
      }
   }
   write( DEFUART, "malloc overflow: want=" );
   writeHex( DEFUART, numBytes );
   write( DEFUART, ", avail=" );
   writeHex( DEFUART, avail );
   write( DEFUART, ", next=" );
   writeHex( DEFUART, (unsigned long)nextAddr );
   write( DEFUART, ", top=" );
   writeHex( DEFUART, (unsigned long)&rval );
   write( DEFUART, "\r\n" );
   assert( nextAddr < (unsigned long)&rval ); // less than current stack depth
/*
   while(1){
      setLED(avail++);
   }
*/ 
   return 0 ;
}

int validHeapAddr( void const *p )
{
   unsigned const addr = (unsigned)p ;
   return ( nextAddr > addr ) && ( (unsigned)&bssEnd <= addr );
}

unsigned strlen( char const *s ){
	char const *s0 = s ;
	while( *s++ )
		;
	return s-s0-1 ;
}

char *stpcpy( char *dest, char const *src ){
	char c ;
	while( 0 != ( c = *src++ ) ){
		*dest++ = c ;
	}
	*dest = 0 ;
	return dest ;
}

char *toDecimal( char *dest, unsigned long value ){
	int sig = 0 ;
	unsigned long divisor = 1000000000 ;

	while( divisor ){
		unsigned long dig = value/divisor ;
		if( dig || sig ){
			*dest++ = '0' + dig ;
			sig = 1 ;
		}
		value %= divisor ;
		divisor /= 10 ;
	}
	if( 0 == sig )
		*dest++ = '0' ;
	*dest = 0 ;
	return dest ;
}

char *toHex( char *dest, unsigned long value ){
	unsigned long mask = 0xF0000000 ;
	unsigned long shift = 28 ;
	while( mask ){
		unsigned char nib = (value&mask) >> shift ;
		if( nib < 10 )
			*dest = '0' + nib ;
		else
			*dest = 'A' + nib - 10 ;
		dest++ ;
		mask >>= 4 ;
		shift -= 4 ;
	}
	
	*dest = 0 ;
	return dest ;
}
