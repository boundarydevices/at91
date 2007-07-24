/*
 * Module flash.c
 *
 * This module defines the flash writing routines as
 * declared in flash.h
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */


#include "flash.h"
#include "hardware.h"
#include "usart.h"
// #define DEBUG
#include "debug.h"

#define VALID_ADDR( addr ) ((FLASH_BASE <= (addr)) && ((FLASH_BASE+FLASH_SIZE-1)>=(addr)))
#define FLASH_PAGE(addr) (((addr)-FLASH_BASE)/FLASH_PAGE_SIZE)
#define LOCK_BIT(addr) (((addr)-FLASH_BASE)/FLASH_LOCK_SIZE)

static void copyLongs( unsigned long       *dest,
		       unsigned long const *src,
		       unsigned             count )
{
   while( 0 < count-- )
      *dest++ = *src++ ;
}

int flashProg( unsigned addr, unsigned length, void const *data )
{
   int rval = -1 ;
   if( VALID_ADDR(addr) ){
      if( 0 == (addr&(FLASH_PAGE_SIZE-1)) ){
         if( 0 == (length&(FLASH_PAGE_SIZE-1)) ){
            rval = 0 ;

            while( 0 < length ){
               int const lockbit = LOCK_BIT(addr);
               int const page = FLASH_PAGE(addr);

               if( EFC0->MC_FSR & (1<<(lockbit+16)) ){
                  DEBUGMSG( "unlock seg 0x" ); DEBUGHEXCHAR( lockbit ); DEBUGMSG( " here\n" );
                  EFC0->MC_FCR = (0x5A<<24)|(page<<8)|EFC_COMMAND_CLEARLOCKBIT ;
                  while( 0 == (EFC0->MC_FSR & MC_FSR_FRDY) )
                     ;
                  if( 0 != (EFC0->MC_FSR & (MC_FSR_LOCKE|MC_FSR_PROGE)) ){
                     rval = 4 ;
                     break ;
                  }
               }
               DEBUGMSG( "program 0x" ); DEBUGHEX( addr ); DEBUGMSG( " here\r\n" );

               // fill the write buffer
               copyLongs( (unsigned long *)addr, (unsigned long *)data, FLASH_PAGE_SIZE/sizeof(unsigned long) );

               // now program
               EFC0->MC_FCR = (0x5A<<24)|(page<<8)|EFC_COMMAND_WRITEPAGE ;

               // wait for completion
               while( 0 == (EFC0->MC_FSR & MC_FSR_FRDY) )
                  ;

               if( 0 != (EFC0->MC_FSR & (MC_FSR_LOCKE|MC_FSR_PROGE)) ){
                  rval = 5 ;
                  break ;
               }

               length -= FLASH_PAGE_SIZE ;
               addr += FLASH_PAGE_SIZE ;
               data = ((char *)data) + FLASH_PAGE_SIZE ;
            }
         }
         else
            rval = 3 ;
      }
      else
         rval = 2 ;
   }
   else
      rval = 1 ;

   return rval ;
}

int flashVerify( unsigned addr, unsigned length, void const *data )
{
   int rval = -1 ;
   return rval ;
}


