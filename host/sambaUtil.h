#ifndef __SAMBAUTIL_H__
#define __SAMBAUTIL_H__ "$Id$"

/*
 * sambaUtil.h
 *
 * This header file declares utility routines for talking
 * to the SAM-BA boot loader software through USB. (It could
 * also be used to talk over serial if such a thing worked)
 *
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

/*
 * Reads a response from the AT91SAM7X (as determined by the
 * presence of a terminator of ">\n".
 *
 * returns true and length if successful, false if a timeout 
 * (1s) occurs or we read past the max length
 */
extern bool samba_response( int fdUp, char *response, unsigned max, unsigned &length, unsigned long timeout=1000 );
							      
/*
 * Verify a SAM-BA connection by sending a V# command and reading
 * the response.
 *
 * Returns true if the connection looks good.
 * 
 * Returns a string to identify the image, and a run address, length 
 * and CRC if the message looks like one of our apps (using at91/sambaClone.c).
 *
 * In general, if length or address is zero, we're probably running Samba.
 */
extern bool samba_verify( int fdUp,
			  char const  *&image,	// image name: free() when done
			  unsigned     &addr, 
			  unsigned     &length,
			  unsigned     &crc );

/*
 * Uploads a file to the specified address
 * Returns true to indicate success, false if a timeout occurs.
 */
extern bool samba_upload( int fdUp, unsigned long address, 
			  void const *data, unsigned length );

/*
 * Download a file from the specified address/length
 * Returns true to indicate success, false if a timeout occurs.
 */
extern bool samba_download( int fdDev, 
			    unsigned long address, 
			    unsigned long length,
			    int fdOut );

/*
 * Reads a memory value (longword only)
 * Returns true and the value if successful, false if it times out
 */
extern bool samba_readreg_long( int fdDev, unsigned long address, unsigned long &value );

/*
 * Reads a memory value (longword only)
 * Returns true and the value if successful, false if it times out
 */
extern bool samba_writereg_long( int fdDev, unsigned long address, unsigned long value );

/*
 * Returns true if the memory range from the
 * device matches the content passed in.
 */
extern bool samba_verify_mem( int fdDev, 
                              void const *data,
                              unsigned long address, 
                              unsigned long count );

extern bool samba_debug ;

/*
 * parse args for a samba debug flag
 *
 */
extern void parse_samba_args( int &argc, char const **argv );

/*
 * Use this to flush any stale in-bound data
 */
void eatSambaData( int fdUp );

#endif

