/*
 * Module uploadFlash.cpp
 *
 * This module defines the uploadFlash() routine as declared
 * in uploadFlash.h
 *
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */


#include "uploadFlash.h"
#include "machineConst.h"
#include "appMsgDirect.h"
#include "msgFlash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/errno.h>
#include <unistd.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/fcntl.h>

inline long long timeValToMs( struct timeval const &tv )
{
   return ((long long)tv.tv_sec*1000)+((long long)tv.tv_usec / 1000 );
}

inline long long tickMs()
{
   struct timeval now ; gettimeofday( &now, 0 );
   return timeValToMs( now );
}

typedef struct {
        struct appHeader_t 	appHdr_ ;
        msgFlashRequest_t	flashHdr_ ;
	unsigned char		data_[PAGE_SIZE];
} flashProgram_t ;

typedef struct {
	struct appHeader_t	appHdr_ ;
	msgFlashResponse_t	flashHdr_ ;
} flashResponse_t ;

static bool readAll( int fdUpload, void *data, unsigned length )
{
   while( 0 < length ){
      struct pollfd fds ;
      fds.fd = fdUpload ;
      fds.events = POLLIN ;
      int rval ;
      if( ( 1 == poll( &fds, 1, 2000 ) )
	  &&
	  ( 0 < ( rval = read( fdUpload, data, length ) ) ) ){
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

static char const devName[] = {
        "/dev/atmel_flash"
};

bool uploadFlash( int fdx, unsigned long addr, char const *data, unsigned length )
{
	if( (addr < FLASH_BASE)
	    ||
	    (addr >= FLASH_BASE+FLASH_SIZE)
	    ||
	    (0 != (addr & (PAGE_SIZE-1))) ){
                printf( "Invalid flash address or length: %lx\n", addr );
		return false ;
        }

	printf( "programming flash at address %lx\n", addr );

	bool worked = false ;
	int fdOut = open( devName, O_WRONLY );
	if( 0 <= fdOut ){
		worked = true ;
		unsigned long total = 0 ;
		long long const start = tickMs();
		while( worked && ( 0 < length ) ){
			int numWritten = write( fdOut, data, length );
			if( 0 < numWritten ){
				data += numWritten ;
				length -= numWritten ;
				total += numWritten ;
			}
			else {
				fprintf( stderr, "write(%s):%m", devName );
				worked = false ;
			}
		}
		close( fdOut );
		printf( "wrote %lu bytes in %llu ms\n", total, tickMs()-start );
	}
	else
		perror( devName );

	return worked ;
}


