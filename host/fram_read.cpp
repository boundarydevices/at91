/*
 * Program fram_read.cpp
 *
 * This program attempts to read data from the FRAM on 
 * an attached AT91SAM7X device over USB using the FRAM 
 * application protocol as defined in atmelUSB.pdf
 * 
 * Command-line arguments include the FRAM address, length,
 * and optionally the USB device, and an output filename.
 *
 * Example:
 *
 *	fram_read 0 100 fram.out -d /dev/my_fram 
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#include "sambaUtil.h"
#include <stdio.h>
#include <poll.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include "appMsgDirect.h"
#include "msgFRAM.h"
#include <errno.h>
#include <assert.h>
#include "hexDump.h"

typedef struct {
        struct appHeader_t 	appHdr_ ;
        msgFramHeader_t		framHdr_ ;
} framRequest_t ;

static bool readAll( int fdUpload, void *data, unsigned length )
{
   while( 0 < length ){
      int rval = read( fdUpload, data, length );
      if( 0 < rval ){
         assert( length >= rval );
         data = (char *)data + rval ;
         length -= rval ;
      }
      else {
            fprintf( stderr, "read err: %d: %d: %m\n", rval, errno );
            return false ;
      }
   }

   return true ;
}

static void dumpData( void const *data, unsigned len ){
	hexDumper_t dump(data,len);
	while( dump.nextLine() )
		printf( "%s\n", dump.getLine() );
}

static bool framRead( int fdDev, unsigned long addr, unsigned long length, FILE *fOut )
{
	int rval = lseek(fdDev, addr, SEEK_SET);
	if( rval != addr ){
		perror( "lseek" );
		return false ;
	}
	while( 0 < length ){
		char buf[4096];
		unsigned count = (length > sizeof(buf)) ? sizeof(buf) : length ;
		int numRead = read( fdDev, buf, count);
		if( 0 < numRead ){
			printf( "%u bytes read from %u\n", numRead, addr );
			if( fOut ){
				fwrite( buf, numRead, 1, fOut );
				fflush( fOut );
			}
			else {
			   hexDumper_t dump( buf, numRead, addr );
			   while( dump.nextLine() )
			      printf( "%s\n", dump.getLine() );
			}
			length -= numRead ;
			addr += numRead ;
		}
		else {
			fprintf( stderr, "%m at position %u\n", addr );
			break ;
		}
	}

	return 0 == length ;
}

static char const *device = "/dev/fram" ;

static void eatArg( int &argc, int arg, char const **argv ){
	// pull from argument list
	for( int j = arg+1 ; j < argc ; j++ ){
		argv[j-1] = argv[j];
	}
	argc-- ;
}

static void parseArgs( int &argc, char const **argv )
{
	for( int i = 1 ; i < argc ; i++ ){
		char const *arg = argv[i];
		if( '-' == *arg ){
			arg++ ;
			while( *arg ){
				char const c = toupper(*arg++);
				if( 'D' == c ){
					if( i < argc-1 ){
						device = argv[i+1];
						printf( "device == %s\n", device );
						eatArg(argc,i,argv);
					}
				}
				else
					fprintf( stderr, "Invalid flag  -%c\n", c );
			}

			eatArg(argc,i,argv);
			--i ;
		}
	}
}

int main( int argc, char const *argv[] )
{
	parseArgs(argc,argv);
	int rval = 1 ;
	if( 3 <= argc ){
		int const fdUpload = open( device, O_RDWR );
		if( 0 <= fdUpload ){
			char *endPtr ;
			unsigned long addr = strtoul( argv[1], &endPtr, 0 );
			if( ( 65536 >= addr ) && ( 0 != endPtr ) && ( '\0' == *endPtr ) ){
				unsigned long length = strtoul( argv[2], &endPtr, 0 );
				if( ( 0 < length ) && ( 0 != endPtr ) && ( '\0' == *endPtr ) ){
					FILE *fOut = 0 ;

					if( 4 < argc ){
						fOut = fopen( argv[3], "wb" );
						if( 0 == fOut ){
							perror( argv[3] );
							return 1 ;
						}
					}
					if( framRead( fdUpload, addr, length, fOut ) ){
						printf( "read %lu bytes from address 0x%lx\n", length, addr );
					} else 
						fprintf( stderr, "Error reading from FRAM\n" );
				}
				else
					fprintf( stderr, "Invalid length %s. Use decimal or 0xHEX\n", argv[2] );
			}
			else
				fprintf( stderr, "Invalid address %s. Use decimal or 0xHEX\n", argv[1] );
			close( fdUpload );
		}
		else
			perror( argv[1] );
	}
	else {
		fprintf( stderr, "Usage: %s 0 100 [fram.out] [-d /dev/my_framdev ]\n"
			 , argv[0] );
	}
	return rval ;
}

