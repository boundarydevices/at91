#ifndef __USART_H__
#define __USART_H__ "$Id$"

/*
 * usart.h
 *
 * This header file declares some initialization routines
 * for USARTs and 
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#include "hardware.h"

#define NUM_USARTS 2
StructUSART * const usarts[NUM_USARTS];

#define DEFAULT_UART 0
#define DEFUART usarts[DEFAULT_UART]

void enableUART( unsigned which, unsigned baud );

void waitUartIdle(StructUSART *usart);
void write( StructUSART *usart, char const *str );
void writeHexChar( StructUSART *usart, unsigned char c );
void writeHex( StructUSART *usart, unsigned long val );
void writeChar( StructUSART *usart, unsigned char c );

// returns non-zero and the character in *inChar if a
// character is available
int read( StructUSART *usart, char *inChar );

#endif

