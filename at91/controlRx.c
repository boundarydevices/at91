/*
 * Module controlRx.c
 *
 * This module defines the handlers for the control
 * endpoint (endpoint zero).
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */


#include "controlRx.h"
#include "usb_ll.h"
#include "usbEndpoints.h"
#include "usart.h"
#include "usb.h"
#include "usbDesc.h"
#include "assert.h"

// #define DEBUG
#include "debug.h"

static struct sg_t txSG_[2] = {
	{ 0 }
,	{ 0 }
};

static unsigned char wAddress = 0 ;

static void usbtx_callback
	( void          *opaque,
          unsigned       epNum,
          struct sg_t   *data )
{
	assert( opaque == (void *)txSG_ );
	txSG_->length = txSG_->offset = 0 ;
	txSG_->data = 0 ;
}

static void setaddr_tx_callback
	( void          *opaque,
          unsigned       epNum,
          struct sg_t   *data )
{
	if( (UDP->UDP_FADDR & AT91C_UDP_FADD) != wAddress ){
		UDP->UDP_FADDR = AT91C_UDP_FEN | wAddress ; // set address
		UDP->UDP_GLBSTATE |= AT91C_UDP_FADDEN;
	}

	usbtx_callback(opaque,epNum,data);
}

static void usb_tx( void const *data, unsigned len, usbll_tx_callback_t cb )
{
	if( !SG_DONE(txSG_) ){
		DEBUGMSG( "--> secondary transmit ignored\r\n" );
		unsigned i ;
		char const *bytes = (char *)data ;
		for( i = 0 ; i < len ; i++ ){
			DEBUGHEXCHAR( *bytes++ ); DEBUGCHAR( ' ' );
		}
		DEBUGMSG( "\r\nleftover:\r\n" );
		bytes = (char *)txSG_->data ;
                for( i = txSG_->offset ; i < txSG_->length ; i++ ){
                        DEBUGHEXCHAR( *bytes++ ); DEBUGCHAR( ' ' );
		}
		DEBUGMSG( "\r\n" );
		return ;
	}
	txSG_->length = len ;
	txSG_->data = (void *)data ;
	txSG_->offset = 0 ;
	if( 0 == cb )
		cb = usbtx_callback ;
	usbll_transmit( USBEP_CONTROL, txSG_, cb, txSG_ );
}

// returns true if handled
static int handleResetRq(struct S_usb_request const *rxSetup)
{
    int handled = 0 ;

    // Handle incoming request
    switch (rxSetup->bRequest) {
	//----------------------
        case USB_GET_DESCRIPTOR:
	//----------------------
		// The HBYTE macro returns the upper byte of a word
		switch( rxSetup->wValue >> 8 ){
			//-------------------------
			case USB_DEVICE_DESCRIPTOR:
			//-------------------------
				usb_tx( &sDeviceDescriptor, sizeof(sDeviceDescriptor), 0 ); // sizeof(sDeviceDescriptor) );
				handled = 1 ;
				DEBUGMSG( "gDescDev\r\n" );
				break;

                        //--------------------------------
                        case USB_CONFIGURATION_DESCRIPTOR:
                        //--------------------------------
				{
					unsigned len = sizeof(sConfigurationDescriptor);
					if( rxSetup->wLength < len )
						len = rxSetup->wLength ;
					usb_tx( &sConfigurationDescriptor, len, 0 );
					DEBUGMSG( "gDescCfg 0x" ); DEBUGHEX( len ); DEBUGMSG( "\r\n" );
					handled = 1 ;
					break;
				}

			//-------------------------
                        case USB_STRING_DESCRIPTOR:
                        //-------------------------
				{
					unsigned char stringId = rxSetup->wValue & 0xFF ;
					if( stringId < NUMUSB_STRINGS ){
						unsigned len = *usbStringDescriptors[stringId];
						if( rxSetup->wLength < len )
							len = rxSetup->wLength ;
						usb_tx(usbStringDescriptors[stringId],len,0);
						handled = 1 ;
                                                DEBUGMSG( "str[" ); DEBUGHEXCHAR( stringId ); DEBUGMSG( "]\r\n" );
					}
					else {
                                                DEBUGMSG( "invalidStr[" ); DEBUGHEX( rxSetup->wValue ); DEBUGMSG( "]\r\n" );
					}

				}

				break;
			default:
				DEBUGMSG( "unknown descriptor\r\n" );
		}
		break;

        //-------------------
        case USB_SET_ADDRESS:
        //-------------------
		{
                        wAddress = rxSetup->wValue;
			usb_tx( "", 0 ,setaddr_tx_callback);
			
/*
			if(0 == wAddress){
				UDP->UDP_GLBSTATE = 0 ;
			} else {
				UDP->UDP_GLBSTATE = AT91C_UDP_FADDEN;
			}
			UDP->UDP_FADDR = AT91C_UDP_FEN | wAddress ;
*/

			DEBUGMSG( "sAddr:" ); DEBUGHEXCHAR( wAddress ); DEBUGMSG( "\r\n" );
			handled = 1 ;
			break;
		}

        //-------------------------
	case USB_SET_CONFIGURATION:
	//-------------------------
		{
			usb_tx( "", 0 ,0); // ok
			DEBUGMSG( "sCfg\r\n");
			UDP->UDP_RSTEP = ( 1 << USBEP_BULKOUT )
				       | ( 1 << USBEP_BULKOUT )
				       | ( 1 << USBEP_INTERRUPT );
			UDP->UDP_RSTEP &= ~( 1 << USBEP_BULKOUT );
			UDP->UDP_CSR[USBEP_BULKOUT] = AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_BULK_OUT ;
			UDP->UDP_RSTEP &= ~( 1 << USBEP_BULKIN );
			UDP->UDP_CSR[USBEP_BULKIN] = AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_BULK_IN | AT91C_UDP_DIR ;
			handled = 1 ;
			break;
		}

        //-------------------------
        case USB_GET_CONFIGURATION:
	//-------------------------
		DEBUGMSG( "gCfg\r\n");
		break;
        //---------------------
        case USB_CLEAR_FEATURE:
        //---------------------
		DEBUGMSG( "cFeat\r\n");
                switch (rxSetup->wValue) {
			//---------------------
                        case USB_ENDPOINT_HALT:
                        //---------------------
                                DEBUGMSG( "Hlt\r\n" );
				usbll_resume( rxSetup->wIndex & 7 );
				usb_tx( "", 0 ,0);
				break;
                        //----------------------------
                        case USB_DEVICE_REMOTE_WAKEUP:
                        //----------------------------
                                DEBUGMSG( "RmWak\r\n");
				usb_tx( "", 0 ,0);
				break;
                        //------
                        default:
                        //------
                                DEBUGHEX( rxSetup->wValue ); DEBUGMSG( "::Stalled\r\n");
                                usb_tx( "", 0 ,0);
//                                UDP_SETEPFLAGS(UDP->UDP_CSR + 0, AT91C_UDP_FORCESTALL);
		}
		break;

        //------------------
        case USB_GET_STATUS:
        //------------------
                DEBUGMSG( "gSta\r\n");

		switch (USB_REQUEST_RECIPIENT(rxSetup)) {
			//-------------------------
			case USB_RECIPIENT_DEVICE:
			//-------------------------
                                DEBUGMSG( "Dev\r\n");
                                break;
                        //---------------------------
                        case USB_RECIPIENT_ENDPOINT:
                        //---------------------------
                                DEBUGMSG( "Ept\r\n");
                                break;

                        //------
                        default:
                        //------
                                DEBUGMSG( "W: Unsupported GetStatus\r\n" );
                                UDP_SETEPFLAGS(UDP->UDP_CSR + 0, AT91C_UDP_FORCESTALL);
                }
                break;

	//-------------------
	case USB_SET_FEATURE:
	//-------------------
                DEBUGMSG( "sFeat ");

		switch (rxSetup->wValue) {
                        //---------------------
                        case USB_ENDPOINT_HALT:
                        //---------------------
                                DEBUGMSG( "EPHalt\r\n" );
                                usbll_halt( rxSetup->wIndex & 7 );
				usb_tx( "", 0 ,0);
                                break;

                        //----------------------------
                        case USB_DEVICE_REMOTE_WAKEUP:
                        //----------------------------
                                DEBUGMSG( "DRW\r\n" );
				usb_tx( "", 0 ,0);
                                break;
                        //------
                        default:
                        //------
                                DEBUGMSG( "W: Unsupported SetFeature\r\n" );
                                UDP_SETEPFLAGS(UDP->UDP_CSR + 0, AT91C_UDP_FORCESTALL);
		}
                break;

    
	//------
        default:
        //------
                DEBUGMSG( "Unsupported request\r\n" );
                UDP_SETEPFLAGS(UDP->UDP_CSR + 0, AT91C_UDP_FORCESTALL);
    }


    if( !handled ){
	write( DEFUART, "rt 0x" ); writeHexChar( DEFUART, rxSetup->bmRequestType ); write( DEFUART, "\r\n" );
	write( DEFUART, "rq 0x" ); writeHexChar( DEFUART, rxSetup->bRequest ); write( DEFUART, "\r\n" );
	write( DEFUART, "wv 0x" ); writeHex( DEFUART, rxSetup->wValue ); write( DEFUART, "\r\n" );
	write( DEFUART, "wi 0x" ); writeHex( DEFUART, rxSetup->wIndex ); write( DEFUART, "\r\n" );
	write( DEFUART, "wl 0x" ); writeHex( DEFUART, rxSetup->wLength ); write( DEFUART, "\r\n" );
    }

    return handled ;
}

void setupHandler
	( void *opaque,
	  unsigned char const *data,
	  unsigned length )
{
        struct S_usb_request const *rxSetup = (struct S_usb_request const *)data ;
	assert( sizeof(*rxSetup) == length );
	handleResetRq(rxSetup);
}

static unsigned char buffer[32];

void controlRxInit(void)
{
	DEBUGMSG( __FUNCTION__ ); DEBUGMSG("\r\n");
	usbll_onRxData( USBEP_CONTROL, controlRxCallback, buffer, 0 );
	usbll_onSetup( setupHandler, 0 );
}


void controlRxCallback
	( void *opaque,
          unsigned epNum,
          unsigned char *data,
          unsigned length )
{
	unsigned i ;
	DEBUGMSG( __FUNCTION__ ); DEBUGMSG(":"); DEBUGHEX(length); DEBUGMSG(" bytes\r\n");
	for( i = 0 ; i < length ; i++ ){
		DEBUGHEXCHAR(*data); DEBUGCHAR(' ');
		data++ ;
	}
	DEBUGMSG("\r\n" );
}

