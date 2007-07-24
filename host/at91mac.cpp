/*
 * at91mac.cpp
 *
 * Get or set the mac address on an attached SEFRAM board.
 *
 * (for test purposes). In production, the ifconfig utility
 * and Linux driver are used...
 *
 */
#include <stdio.h>
#include "msgEthernet.h"
#include "appMsgDirect.h"
#include "sambaUtil.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "hexDump.h"

#define ETH_ALEN        6
/*
 * returns -1 if not valid hex
 */
static int hexValue( char c )
{
        if( ( '0' <= c ) && ( '9' >= c ) )
        {
                return c-'0' ;
        }
        else if( ( 'A' <= c ) && ( 'F' >= c ) )
        {
                return c-'A'+10 ;
        }
        else if( ( 'a' <= c ) && ( 'f' >= c ) )
        {
                return c-'a'+10 ;
        }
        else
                return -1 ;
}

// returns non-zero to indicate success
static int parse_mac( char const    *macString, // input
                      unsigned char *macaddr )  // output: not NULL-terminated
{
        int i ;
        for( i = 0 ; i < ETH_ALEN ; i++ )
        {
                char high, low, term ;
                int  highval, lowval ;
                high = *macString++ ;
                
                if( ( 0 == high ) 
                    || 
                    ( 0 > ( highval = hexValue(high) ) ) )
                        break ;
                low  = *macString++ ;
                if( ( 0 == low ) 
                    || 
                    ( 0 > ( lowval = hexValue(low) ) ) )
                        break ;
                
                term = *macString++ ;
                if( ETH_ALEN-1 > i )
                {
                        if( ( '-' != term ) && ( ':' != term ) )
                                break ;
                }
                else if( '\0' != term )
                        break ;
                        
                *macaddr++ = (highval<<4) | lowval ;
        }

        return ( ETH_ALEN == i );
}

static char const * const devName = "/dev/at91sam7x0" ;

struct request_t {
        struct appHeader_t      appHdr_ ;
        unsigned short          msgType_ ;
        unsigned char           mac_[ETH_ALEN];
};


int main( int argc, char const **argv )
{
        int rval = -1 ;
        parse_samba_args(argc,argv);

        int fdDev = open( devName, O_RDWR );
        
        if( 0 <= fdDev ){
		char const *imageName ;
		unsigned runaddr, length, crc ;
                if( samba_verify( fdDev, imageName, runaddr, length, crc ) ){
                        struct request_t req ;
                        setAppHeader( req.appHdr_, USBAPP_ETHERNET, sizeof(req.msgType_) );
                        req.msgType_ = MSGETHERNET_GETMAC ;
                        unsigned requestSize = sizeof(req.appHdr_)
                                             + sizeof(req.msgType_);

                        int numWritten = write( fdDev, &req, requestSize );
                        while( 1 ){
                           unsigned char responseBuf[2048];
                           int numRead = read( fdDev, responseBuf, sizeof( responseBuf ) );
                           if( 0 < numRead ){
                              if( sizeof(appHeader_t) < numRead ){
                                 appHeader_t const &hdr = *(appHeader_t const *)responseBuf ;
                                 if( !validAppHeader( hdr, 1544 ) ){
                                    if( samba_debug ) printf( "--- not for us\r\n" );
                                    continue ;
                                 }

                                 if( USBAPP_ETHERNET != hdr.appId ){
                                    if( samba_debug ) printf( "--- not ethernet %u\n", hdr.appId );
                                    continue ;
                                 }

                                 if( numRead < hdr.length + sizeof(appHeader_t) ){
                                    unsigned left = hdr.length + sizeof(appHeader_t) - numRead ;
                                    if( samba_debug ) printf( "read %u bytes more\n", left );
                                    int more = read( fdDev, responseBuf+numRead, left );
                                    if( more != left ){
                                       printf( "short read %d of %u\n", more, left );
                                       continue ;
                                    }
                                    numRead += more ;
                                 }

                                 msgEthernetResponse_t const &ersp = *(msgEthernetResponse_t *)(&hdr+1);
                                 if( MSGETHERNET_GETMAC != ersp.msgType_ ){
                                    if( samba_debug ) printf( "--- not getmac rx: %u\n", ersp.msgType_ );
                                    continue ;
                                 }

                                 if( 0 != ersp.result_ ){
                                    if( samba_debug ) printf( "--- bad rx status: %u\n", ersp.result_ );
                                    continue ;
                                 }
                                 unsigned char const *data = (unsigned char const *)( &ersp+1 );
                                 if( samba_debug ) printf( "--- first byte of response == %02x\n", *data );

                                 unsigned payload = numRead-(data-responseBuf);
                                 
                                 if( ETH_ALEN == payload ){
                                    if( 1 == argc ){
                                       printf( "%02X:%02X:%02X:%02X:%02X:%02X", 
                                               data[0], data[1], data[2], data[3], data[4], data[5] );
                                       if( isatty(fileno(stdout)) )
                                          printf( "\n" );
                                       rval = 0 ;
                                    }
                                    break ;
                                 }
                                 else {
                                      printf( "%u, not %u bytes\n", payload, ETH_ALEN );
                                      hexDumper_t dumpRx( data, payload );
                                      while( dumpRx.nextLine() ){
                                         printf( "%s\n", dumpRx.getLine() );
                                      }
                                 }
                              }
                           }
                           else
                              break ;
                        }
                              
                        if( 2 <= argc ){
                           if( !parse_mac( argv[1], req.mac_ ) ){
                              printf( "Invalid mac <%s>\n", argv[1] );
                              return -1 ;
                           }
                           req.msgType_ = MSGETHERNET_SETMAC ;
                           req.appHdr_.length += sizeof(req.mac_);
                           requestSize += sizeof(req.mac_);
   
                           int numWritten = write( fdDev, &req, requestSize );
                           if( samba_debug ) printf( "wrote %d of %u bytes\n", numWritten, requestSize );
                           if( requestSize != numWritten ){
                              perror( "write2" );
                              return -1 ;
                           }
                           while( 1 ){
                              unsigned char responseBuf[2048];
                              int numRead = read( fdDev, responseBuf, sizeof( responseBuf ) );
                              if( 0 < numRead ){
                                 if( sizeof(appHeader_t) < numRead ){
                                    appHeader_t const &hdr = *(appHeader_t const *)responseBuf ;
                                    if( !validAppHeader( hdr, 1544 ) ){
                                       printf( "--- not for us\r\n" );
                                       continue ;
                                    }
   
                                    if( USBAPP_ETHERNET != hdr.appId ){
                                       printf( "--- not ethernet %u\n", hdr.appId );
                                       continue ;
                                    }
   
                                    if( numRead < hdr.length + sizeof(appHeader_t) ){
                                       unsigned left = hdr.length + sizeof(appHeader_t) - numRead ;
                                       printf( "read %u bytes more\n", left );
                                       int more = read( fdDev, responseBuf+numRead, left );
                                       if( more != left ){
                                          printf( "short read %d of %u\n", more, left );
                                          continue ;
                                       }
                                       numRead += more ;
                                    }
   
                                    msgEthernetResponse_t const &ersp = *(msgEthernetResponse_t *)(&hdr+1);
                                    if( MSGETHERNET_SETMAC != ersp.msgType_ ){
                                       printf( "--- not getmac rx: %u\n", ersp.msgType_ );
                                       continue ;
                                    }
   
                                    if( 0 != ersp.result_ ){
                                       printf( "--- bad rx status: %u\n", ersp.result_ );
                                       break ;
                                    }
                                    unsigned char const *data = (unsigned char const *)( &ersp+1 );
                                    unsigned payload = numRead-(data-responseBuf);
                                    
                                    if( 0 == payload ){
                                       rval = 0 ; // success
                                       break ;
                                    }
                                    else {
                                       printf( "response payload == %u bytes\n", payload );
                                       hexDumper_t dumpRx( data, payload );
                                       while( dumpRx.nextLine() ){
                                          printf( "%s\n", dumpRx.getLine() );
                                       }
                                       break ;
                                    }
                                 }
                              }
                              else
                                 break ;
                           }
                        }
                }
                else
                        printf( "Error verifying connection\n" );
                close( fdDev );
        }
        else
                perror( devName );
        return rval ;
}

