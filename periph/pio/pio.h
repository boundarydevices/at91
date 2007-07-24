//*----------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : pio.h
//* Object              : Parallel I/O Header File
//*
//* 1.0 01/04/00 JCZ    : Creation
//*----------------------------------------------------------------------------

#ifndef pio_h
#define pio_h

#include    "periph/stdc/std_c.h"
#include    "periph/aic/lib_aic.h"

/*------------------------------------------------------------*/
/* User Interface Parallel I/O Interface Structure Definition */
/*------------------------------------------------------------*/

typedef struct
{
    at91_reg        PIO_PER ;           /* PIO Enable Register */
    at91_reg        PIO_PDR ;           /* PIO Disable Register */
    at91_reg        PIO_PSR ;           /* PIO Status Register */
    at91_reg        Reserved0 ;
    at91_reg        PIO_OER ;           /* Output Enable Register */
    at91_reg        PIO_ODR ;           /* Output Disable Register */
    at91_reg        PIO_OSR ;           /* Output Status Register */
    at91_reg        Reserved1 ;
    at91_reg        PIO_IFER ;          /* Input Filter Enable Register */
    at91_reg        PIO_IFDR ;          /* Input Filter Disable Register */
    at91_reg        PIO_IFSR ;          /* Input Filter Status Register */
    at91_reg        Reserved2 ;
    at91_reg        PIO_SODR ;          /* Set Output Data Register */
    at91_reg        PIO_CODR ;          /* Clear Output Data Register */
    at91_reg        PIO_ODSR ;          /* Output Data Status Register */
    at91_reg        PIO_PDSR ;          /* Pin Data Status Register */
    at91_reg        PIO_IER ;           /* Interrupt Enable Register */
    at91_reg        PIO_IDR ;           /* Interrupt Disable Register */
    at91_reg        PIO_IMR ;           /* Interrupt Mask Register */
    at91_reg        PIO_ISR ;           /* Interrupt Status Register */
    at91_reg        PIO_MDER ;          /* Multi Driver Enable Register */
    at91_reg        PIO_MDDR ;          /* Multi Driver Disable Register */
    at91_reg        PIO_MDSR ;          /* Multi Driver Status Register */
} StructPIO ;

#endif /* pio_h */
