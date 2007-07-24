/*
 * Module usbDesc.c
 *
 * This module defines the device and endpoint descriptors
 * for the SEFRAM board.
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#include "usbDesc.h"
#include "llInit.h"

struct S_usb_device_descriptor_s const sDeviceDescriptor = {
    sizeof(S_usb_device_descriptor), // Size of this descriptor in bytes
    USB_DEVICE_DESCRIPTOR,           // DEVICE Descriptor Type
    USB1_10,                         // USB Specification 1.1
    USB_CLASS_COMMUNICATION,         // Class is specified in the interface descriptor.
    0x00,                            // Subclass is specified in the interface descriptor.
    0x00,                            // Protocol is specified in the interface descriptor.
    USB_ENDPOINT0_MAXPACKETSIZE,     // Maximum packet size for endpoint zero
    USB_VENDOR_ATMEL,                // Vendor ID "ATMEL"
    0x6124,                          // Product ID
    0x0110,                          // Device release number
    0x01,                            // Index 1: manufacturer string
    0x02,                            // Index 2: product string
    0x03,                            // Index 3: serial number string
    0x01                             // One possible configurations
};

struct S_core_configuration_descriptor const sConfigurationDescriptor = {
    // Configuration descriptor
    {
        sizeof(struct S_usb_configuration_descriptor),  // Size of this descriptor
        USB_CONFIGURATION_DESCRIPTOR,            // CONFIGURATION descriptor
        sizeof(struct S_core_configuration_descriptor), // Total length
        0x01,                                    // Number of interfaces
        0x00,                                    // Value to select this configuration
        0x00,                                    // No index for describing this configuration
        USB_CONFIG_SELF_NOWAKEUP,                // Device attributes
        USB_POWER_MA(100)                        // maximum power consumption in mA
    },
    // Interface Descriptor
    {
        sizeof(struct S_usb_interface_descriptor), // Size of this descriptor in bytes
        USB_INTERFACE_DESCRIPTOR,           // INTERFACE Descriptor Type
        0x00,                               // Interface number 0
        0x00,                               // Value used to select this setting
        0x02,                               // Number of endpoints used by this
                                            // interface (excluding endpoint 0).
        USB_CLASS_COMMUNICATION,            // Interface class
        0x02,                               // Interface subclass
        0x00,                               // Interface protocol
        0x04                                // Index of string descriptor (SEFRAM)
    },
    // Bulk-OUT endpoint descriptor
    {
        sizeof(struct S_usb_endpoint_descriptor), // Size of this descriptor in bytes
        USB_ENDPOINT_DESCRIPTOR,           // ENDPOINT descriptor type
        USB_ENDPOINT_OUT | 0x01,   	   // OUT endpoint, address = 0x01
        ENDPOINT_TYPE_BULK,                // Bulk endpoint
        64,                                // Endpoint size is 64 bytes
        0x00                               // Must be 0x00 for full-speed bulk
    },                                     // endpoints
    // Bulk-IN endpoint descriptor
    {
        sizeof(struct S_usb_endpoint_descriptor), // Size of this descriptor in bytes
        USB_ENDPOINT_DESCRIPTOR,           // ENDPOINT descriptor type
        USB_ENDPOINT_IN | 0x02,	           // IN endpoint, address = 0x02
        ENDPOINT_TYPE_BULK,                // Bulk endpoint
        64,                                // Endpoint size is 64 bytes
        0x00                               // Must be 0x00 for full-speed bulk
    }                                      // endpoints
};

// String descriptors
//! \brief  Language ID
static const struct S_usb_language_id sLanguageID = {
    USB_STRING_DESCRIPTOR_SIZE(1),
    USB_STRING_DESCRIPTOR,
    USB_LANGUAGE_ENGLISH_US
};

//! \brief  Manufacturer description
static const char pManufacturer[] = {
    USB_STRING_DESCRIPTOR_SIZE(5),
    USB_STRING_DESCRIPTOR,
    USB_UNICODE('A'),
    USB_UNICODE('t'),
    USB_UNICODE('m'),
    USB_UNICODE('e'),
    USB_UNICODE('l')
};

//! \brief  Product descriptor
static const char pProduct[] = {

    USB_STRING_DESCRIPTOR_SIZE(15),
    USB_STRING_DESCRIPTOR,
    USB_UNICODE('A'),
    USB_UNICODE('t'),
    USB_UNICODE('m'),
    USB_UNICODE('e'),
    USB_UNICODE('l'),
    USB_UNICODE(' '),
    USB_UNICODE('A'),
    USB_UNICODE('T'),
    USB_UNICODE('9'),
    USB_UNICODE('1'),
    USB_UNICODE(' '),
    USB_UNICODE('E'),
    USB_UNICODE('n'),
    USB_UNICODE('u'),
    USB_UNICODE('m')
};

//! \brief  Serial number
static char pSerial[] = {
    USB_STRING_DESCRIPTOR_SIZE(17),
    USB_STRING_DESCRIPTOR,
    USB_UNICODE('0'),
    USB_UNICODE('1'),
    USB_UNICODE('2'),
    USB_UNICODE('3'),
    USB_UNICODE('4'),
    USB_UNICODE('5'),
    USB_UNICODE('6'),
    USB_UNICODE('7'),
    USB_UNICODE('/'),
    USB_UNICODE('0'),
    USB_UNICODE('1'),
    USB_UNICODE('2'),
    USB_UNICODE('3'),
    USB_UNICODE('4'),
    USB_UNICODE('5'),
    USB_UNICODE('6'),
    USB_UNICODE('7'),
};

static const char pInterface[] = {
    USB_STRING_DESCRIPTOR_SIZE(6),
    USB_STRING_DESCRIPTOR,
    USB_UNICODE('S'),
    USB_UNICODE('E'),
    USB_UNICODE('F'),
    USB_UNICODE('R'),
    USB_UNICODE('A'),
    USB_UNICODE('M')
};

//! \brief  List of string descriptors used by the device
char const *usbStringDescriptors[NUMUSB_STRINGS] = {
    (char *) &sLanguageID,
    pManufacturer,
    pProduct,
    pSerial,
    pInterface
};

//! \brief  List of descriptors used by the device
//! \see    S_std_descriptors
static struct S_std_descriptors const sDescriptors = {
    &sDeviceDescriptor,
    (struct S_usb_configuration_descriptor *) &sConfigurationDescriptor,
    usbStringDescriptors,
    0
};

void setUsbProduct( char const *appName )
{
	char temp[9];
	unsigned i ;
	char *nextOut ;

	usbStringDescriptors[2] = appName ;

        toHex( temp, imageLength );

        nextOut = pSerial+2 ;
	for( i = 0 ; i < 8 ; i++ ){
		*nextOut = temp[i];
		nextOut += 2 ; // unicode
	}

	nextOut += 2 ; // skip slash

	toHex( temp, imageCRC );
	for( i = 0 ; i < 8 ; i++ ){
		*nextOut = temp[i];
		nextOut += 2 ; // unicode
	}
}

