#ifndef __INIT_H__
#define __INIT_H__ "$Id$"

/*
 * init.h
 *
 * This header file declares the init() routine for
 * at91 applications. This routine is essentially
 * the familiar main() found in C applications, but
 * since an AT91 app never returns there's a conflict
 * using the name main().
 *
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

void init( void ) __attribute__ ((noreturn));

#endif

