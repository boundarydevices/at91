/*
 * Program at91up.cpp
 *
 * This program will upload and run a module to an attached AT91SAM7X
 * device over USB using the SAM-BA bootstrap protocol.
 * 
 * Command-line arguments include the run address and  the program to run.
 *
 * Options include:
 *	-v	verify program after download
 *	-d	debug SAMBA interactions
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
#include "uploadFlash.h"
#include "linux/inotify.h"
#include "machineConst.h"
#include "inotify.h"

static char const deviceName[] = {
	"/dev/at91sam7x0"
};
static bool verify = false ;
static bool runFromFlash = true ;
static bool runningSamba = false ;
static char const *imgName = "usbPeriph.rom.bin" ;
static int fdImage = -1 ;
static void *imgData = 0 ;
static unsigned imgAddr = 0x100000 ;
static unsigned imgSize = 0 ; // full size of the file
static unsigned imgExeSize = 0 ; // executable portion
static unsigned imgCRC = 0 ;

#define IMGLEN_OFFS 0x100

/*
#define FLASH_BASE 0x00100000
#define FLASH_PAGE_SIZE 256
#define NUM_FLASH_PAGES 1024
#define FLASH_SIZE (FLASH_PAGE_SIZE*NUM_FLASH_PAGES)

#define SRAM_START 0x00200000
#define SRAM_SIZE  0x00010000

*/


static void parseArgs( int &argc, char const **argv )
{
	unsigned argIdx = 0 ;
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
		else if( 0 == argIdx ){
			char *endPtr ;
			unsigned long addr = strtoul( arg, &endPtr, 0 );
			if( ( 0 < addr ) && ( 0 != endPtr ) && ( '\0' == *endPtr ) ){
				imgAddr = addr ;
				// checked later after file size is known
				argIdx++ ;
			}
			else {
				fprintf( stderr, "Invalid run address <%s>, use decimal or 0xHEX\n", arg );
				exit(1);
			}
		}
		else if( 1 == argIdx ){
                        imgName = arg ;
			argIdx++ ;
		}
	}
			
	fdImage = open( imgName, O_RDONLY );
	if( 0 > fdImage ){
		perror( imgName );
		exit(1);
	}
        struct stat st ;
	fstat( fdImage, &st );
	imgSize = st.st_size ;

	if( (FLASH_BASE <= imgAddr) 
	     && 
	    (FLASH_BASE+FLASH_SIZE > imgAddr+imgSize ) ){
		runFromFlash = true ;
	} else if( (SRAM_START <= imgAddr) 
		    && 
		   (SRAM_START+SRAM_SIZE > imgAddr+imgSize)){
		runFromFlash = false ;
	} else {
		fprintf( stderr, "Invalid address 0x%08lx for image size %lu\n",
			 imgAddr, imgSize );
		exit(1);
	}
	imgData = mmap( 0, st.st_size, PROT_READ, MAP_PRIVATE, fdImage, 0 );
	if( MAP_FAILED == imgData ){
		fprintf( stderr, "mmap(%s): %m", imgName );
		exit(1);
	}
	memcpy(&imgExeSize,((char *)imgData)+IMGLEN_OFFS, sizeof(imgExeSize));
	if( imgExeSize != imgSize-4){
		fprintf( stderr, "Image %s does not have proper length tag: %lu != %lu\n", 
			 imgName, imgExeSize, imgSize-4 );
		exit(1);
	}
	memcpy(&imgCRC,((char *)imgData)+imgExeSize,sizeof(imgCRC));
}

static bool go(int fdDev, unsigned addr ){
	char cmd[80];
	int cmdLen = snprintf( cmd, sizeof(cmd), "G%X#", addr );
	int numSent = write( fdDev, cmd, cmdLen );
	bool const rval = ( cmdLen == numSent );
	if( samba_debug ){
		if( rval ){
			printf( "program started\n" );
			int in = inotify_init();
			if( 0 <= in ){
				printf( "inotify initialized\n" );
			}
			else
				perror( "inotify_init" );
		}
		else
			printf( "Error %d/%d sending <Go> command\n", numSent, cmdLen );
	}
	return rval ;
}

#define FLASHER_PROG "usbFlash.ram.bin"
#define FLASHER_ADDR 0x202000

static bool uploadFlasher(int fdDev)
{
	bool worked = false ;
	int fdFlasher = open( FLASHER_PROG, O_RDONLY );
	if( 0 <= fdFlasher ){
		struct stat st ;
		fstat( fdFlasher, &st );
		void *mem = mmap( 0, st.st_size, PROT_READ, MAP_PRIVATE, fdFlasher, 0 );
		if( MAP_FAILED != mem )
		{
if( samba_debug ) printf( "Uploading %u bytes of %s\n", st.st_size, FLASHER_PROG );
			if( samba_upload( fdDev, FLASHER_ADDR, mem, st.st_size ) ){
if( samba_debug ) printf( "%lu bytes uploaded\n", st.st_size );
				if( verify ){
				   if( samba_verify_mem( fdDev, mem, FLASHER_ADDR, st.st_size ) ){
				      fprintf( stderr, "verified\n" );
				      worked = true ;
				   }
				}
				else
					worked = true ;
			}
			else
				printf( "Upload(flasher) error\n" );
			munmap( mem, st.st_size );
		}
		else
			perror( "mmap(flasher)" );
		close( fdFlasher );
	}
	else
		perror( FLASHER_PROG );

	if( !worked ){
		printf( "Error uploading " FLASHER_PROG "\n" );
	} else if( samba_debug ){
		printf( "Uploaded " FLASHER_PROG "\n" );
	}

	return worked ;
}

static void worked( void ){
	printf( "ok" );
	if( isatty( 1 ) ){
		printf( "\n" );
	}
}

int main( int argc, char const *argv[] )
{
	int rval = -1 ;
	parse_samba_args( argc, argv );
	parseArgs( argc, argv );
	if( verify )
		printf( "verify enabled\n" );
	if( 3 == argc ){
		int const fdDev = open( deviceName, O_RDWR );
		if( 0 <= fdDev ){
			fcntl( fdDev, F_SETFL, O_NONBLOCK );
			char const *runningImage ;
			unsigned runAddr, runLength, runCRC ;
			if( samba_verify( fdDev, runningImage, runAddr, runLength, runCRC ) ){
				if( ( imgAddr == runAddr )
				    &&
				    ( imgExeSize == runLength )
				    &&
				    ( imgCRC == runCRC ) ){
					printf( "already running %s at 0x%x\n", imgName, imgAddr );
					worked();
					return 0 ;
				}
				printf( "ready to upload %lu bytes from %s to address 0x%08lx\n", imgSize, imgName, imgAddr );
				printf( "running addr 0x%x, length %u, crc %u\n", runAddr, runLength, runCRC );
				printf( "img addr 0x%x, length %u, crc %u\n", imgAddr, imgExeSize, imgCRC );
                                runningSamba = (0 == runAddr) || (0==runLength);
/*
 * Three cases of interest:
 *	run from flash, running samba
 *	run from flash, running flasher
 *	run from RAM, running samba
 */
				if( runFromFlash ){
					unsigned long flashLen ;
					unsigned long flashCRC = 0 ;
					if( !samba_readreg_long( fdDev, imgAddr+IMGLEN_OFFS, flashLen ) ){
						fprintf( stderr, "Error reading length from flash\n" );
						return -1 ;
					}
					if( samba_debug ){
						printf( "image length in file %lu, flash == %lu\n", imgExeSize, flashLen );
					}
					if( ( flashLen == imgExeSize ) 
					    && 
                                            samba_readreg_long( fdDev, imgAddr+imgExeSize, flashCRC ) 
					    &&
					    (flashCRC == imgCRC ) ){
						if( samba_debug )
							printf( "crc match (0x%X). Just jump to 0x%X\n", flashCRC, imgAddr );
							if( go(fdDev, imgAddr) ){
								rval = 0 ;
							}
					} else if( runningSamba ){
						if( uploadFlasher(fdDev) ){
							if( samba_debug ) 
								printf( "flasher uploaded\n" );
							if( go(fdDev, FLASHER_ADDR) ){
								if( samba_debug ) 
									fprintf( stderr, "Flasher started. Now burn\n" );
								sleep( 1 );
								eatSambaData(fdDev);
								char const *flasherIdent ;
								unsigned flasherAddr ;
								unsigned flasherLen ;
								unsigned flasherCRC ;
                                                                unsigned iter = 0 ;
								for( iter = 0 ; iter < 2 ; iter++ ){
									if( samba_debug )
										printf( "try to verify samba\n" );
									if( samba_verify( fdDev, flasherIdent, flasherAddr, flasherLen, flasherCRC ) )
										break ;
									sleep( 1 );
									eatSambaData(fdDev);
								}
								if( samba_debug ) 
									fprintf( stderr, "Flasher verified. Now upload firmware\n" );
								if( (2 > iter) 
								    && 
								    uploadFlash( fdDev, imgAddr, (char const *)imgData, imgSize ) ){
									if( samba_debug )
										printf( "%u bytes written to flash\n", imgSize );
									if( go(fdDev, imgAddr) ){
										printf( "Now running %s at address 0x%x\n", imgName, imgAddr );
										sleep(1);
										eatSambaData(fdDev);
										rval = 0 ;
									}
									else
										printf( "Error starting %s at address 0x%x\n", imgName, imgAddr );
								}
								else
									fprintf( stderr, "Error validating flasher or uploading flash\n" );
							}
							else
								fprintf( stderr, "Error starting flasher\n" );
						}
					}
					else if( 0 != strstr( runningImage, "usbFlash.c" ) ){
						printf( "just program flash\n" );
						if( samba_debug ){
							printf( "flashCRC at 0x%x (flash says 0x%x, file says 0x%x)\n",
                                                                imgAddr+imgExeSize, flashCRC, imgCRC );
						}
						if( uploadFlash( fdDev, imgAddr, (char const *)imgData, imgSize ) ){
							printf( "%u bytes written to flash\n", imgSize );
							if( go(fdDev, imgAddr) ){
								printf( "Now running %s at address 0x%x\n", imgName, imgAddr );
								sleep(1);
								eatSambaData(fdDev);
								rval = 0 ;
							}
							else
								printf( "Error starting %s at address 0x%x\n", imgName, imgAddr );
						}
					}
					else
						fprintf( stderr, "Can't run flash image from %s\n", runningImage );
				} else if( runningSamba ){
					if( samba_upload( fdDev, imgAddr, imgData, imgSize ) ){
						if( samba_debug ){
							printf( "%lu bytes uploaded\n", imgSize );
						}

						bool worked = false ;
						if( verify ){
                                                        if( samba_verify_mem( fdDev, imgData, imgAddr, imgSize ) ){
								fprintf( stderr, "verified\n" );
								worked = true ;
							}
						}
						else
							worked = true ;

						if( worked ){
							if( go(fdDev, imgAddr) ){
								rval = 0 ;
							}
						}
					}
					else
						printf( "Upload error\n" );
				} else {
					fprintf( stderr, "Can't start this image from %s\n", runningImage );
				}
			}
			else
				printf( "Error verifying connection\n" );
			close( fdDev );
		}
		else
			perror( deviceName );
	}
	else {
		fprintf( stderr, "Usage: %s 0x202000|0x100000 myProg.bin\n"
				 "    -v    verify program after download\n"
				 "    -d    debug SAM-BA interactions\n"
			 , argv[0] );
	}
	if( 0 == rval ){
		worked();
	}
	return rval ;
}

