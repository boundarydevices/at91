/*
 * Program uartEcho.cpp
 *
 * This program sets each serial port to 115200,8,N,1
 * and simply echoes each character read, changing
 * case along the way.
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

char const * const appName_ = __FILE__ " " __DATE__ " " __TIME__ ;

void init( void )
{
	int i, j ;

	setLED(1);
	
	// initialize com ports
	for( i = 0 ; i < NUM_USARTS ; i++ ){
		enableUART(i,115200);
	}

	setLED(2);
	j = 3 ;
	while(1){
		for( i = 0 ; i < NUM_USARTS ; i++ ){
			char c ;
			StructUSART *const usart = usarts[i];
			if( read( usart, &c ) ){
				if( i )
					c ^= 0x20 ;
				writeChar( usart, c );
				setLED(c);
				writeChar( usart, 0x30+(j%10) );
				write( usart, "\r\n" );
	                        j++ ;
        		}
		}
	}
}
