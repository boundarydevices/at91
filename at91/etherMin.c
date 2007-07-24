/* 
 * minimum ethernet PHY register access routine
 */
#include "llInit.h"
#include "init.h"
#include "hardware.h"
#include "memory.h"
#include "usart.h"
#include "leds.h"
#include "usb_ll.h"
#include "sambaClone.h"
#include "messageRx.h"
#include "appSerial.h"
#include "appFRAM.h"
#include "appIds.h"

static void execute( char const *cmd )
{
	unsigned long reg = 0 ;
	while( 1 ){
		char const c = *cmd++ ;
		if( ( '0' <= c ) && ( '9' >= c ) ){
			reg <<= 4 ;
			reg += (c-'0');
		} else if( ( 'a' <= c ) && ( 'f' >= c ) ){
			reg <<= 4 ;
			reg += (c-'a'+10);
		}
		else {
			--cmd ;
			break ;
		}
	}
	
	write( DEFUART, "reg <" ); writeHex( DEFUART, reg ); write( DEFUART, ">  " );
	if( *cmd ){
		if( 0 != (reg&3) ){
			write( DEFUART, "bad address\r\n" );
		} else {
			unsigned value = 0 ;
			cmd++ ;
			while( 1 ){
				char const c = *cmd++ ;
				if( ( '0' <= c ) && ( '9' >= c ) ){
					value <<= 4 ;
					value += (c-'0');
				} else if( ( 'a' <= c ) && ( 'f' >= c ) ){
					value <<= 4 ;
					value += (c-'a'+10);
				}
				else {
					--cmd ;
					break ;
				}
			}
			write( DEFUART, "== <" ); writeHex( DEFUART, value ); write( DEFUART, ">\r\n" );
			*( (unsigned long *)reg ) = value ;
		}
	} else {
		write( DEFUART, "== 0x" ); writeHex( DEFUART, *((unsigned long *)reg) ); write( DEFUART, "\r\n" );
	}
}

void lowlevel_Init( void )
{
	int volatile i = 0 ;
	char inBuf[80];
	char *nextIn = inBuf ;
	char *const endIn = inBuf + sizeof(inBuf);

	enableUART(0,115200);
	enableUART(1,115200);

	PMC->PMC_PCER = PMC_PERIPHS ; // enable clocks
	PIOB->PIO_OER = PIOB_LEDMASK ;
	PIOB->PIO_CODR = PIOB_LEDMASK ; // light 'em up
//	PIOB->PIO_SODR = PIOB_LEDMASK ; // shut 'em down
        PIOB->PIO_PDR = EMAC_PIO_MASK ; // ethernet, not GPIO
        PIOA->PIO_PDR = UART_PIO_MASK | SPI0_PIO_MASK ; // these pins controlled by peripherals

        //
        // set SPI NPCS0 pin as a GPIO output
        //
        PIOA->PIO_PER = SPI0_NPCS0_MASK ;
        PIOA->PIO_OER = SPI0_NPCS0_MASK ;
//        PIOA->PIO_MDER = SPI0_NPCS0_MASK ; // -- need open-drain?
        PIOA->PIO_SODR = SPI0_NPCS0_MASK ; // normally high
	PIOA->PIO_PDR = SPI0_NPCS0_MASK ; // now let AT91 control it

	if( &textEnd != &dataStart ){
           memcpy( &dataStart, &textEnd, &dataEnd-&dataStart);
        }

	memset( &bssStart, 0, (char *)&bssEnd-(char *)&bssStart );

	usbll_init();
        messageRxInit();
	framInit();
	samba_init();

	setUsbCallback(USBAPP_FRAM,framUsbRx);
	
	while( 1 ){
		char c ;
		if( read( DEFUART, &c ) ){
			if( '\x0d' != c ){
				c |= '\x20' ;  // lower-case
				*nextIn++ = c ;
				if( nextIn < endIn ){
					writeChar( DEFUART, c );
				}
				else {
					write( DEFUART, "----> clear\r\n" );
					nextIn = inBuf ;
				}
			}
			else {
				*nextIn = 0 ;
				nextIn = inBuf ;
				write( DEFUART, "\r\nexecute <" );
				write( DEFUART, inBuf );
				write( DEFUART, "> here\r\n" );
				execute( inBuf );
			}
		}
		setLED(i++ >> 10);
		usbll_poll();
	}
}
