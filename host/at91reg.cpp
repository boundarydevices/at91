/*
 * Program at9reg.cpp
 *
 * This program allows a user to read/write register or memory
 * values on an attached AT91 device.
 * 
 * Command-line arguments include the USB device, the memory address 
 * and an optional value.
 *
 * Example:
 *
 *	at91up /dev/at91sam7x 0x202000 0xdeadbeef
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

int main( int argc, char const **argv )
{
	parse_samba_args(argc,argv);

	int rval = -1 ;
	if( 3 <= argc ){
		int const fdDev = open( argv[1], O_RDWR );
		if( 0 <= fdDev ){
			fcntl( fdDev, F_SETFL, O_NONBLOCK );
			char *endPtr ;
			unsigned long addr = strtoul( argv[2], &endPtr, 0 );
			if( ( endPtr != argv[3] ) && ( 0 != endPtr ) && ( '\0' == *endPtr ) ){
				char const *imageName ;
				unsigned runAddr, length, crc ;
				if( samba_verify( fdDev, imageName, runAddr, length, crc ) ){
					if( samba_debug ){
						if( 0 != length )
							printf( "Image <%s>, addr 0x%x, length %u, crc %u\n", imageName, runAddr, length, crc );
						else
							printf( "probably running Samba: %u, %u, %u\n", runAddr, length, crc );
					}
					unsigned long value ;
					if( samba_readreg_long( fdDev, addr, value ) ){
						printf( "0x%08lx\n", value );
						if( 4 <= argc ){
							unsigned long newVal = strtoul( argv[3], &endPtr, 0 );
							if( ( endPtr != argv[3] ) && ( 0 != endPtr ) && ( '\0' == *endPtr ) ){
								if( samba_writereg_long( fdDev, addr, newVal ) ){
									printf( "value stored\n" );
                                                                        rval = 0 ;
								}
							}
							else
								fprintf( stderr, "Invalid value %s. Use decimal or 0xHEX\n", argv[3] );
						} // have value argument
						else
							rval = 0 ;
					}
				}
				else
					printf( "Error verifying connection\n" );
			}
			else
				fprintf( stderr, "Invalid address %s. Use decimal or 0xHEX\n", argv[2] );
			close( fdDev );
		}
		else
			perror( argv[1] );
	}
	else
		fprintf( stderr, "Usage: %s /dev/at91sam7x 0x202000 [0xvalue]\n", argv[0] );
	return rval ;
}

