/*
 * Program at91down.cpp
 *
 * This program will download a section of RAM from an
 * attached AT91SAM7X device over USB using the SAM-BA 
 * bootstrap protocol.
 * 
 * Command-line arguments include the USB device, the start
 * address, length, and program name.
 *
 * Example:
 *
 *	at91down /dev/ttyUSB1 0x202000 0x1000 myFile.out
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

int main( int argc, char const * const argv[] )
{
	if( 5 == argc ){
		int const fdUpload = open( argv[1], O_RDWR );
		if( 0 <= fdUpload ){
			char *endPtr ;
			unsigned long addr = strtoul( argv[2], &endPtr, 0 );
			if( ( 0 < addr ) && ( 0 != endPtr ) && ( '\0' == *endPtr ) ){
				unsigned long length = strtoul( argv[3], &endPtr, 0 );
				if( ( 0 < length ) && ( 0 != endPtr ) && ( '\0' == *endPtr ) ){
					char const * const outFileName = argv[4];
					FILE *fOut = fopen( outFileName, "wb" );
					if( fOut ){
						char const *imageName ;
						unsigned runaddr, length, crc ;

						if( samba_verify( fdUpload, imageName, runaddr, length, crc ) ){
							printf( "ready to download %lu bytes to file %s\n", length, outFileName );
							if( samba_download( fdUpload, addr, length, fileno(fOut) ) ){
								printf( "%lu bytes saved to %s\n", length, outFileName );
							}
							else
								printf( "Error downloading file\n" );
						}
						else
							printf( "Error verifying connection\n" );
						fclose( fOut );
					}
					else
						perror( outFileName );
				}
				else
					fprintf( stderr, "Invalid length %s. Use decimal or 0xHEX\n", argv[3] );
			}
			else
				fprintf( stderr, "Invalid address %s. Use decimal or 0xHEX\n", argv[2] );
			close( fdUpload );
		}
		else
			perror( argv[1] );
	}
	else
		fprintf( stderr, "Usage: %s /dev/ttyUSB1 0x202000 0x10000 myFile.out\n", argv[0] );
	return 0 ;
}

