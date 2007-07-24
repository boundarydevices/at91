/*
 * Module llInit.cpp
 *
 * This module defines the lowlevel_Init() routine
 * as declared in llInit.h
 *
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */


#include "llInit.h"
#include "init.h"
#include "hardware.h"
#include "memory.h"
#include "usart.h"
#include "sambaClone.h"

unsigned long imageCRC = 0xDEADBEEF ; // non-zero to force into data segment

void lowlevel_Init( void )
{
	char const *crcAddr ;
        PMC->CKGR_MCKR = 0x07 ; // PLL, prescale x 2
	PMC->PMC_PCER = PMC_PERIPHS ; // enable clocks

	// master clock is 48MHz, number of clocks in 1.5uS is 72, round up
	EFC0->MC_FMR = ( EFC0->MC_FMR & 0xFFFF) | (73 << 16);

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
        } // copy (initialized) data segment
	
	enableUART(0,115200);
	enableUART(1,115200);

	// save image CRC (overlaid with BSS when running from RAM)
        crcAddr = &textEnd+(&dataEnd-&dataStart);
	memcpy( &imageCRC, crcAddr, sizeof(imageCRC));
	write( DEFUART, "imageCRC: " ); writeHex( DEFUART, imageCRC ); write( DEFUART, "\r\n" );
	memset( &bssStart, 0, (char *)&bssEnd-(char *)&bssStart );

	write( DEFUART, "starting app <" );
	write( DEFUART, appName_ );
	write( DEFUART, ">\r\n" );
	waitUartIdle(DEFUART);
	init();
}
