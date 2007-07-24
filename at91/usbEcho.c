/*
 * Program usbPoll.cpp
 *
 * This program tests polling the USB Device Port on the
 * AT91SAM7X.
 *
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */


#include "init.h"
#include "hardware.h"
#include "leds.h"
#include "usart.h"

static char const * const epTypes_[] = {
	"Ctrl"
,	"Is-Out"
,	"B-Out"
,	"Int-Out"
,	"???"
,	"Is-In"
,	"B-In"
,	"Int-In"
};

extern char const textStart ;
extern char const textEnd ;
extern char const dataStart ;
extern char const dataEnd ;
extern char const bssStart ;
extern char const bssEnd ;

char const * const appName_ = __FILE__ " " __DATE__ " " __TIME__ ;

void init( void )
{
	setLED(1);

	enableUART(0,115200);
	enableUART(1,115200);
	
	setLED(2);
	while(1){
		char c ;
		int i ;
		if( read( DEFUART, &c ) ){
			write( DEFUART, "frame num: " ); writeHex( DEFUART, UDP->UDP_NUM ); write( DEFUART, "\r\n" );
			setLED( c );
			switch( c & ~0x20 ){
				case 'C' : {
					for( i = 0 ; i < UDP_MAXENDPOINTS ; i++ ){
						unsigned int const dStatus = UDP->UDP_CSR[i];
						unsigned short epType = (dStatus & AT91C_UDP_EPTYPE) >> 8 ;
						write( DEFUART, "CSR[" ); writeHex( DEFUART, i ); write( DEFUART, "] == 0x" );
							writeHex( DEFUART, dStatus ); write( DEFUART, "   (" );
							write( DEFUART, epTypes_[epType] ); write( DEFUART, ")\r\n" );
					}
					break ;
				}
				case 'I' : {
					at91_reg *csr = UDP->UDP_CSR+3 ;
                                        at91_reg *fifo = UDP->UDP_FDR+3 ;
					for( i = 0 ; i < 8 ; i++ )
						*fifo = 'x' ;
					UDP_SETEPFLAGS(csr, AT91C_UDP_TXPKTRDY);

					break ;
				}

				case 'A' : {
					write( DEFUART, "text: " ); 
							writeHex( DEFUART, (unsigned long)&textStart ); write( DEFUART, ".." );
							writeHex(DEFUART, (unsigned long)&textEnd); write( DEFUART, "\r\n" );
					write( DEFUART, "data: " ); 
							writeHex( DEFUART, (unsigned long)&dataStart ); write( DEFUART, ".." );
							writeHex(DEFUART, (unsigned long)&dataEnd); write( DEFUART, "\r\n" );
					write( DEFUART, "bss: " );  
							writeHex( DEFUART, (unsigned long)&bssStart ); write( DEFUART, ".." );
							writeHex(DEFUART, (unsigned long)&bssEnd); write( DEFUART, "\r\n" );

					break ;
				}

				default: {
					write( DEFUART,
					       "Unknown command:\r\n"
					       "   C	- Show CSRs for each endpoint\r\n" 
					       "   I	- Send junk to ISO/INT endpoint(EP#3)\r\n" 
					       "   A	- Show linker addresses\r\n" 
					);
					break ;
				}
			}
		}

		for( i = 0 ; i < UDP_MAXENDPOINTS ; i++ ){
                        unsigned int dStatus = UDP->UDP_CSR[i];
			unsigned int const rxMask = dStatus & (AT91C_UDP_RX_DATA_BK0|AT91C_UDP_RX_DATA_BK1);
			if( dStatus & AT91C_UDP_TXCOMP ){
				UDP_CLEAREPFLAGS(UDP->UDP_CSR+i, AT91C_UDP_TXCOMP);
				write( DEFUART, "txComp: " ); writeHexChar( DEFUART, i ); write( DEFUART, "\r\n" );
			}
			if( rxMask ){
				unsigned short rxLength = ( dStatus >> 16 ) & 0x7FF ;
				unsigned j ;
				unsigned short epType = (dStatus & AT91C_UDP_EPTYPE);
				char rxData[512];
				at91_reg *fifo = UDP->UDP_FDR+i;
				unsigned numRead = rxLength ;
				if( numRead > sizeof( rxData ) ){
					numRead = sizeof(rxData);
				}

				for( j = 0 ; j < numRead ; j++ ){
					rxData[j] = *fifo ;
				}
                                UDP_CLEAREPFLAGS( UDP->UDP_CSR+i, rxMask );
				write( DEFUART, "rxEP:" ); writeHexChar( DEFUART, i ); 
					write( DEFUART, "  count:" ); writeHex( DEFUART, rxLength ); write( DEFUART, "\r\n" );
				for( j = 0 ; j < numRead ; j++ ){
					writeHexChar( DEFUART, rxData[j] ); writeChar( DEFUART, ' ' );
					if( 0x0f == ( j & 0x0f ) ){
						write( DEFUART, "\r\n" );
					}
				}
				if( 0 != ( numRead & 0x0f ) )
                                        write( DEFUART, "\r\n" );
				if( AT91C_UDP_EPTYPE_BULK_OUT == epType ){
					write( DEFUART, "--> Bulk out\r\n" );
					for( j = 0 ; j < UDP_MAXENDPOINTS ; j++ ){
						at91_reg *csr = UDP->UDP_CSR + j;
						dStatus = *csr ;
						epType = (dStatus & AT91C_UDP_EPTYPE);
						if( AT91C_UDP_EPTYPE_BULK_IN == epType ){
							unsigned l ;
                                                        at91_reg *fifo = UDP->UDP_FDR+j ;
							write( DEFUART, "--> Bulk in on ep " );
							writeHexChar( DEFUART, j );
							write( DEFUART, "\r\n" );

							for( l = 0 ; l < numRead ; l++ )
								*fifo = rxData[l];
							UDP_SETEPFLAGS(csr, AT91C_UDP_TXPKTRDY);
							break ;
						} // found bulk out
					} // find bulk out
				} // received bulk in... echo to bulk out
				else {
					write( DEFUART, "epType " ); writeHex( DEFUART, epType ); write( DEFUART, "\r\n" );
				}
			} // received something
		}
	}
}
