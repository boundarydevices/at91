/*
 * Program fram_write.cpp
 *
 * This program attempts to write data to the FRAM on an attached AT91SAM7X
 * device over USB using the FRAM application protocol as defined in atmelUSB.pdf
 * 
 * Command-line arguments include the USB device, the FRAM address and 
 * an input filename.
 *
 * Example:
 *
 *	fram_write /dev/ttyUSB1 0 fram.in -vg
 *
 * Options include:
 *	-v	verify data after download
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

static bool framRead( int fdDev, unsigned long addr, unsigned long length, FILE *fOut )
{
	framRequest_t req ;
	setAppHeader( req.appHdr_, USBAPP_FRAM, sizeof(req)-sizeof(req.appHdr_) );
	req.framHdr_.msgType_ = MSGFRAM_READ ;
	req.framHdr_.offs_    = addr ;
	req.framHdr_.length_  = length ;
	int rval = write( fdDev, &req, sizeof(req) );
	if( sizeof(req) != rval ){
		fprintf( stderr, "write err: %d: %d: %m\n", rval, errno );
		return false ;
	}

	unsigned offs = 0 ;
	while( 0 < length ){
                framRequest_t resp ;
		if( !readAll( fdDev, &resp, sizeof(resp) ) ){
			perror( "readAll(resp)" );
			return false ;
		}
		if( !validAppHeader(resp.appHdr_,64) ){
			fprintf( stderr, "Invalid resp app header: %02x.%02x.%04x\n",
				 resp.appHdr_.bd, resp.appHdr_.appId, resp.appHdr_.length );
			return false ;
		}
                printf( "have app header: length == %x/%x\n", resp.appHdr_.length, resp.framHdr_.length_ );
		if( MSGFRAM_READ != resp.framHdr_.msgType_ ){
			fprintf( stderr, "Invalid resp flash header: %04x:%04x:%04x\n",
				 resp.framHdr_.msgType_, resp.framHdr_.offs_, resp.framHdr_.length_ );
			return false ;
		}
		if( 0 == resp.framHdr_.length_ ){
			fprintf( stderr, "FRAM read error\n" );
			return false ;
		}

		char data[64];
		if( sizeof(data) < resp.framHdr_.length_ ){
			fprintf( stderr, "Invalid read length: %u\n", resp.framHdr_.length_ );
			return false ;
		}
		if( !readAll( fdDev, data, resp.framHdr_.length_ ) ){
			perror( "readAll(data)" );
			return false ;
		}
		printf( "read %u bytes\n", resp.framHdr_.length_ );
                if( fOut ){
			fwrite( data, resp.framHdr_.length_, 1, fOut );
			fflush( fOut );
                }
                else {
                   hexDumper_t dump( data, resp.framHdr_.length_, offs );
                   while( dump.nextLine() )
                      printf( "%s\n", dump.getLine() );
                }
		length -= resp.framHdr_.length_ ;
		offs += resp.framHdr_.length_ ;
	}

	// failures return from above
	return true ; 
}

static bool writeAll( int fdUpload, void const *data, unsigned length )
{
   while( 0 < length ){
      int rval = write( fdUpload, data, length );
      if( 0 < rval ){
         assert( length >= rval );
         data = (char *)data + rval ;
         length -= rval ;
      }
      else {
            fprintf( stderr, "write err: %d: %d: %m\n", rval, errno );
            return false ;
      }
   }

   return true ;
}

static bool fram_write( 
	int fdDev, 
	unsigned long addr, 
	unsigned long length,
	unsigned char const *data )
{
	unsigned const outBytes = sizeof(framRequest_t)+length ;
	framRequest_t *request = (framRequest_t *)malloc( outBytes );
	memcpy(request+1,data,length);

	setAppHeader( request->appHdr_, USBAPP_FRAM, outBytes-sizeof(request->appHdr_) );
	request->framHdr_.msgType_ = MSGFRAM_WRITE ;
	request->framHdr_.offs_    = addr ;
	request->framHdr_.length_  = length ;
	if( !writeAll( fdDev, request, outBytes ) ){
		fprintf( stderr, "Error sending write request\n" );
		free( request );
		return false ;
	}
	free(request);
	
	framRequest_t resp ;
	if( !readAll( fdDev, &resp, sizeof(resp) ) ){
		perror( "readAll(resp)" );
		return false ;
	}

	if( !validAppHeader(resp.appHdr_,sizeof(resp.framHdr_)) ){
		fprintf( stderr, "Invalid resp app header: %02x.%02x.%04x\n",
			 resp.appHdr_.bd, resp.appHdr_.appId, resp.appHdr_.length );
		return false ;
	}
	if(USBAPP_FRAM != resp.appHdr_.appId){
		fprintf( stderr, "Msg not for us: 0x%x\n", resp.appHdr_.appId);
		return false ;
	}
	printf( "have app header: length == %x/%x\n", resp.appHdr_.length, resp.framHdr_.length_ );
	if( MSGFRAM_WRITE != resp.framHdr_.msgType_ ){
		fprintf( stderr, "Invalid resp flash header: %04x:%04x:%04x\n",
			 resp.framHdr_.msgType_, resp.framHdr_.offs_, resp.framHdr_.length_ );
		return false ;
	}

	if( length != resp.framHdr_.length_ ){
		fprintf( stderr, "FRAM write error: %lu of %lu\n", resp.framHdr_.length_, length );
		return false ;
	}

	printf( "wrote %u bytes\n", length );
	return true ;
}

static bool verify = false ;

static void parseArgs( int &argc, char const **argv )
{
	for( int i = 1 ; i < argc ; i++ ){
		char const *arg = argv[i];
		if( '-' == *arg ){
			arg++ ;
			while( *arg ){
				char const c = toupper(*arg++);
				if( 'V' == c ){
					verify = true ;
				}
				else
					fprintf( stderr, "Invalid flag  -%c\n", c );
			}

			// pull from argument list
			for( int j = i+1 ; j < argc ; j++ ){
				argv[j-1] = argv[j];
			}
			--i ;
			--argc ;
		}
	}
}

int main( int argc, char const *argv[] )
{
	parseArgs( argc, argv );
	
        int rval = 1 ;
	if( 4 == argc ){
		int const fdUpload = open( argv[1], O_RDWR );
		if( 0 <= fdUpload ){
			char *endPtr ;
			unsigned long addr = strtoul( argv[2], &endPtr, 0 );
			if( ( 65536 >= addr ) && ( 0 != endPtr ) && ( '\0' == *endPtr ) ){
				int const fdIn = open( argv[3], O_RDONLY );
				if( 0 <= fdIn ){
					char const *imageName ;
					unsigned runaddr, length, crc ;
					if( samba_verify( fdUpload, imageName, runaddr, length, crc ) ){
						struct stat st ;
						fstat( fdIn, &st );
						void *mem = mmap( 0, st.st_size, PROT_READ, MAP_PRIVATE, fdIn, 0 );
						if( MAP_FAILED != mem )
						{
							printf( "ready to write %lu bytes to address 0x%lx\n", st.st_size, addr );
							
							if( fram_write( fdUpload, addr, st.st_size, (unsigned char *)mem ) ){
								printf( "wrote data successfully\n" );

								rval = 0 ;

								FILE *fOut = 0 ;

								if( verify ){
									fOut = tmpfile();
									if( 0 == fOut ){
										perror( "tmpfile" );
										return 1 ;
									}
									printf( "verifying data\n" );
									if( framRead( fdUpload, addr, st.st_size, fOut ) ){
										printf( "read %lu bytes from address 0x%lx\n", st.st_size, addr );
										if( fOut ){
											fclose( fOut );
										}
									}
									else {
										fprintf( stderr, "Error reading data from FRAM\n" );
									}
								}
							}
							else
								fprintf( stderr, "Error writing data\n" );

							munmap( mem, st.st_size );
						}
						else
							perror( "mmap" );
					}
					else
						printf( "Error verifying connection\n" );
					close( fdIn );
				}
				else
					perror( argv[3] );
			}
			else
				fprintf( stderr, "Invalid address %s. Use decimal or 0xHEX\n", argv[2] );
			close( fdUpload );
		}
		else
			perror( argv[1] );
	}
	else {
		fprintf( stderr, "Usage: %s /dev/ttyUSB1 0 fram.in -v\n"
			 , argv[0] );
	}
	return rval ;
}

