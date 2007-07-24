/*
 * Simple wrapper for the zLib crc32() routine.
 * (See usr/lib/zlib.h)
 *
 * Usage is: 
 *
 * 	crc32 string | [@fileName]
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <zlib.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

int main( int argc, char const * const argv[] )
{
	if( 1 == argc ){
		fprintf( stderr, "Usage: %s string|@fileName\n", argv[0] );
		return -1 ;
	}

	int rval = -1 ;
	for( int i = 1 ; i < argc ; i++ ){
		char const *arg = argv[i];
		if( '@' == *arg ){
			int fd = open( arg+1, O_RDONLY );
			if( 0 <= fd ){
				uLong crc = 0 ;
				char inBuf[4096];
				int  numRead ;
				while( 0 < (numRead = read( fd, inBuf, sizeof(inBuf) ) ) ){
					crc = crc32(crc,(Bytef *)inBuf,numRead);
				}
				close(fd);
				rval = 0 ;
				printf( "%lu", crc );
				if( isatty(1) )
					printf("\n");
			}
			else {
				perror( arg );
				rval = -2 ;
				break ;
			}
		} else {
			uLong crc = crc32(0, (const Bytef *)arg, strlen(arg));
			printf( "%lu", crc );
			if( isatty(1) )
				printf( "\n" );
			rval = 0 ;
		}
	}
	
	return rval ;
}
