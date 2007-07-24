#ifndef __LLINIT_H__
#define __LLINIT_H__ "$Id$"

/*
 * llInit.h
 *
 * This header file declares the lowlevel_Init() routine,
 * which will initialize the BSS segment when running 
 * from ROM.
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

void lowlevel_Init( void ) __attribute__ ((noreturn));

//
// linker-generated symbols
//
extern char const textStart ;
extern char const textEnd ;
extern char dataStart ;
extern char dataEnd ;
extern char bssStart ;
extern char bssEnd ;
extern unsigned long imageLength ;
extern unsigned long imageCRC ;

#endif

