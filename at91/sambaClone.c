/*
 * Module sambaClone.c
 *
 * This module defines the samba clone
 * application routines as declaredin sambaClone.h
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#include "sambaClone.h"
#include "usbEndpoints.h"
#include "usb_ll.h"
#include "usart.h"
#include "assert.h"
#include "memory.h"
// #define DEBUG
#include "debug.h"
#include "llInit.h"
#include "reboot.h"

typedef enum {
   CMDCHAR,
   ADDRESS,
   LENGTH,
   SENDFILE,
} sambaState_e ;

static unsigned char  cmd = '\0' ;
static sambaState_e   state = CMDCHAR ;
static unsigned long  address = 0 ;
static unsigned char  txBuf[64];
static unsigned long  cmdParam = 0 ;
static int            txPending = 0 ;
static struct sg_t    sg[2];

void samba_init( void )
{
	memset( sg, 0, sizeof(sg) );
}

static void sambaTxCallback
	( void *opaque,
          unsigned epNum,
	  struct sg_t *data )
{
	assert( USBEP_BULKIN == epNum );
        txPending = 0 ;
}

static void send( unsigned char const *data, unsigned length, usbll_tx_callback_t callback )
{
DEBUGMSG( __FUNCTION__ ); DEBUGMSG( ", length 0x" ); DEBUGHEX( length ); DEBUGMSG( "\r\n" );
	sg[0].data = (unsigned char *)data ;
	sg[0].length = length ;
	sg[0].offset = 0 ;
	usbll_transmit( USBEP_BULKIN, sg, callback, 0 );
}

static void sendPrompt
	( void *opaque,
          unsigned epNum,
          struct sg_t *data )
{
   assert( USBEP_BULKIN == epNum );
   DEBUGMSG( __FUNCTION__ ); DEBUGMSG( "\r\n" );
   send( ">", 1, sambaTxCallback );
}

static void rxFileHeaderComplete
	( void *opaque,
          unsigned epNum,
          struct sg_t *data )
{
   assert( USBEP_BULKIN == epNum );
   DEBUGMSG( __FUNCTION__ ); DEBUGMSG( "\r\n" );
   send( (unsigned char *)address, cmdParam, sendPrompt );
}

static unsigned char invalidCommandResp[] = {
   "\n\r"
   "Invalid command"
   "\n\r"
   ">"
};

static unsigned char versionString[128] = {
   "\n\r"
   "$Id$"
   "\n\r"
   ">"
};

static void makeVersionString()
{
	char *next = stpcpy( versionString+2, appName_ );
	next = stpcpy( next, "\n\rlength\t" );
	next = toDecimal(next,imageLength);
	next = stpcpy( next, "\n\rcrc\t" );
	next = toDecimal(next,imageCRC);
	next = stpcpy( next, "\n\raddr\t" );
	next = toDecimal(next,(unsigned long)&textStart);
	next = stpcpy( next, "\n\r>" );
	DEBUGMSG( "Version string: " ); DEBUGHEX( (unsigned long)(next-(char *)versionString) ); DEBUGMSG( " bytes\r\n" );
}

typedef void (*routine_t)(void);

static void executeCommand( void )
{
   DEBUGMSG( "cmd:" ); DEBUGCHAR( cmd );
   DEBUGMSG( ": 0x" ); DEBUGHEX( address );
   DEBUGMSG( "/0x" ); DEBUGHEX( cmdParam );
   DEBUGMSG( "\r\n" );

   state = CMDCHAR ;
   if( txPending ){
      DEBUGMSG( "double-command2\n" );
      return ;
   }

   switch( cmd ){
      case 'w' : // read longword
      {
         int i ;
         char *nextOut = (char *)txBuf ;
         unsigned long mask ;
         unsigned shift ;
         unsigned long value ;

         memcpy( &value, (void *)address, sizeof(value) );

         *nextOut++ = '\n' ; // SAMBA does it this way
         *nextOut++ = '\r' ;
         *nextOut++ = '0' ;
         *nextOut++ = 'x' ;

         mask = 0xF0000000 ;
         shift = 28 ;

         for( i = 0 ; i < 8 ; i++ ){
            unsigned char nib = ( value & mask ) >> shift ;
            if( nib < 10 )
               *nextOut++ = '0' + nib ;
            else
               *nextOut++ = ('A' + nib - 10);
            mask >>= 4 ;
            shift -= 4 ;
         }
         *nextOut++ = '\n' ;
         *nextOut++ = '\r' ;
         *nextOut++ = '>' ;

         txPending = 1 ;
         send( txBuf, ((unsigned char *)nextOut)-txBuf, sambaTxCallback );
//         DEBUGMSG( "read longword\r\n" );
         break ;
      }
      case 'W' : // write longword
      {
         char *nextOut = (char *)txBuf ;
         *nextOut++ = '\n' ;
         *nextOut++ = '\r' ;
         *nextOut++ = '>' ;
	 *(unsigned long *)address = cmdParam ;
         txPending = 2 ;
         send( txBuf, ((unsigned char *)nextOut)-txBuf, sambaTxCallback );
//         DEBUGMSG( "write longword 0x" ); DEBUGHEX( address ); DEBUGMSG( " == " ); DEBUGHEX( cmdParam ); DEBUGMSG( "\r\n" );
         break ;
      }
      case 'S' : // send file
      {
//         DEBUGMSG( "send file\r\n" );
         state = SENDFILE ;
         break ;
      }
      case 'R' : // receive file
      {
//         DEBUGMSG( "receive file1\r\n" );
         txPending = 3 ;
         send( "\n\r", 2, rxFileHeaderComplete );
         break ;
      }
      case 'V' : {
	 unsigned length ;
//         DEBUGMSG( "Version command\r\n" );
         txPending = 4 ;
	 makeVersionString();
	 length = strlen(versionString);
         send( versionString, length, sambaTxCallback );
	 break ;
      }
      case 'G' : {
         if( ( 0 == address ) && 0 ){
             reboot();
         }
         else {
            ((routine_t)address)();
         }
         break ;
      }
      default : {
	 send(invalidCommandResp,sizeof(invalidCommandResp),sambaTxCallback);
      }
   }
}

static char const knownCmdChars[] = {
	"wWSRVG"
};

static int isCmdChar( char c ){
	char const *cmdChars = knownCmdChars ;
	while( *cmdChars ){
		if(*cmdChars == c ){
			return 1 ;
		}
                cmdChars++ ;
	}
	return 0 ;
}

void sambaRx
	( unsigned char const *data,
	  unsigned length )
{
#ifdef DEBUG
   int i ;
   DEBUGMSG( "sambaRx:" );
   for( i = 0 ; i < length ; i++ )
      DEBUGCHAR( data[i] );
   DEBUGMSG( "\r\n" );
#endif

   if( txPending ){
      DEBUGMSG( "double-command\n" );
      return ;
   }

   while( 0 < length ){
      unsigned char const nextChar = *data++ ;
      --length ;
DEBUGMSG( "state 0x" ); DEBUGHEXCHAR( state ); DEBUGMSG( "\r\n" );
      switch( state ){
         case CMDCHAR : {
            if( isCmdChar(nextChar) ){
               cmd = nextChar ;
               state = ADDRESS ;
               address = 0 ;
               cmdParam = 0 ;
            } else {
               DEBUGMSG( "Invalid cmdChar: " ); DEBUGHEXCHAR(nextChar); DEBUGMSG("\r\n");
            }
            break ;
         }
         case ADDRESS : {
            if( ( '0' <= nextChar ) && ( '9' >= nextChar ) ){
               address <<= 4 ;
               address += nextChar-'0' ;
            } else if( ( 'A' <= nextChar ) && ( 'F' >= nextChar ) ){
               address <<= 4 ;
               address += nextChar-'A'+10 ;
            } else if( ( 'a' <= nextChar ) && ( 'f' >= nextChar ) ){
               address <<= 4 ;
               address += nextChar-'a'+10 ;
            } else if( ',' == nextChar ){
               cmdParam = 0 ;
               state = LENGTH ;
            } else if( '#' == nextChar ){
               executeCommand();
               state = CMDCHAR ;
            }
            break ;
         }
         case LENGTH : {
            if( ( '0' <= nextChar ) && ( '9' >= nextChar ) ){
               cmdParam <<= 4 ;
               cmdParam += nextChar-'0' ;
            } else if( ( 'A' <= nextChar ) && ( 'F' >= nextChar ) ){
               cmdParam <<= 4 ;
               cmdParam += nextChar-'A'+10 ;
            } else if( ( 'a' <= nextChar ) && ( 'f' >= nextChar ) ){
               cmdParam <<= 4 ;
               cmdParam += nextChar-'a'+10 ;
            }
            else if( '#' == nextChar ){
               executeCommand();
               state = CMDCHAR ;
            }
            break ;
         }
         case SENDFILE : {
            unsigned max = ++length ;     // didn't need to eat last char
            data-- ;
            if( max > cmdParam ){
               max = cmdParam ;
            }

//            DEBUGMSG( "receive 0x" ); DEBUGHEX( max ); DEBUGMSG( " of " ); DEBUGHEX( cmdParam ); DEBUGMSG( " bytes\r\n" );

            memcpy( (void *)address, data, max );
            length   -= max ;
            data     += max ;
            cmdParam -= max ;
            address  += max ;

            if( 0 == cmdParam ){
               assert( 0 == length );
               txPending = 5 ;
               DEBUGMSG( "done receiving file\r\n" ); 
               send( ">", 1, sambaTxCallback );
               state = CMDCHAR ;
            }

            break ;
         }
      }
   }
}

