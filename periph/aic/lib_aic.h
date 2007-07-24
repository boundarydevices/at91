//*----------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : lib_aic.h
//* Object              : Advanced Interrupt Controller Library Function.
//*                       Prototyping File.
//*
//* 1.0 01/04/00 JCZ    : Creation
//*----------------------------------------------------------------------------

#ifndef lib_aic_h
#define lib_aic_h

#include    "periph/aic/aic.h"

typedef void (TypeAICHandler) (void) ;

/* Spurious Vector Reference (written in assembly) */
extern void at91_spurious_handler ( void ) ;

/* Function Prototyping */
extern void at91_default_irq_handler ( void );
extern void at91_default_fiq_handler ( void );
extern void at91_irq_open ( u_int irq_id,u_int priority,
                            u_int src_type, TypeAICHandler handler_pt ) ;

extern void at91_irq_close ( u_int irq_id ) ;
extern void at91_irq_trig_cmd ( u_int irq_id, u_int mask  ) ;
extern u_int at91_irq_get_status ( void ) ;

#endif /* lib_aic_h */
