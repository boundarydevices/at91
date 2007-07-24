/*
 * Check a binary image for a length field at offset
 * 0x100 and a crc32 field in the last four bytes of 
 * the file.
 *
 * Usage is: 
 * 	crcCheck fileName
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <zlib.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

static unsigned const lengthPos = 0x100 ;

int main( int argc, char const * const argv[] )
{
	if( 1 == argc ){
		fprintf( stderr, "Usage: %s fileName\n", argv[0] );
		return -1 ;
	}

	int rval = -1 ;
	for( int i = 1 ; i < argc ; i++ ){
		char const *fileName = argv[i];
		int fd = open( fileName, O_RDONLY );
		if( 0 <= fd ){
			uLong crc = 0 ;
			char inBuf[4096];
			int  numRead ;
			if( lengthPos == ( numRead = read( fd, inBuf, lengthPos ) ) ){
				crc = crc32(0,(Bytef *)inBuf,numRead);
				unsigned long noCRC = 0 ;
				crc = crc32(crc,(Bytef *)&noCRC, sizeof(noCRC) ); // placeholder
				unsigned long expectedLen ;
				unsigned long fileCRC = 0xDEADBEEF ;
				if( sizeof(expectedLen) == ( numRead = read( fd, &expectedLen, sizeof(expectedLen) ) ) ){
					while( 0 < (numRead = read( fd, inBuf, sizeof(inBuf) ) ) ){
						if( numRead < sizeof(inBuf) ){
							numRead -= sizeof(fileCRC);
						} // last read
                                                crc = crc32(crc,(Bytef *)inBuf,numRead);
						if( numRead < sizeof(inBuf) ){
							memcpy( &fileCRC, inBuf+numRead, sizeof(fileCRC) );
							break ;
						}
					}
					unsigned long fileLength = lseek(fd,0,SEEK_CUR)-sizeof(fileCRC);
					if( ( crc == fileCRC ) && ( expectedLen == fileLength ) ){
						printf( "%lu", crc );
						rval = 0 ;
					}
					else {
						printf( "%s: calcCRC %lx, fileCRC %lx, length %lu, read %lu\n", 
							fileName, crc, fileCRC, expectedLen, fileLength );
					}
					if( isatty(1) )
						printf("\n");
				}
				else
					fprintf( stderr, "read2: %d\n", numRead );

			}
			else
				fprintf( stderr, "read1: %d\n", numRead );
			close(fd);
		}
		else {
			perror( fileName );
			rval = -2 ;
			break ;
		}
	}
	
	return rval ;
}
