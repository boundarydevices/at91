/*
 * Program setPhy.c
 *
 * This program tests access to the PHY
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
#include "assert.h"

#define PHY_ADDR 0

static unsigned short readPhyReg( int reg ){
   EMAC->EMAC_NCR = AT91_EMAC_MAN_SOF 
                  | AT91_EMAC_MAN_RD 
                  | AT91_EMAC_MAN_CODE 
                  | AT91_EMAC_MAN_PHYA(PHY_ADDR) 
                  | AT91_EMAC_MAN_REGA(reg);
}

inline void writePhyReg( int reg, unsigned short value )
{
   EMAC->EMAC_NCR = AT91_EMAC_MAN_SOF 
                  | AT91_EMAC_MAN_WR 
                  | AT91_EMAC_MAN_CODE 
                  | AT91_EMAC_MAN_PHYA(PHY_ADDR) 
                  | AT91_EMAC_MAN_REGA(reg)
                  | AT91_EMAC_MAN_DATA(value);
}

void init( void )
{
        EMAC->EMAC_NCR = AT91_EMAC_NCR_MPE ; // Allow management access
	while(1){
           char c ;
           if( read( DEFUART, &c ) ){
              int i, stat, prevStat ;
              stat = EMAC->EMAC_NSR ;
              write( DEFUART, "... before: NSR 0x" ); writeHex( DEFUART, stat ); write( DEFUART, "\r\n" );
              writePhyReg( 0, 0 );
              stat = EMAC->EMAC_NSR ;
              prevStat = ~stat ;
              for( i = 0 ; i < 10000 ; i++ ){
                 if( prevStat != stat ){
                    prevStat = stat ;
                    write( DEFUART, "NSR 0x" ); writeHex( DEFUART, stat ); write( DEFUART, "\r\n" );
                 }
                 stat = EMAC->EMAC_NSR ;
              }

              write( DEFUART, "done\r\n" );
           } // read something
	}
}
