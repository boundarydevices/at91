#ifndef __HARDWARE_H__
#define __HARDWARE_H__ "$Id$"

/*
 * hardware.h
 *
 * This header file declares accessor macros for the 
 * base hardware on the AT91SAM7X processors.
 *
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#ifndef _ASMLANGUAGE

#include "periph/usart/usart.h"
#include "periph/pio/pio.h"
#include "periph/spi/spi.h"

#define USART0 ((StructUSART *)0xFFFC0000)
#define USART1 ((StructUSART *)0xFFFC4000)

#define PIOA   ((StructPIO *)0xFFFFF400)
#define PIOB   ((StructPIO *)0xFFFFF600)

#define UART_PIO_MASK 0x3FF // USART0/1	low 10 bits

#define PIOB_LEDMASK	0x7800000
#define PIOB_LEDSHIFT	23

#define PIOB_USB_PULLUP	0x80000		/* PB19 */

typedef struct
{
    at91_reg        WD_CR ;        /* Control Register */
    at91_reg        WD_MR ;        /* Mode Register */
    at91_reg        WD_SR ;        /* Status Register */
} StructWD ;

#define WDT ((StructWD *)0xFFFFFD40)

typedef struct
{
    at91_reg        RSTC_CR ;        /* Control Register */
    at91_reg        RSTC_SR ;        /* Status Register */
    at91_reg        RSTC_MR ;        /* Mode Register */
} StructRSTC ;


#define RSTC ((StructRSTC *)0xFFFFFD00)

#define UDP_MAXENDPOINTS 6

extern unsigned const endpointMaxSize[UDP_MAXENDPOINTS];

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR USB Device Interface
// *****************************************************************************
typedef struct _AT91S_UDP {
	at91_reg	 UDP_NUM; 	// Frame Number Register
	at91_reg	 UDP_GLBSTATE; 	// Global State Register
	at91_reg	 UDP_FADDR; 	// Function Address Register
	at91_reg	 Reserved0[1]; 	// 
	at91_reg	 UDP_IER; 	// Interrupt Enable Register
	at91_reg	 UDP_IDR; 	// Interrupt Disable Register
	at91_reg	 UDP_IMR; 	// Interrupt Mask Register
	at91_reg	 UDP_ISR; 	// Interrupt Status Register
	at91_reg	 UDP_ICR; 	// Interrupt Clear Register
	at91_reg	 Reserved1[1]; 	// 
	at91_reg	 UDP_RSTEP; 	// Reset Endpoint Register
	at91_reg	 Reserved2[1]; 	// 
	at91_reg	 UDP_CSR[6]; 	// Endpoint Control and Status Register
	at91_reg	 Reserved3[2]; 	// 
	at91_reg	 UDP_FDR[6]; 	// Endpoint FIFO Data Register
	at91_reg	 Reserved4[3]; 	// 
	at91_reg	 UDP_TXVC; 	// Transceiver Control Register
} AT91S_UDP, *AT91PS_UDP;

#define UDP ((AT91S_UDP *)0xFFFB0000)

// -------- UDP_FRM_NUM : (UDP Offset: 0x0) USB Frame Number Register -------- 
#define AT91C_UDP_FRM_NUM     ((unsigned int) 0x7FF <<  0) // (UDP) Frame Number as Defined in the Packet Field Formats
#define AT91C_UDP_FRM_ERR     ((unsigned int) 0x1 << 16) // (UDP) Frame Error
#define AT91C_UDP_FRM_OK      ((unsigned int) 0x1 << 17) // (UDP) Frame OK
// -------- UDP_GLB_STATE : (UDP Offset: 0x4) USB Global State Register -------- 
#define AT91C_UDP_FADDEN      ((unsigned int) 0x1 <<  0) // (UDP) Function Address Enable
#define AT91C_UDP_CONFG       ((unsigned int) 0x1 <<  1) // (UDP) Configured
#define AT91C_UDP_ESR         ((unsigned int) 0x1 <<  2) // (UDP) Enable Send Resume
#define AT91C_UDP_RSMINPR     ((unsigned int) 0x1 <<  3) // (UDP) A Resume Has Been Sent to the Host
#define AT91C_UDP_RMWUPE      ((unsigned int) 0x1 <<  4) // (UDP) Remote Wake Up Enable
// -------- UDP_FADDR : (UDP Offset: 0x8) USB Function Address Register -------- 
#define AT91C_UDP_FADD        ((unsigned int) 0xFF <<  0) // (UDP) Function Address Value
#define AT91C_UDP_FEN         ((unsigned int) 0x1 <<  8) // (UDP) Function Enable
// -------- UDP_IER : (UDP Offset: 0x10) USB Interrupt Enable Register -------- 
#define AT91C_UDP_EPINT0      ((unsigned int) 0x1 <<  0) // (UDP) Endpoint 0 Interrupt
#define AT91C_UDP_EPINT1      ((unsigned int) 0x1 <<  1) // (UDP) Endpoint 0 Interrupt
#define AT91C_UDP_EPINT2      ((unsigned int) 0x1 <<  2) // (UDP) Endpoint 2 Interrupt
#define AT91C_UDP_EPINT3      ((unsigned int) 0x1 <<  3) // (UDP) Endpoint 3 Interrupt
#define AT91C_UDP_EPINT4      ((unsigned int) 0x1 <<  4) // (UDP) Endpoint 4 Interrupt
#define AT91C_UDP_EPINT5      ((unsigned int) 0x1 <<  5) // (UDP) Endpoint 5 Interrupt
#define AT91C_UDP_RXSUSP      ((unsigned int) 0x1 <<  8) // (UDP) USB Suspend Interrupt
#define AT91C_UDP_RXRSM       ((unsigned int) 0x1 <<  9) // (UDP) USB Resume Interrupt
#define AT91C_UDP_EXTRSM      ((unsigned int) 0x1 << 10) // (UDP) USB External Resume Interrupt
#define AT91C_UDP_SOFINT      ((unsigned int) 0x1 << 11) // (UDP) USB Start Of frame Interrupt
#define AT91C_UDP_WAKEUP      ((unsigned int) 0x1 << 13) // (UDP) USB Resume Interrupt
// -------- UDP_IDR : (UDP Offset: 0x14) USB Interrupt Disable Register -------- 
// -------- UDP_IMR : (UDP Offset: 0x18) USB Interrupt Mask Register -------- 
// -------- UDP_ISR : (UDP Offset: 0x1c) USB Interrupt Status Register -------- 
#define AT91C_UDP_ENDBUSRES   ((unsigned int) 0x1 << 12) // (UDP) USB End Of Bus Reset Interrupt
// -------- UDP_ICR : (UDP Offset: 0x20) USB Interrupt Clear Register -------- 
// -------- UDP_RST_EP : (UDP Offset: 0x28) USB Reset Endpoint Register -------- 
#define AT91C_UDP_EP0         ((unsigned int) 0x1 <<  0) // (UDP) Reset Endpoint 0
#define AT91C_UDP_EP1         ((unsigned int) 0x1 <<  1) // (UDP) Reset Endpoint 1
#define AT91C_UDP_EP2         ((unsigned int) 0x1 <<  2) // (UDP) Reset Endpoint 2
#define AT91C_UDP_EP3         ((unsigned int) 0x1 <<  3) // (UDP) Reset Endpoint 3
#define AT91C_UDP_EP4         ((unsigned int) 0x1 <<  4) // (UDP) Reset Endpoint 4
#define AT91C_UDP_EP5         ((unsigned int) 0x1 <<  5) // (UDP) Reset Endpoint 5
// -------- UDP_CSR : (UDP Offset: 0x30) USB Endpoint Control and Status Register -------- 
#define AT91C_UDP_TXCOMP      ((unsigned int) 0x1 <<  0) // (UDP) Generates an IN packet with data previously written in the DPR
#define AT91C_UDP_RX_DATA_BK0 ((unsigned int) 0x1 <<  1) // (UDP) Receive Data Bank 0
#define AT91C_UDP_RXSETUP     ((unsigned int) 0x1 <<  2) // (UDP) Sends STALL to the Host (Control endpoints)
#define AT91C_UDP_ISOERROR    ((unsigned int) 0x1 <<  3) // (UDP) Isochronous error (Isochronous endpoints)
#define AT91C_UDP_STALLSENT   ((unsigned int) 0x1 <<  3) // (UDP) Stall sent (Control, bulk, interrupt endpoints)
#define AT91C_UDP_TXPKTRDY    ((unsigned int) 0x1 <<  4) // (UDP) Transmit Packet Ready
#define AT91C_UDP_FORCESTALL  ((unsigned int) 0x1 <<  5) // (UDP) Force Stall (used by Control, Bulk and Isochronous endpoints).
#define AT91C_UDP_RX_DATA_BK1 ((unsigned int) 0x1 <<  6) // (UDP) Receive Data Bank 1 (only used by endpoints with ping-pong attributes).
#define AT91C_UDP_DIR         ((unsigned int) 0x1 <<  7) // (UDP) Transfer Direction
#define AT91C_UDP_EPTYPE      ((unsigned int) 0x7 <<  8) // (UDP) Endpoint type
#define 	AT91C_UDP_EPTYPE_CTRL                 ((unsigned int) 0x0 <<  8) // (UDP) Control
#define 	AT91C_UDP_EPTYPE_ISO_OUT              ((unsigned int) 0x1 <<  8) // (UDP) Isochronous OUT
#define 	AT91C_UDP_EPTYPE_BULK_OUT             ((unsigned int) 0x2 <<  8) // (UDP) Bulk OUT
#define 	AT91C_UDP_EPTYPE_INT_OUT              ((unsigned int) 0x3 <<  8) // (UDP) Interrupt OUT
#define 	AT91C_UDP_EPTYPE_ISO_IN               ((unsigned int) 0x5 <<  8) // (UDP) Isochronous IN
#define 	AT91C_UDP_EPTYPE_BULK_IN              ((unsigned int) 0x6 <<  8) // (UDP) Bulk IN
#define 	AT91C_UDP_EPTYPE_INT_IN               ((unsigned int) 0x7 <<  8) // (UDP) Interrupt IN
#define AT91C_UDP_DTGLE       ((unsigned int) 0x1 << 11) // (UDP) Data Toggle
#define AT91C_UDP_EPEDS       ((unsigned int) 0x1 << 15) // (UDP) Endpoint Enable Disable
#define AT91C_UDP_RXBYTECNT   ((unsigned int) 0x7FF << 16) // (UDP) Number Of Bytes Available in the FIFO
// -------- UDP_TXVC : (UDP Offset: 0x74) Transceiver Control Register -------- 
#define AT91C_UDP_TXVDIS      ((unsigned int) 0x1 <<  8) // (UDP) 
#define AT91C_UDP_PUON        ((unsigned int) 0x1 <<  9) // (UDP) Pull-up ON

//------------------------------------------------------------------------------
//      Macros
//------------------------------------------------------------------------------

extern void clearEpFlags(unsigned volatile *reg, unsigned flags );

// \brief  Clear flags in the UDP_CSR register and waits for synchronization
// \param  epCSR     Pointer to CSR for this endpoint
// \param  flags     Flags to clear
#define UDP_CLEAREPFLAGS(epCSR, flags) clearEpFlags(epCSR,flags)

extern void setEpFlags(unsigned volatile *reg, unsigned flags );

// \brief  Set flags in the UDP_CSR register and waits for synchronization
// \param  pUsb      Pointer to a S_usb instance
// \param  bEndpoint Index of endpoint
// \param  dFlags    Flags to clear
#define UDP_SETEPFLAGS(epCSR, flags) setEpFlags(epCSR,flags)


/*--------------------------------------------------*/
/* Power Management Controller Structure Definition */
/*--------------------------------------------------*/

typedef struct {
    at91_reg    PMC_SCER ;  		/*  0: System Clock Enable  Register */
    at91_reg    PMC_SCDR ;  		/*  4: System Clock Disable Register */
    at91_reg    PMC_SCSR ;  		/*  8: System Clock Status  Register */
    at91_reg    Reserved0 ;		/*  C: */
    at91_reg    PMC_PCER ;  		/* 10: Peripheral Clock Enable  Register */
    at91_reg    PMC_PCDR ;  		/* 14: Peripheral Clock Disable Register */
    at91_reg    PMC_PCSR ;  		/* 18: Peripheral Clock Status  Register */
    at91_reg    Reserved1 ;
    at91_reg    CKGR_MOR ;
    at91_reg    CKGR_MCFR ;
    at91_reg    Reserved2 ;
    at91_reg    CKGR_PLLR ;
    at91_reg    CKGR_MCKR ;
    at91_reg    Reserved3 ;
    at91_reg    Reserved4 ;
    at91_reg    PMC_PCK0 ;
    at91_reg    PMC_PCK1 ;
    at91_reg	Unused[(0x60-0x48)/4];
    at91_reg    PMC_IER ;
    at91_reg    PMC_IDR ;
    at91_reg    PMC_SR ;
    at91_reg    PMC_IMR ;
} StructPMC ;

/*--------------------------------------------------*/
/*          Ethernet MAC Structure Definition       */
/*--------------------------------------------------*/

typedef struct {
    at91_reg   EMAC_NCR ;   /* 0x00 Network Control Register EMAC_NCR 0 */
    at91_reg   EMAC_NCFG ;  /* 0x04 Network Configuration Register EMAC_NCFG 0x800 */
    at91_reg   EMAC_NSR ;   /* 0x08 Network Status Register EMAC_NSR   */
    at91_reg   Reserved0,   /* 0x0C */
               Reserved1 ;  /* 0x10 */
    at91_reg   EMAC_TSR ;   /* 0x14 Transmit Status Register EMAC_TSR 0x0000_0000 */
    at91_reg   EMAC_RBQP ;  /* 0x18 Receive Buffer Queue Pointer Register EMAC_RBQP 0x0000_0000 */
    at91_reg   EMAC_TBQP ;  /* 0x1C Transmit Buffer Queue Pointer Register EMAC_TBQP 0x0000_0000 */
    at91_reg   EMAC_RSR ;   /* 0x20 Receive Status Register EMAC_RSR 0x0000_0000 */
    at91_reg   EMAC_ISR ;   /* 0x24 Interrupt Status Register EMAC_ISR 0x0000_0000 */
    at91_reg   EMAC_IER ;   /* 0x28 Interrupt Enable Register  */
    at91_reg   EMAC_IDR ;   /* 0x2C Interrupt Disable Register  */
    at91_reg   EMAC_IMR ;   /* 0x30 Interrupt Mask Register EMAC_IMR  0x0000_3FFF */
    at91_reg   EMAC_MAN ;   /* 0x34 Phy Maintenance Register EMAC_MAN 0x0000_0000 */
    at91_reg   EMAC_PTR ;   /* 0x38 Pause Time Register EMAC_PTR 0x0000_0000 */
    at91_reg   EMAC_PFR ;   /* 0x3C Pause Frames Received Register EMAC_PFR 0x0000_0000 */
    at91_reg   EMAC_FTO ;   /* 0x40 Frames Transmitted Ok Register EMAC_FTO 0x0000_0000 */
    at91_reg   EMAC_SCF ;   /* 0x44 Single Collision Frames Register EMAC_SCF 0x0000_0000 */
    at91_reg   EMAC_MCF ;   /* 0x48 Multiple Collision Frames Register EMAC_MCF 0x0000_0000 */
    at91_reg   EMAC_FRO ;   /* 0x4C Frames Received Ok Register EMAC_FRO 0x0000_0000 */
    at91_reg   EMAC_FCSE ;  /* 0x50 Frame Check Sequence Errors Register EMAC_FCSE 0x0000_0000 */
    at91_reg   EMAC_ALE ;   /* 0x54 Alignment Errors Register EMAC_ALE 0x0000_0000 */
    at91_reg   EMAC_DTF ;   /* 0x58 Deferred Transmission Frames Register EMAC_DTF 0x0000_0000 */
    at91_reg   EMAC_LCOL ;  /* 0x5C Late Collisions Register EMAC_LCOL 0x0000_0000 */
    at91_reg   EMAC_ECOL ;  /* 0x60 Excessive Collisions Register EMAC_ECOL 0x0000_0000 */
    at91_reg   EMAC_TUND ;  /* 0x64 Transmit Underrun Errors Register EMAC_TUND 0x0000_0000 */
    at91_reg   EMAC_CSE ;   /* 0x68 Carrier Sense Errors Register EMAC_CSE 0x0000_0000 */
    at91_reg   EMAC_RRE ;   /* 0x6C Receive Resource Errors Register EMAC_RRE 0x0000_0000 */
    at91_reg   EMAC_ROV ;   /* 0x70 Receive Overrun Errors Register EMAC_ROV 0x0000_0000 */
    at91_reg   EMAC_RSE ;   /* 0x74 Receive Symbol Errors Register EMAC_RSE 0x0000_0000 */
    at91_reg   EMAC_ELE ;   /* 0x78 Excessive Length Errors Register EMAC_ELE 0x0000_0000 */
    at91_reg   EMAC_RJA ;   /* 0x7C Receive Jabbers Register EMAC_RJA 0x0000_0000 */
    at91_reg   EMAC_USF ;   /* 0x80 Undersize Frames Register EMAC_USF 0x0000_0000 */
    at91_reg   EMAC_STE ;   /* 0x84 SQE Test Errors Register EMAC_STE 0x0000_0000 */
    at91_reg   EMAC_RLE ;   /* 0x88 Received Length Field Mismatch Register EMAC_RLE 0x0000_0000 */
    at91_reg   Reserved2;   /* 0x8C */
    at91_reg   EMAC_HRB ;   /* 0x90 Hash Register Bottom [31:0] Register EMAC_HRB 0x0000_0000 */
    at91_reg   EMAC_HRT ;   /* 0x94 Hash Register Top [63:32] Register EMAC_HRT 0x0000_0000 */
    at91_reg   EMAC_SA1B ;  /* 0x98 Specific Address 1 Bottom Register EMAC_SA1B 0x0000_0000 */
    at91_reg   EMAC_SA1T ;  /* 0x9C Specific Address 1 Top Register EMAC_SA1T 0x0000_0000 */
    at91_reg   EMAC_SA2B ;  /* 0xA0 Specific Address 2 Bottom Register EMAC_SA2B 0x0000_0000 */
    at91_reg   EMAC_SA2T ;  /* 0xA4 Specific Address 2 Top Register EMAC_SA2T 0x0000_0000 */
    at91_reg   EMAC_SA3B ;  /* 0xA8 Specific Address 3 Bottom Register EMAC_SA3B 0x0000_0000 */
    at91_reg   EMAC_SA3T ;  /* 0xAC Specific Address 3 Top Register EMAC_SA3T 0x0000_0000 */
    at91_reg   EMAC_SA4B ;  /* 0xB0 Specific Address 4 Bottom Register EMAC_SA4B 0x0000_0000 */
    at91_reg   EMAC_SA4T ;  /* 0xB4 Specific Address 4 Top Register EMAC_SA4T 0x0000_0000 */
    at91_reg   EMAC_TID ;   /* 0xB8 Type ID Checking Register EMAC_TID 0x0000_0000 */
    at91_reg   Reserved3 ;  /* 0xBC */
    at91_reg   EMAC_USRIO ; /* 0xC0 User Input/output Register EMAC_USRIO 0x0000_0000 */
} StructEMAC ;

#define EMAC_PIO_MASK ((1<<23)-1)    // GPIOB pins 0-22 are all ethernet

#ifndef AT91_EMAC
#define AT91_EMAC 0xFFFDC000
#define EMAC ((StructEMAC *)AT91_EMAC)
#endif

#define AT91_EMAC_NCR  (0x00) // Network Control
#define AT91_EMAC_NCR_LB     (1 <<  0) // Loopback
#define AT91_EMAC_NCR_LBL    (1 <<  1) // Loopback Local 
#define AT91_EMAC_NCR_RE     (1 <<  2) // Receiver Enable
#define AT91_EMAC_NCR_TX     (1 <<  3) // Transmit Enable
#define AT91_EMAC_NCR_MPE    (1 <<  4) // Management Port Enable
#define AT91_EMAC_NCR_CSR    (1 <<  5) // Clear Statistics Registers
#define AT91_EMAC_NCR_ISR    (1 <<  6) // Increment Statistics Registers
#define AT91_EMAC_NCR_WES    (1 <<  7) // Write Enable for Statistics Registers
#define AT91_EMAC_NCR_BP     (1 <<  8) // Back Pressure 
#define AT91_EMAC_NCR_TSTART (1 <<  9) // Start Transmitter
#define AT91_EMAC_NCR_THALT  (1 << 10) // Halt Transmitter

#define AT91_EMAC_NCFG  (0x04) // Network Configuration
#define AT91_EMAC_NCFG_SPD_10Mbps  (0 <<  0) // 10Mbps line speed
#define AT91_EMAC_NCFG_SPD_100Mbps (1 <<  0) // 100Mbps line speed
#define AT91_EMAC_NCFG_FD          (1 <<  1) // Full Deplex
#define AT91_EMAC_NCFG_BR          (1 <<  2) // Bit Rate
#define AT91_EMAC_NCFG_CAF         (1 <<  4) // Copy All Frames
#define AT91_EMAC_NCFG_NBC         (1 <<  5) // Don't receiver Broadcasts
#define AT91_EMAC_NCFG_MTI         (1 <<  6) // Multicast Hash Enable
#define AT91_EMAC_NCFG_UNI         (1 <<  7) // Unicast hash enable
#define AT91_EMAC_NCFG_BIG         (1 <<  8) // Receive upto 1522 byte frames
#define AT91_EMAC_NCFG_EAE         (1 <<  9) // External Address match Enable
#define AT91_EMAC_NCFG_CLK_HCLK_8  (0 << 10) // HCLK divided by 8
#define AT91_EMAC_NCFG_CLK_HCLK_16 (1 << 10) // HCLK divided by 16
#define AT91_EMAC_NCFG_CLK_HCLK_32 (2 << 10) // HCLK divided by 32
#define AT91_EMAC_NCFG_CLK_HCLK_64 (3 << 10) // HCLK divided by 64
#define AT91_EMAC_NCFG_CLK_MASK    (3 << 10) // HCLK mask
#define AT91_EMAC_NCFG_CLK_RTY     (1 << 12) // Retry Test
#define AT91_EMAC_NCFG_CLK_RMII    (1 << 13) // Enable RMII mode
#define AT91_EMAC_NCFG_CLK_MII     (0 << 13) // Enable MII mode
#define AT91_EMAC_NCFG_RLCE        (0 << 16) // Receive Length Check Enable

#define AT91_EMAC_NSR   (0x08) // Network Status
#define AT91_EMAC_NSR_MDIO_MASK (1 << 1) // MDIO Pin status
#define AT91_EMAC_NSR_IDLE      (1 << 2) // PHY logical is idle

#define AT91_EMAC_TSR  (0x14) // Transmit Status
#define AT91_EMAC_TSR_OVR    (1 << 0) // Overrun
#define AT91_EMAC_TSR_COL    (1 << 1) // Collision occurred
#define AT91_EMAC_TSR_RLE    (1 << 2) // Retry Limit Exceeded
#define AT91_EMAC_TSR_TXIDLE (1 << 3) // Transmitter Idle
#define AT91_EMAC_TSR_BNQ    (1 << 4) // Buffer Not Queues
#define AT91_EMAC_TSR_COMP   (1 << 5) // Transmission Complete
#define AT91_EMAC_TSR_UND    (1 << 6) // Transmit Underrun

#define AT91_EMAC_RBQP (0x18) // Receiver Buffer Queue Pointer
#define AT91_EMAC_TBQP (0x1c) // Transmit Buffer Queue Pointer

#define AT91_EMAC_RSR  (0x20) // Receiver Status
#define AT91_EMAC_RSR_BNA (1 << 0) // Buffer Not Available
#define AT91_EMAC_RSR_REC (1 << 1) // Frame Received
#define AT91_EMAC_RSR_OVR (1 << 2) // Transmit Buffer Overrun

#define AT91_EMAC_ISR  (0x24) // Interrupt Status
#define AT91_EMAC_ISR_DONE  (1 <<  0) // Management Done
#define AT91_EMAC_ISR_RCOM  (1 <<  1) // Receiver Complete
#define AT91_EMAC_ISR_RBNA  (1 <<  2) // Receiver Buffer Not Available
#define AT91_EMAC_ISR_TOVR  (1 <<  3) // Transmit Buffer Overrun
#define AT91_EMAC_ISR_TUND  (1 <<  4) // Transmit Error: Buffer under run
#define AT91_EMAC_ISR_RTRY  (1 <<  5) // Transmit Error: Retry Limit Exceeded
#define AT91_EMAC_ISR_TBRE  (1 <<  6) // Transmit Buffer Register Empty
#define AT91_EMAC_ISR_TCOM  (1 <<  7) // Transmit Complete
#define AT91_EMAC_ISR_TIDLE (1 <<  8) // Transmitter Idle
#define AT91_EMAC_ISR_LINK  (1 <<  9) // Link pin changed state
#define AT91_EMAC_ISR_ROVR  (1 << 10) // Receiver Overrun
#define AT91_EMAC_ISR_HRESP (1 << 11) // HRESP not OK
#define AT91_EMAC_IER  (0x28) // Interrupt Enable
#define AT91_EMAC_IDR  (0x2c) // Interrupt Disable
#define AT91_EMAC_IMR  (0x30) // Interrupt Mask

#define AT91_EMAC_MAN  (0x34) // PHY Maintenance
#define AT91_EMAC_MAN_DATA_MASK  (0xffff<<0)    // Data to/from PHY
#define AT91_EMAC_MAN_CODE       (2<<16)        // Code
#define AT91_EMAC_MAN_REGA_MASK  (0x1f<<18)     // Register Address Mask
#define AT91_EMAC_MAN_REGA_SHIFT (18)           // Register Address Shift
#define AT91_EMAC_MAN_PHY_MASK   (0x1f<<23)     // PHY Address Mask
#define AT91_EMAC_MAN_PHY_SHIFT  (23)           // PHY Address Shift
#define AT91_EMAC_MAN_RD         (2<<28)        // Read operation
#define AT91_EMAC_MAN_WR         (1<<28)        // Write Operation
#define AT91_EMAC_MAN_SOF        (1<<30)        // Must be set to 01
#define AT91_EMAC_MAN_PHYA(x)    ((x&0x1f)<<23) // Create a PHY Address
#define AT91_EMAC_MAN_REGA(x)    ((x&0x1f)<<18) // Create a Register Address
#define AT91_EMAC_MAN_DATA(x)    (x&0xffff)     // Create a Data word
                                          

#define AT91_EMAC_PTR  (0x38) // Pause Time Register
#define AT91_EMAC_PFR  (0x3C) // Pause Frames Received
#define AT91_EMAC_FTO  (0x40) // Frames Transmitted OK
#define AT91_EMAC_SCF  (0x44) // Single Collision Frame
#define AT91_EMAC_MCF  (0x48) // Multiple Collision Frame
#define AT91_EMAC_FRO  (0x4c) // Frames Received OK
#define AT91_EMAC_FCSE (0x50) // Frame Check Sequence Error
#define AT91_EMAC_ALE  (0x54) // Alignment Error
#define AT91_EMAC_DTR  (0x58) // Deferred Transmission Frame
#define AT91_EMAC_LCOL (0x5c) // Late Collision
#define AT91_EMAC_XCOL (0x60) // Excessive Collisions - ECOL!!
#define AT91_EMAC_TUND (0x64) // Transmit Underrun Error
#define AT91_EMAC_CSE  (0x68) // Carrier Sense Error
#define AT91_EMAC_RRE  (0x6c) // Receive Resource Errors
#define AT91_EMAC_ROV  (0x70) // Receive Overrun
#define AT91_EMAC_RSE  (0x74) // Receiver Symbol erros 
#define AT91_EMAC_ELE  (0x78) // Excessive Length Errors
#define AT91_EMAC_RJE  (0x7c) // Receive Jabber Errors
#define AT91_EMAC_USF  (0x80) // Undersize Frame Errors
#define AT91_EMAC_STE  (0x84) // SQE Test Errors
#define AT91_EMAC_RLE  (0x88) // Receive Length Field Mismatch
                              
#define AT91_EMAC_HRB  (0x90) // Hash Address Low  [31:0]
#define AT91_EMAC_HRT  (0x94) // Hash Address High [63:32]
#define AT91_EMAC_SA1L (0x98) // Specific Address 1 Low, First 4 bytes
#define AT91_EMAC_SA1H (0x9c) // Specific Address 1 High, Last 2 bytes
#define AT91_EMAC_SA2L (0xa0) // Specific Address 2 Low, First 4 bytes
#define AT91_EMAC_SA2H (0xa4) // Specific Address 2 High, Last 2 bytes
#define AT91_EMAC_SA3L (0xa8) // Specific Address 3 Low, First 4 bytes
#define AT91_EMAC_SA3H (0xac) // Specific Address 3 High, Last 2 bytes
#define AT91_EMAC_SA4L (0xb0) // Specific Address 4 Low, First 4 bytes
#define AT91_EMAC_SA4H (0xb4) // Specific Address 4 High, Last 2 bytes
#define AT91_EMAC_TID  (0xb8) // Type ID Checking Register

#define AT91_EMAC_USRIO  (0xc0) // User IO Register
#define AT91_EMAC_USRIO_RMII   (1<<0) // RMII Mode
#define AT91_EMAC_USRIO_CLKEN  (1<<1) // Clock Enable

// Receiver Buffer Descriptor
#define AT91_EMAC_RBD_ADDR 0x0  // Address to beginning of buffer
#define AT91_EMAC_RBD_ADDR_MASK   (0xFFFFFFFC) // Address Mask masking the reserved bits
#define AT91_EMAC_RBD_ADDR_OWNER_EMAC (0 << 0) // EMAC owns receiver buffer
#define AT91_EMAC_RBD_ADDR_OWNER_SW   (1 << 0) // SW owns receiver buffer
#define AT91_EMAC_RBD_ADDR_WRAP       (1 << 1) // Last receiver buffer
#define AT91_EMAC_RBD_SR   0x1  // Buffer Status
#define AT91_EMAC_RBD_SR_LEN_MASK     (0xfff)   // Length of data
#define AT91_EMAC_RBD_SR_SOF          (1 << 14) // Start of Frame
#define AT91_EMAC_RBD_SR_EOF          (1 << 15) // End of Frame
#define AT91_EMAC_RBD_SR_CFI          (1 << 16) // Concatination Format Ind
#define AT91_EMAC_RDB_SR_VLAN_SHIFT   (17)      // VLAN priority tag 
#define AT91_EMAC_RDB_SR_VLAN_MASK    (7 << 17)
#define AT91_EMAC_RDB_SR_PRIORTY_TAG  (1 << 20) // Priority Tag Detected
#define AT91_EMAC_RDB_SR_VLAN_TAG     (1 << 21) // Priority Tag Detected
#define AT91_EMAC_RBD_SR_TYPE_ID      (1 << 22) // Type ID match
#define AT91_EMAC_RBD_SR_SA4M         (1 << 23) // Specific Address 4 match
#define AT91_EMAC_RBD_SR_SA3M         (1 << 24) // Specific Address 3 match
#define AT91_EMAC_RBD_SR_SA2M         (1 << 25) // Specific Address 2 match
#define AT91_EMAC_RBD_SR_SA1M         (1 << 26) // Specific Address 1 match
#define AT91_EMAC_RBD_SR_EXTNM        (1 << 28) // External Address match
#define AT91_EMAC_RBD_SR_UNICAST      (1 << 29) // Unicast hash match
#define AT91_EMAC_RBD_SR_MULTICAST    (1 << 30) // Multicast hash match
#define AT91_EMAC_RBD_SR_BROADCAST    (1 << 31) // Broadcast

// Transmit Buffer Descriptor
#define AT91_EMAC_TBD_ADDR 0x0  // Address to beginning of buffer
#define AT91_EMAC_TBD_SR   0x1  // Buffer Status
#define AT91_EMAC_TBD_SR_LEN_MASK     (0xfff)   // Length of data
#define AT91_EMAC_TBD_SR_EOF          (1 << 15) // End of Frame
#define AT91_EMAC_TBD_SR_NCRC         (1 << 16) // No CRC added by EMAC
#define AT91_EMAC_TBD_SR_EXHAUST      (1 << 27) // Buffers exhausted
#define AT91_EMAC_TBD_SR_TXUNDER      (1 << 28) // Transmit Underrun
#define AT91_EMAC_TBD_SR_RTRY         (1 << 29) // Retry limit exceeded
#define AT91_EMAC_TBD_SR_WRAP         (1 << 30) // Marks last descriptor
#define AT91_EMAC_TBD_SR_USED         (1 << 31) // Buffer used

// Transceiver mode
#define PHY_BMCR             0x00    // Register number
#define PHY_BMCR_RESET       0x8000
#define PHY_BMCR_LOOPBACK    0x4000
#define PHY_BMCR_100MB       0x2000
#define PHY_BMCR_AUTO_NEG    0x1000
#define PHY_BMCR_POWER_DOWN  0x0800
#define PHY_BMCR_ISOLATE     0x0400
#define PHY_BMCR_RESTART     0x0200
#define PHY_BMCR_FULL_DUPLEX 0x0100
#define PHY_BMCR_COLL_TEST   0x0080

#define PHY_BMSR             0x01    // Status register
#define PHY_BMSR_100T4       0x8000
#define PHY_BMSR_100FDX      0x4000
#define PHY_BMSR_100HDX      0x2000
#define PHY_BMSR_10FDX       0x1000
#define PHY_BMSR_10HDX       0x0800
#define PHY_BMSR_AUTO_NEG    0x0020  
#define PHY_BMSR_LINK        0x0004

#define PHY_ID1              0x02    // Chip ID register (high 16 bits)
#define PHY_ID2              0x03    // Chip ID register (low 16 bits)

#define PHY_AN_ADV           0x04    // Auto negotiation advertisement register
#define PHY_AN_ADV_10HDX     0x0020
#define PHY_AN_ADV_10FDX     0x0040
#define PHY_AN_ADV_100HDX    0x0080
#define PHY_AN_ADV_100FDX    0x0100
#define PHY_AN_ADV_100_T4    0x0200

#define PHY_AN_PAR           0x05    // Auto negotiation link partner ability
#define PHY_AN_PAR_10HDX     0x0020
#define PHY_AN_PAR_10FDX     0x0040
#define PHY_AN_PAR_100HDX    0x0080
#define PHY_AN_PAR_100FDX    0x0100
#define PHY_AN_PAR_100_T4    0x0200

typedef struct {
   at91_reg    pdmaRPR ;      // 0x100 Receive Pointer Register PERIPH_RPR Read/Write 0x0
   at91_reg    pdmaRCR ;      // 0x104 Receive Counter Register PERIPH_RCR Read/Write 0x0
   at91_reg    pdmaTPR ;      // 0x108 Transmit Pointer Register PERIPH_TPR Read/Write 0x0
   at91_reg    pdmaTCR ;      // 0x10C Transmit Counter Register PERIPH_TCR Read/Write 0x0
   at91_reg    pdmaRNPR ;     // 0x110 Receive Next Pointer Register PERIPH_RNPR Read/Write 0x0
   at91_reg    pdmaRNCR ;     // 0x114 Receive Next Counter Register PERIPH_RNCR Read/Write 0x0
   at91_reg    pdmaTNPR ;     // 0x118 Transmit Next Pointer Register PERIPH_TNPR Read/Write 0x0
   at91_reg    pdmaTNCR ;     // 0x11C Transmit Next Counter Register PERIPH_TNCR Read/Write 0x0
   at91_reg    pdmaPTCR ;     // 0x120 PDC Transfer Control Register PERIPH_PTCR Write-only -
   at91_reg    pdmaPTSR ;     // 0x124 PDC Transfer Status Register PERIPH_PTSR Read-only 0x0c
} StructPeriphDMA ;

typedef struct {
   at91_reg    MC_FMR ;       // 0x60 MC Flash Mode Register
   at91_reg    MC_FCR ;       // 0x64 MC Flash Command Register
   at91_reg    MC_FSR ;       // 0x68 MC Flash Status Register
} StructEFC ;

typedef struct
{
    at91_reg       RTT_MR ;     /* Mode Register */
    at91_reg       RTT_ALARM ;  /* Alarm Register */
    at91_reg       RTT_VALUE ;  /* Value Register */
    at91_reg       RTT_STAT ;   /* Status Register */
} StructRTT ;

#define RTT_MR_RTTRST (1<<18)

#define RTT ((StructRTT *)0xFFFFFD20)

#endif

#define PMC	((StructPMC *)0xFFFFFC00)

#define PMC_PID_AIC             0       // Advanced Interrupt Controller FIQ
#define PMC_PID_SYSIRQ          1       // SYSIRQ(1)
#define PMC_PID_PIOA            2       // PIOA Parallel I/O Controller A
#define PMC_PID_PIOB            3       // PIOB Parallel I/O Controller B
#define PMC_PID_SPI0            4       // SPI0 Serial Peripheral Interface 0
#define PMC_PID_SPI1            5       // SPI1 Serial Peripheral Interface 1
#define PMC_PID_US0             6       // US0 USART 0
#define PMC_PID_US1             7       // US1 USART 1
#define PMC_PID_SSC             8       // SSC Synchronous Serial Controller
#define PMC_PID_TWI             9       // TWI Two-wire Interface
#define PMC_PID_PWMC            10      // PWMC Pulse Width Modulation Controller
#define PMC_PID_UDP             11      // UDP USB device Port
#define PMC_PID_TC0             12      // TC0 Timer/Counter 0
#define PMC_PID_TC1             13      // TC1 Timer/Counter 1
#define PMC_PID_TC2             14      // TC2 Timer/Counter 2
#define PMC_PID_CAN             15      // CAN CAN Controller
#define PMC_PID_EMAC            16      // EMAC Ethernet MAC
#define PMC_PID_ADC             17      // ADC(1) Analog-to Digital Converter
#define PMC_PID_29              18 -    // 29 Reserved
#define PMC_PID_AIC_IRQ0        30      // AIC Advanced Interrupt Controller IRQ0
#define PMC_PID_AIC_IRQ1        31      // AIC Advanced Interrupt Controller IRQ1

#define PMC_PERIPHS	( (1<<PMC_PID_PIOA) \
			 |(1<<PMC_PID_PIOB) \
			 |(1<<PMC_PID_US0) \
			 |(1<<PMC_PID_US1) \
			 |(1<<PMC_PID_SPI0) \
			 |(1<<PMC_PID_EMAC) \
			)

/*
 * PSR bits
 */
#define USR26_MODE	0x00000000
#define FIQ26_MODE	0x00000001
#define IRQ26_MODE	0x00000002
#define SVC26_MODE	0x00000003
#define USR_MODE	0x00000010
#define FIQ_MODE	0x00000011
#define IRQ_MODE	0x00000012
#define SVC_MODE	0x00000013
#define ABT_MODE	0x00000017
#define UND_MODE	0x0000001b
#define SYSTEM_MODE	0x0000001f
#define MODE32_BIT	0x00000010
#define MODE_MASK	0x0000001f
#define PSR_T_BIT	0x00000020
#define PSR_F_BIT	0x00000040
#define PSR_I_BIT	0x00000080
#define PSR_J_BIT	0x01000000
#define PSR_Q_BIT	0x08000000
#define PSR_V_BIT	0x10000000
#define PSR_C_BIT	0x20000000
#define PSR_Z_BIT	0x40000000
#define PSR_N_BIT	0x80000000
#define PCMASK		0


#define SPI0 ((StructSPI *)0xFFFE0000)
#define SPI0DMA ((StructPeriphDMA *)0xFFFE0100)
#define SPI1 ((StructSPI *)0xFFFE4000)
#define SPI1DMA ((StructPeriphDMA *)0xFFFE4100)

//
// FRAM is connected to NPCS0 (PA12)
// This line needs Multi-Drive enabled on the GPIO pin
//
#define SPI0_NPCS0_MASK (1<<12)

//
// FRAM uses SPI0
//    PA12  - NPCS0
//    PA16  - MISO
//    PA17  - MOSI
//    PA18  - SPCK
//
#define SPI0_PIO_MASK (0x71000)

/*
 * Peripheral DMA Controller registers
 *
 * Copies of this register set exist for many peripherals:
 *    USART0/USART1
 *    SPI
 */
#define PERIPH_RPR (0x100-0x100)       // Receive Pointer Register PERIPH_RPR Read/Write 0x0
#define PERIPH_RCR (0x104-0x100)       // Receive Counter Register PERIPH_RCR Read/Write 0x0
#define PERIPH_TPR (0x108-0x100)       // Transmit Pointer Register PERIPH_TPR Read/Write 0x0
#define PERIPH_TCR (0x10C-0x100)       // Transmit Counter Register PERIPH_TCR Read/Write 0x0
#define PERIPH_RNPR (0x110-0x100)      // Receive Next Pointer Register PERIPH_RNPR Read/Write 0x0
#define PERIPH_RNCR (0x114-0x100)      // Receive Next Counter Register PERIPH_RNCR Read/Write 0x0
#define PERIPH_TNPR (0x118-0x100)      // Transmit Next Pointer Register PERIPH_TNPR Read/Write 0x0
#define PERIPH_TNCR (0x11C-0x100)      // Transmit Next Counter Register PERIPH_TNCR Read/Write 0x0
#define PERIPH_PTCR (0x120-0x100)      // PDC Transfer Control Register PERIPH_PTCR Write-only -
#define PERIPH_PTSR (0x124-0x100)      // PDC Transfer Status Register PERIPH_PTSR Read-only 0x0c

/*
 * Embedded Flash Controller
 */
#define EFC0 ((StructEFC *)0xFFFFFF60)
#define EFC1 ((StructEFC *)0xFFFFFF70)

#define FLASH_BASE 0x00100000
#define FLASH_PAGE_SIZE 256
#define NUM_FLASH_PAGES 1024
#define FLASH_SIZE (FLASH_PAGE_SIZE*NUM_FLASH_PAGES)
#define FLASH_LOCK_BITS 8
#define FLASH_LOCK_SIZE (FLASH_SIZE/FLASH_LOCK_BITS)

#define MC_FSR_FRDY   1
#define MC_FSR_LOCKE  4
#define MC_FSR_PROGE  8

#define SRAM_START 0x00200000

/*
 * AT91SAM7X128
 */
#define SRAM_MAX	0x8000

/*
 * AT91SAM7X256
 */
//#define SRAM_MAX	0x10000

#endif

