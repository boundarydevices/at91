/*
 * Program at91up.cpp
 *
 * This program will upload and run a module to an attached AT91SAM7X
 * device over USB using the SAM-BA bootstrap protocol.
 * 
 * Command-line arguments include the USB device, the run address and 
 * the program name.
 *
 * Options include:
 *
 *	-v	verify program after download
 *	-g	issue "Go" command after download (and verify if needed)
 * 
 * Example:
 *
 *	at91up /dev/ttyUSB1 0x202000 myProg.bin
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

static bool verify = false ;
static bool go = false ;

static void parseArgs( int &argc, char const **argv )
{
	for( int i = 1 ; i < argc ; i++ ){
		char const *arg = argv[i];
		if( '-' == *arg ){
			arg++ ;
			while( *arg ){
				char const c = toupper(*arg++);
				if( 'G' == c ){
					go = true ;
				} else if( 'V' == c ){
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
	parse_samba_args( argc, argv );
	parseArgs( argc, argv );
	if( verify )
		printf( "verify enabled\n" );
	if( go )
		printf( "Go enabled\n" );
	if( 4 == argc ){
		int const fdUpload = open( argv[1], O_RDWR );
		if( 0 <= fdUpload ){
			char *endPtr ;
			unsigned long addr = strtoul( argv[2], &endPtr, 0 );
			if( ( 0 < addr ) && ( 0 != endPtr ) && ( '\0' == *endPtr ) ){
				int const fdIn = open( argv[3], O_RDONLY );
				if( 0 <= fdIn ){
					char const *imageName ;
					unsigned runAddr, runLength, runCRC ;
					if( samba_verify( fdUpload, imageName, runAddr, runLength, runCRC ) ){
						struct stat st ;
						fstat( fdIn, &st );
						void *mem = mmap( 0, st.st_size, PROT_READ, MAP_PRIVATE, fdIn, 0 );
						if( MAP_FAILED != mem )
						{
							printf( "ready to upload %lu bytes\n", st.st_size );
							if( samba_upload( fdUpload, addr, mem, st.st_size ) ){
								printf( "%lu bytes uploaded\n", st.st_size );
								bool worked = false ;
								if( verify ){
                                                                   if( samba_verify_mem( fdUpload, mem, addr, st.st_size ) ){
                                                                      fprintf( stderr, "verified\n" );
                                                                      worked = true ;
                                                                   }
								   else
                                                                      fprintf( stderr, "verify error\n" );
								}
								else
									worked = true ;

								if( worked && go ){
                                                                        char cmd[80];
									int cmdLen = snprintf( cmd, sizeof(cmd), "G%lX#", addr );
                                                                        int numSent = write( fdUpload, cmd, cmdLen );
									if( cmdLen == numSent ){
										printf( "program started\n" );
									}
									else
										printf( "Error sending <Go> command\n" );
								}
							}
							else
								printf( "Upload error\n" );
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
		fprintf( stderr, "Usage: %s /dev/ttyUSB1 0x202000 myProg.bin\n"
				 "    -v    verify program after download\n"
				 "    -g    issue 'Go' command after download (and verify if needed)\n"
			 , argv[0] );
	}
	return 0 ;
}

