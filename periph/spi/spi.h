//*---------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*---------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*-----------------------------------------------------------------------------
//* File Name           : spi.h
//* Object              : Serial Peripheral Interface Definition File
//*
//* 1.0 01/04/00 JCZ    : Creation
//*---------------------------------------------------------------------------

#ifndef spi_h
#define spi_h

/*--------------------------*/
/* SPI Structure Definition */
/*--------------------------*/

typedef struct
{
    at91_reg        SP_CR ;     /* Control Register */
    at91_reg        SP_MR ;     /* Mode Register */
    at91_reg        SP_RDR ;    /* Receive Data Register */
    at91_reg        SP_TDR ;    /* Transmit Data Register */
    at91_reg        SP_SR ;     /* Status Register */
    at91_reg        SP_IER ;    /* Interrupt Enable Register */
    at91_reg        SP_IDR ;    /* Interrupt Disable Register */
    at91_reg        SP_IMR ;    /* Interrupt Mask Register */
    at91_reg        SP_RPR ;    /* Receive Pointer_Register */
    at91_reg        SP_RCR ;    /* Receive Counter Register */
    at91_reg        SP_TPR ;    /* Transmit Pointer_Register */
    at91_reg        SP_TCR ;    /* Transmit Counter Register */
    at91_reg        SP_CSR[4] ; /* Chip Select Register 0 to 3 */
} StructSPI ;

/*------------------*/
/* Control Register */
/*------------------*/

#define SP_SPIEN                0x1         /* SPI Enable */
#define SP_SPIDIS               0x2         /* SPI Disable */
#define SP_SWRST                0x80        /* SPI Software Reset */

/*---------------*/
/* Mode Register */
/*---------------*/

#define SP_MSTR                 0x1         /* Master Mode Select */

#define SP_PS                   0x2         /* Peripheral Select */
#define SP_PS_FIXED             0x0         /* Fixed Peripheral Select */
#define SP_PS_VARIABLE          0x2         /* Variable Peripheral Select */

#define SP_PCSDEC               0x4         /* Chip Select Decode */
#define SP_DIV32                0x8         /* Clock Selection */
#define SP_LLB                  0x80        /* Local Loopback */

#define SP_PCS                  0xF0000     /* Peripheral Chip Select */
#define SP_PCS0                 0xE0000     /* Chip Select 0 */
#define SP_PCS1                 0xD0000     /* Chip Select 1 */
#define SP_PCS2                 0xB0000     /* Chip Select 2 */
#define SP_PCS3                 0x70000     /* Chip Select 3 */

#define SP_DLYBCS               0xFF000000  /* Delay Before Chip Selects */

/*-----------------*/
/* Status Register */
/*-----------------*/

#define SP_RDRF                 0x1         /* Receive Data Register Full */
#define SP_TDRE                 0x2         /* Transmitte Data Register Empty */
#define SP_MODF                 0x4         /* Mode Fault */
#define SP_OVRES                0x8         /* Overrun Error Status */
#define SP_ENDRX                0x10        /* End of Receiver Transfer */
#define SP_ENDTX                0x20        /* End of Transmitter Transfer */
#define SP_SPIENS               0x10000     /* SPI Enable Status */

/*------------------------------------*/
/* Receive and Transmit Data Register */
/*------------------------------------*/

#define SP_RD                   0xFFFF      /* Receive Data */
#define SP_TD                   0xFFFF      /* Transmit Data */

/*----------------------*/
/* Chip Select Register */
/*----------------------*/

#define SP_CPOL                 0x1         /* Clock Polarity */
#define SP_NCPHA                0x2         /* Clock Phase */
#define SP_BITS                 0xF0        /* Bits Per Transfer */

#define SP_BITS                 0xF0        /* Bits Per Transfer */
#define SP_BITS_8               0x00        /* 8 Bits Per Transfer */
#define SP_BITS_9               0x10        /* 9 Bits Per Transfer */
#define SP_BITS_10              0x20        /* 10 Bits Per Transfer */
#define SP_BITS_11              0x30        /* 11 Bits Per Transfer */
#define SP_BITS_12              0x40        /* 12 Bits Per Transfer */
#define SP_BITS_13              0x50        /* 13 Bits Per Transfer */
#define SP_BITS_14              0x60        /* 14 Bits Per Transfer */
#define SP_BITS_15              0x70        /* 15 Bits Per Transfer */
#define SP_BITS_16              0x80        /* 16 Bits Per Transfer */

#define SP_SCBR                 0xFF00      /* Serial Clock Baud Rate */
#define SP_DLYBS                0xFF0000    /* Delay Before SPCK */
#define SP_DLYBCT               0xFF000000  /* Delay Between Consecutive Transfer */

#endif /* spi_h */
