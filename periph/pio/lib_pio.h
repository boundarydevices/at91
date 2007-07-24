//*----------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : lib_pio.h
//* Object              : Parallel I/O Function Prototyping File.
//*
//* 1.0 01/04/00 JCZ    : Creation
//*----------------------------------------------------------------------------

#ifndef lib_pio_h
#define lib_pio_h

#include "periph/pio/pio.h"
#include "periph/aic/lib_aic.h"

/* Configuration bits Definition : argument <config> of at91_pio_open */
#define PIO_SENSE_BIT           0x1
#define PIO_OUTPUT              0x1
#define PIO_INPUT               0x0
#define PIO_FILTER_BIT          0x2
#define PIO_FILTER_ON           0x2
#define PIO_FILTER_OFF          0x0
#define PIO_OPENDRAIN_BIT       0x4
#define PIO_OPENDRAIN_ON        0x4
#define PIO_OPENDRAIN_OFF       0x0
#define PIO_INPUT_IRQ_BIT       0x8
#define PIO_INPUT_IRQ_ON        0x8
#define PIO_INPUT_IRQ_OFF       0x0
#define RESET_PIO_CONF          (PIO_INPUT|PIO_FILTER_OFF|PIO_OPENDRAIN_OFF|PIO_INPUT_IRQ_OFF)

/* Configuration bits Definition : argument <state> of at91_pio_write */
#define PIO_SET_OUT         0x0
#define PIO_CLEAR_OUT       0x1

/*-------------------------------------*/
/* PIO Controller Descriptor Structure */
/*-------------------------------------*/
typedef struct
{
    StructPIO       *pio_base ;         /* Base Address */
    u_char          periph_id ;         /* Peripheral Identifier */
    u_char          pio_number ;        /* Total Pin Number */
} PioCtrlDesc ;

/* Functions Prototyping */
extern void  at91_pio_open  ( const PioCtrlDesc *pio_pt, u_int mask, u_int config ) ;
extern void  at91_pio_close ( const PioCtrlDesc *pio_pt, u_int mask ) ;
extern void  at91_pio_write ( const PioCtrlDesc *pio_pt, u_int mask, u_int data ) ;
extern u_int at91_pio_read  ( const PioCtrlDesc *pio_pt ) ;
extern void  at91_pio_set_mode  ( const PioCtrlDesc *pio_pt, u_int mask, u_int mode ) ;

#endif /* lib_pio_h */
