/*
 * Program at91adler.cpp
 *
 * This program will request an adler32 value from a region of flash
 * and optionally compare it against a file on 
 * 
 * Command-line arguments include the USB device, the flash address, 
 * and either a length or a filename.
 *
 * Example:
 *
 *	at91adler /dev/ttyUSB1 0x00100000 myProg.bin
 *
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
#include "msgFlash.h"
#include <errno.h>
#include <assert.h>
#include <zlib.h>
#include "machineConst.h"

typedef struct {
        struct appHeader_t 	appHdr_ ;
        msgFlashRequest_t	flashHdr_ ;
} flashRequest_t ;

typedef struct {
	struct appHeader_t	appHdr_ ;
	msgFlashResponse_t	flashHdr_ ;
} flashResponse_t ;

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

static int getAdler
	( int		fdDev,
	  unsigned long addr,
	  unsigned 	length,
	  char const   *fileName,
	  void const   *mem )
{
	flashRequest_t msg ;
	setAppHeader( msg.appHdr_, USBAPP_FLASH, sizeof(msg)-sizeof(msg.appHdr_) );
	msg.flashHdr_.msgType_ = MSGFLASH_ADLER ;
	msg.flashHdr_.offs_ = addr-FLASH_BASE ;
	msg.flashHdr_.length_ = length ;
	int rval = write( fdDev, &msg, sizeof(msg) );
	if( sizeof(msg) != rval ){
		fprintf( stderr, "write err: %d: %d: %m\n", rval, errno );
		return -1 ;
	}
	printf( "write %d\n", rval );
	flashResponse_t response ;
	if( !readAll( fdDev, &response, sizeof(response) ) ){
		return false ;
	}
	printf( "read %d\n", sizeof(response) );
	if( !validAppHeader(response.appHdr_,sizeof(response.flashHdr_)) ){
		fprintf( stderr, "Invalid response app header: %02x.%02x.%04x\n",
			 response.appHdr_.bd, response.appHdr_.appId, response.appHdr_.length );
		return -2 ;
	}
	if( MSGFLASH_ADLER != response.flashHdr_.msgType_ ){
		fprintf( stderr, "Invalid response flash header: %04x:%04x\n",
			 response.flashHdr_.msgType_, response.flashHdr_.rval_ );
		return -3 ;
	}

	printf( "adler == 0x%x...", response.flashHdr_.rval_ );
	fflush(stdout);
	if( fileName && mem ){
		int adler = adler32( 0, (const Bytef *)mem, length );
		if( (unsigned) adler != response.flashHdr_.rval_ ){
			printf( "mismatch: dev:%x, %s:%x\n"
                                , response.flashHdr_.rval_
				, fileName
				, adler );
			return -4 ;
		}
		else {
			printf( "matches %s\n", fileName );
			return 0 ;
		}
	}
	else {
		printf( "\n" );
		return 0 ;
	}
}

int main( int argc, char const *argv[] )
{
	if( 4 == argc ){
		int const fdUpload = open( argv[1], O_RDWR );
		if( 0 <= fdUpload ){
			char *endPtr ;
			unsigned long addr = strtoul( argv[2], &endPtr, 0 );
			if( ( 0 < addr ) && ( 0 != endPtr ) && ( '\0' == *endPtr ) ){
printf( "address == (%s)0x%08lx\n", argv[2], addr );
				struct stat st ;
				char const *fileName = 0 ;
				int fdIn ;
                                void *mem ;
				unsigned length = strtoul( argv[3], 0, 0 );
				if( 0 == stat( argv[3], &st ) ){
					fileName = argv[3];
					length = st.st_size ;
                                        fdIn = open( fileName, O_RDONLY );
					if( 0 > fdIn ){
						perror( fileName );
						return 1 ;
					}
                                        mem = mmap( 0, st.st_size, PROT_READ, MAP_PRIVATE, fdIn, 0 );
					if( MAP_FAILED == mem ){
						perror( fileName );
						return 1 ;
					}
				}

				char const *imageName ;
				unsigned runaddr, imgLen, crc ;
				if( samba_verify( fdUpload, imageName, runaddr, imgLen, crc ) ){
					printf( "ready to adler %u bytes at 0x%lx\n", length, addr );
					return getAdler( fdUpload, addr, length, fileName, mem );
				}
				else
					fprintf( stderr, "Invalid USB connection\n" );
			}
			else
				fprintf( stderr, "Invalid address %s. Use decimal or 0xHEX\n", argv[2] );
			close( fdUpload );
		}
		else
			perror( argv[1] );
	}
	else {
		fprintf( stderr, "Usage: %s /dev/ttyUSB1 0x202000 [length|myProg.bin]\n"
			 , argv[0] );
	}
	return 1 ;
}

