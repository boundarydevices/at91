/*
 * Module sambaUtil.cpp
 *
 * This module defines the utility routines declared in 
 * sambaUtil.h
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
#include <sys/errno.h>
#include <ctype.h>

bool samba_debug = false ;

static void dumpResponse( char const *response, unsigned len ){
	while( len-- ){
		char const c = *response++ ;
		if( isgraph(c) )
			printf( "%c", c );
		else
			printf( "<%02x>", (unsigned char)c );
	}
}

static bool waitResponse( int fdUp, unsigned long ms=1000 ){
	struct pollfd fds ;
	fds.fd = fdUp ;
	fds.events = POLLIN ;
	return ( 1 == poll( &fds, 1, ms ) );
}

void eatSambaData( int fdUp ){
	do {
		if( waitResponse( fdUp, 0 ) ){
			char inData[64];
			int numRead = read( fdUp, inData, sizeof(inData) );
			if( 0 < numRead ){
				if( samba_debug ){
					printf( "eatData: " );
					dumpResponse( inData, numRead );
				}
			}
			else
				break ;
		}
		else
			break ;
	} while( 1 );
}

bool samba_response( int fdUp, char *response, unsigned max, unsigned &length, unsigned long ms )
{
	char *const startResp = response ;
	do {
		if( waitResponse( fdUp, ms ) ){
			int numRead = read( fdUp, response, max);
			if( 0 < numRead ){
				if( samba_debug ) dumpResponse( response, numRead );
				char const last = response[numRead-1];
				char const nextTo = response[numRead-2];
				if( '>' == last ){
                                        length = (response-startResp)+numRead ;
					return true ;
				}
				response += numRead ;
				max -= numRead ;
			}
			else {
				perror( "readDev" );
				break ;
			}
		}
		else {
			if( samba_debug ){
				/* really? No data? */
				int numRead = read( fdUp, response, max);
				printf( "timeout: %d\n", numRead );
				struct pollfd fds ;
				fds.fd = fdUp ;
				fds.events = POLLIN ;
				printf( "poll: %d\n", poll( &fds, 1, 0 ) );
			}
			break ;
		}
	} while( 0 < max ); // !timeout
	
	return false ;
}

static void skipCtrl( char const *&s, unsigned &count ){
	while( 0 < count ){
		if( iscntrl(*s) ){
			s++ ;
			count-- ;
		}
		else
			break ;
	}
}

static void skipNonCtrl( char const *&s, unsigned &count ){
	while( 0 < count ){
		if( !iscntrl(*s) ){
			s++ ;
			count-- ;
		}
		else
			break ;
	}
}

bool samba_verify( int fdUp,
 		  char const *&image,
 		  unsigned    &addr,
 		  unsigned    &length,
 		  unsigned    &crc )
{
	eatSambaData(fdUp);
	addr = length = crc = 0 ;
	image = 0 ;

	int retries = 0 ;
	do {
		int numWritten = write( fdUp, "V#", 2 );
		if( 2 == numWritten ){
			if( samba_debug )
				printf( "sent V#\n" );
			do {
				char inBuf[256];
				unsigned numRead ;
				if( samba_response( fdUp, inBuf, sizeof(inBuf), numRead ) ){
					if( samba_debug ){
						fwrite( inBuf, 1, numRead, stdout );
						fflush(stdout);
					}
					if( 0 < numRead ){
						inBuf[numRead] = '\0' ;
						char const *imgStart = inBuf ;
						skipCtrl(imgStart,numRead);
						char const *imgEnd = imgStart ;
						skipNonCtrl(imgEnd,numRead);
						if( imgStart != imgEnd ){
	       						unsigned imgNameLen = imgEnd-imgStart ;
							image = (char *)malloc(imgNameLen+1);
							memcpy( (char *)image, imgStart, imgNameLen );
							((char *)image)[imgNameLen] = '\0' ;
							skipCtrl(imgEnd,numRead);
							sscanf( imgEnd, "length\t%u\n\rcrc\t%u\n\raddr\t%u", &length, &crc, &addr );
						} // have an image name
						else
							printf( "No img %p/%p, %u\n", imgStart, imgEnd, numRead );
					} // read something
					else
						printf( "Nothing returned\n" );
					return true ;
				} else {
					printf( "timeout\n" );
					break ;
				}
			} while( 1 ); // !timeout
		}
		else {
			perror( "writeDev" );
			break ;
		}
	} while( 1 > ++retries );

	return false ;
}

bool samba_upload( int fdUp, unsigned long address, 
                   void const *data, unsigned length )
{
	bool worked = false ;
	unsigned iter = 0 ;
	char cmd[512];
	int cmdLen = snprintf( cmd, sizeof(cmd), "S%lx,%x#", address, length );
	int numSent = write( fdUp, cmd, cmdLen );
	if( cmdLen == numSent ){
		if( samba_debug )
			printf( "Send <%s>\n", cmd );
		while( 0 < length ){
			numSent = write( fdUp, data, length );
			if( 0 <= numSent ){
				if( samba_debug && ( 0 == ( iter++ & 7 ) ) ) 
					printf( "%p: %02x...%x/%x\n", data, ((unsigned char *)data)[0], numSent, length );
				data = (unsigned char *)data + numSent ;
				length -= numSent ;
			} else if( ( EAGAIN == errno ) || ( -EAGAIN == errno ) ){
				/*
				 * Wait for previous write to complete
				 */
				struct pollfd fds ;
				fds.fd = fdUp ;
				fds.events = POLLOUT ;
				if( 1 != poll( &fds, 1, 1000 ) ){
					fprintf( stderr, "writeUp: Timeout waiting for space\n" );
					break ;
				}
			} else {
				fprintf( stderr, "writeUp: %d: (%d)%m\n", numSent, errno );
				break ;
			}
		}
		char response[80];
		unsigned responseLen ;
		worked = samba_response( fdUp, response, sizeof( response ), responseLen );
	}
	else
		perror( "writeUp" );

	return worked ;
}

bool samba_download( int fdDev, 
 		    unsigned long address, 
 		    unsigned long length,
 		    int fdOut )
{
	bool worked = false ;
	char cmd[512];
	int cmdLen = snprintf( cmd, sizeof(cmd), "R%lx,%x#", address, length );
	int numSent = write( fdDev, cmd, cmdLen );
	if( cmdLen == numSent ){
		if( waitResponse( fdDev ) ){
			char response[512];
                        int numRead ;
			if( ( 2 == ( numRead = read( fdDev, response, 2 ) ) )
			    &&
			    ( 0 == memcmp( response, "\n\r", 2 ) ) ){
				unsigned retries = 0 ;
				while( 0 < length ){
					if( waitResponse( fdDev ) ){
						char response[192];
						numRead = read( fdDev, response, sizeof(response) );
						write( fdOut, response, numRead );
						length -= numRead ;
					} else {
						fprintf( stderr, "Timeout waiting for data\n" );
						if( 3 < ++retries )
							break ;
					}
				}
				worked = ( 0 == length )
					 &&
					 waitResponse( fdDev )
					 && 
					 ( 1 == read( fdDev, response, 1 ) )
					 &&
					 ( '>' == response[0] );
			}
			else
				printf( "Invalid download header: %d: %02x..%02x\n", numRead, response[0], response[1] );
		}
		else
			printf( "timeout waiting for download\n" );
	}
	else {
		perror( "writeUp" );
	}

	return worked ;
}

/*
 * Reads a memory value (longword only)
 * Returns true and the value if successful, false if it times out
 */
bool samba_readreg_long( int fdDev, unsigned long address, unsigned long &value )
{
	if( 0 == ( address & 3 ) ){
                eatSambaData( fdDev );
		char cmd[80];
		int cmdLen = snprintf( cmd, sizeof(cmd), "w%lx,#", address );
		int numWritten = write( fdDev, cmd, cmdLen );
		if( numWritten == cmdLen ){
			if( samba_debug ){
				fwrite( "send<", 1, 5, stdout ); 
				fwrite( cmd, 1, cmdLen, stdout ); 
				fwrite( ">\r\n", 1, 3, stdout );
			}
			char response[80];
			unsigned responseLen ;
			if( samba_response( fdDev, response, sizeof(response)-1, responseLen ) ){
				response[responseLen] = '\0' ;
				if( 1 == sscanf( response, "\n\r0x%08lx\n\r", &value ) ){
					return true ;
				}
				else
					fprintf( stderr, "Unexpected response: <%s>\n", response );
			}
			else
				fprintf( stderr, "Error reading readreg response\n" );

		}
		else
			fprintf( stderr, "short write: %d/%u\n", numWritten, cmdLen );
	}
	else
		fprintf( stderr, "Invalid longword address %08lx\n", address );

	return false ;
}

/*
 * Reads a memory value (longword only)
 * Returns true and the value if successful, false if it times out
 */
bool samba_writereg_long( int fdDev, unsigned long address, unsigned long value )
{
	if( 0 == ( address & 3 ) ){
                eatSambaData( fdDev );
		char cmd[80];
		int cmdLen = snprintf( cmd, sizeof(cmd), "W%lx,%lx#", address, value );
		int numWritten = write( fdDev, cmd, cmdLen );
		if( numWritten == cmdLen ){
			char response[80];
			unsigned responseLen ;
			if( samba_response( fdDev, response, sizeof(response)-1, responseLen ) ){
				response[responseLen] = '\0' ;
				if( 0 != strchr( response, '>' ) ){
					return true ;
				}
				else
					fprintf( stderr, "Unexpected response: <%s>\n", response );
			}
			else
				fprintf( stderr, "Error reading readreg response\n" );

		}
		else
			fprintf( stderr, "short write: %d/%u\n", numWritten, cmdLen );
	}
	else
		fprintf( stderr, "Invalid longword address %08lx\n", address );

	return false ;
}

bool samba_verify_mem( 
   int fdDev, 
   void const *data,
   unsigned long address, 
   unsigned long count )
{
      bool worked = false ;
      FILE *fRx = tmpfile();
      if( fRx ){
              if( samba_download( fdDev, address, count, fileno(fRx) ) ){
                      printf( "file downloaded successfully\n" );
                      fseek( fRx, 0, SEEK_SET );
                      fflush(fRx);
                      void *tmpmem = mmap( 0, count, PROT_READ, MAP_PRIVATE, fileno(fRx), 0 );
                      if( tmpmem ){
                              if( 0 == memcmp( data, tmpmem, count ) ){
                                      worked = true ;
                              }
                              else
                                      fprintf( stderr, "content mismatch after download\n" );
                              munmap( tmpmem, count );
                      }
                      else
                              perror( "map<tmpFile>" );
              }
              else
                      printf( "error downloading file\n" );
              fclose( fRx );
      }
      else
              perror( "tmpfile" );

      return worked ;
}



void parse_samba_args( int &argc, char const **argv )
{
	for( int i = 1 ; i < argc ; i++ ){
		char const *arg = argv[i];
		if( '-' == *arg ){
			int match = 0 ;
			arg++ ;
			while( *arg ){
				char const c = toupper(*arg++);
				if( 'D' == c ){
					samba_debug = true ;
                                        match = 1 ;
				}
			}

			if( match ){
				// pull from argument list
				for( int j = i+1 ; j < argc ; j++ ){
					argv[j-1] = argv[j];
				}
				--i ;
				--argc ;
			}
		}
	}
}

