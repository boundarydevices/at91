/*
 * Program at91reset.cpp
 *
 * This program will issue an ioctl call to reset the 
 * device specified on the command line:
 *
 *	at91reset /dev/at91sam7x-0
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
#include <errno.h>
#include <sys/ioctl.h>

int main( int argc, char const *argv[] )
{
	if( 2 == argc ){
		int const fdDev = open( argv[1], O_RDWR );
		if( 0 <= fdDev ){
			int result = ioctl( fdDev, 0x3232, 0 );
			if( 0 == result ){
				printf( "reset issued successfully\n" );
			}
			else
				fprintf( stderr, "Error %d:%m issuing reset\n", errno );
		}
		else
			perror( argv[1] );
	}
	else {
		fprintf( stderr, "Usage: %s /dev/at91sam7x-0n", argv[0] );
	}
	return 0 ;
}

