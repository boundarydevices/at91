/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support  -  ROUSSET  -
 * ----------------------------------------------------------------------------
 * Copyright (c) 2006, Atmel Corporation

 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaiimer below.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the disclaimer below in the documentation and/or
 * other materials provided with the distribution.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/*
$Id: usb.h 121 2006-10-17 12:54:54Z jjoannic $
*/

#ifndef _USB_H
#define _USB_H

//------------------------------------------------------------------------------
//! \defgroup usb_std_struc USB standard structures
//! \brief Chapter 9 of the USB specification 2.0 (usb_20.pdf) describes a
//!        standard USB device framework. Several structures and associated
//!        constants have been defined on that model and are described here.
//! \see usb_20.pdf - Section 9
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! \defgroup usb_api_struct USB API Structures
//! \brief The USB API uses various custom structures to track the state of
//!        the USB controller, endpoints, and the like. These structures are
//!        described here.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! \defgroup usb_api_methods USB API Methods
//! \brief  Methods provided by the USB API to manipulate a USB driver.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! \defgroup usb_api_callbacks Callback API
//! \brief These callback functions are used by the USB API to notify the
//!        user application of incoming events or actions to perform.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! \defgroup usb_std_req_hlr Standard Request Handler
//! \brief This module provides a way to easily handle standard standard
//!        requests.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//      Definitions
//------------------------------------------------------------------------------

// USB standard definitions
//---------------------------------------------------------
//! \ingroup usb_std_req_hlr
//! \defgroup std_dev_req Standard Device Requests
//! These are the standard request defined for a SETUP transaction. Please refer
//! to Section 9.4 of the USB 2.0 specification (usb_20.pdf) for more
//! information. Table 9.4 defines the bRequest values for each request.
//! \see S_usb_request
//! \see usb_20.pdf - Section 9.4
//! @{

//! \brief  Returns the status for the specified recipient.
//! \see    get_status_const
//! \see    usb_20.pdf - Section 9.4.5
#define USB_GET_STATUS                 0x00

//! \brief  Disables a specific feature of the device
//! \see    usb_20.pdf - Section 9.4.1
//! \see    clr_set_feat_const
#define USB_CLEAR_FEATURE              0x01

// Reserved for futur use              0x02

//! \brief  Enables a specific feature of the device
//! \see    clr_set_feat_const
//! \see    set_feat_const
//! \see    usb_20.pdf - 9.4.9
#define USB_SET_FEATURE                0x03

// Reserved for futur use              0x04

//! \brief  Sets the device address for subsequent accesses
//! \see    usb_20.pdf - Section 9.4.6
#define USB_SET_ADDRESS                0x05

//! \brief  Returns the specified descriptor if it exists
//! \see    usb_20.pdf - Section 9.4.3
#define USB_GET_DESCRIPTOR             0x06

//! \brief  Updates existing descriptors or creates new descriptors
//! \brief  This request is optional
//! \see    usb_20.pdf - Section 9.4.8
#define USB_SET_DESCRIPTOR             0x07

//! \brief  Returns the current configuration value of the device
//! \see    usb_20.pdf - Section 9.4.2
#define USB_GET_CONFIGURATION          0x08

//! \brief  Sets the configuration of the device
//! \see    usb_20.pdf - Section 9.4.7
#define USB_SET_CONFIGURATION          0x09

//! \brief  Returns the specified alternate setting for an interface
//! \see    usb_20.pdf - Section 9.4.4
#define USB_GET_INTERFACE              0x0A

//! \brief  Selects an alternate setting for the selected interface
//! \see    usb_20.pdf - Section 9.4.10
#define USB_SET_INTERFACE              0x0B

//! \brief  Sets and reports an endpoint synchronization frame
//! \see    usb_20.pdf - Section 9.4.11
#define USB_SYNCH_FRAME                0x0C
//! @}

//! \ingroup std_dev_req
//! \defgroup clr_set_feat_const Clear/Set Feature - Constants
//! \brief Useful constants when declaring a Clear Feature or Set Feature
//! standard request.
//! \see std_dev_req
//! \see S_usb_request
//! \see usb_20.pdf - Section 9.4 - Table 9.6
//! @{

//! \name Standard Feature Selectors
//! Possible values for the wValue field of the Clear Feature and Set Feature
//! standard requests.
//! @{

//! \brief Halt feature of an endpoint
#define USB_ENDPOINT_HALT              0x00

//! \brief Remote wake-up feature of the device
#define USB_DEVICE_REMOTE_WAKEUP       0x01

//! \brief USB test mode
#define USB_TEST_MODE                  0x02
//! @}
//! @}

//! \ingroup std_dev_req
//! \defgroup set_feat_const Set Feature - Constants
//! \brief Useful constants when declaring a Set Feature standard request
//! \see usb_20.pdf - Section 7.1.20
//! \see usb_20.pdf - Section 9.2.9 - Table 9.7
//! @{

//! \name Test Mode Selectors
//! \brief Test modes available to probe an USB device.
//! @{

//! \brief Tests the high-output drive level on the D+ line
#define TEST_J                         0x01

//! \brief Tests the high-output drive level on the D- line
#define TEST_K                         0x02

//! \brief Tests the output impedance, low-level output voltage and loading
//!        characteristics
#define TEST_SEO_NAK                   0x03

//! \brief Tests rise and fall times, eye patterns and jitter
#define TEST_PACKET                    0x04

//! \brief Tests the hub disconnect detection
#define TEST_FORCE_ENABLE              0x05
//! @}
//! @}

//! \ingroup std_dev_req
//! \defgroup get_set_desc_const Get/Set Descriptor - Constants
//! \brief Useful constants when declaring a Get Descriptor or Set Descriptor
//!        standard request
//! \see StdReq
//! \see S_usb_device_descriptor
//! \see S_usb_configuration_descriptor
//! \see S_usb_endpoint_descriptor
//! \see S_usb_device_qualifier_descriptor
//! \see S_USB_LANGUAGE_ENGLISH_US
//! \see usb_20.pdf - Section 9.5 - Table 9.5
//! @{

//! \name Descriptor Types
//! \brief Possible bDescriptorType values for the descriptor structures.
//!
//! They can be used with Get Descriptor and Set Descriptor standard requests
//! to retrieve/modify them
//! @{

//! \brief Device descriptor
#define USB_DEVICE_DESCRIPTOR                       0x01

//! \brief Configuration descriptor
#define USB_CONFIGURATION_DESCRIPTOR                0x02

//! \brief String descriptor
#define USB_STRING_DESCRIPTOR                       0x03

//! \brief Interface descriptor
#define USB_INTERFACE_DESCRIPTOR                    0x04

//! \brief Endpoint descriptor
#define USB_ENDPOINT_DESCRIPTOR                     0x05

//! \brief Device qualifier descriptor
#define USB_DEVICE_QUALIFIER_DESCRIPTOR             0x06

//! \brief Other speed configuration descriptor
#define USB_OTHER_SPEED_CONFIGURATION_DESCRIPTOR    0x07

//! \brief Interface power descriptor
#define USB_INTERFACE_POWER_DESCRIPTOR              0x08
//! @}
//! @}

//! \ingroup usb_std_struc
//! \defgroup ept_desc_const Endpoint Descriptor - Constants
//! \brief Useful constants when declaring an endpoint descriptor
//! \see S_usb_endpoint_descriptor
//! \see usb_20.pdf - Section 9.6.6 - Table 9.13
//! @{

//! \name  bEndpointAddress field
//! \brief Values for the bEndpointAddress field of an endpoint descriptor.
//! @{

//! \brief Defines an OUT endpoint
#define USB_ENDPOINT_OUT                            (0 << 7)

//! \brief Defines an IN endpoint
#define USB_ENDPOINT_IN                             (1 << 7)
//! @}

//! \name  bmAttributes field
//! \brief These are the four possible tranfer type values for the bmAttributes
//!        field of an endpoint descriptor.
//! @{

//! \brief Defines a CONTROL endpoint
#define ENDPOINT_TYPE_CONTROL          0x00

//! \brief Defines a ISOCHRONOUS endpoint
#define ENDPOINT_TYPE_ISOCHRONOUS      0x01

//! \brief Defines a BULK endpoint
#define ENDPOINT_TYPE_BULK             0x02

//! \brief Defines an INTERRUPT endpoint
#define ENDPOINT_TYPE_INTERRUPT        0x03
//! @}
//! @}

//! \name  bmRequestType field
//! \brief This bitmapped field identifies the characteristics of the specific
//!        request.
//! \see   usb_209.pdf - Table 9-2. Format of Setup Data
//! @{

//! \brief D6...5: Type
//! \brief Defines a standard request
#define USB_STANDARD_REQUEST                0x00

//! \brief Defines a class request
#define USB_CLASS_REQUEST                   0x01

//! \brief Defines a vendor request
#define USB_VENDOR_REQUEST                  0x02

//! \brief Get the type of bmRequestType
#define USB_REQUEST_TYPE(pSetup)            ((pSetup->bmRequestType & 0x60) >> 5)

//! \brief Get the receipient of bmRequestType
#define USB_REQUEST_RECIPIENT(pSetup)       (pSetup->bmRequestType & 0x1F)

//! \brief Recipient is the whole device
#define USB_RECIPIENT_DEVICE                0x00

//! \brief Recipient is an interface
#define USB_RECIPIENT_INTERFACE             0x01

//! \brief Recipient is an endpoint
#define USB_RECIPIENT_ENDPOINT              0x02

//! @}

//! \ingroup usb_std_struc
//! \defgroup ept_desc_macros Endpoint Descriptor - Macros
//! \brief Useful macros when declaring an endpoint descriptor
//! \see S_usb_endpoint_descriptor
//! \see usb_20.pdf - Section 9.6.6 - Table 9.13
//! @{

//! \name bEndpointAddress field
//! @{

//! \brief Returns an endpoint number
#define USB_ENDPOINT_NUMBER(bEndpointAddress)       (bEndpointAddress & 0x0F)

//! \brief Returns an endpoint direction (IN or OUT)
#define USB_ENDPOINT_DIRECTION(bEndpointAddress)    (bEndpointAddress & 0x80)
//! @}
//! @}

//! \ingroup usb_std_req_hlr
//! \defgroup usb_class_codes USB Class Codes
//! These are the class codes approved by the USB-IF organization. They can be
//! used for the bDeviceClass value of a device descriptor, or the
//! bInterfaceClass value of an interface descriptor.
//! \see S_usb_device_descriptor
//! \see S_usb_interface_descriptor
//! \see http://www.usb.org/developers/defined_class
//! @{

//! \brief Indicates that the class information is determined by the interface
//!        descriptor.
#define USB_CLASS_DEVICE                0x00

//! \brief Audio capable devices
#define USB_CLASS_AUDIO                 0x01

//! \brief Communication devices
#define USB_CLASS_COMMUNICATION         0x02

//! \brief Human-interface devices
#define USB_CLASS_HID                   0x03

//! \brief Human-interface devices requiring real-time physical feedback
#define USB_CLASS_PHYSICAL              0x05

//! \brief Still image capture devices
#define USB_CLASS_STILL_IMAGING         0x06

//! \brief Printer devices
#define USB_CLASS_PRINTER               0x07

//! \brief Mass-storage devices
#define USB_CLASS_MASS_STORAGE          0x08

//! \brief Hub devices
#define USB_CLASS_HUB                   0x09

//! \brief Raw-data communication device
#define USB_CLASS_CDC_DATA              0x0A

//! \brief Smartcards devices
#define USB_CLASS_SMARTCARDS            0x0B

//! \brief Protected content devices
#define USB_CLASS_CONTENT_SECURITY      0x0D

//! \brief Video recording devices
#define USB_CLASS_VIDEO                 0x0E

//! \brief Devices that diagnostic devices
#define USB_CLASS_DIAGNOSTIC_DEVICE     0xDC

//! \brief Wireless controller devices
#define USB_CLASS_WIRELESS_CONTROLLER   0xE0

//! \brief Miscellaneous devices
#define USB_CLASS_MISCELLANEOUS         0xEF

//! \brief Application-specific class code
#define USB_CLASS_APPLICATION_SPECIFIC  0xFE

//! \brief Vendor-specific class code
#define USB_CLASS_VENDOR_SPECIFIC       0xFF
//! @}

//! \ingroup usb_std_struc
//! \defgroup dev_desc_const Device Descriptor - Constants
//! Several useful constants when declaring a device descriptor
//! \see S_usb_device_descriptor
//! \see S_usb_device_qualifier_descriptor
//! \see usb_20.pdf - Section 9.6.1 - Table 9.8
//! @{

//! \name USB specification release codes
//! @{

//! bcdUSB field - USB 2.0 specification code
#define USB2_00                     0x0200
#define USB1_10                     0x0110
//! @}

//! \name Vendor IDs
//! @{

//! idVendor - ATMEL Vendor ID
#define USB_VENDOR_ATMEL            0x03EB
//! @}
//! @}

//! \ingroup usb_std_struc
//! \defgroup cfg_desc_const Configuration Descriptor - Constants
//! Several useful constants when declaring a configuration descriptor
//! \see S_usb_configuration_descriptor
//! \see usb_20.pdf - Section 9.6.3 - Table 9.10
//! @{

//! \name bmAttributes field
//! \brief These are the possible values for the bmAttributes field of a
//!        S_usb_configuration_descriptor.
//! @{

//! \brief Device is bus-powered and does not support remote wakeup
#define USB_CONFIG_BUS_NOWAKEUP    0x80

//! \brief Device is self-powered and does not support remote wakeup
#define USB_CONFIG_SELF_NOWAKEUP   0xC0

//! \brief Device is bus-powered and supports remote wakeup
#define USB_CONFIG_BUS_WAKEUP      0xA0

//! \brief Device is self-powered and supports remote wakeup
#define USB_CONFIG_SELF_WAKEUP     0xE0
//! @}

//! \name Power consumption
//! @{

//! Power consumption macro for the Configuration descriptor
#define USB_POWER_MA(power)        (power/2)
//! @}
//! @}

//! \ingroup usb_std_struc
//! \defgroup str_desc_const String Descriptor - Constants
//! \brief Useful constants when declaring a string descriptor.
//! \see S_usb_string_descriptor
//! \see USB_LANGIDs.pdf
//! @{

//! \name Language IDs
//! \brief These are the supported language IDs as defined by the USB-IF group.
//!        They can be used to specified the languages supported by the string
//!        descriptors of a USB device.
//! @{

//! \brief English (United States)
#define USB_LANGUAGE_ENGLISH_US     0x0409
//! @}
//! @}

//! \ingroup usb_std_struc
//! \defgroup str_desc_macros String Descriptor - Macros
//! \brief Several useful macros when declaring a string descriptor.
//! \see S_usb_string_descriptor
//! @{

//! Converts an ASCII character to its Unicode equivalent
#define USB_UNICODE(a)                      (a), 0x00

//! Calculates the size of a string descriptor given the number of ASCII
//! characters in it
#define USB_STRING_DESCRIPTOR_SIZE(size)    ((size * 2) + 2)
//! @}

//! \ingroup usb_api_methods
//! \defgroup usb_api_ret_val Standard return values
//! \brief Values returned by the API methods.
//! @{

//! \brief Last method has completed successfully
#define USB_STATUS_SUCCESS      0

//! \brief Method was aborted because the recipient (device, endpoint, ...) was
//!        busy
#define USB_STATUS_LOCKED       1

//! \brief Method was aborted because of abnormal status
#define USB_STATUS_ABORTED      2

//! \brief Method was aborted because the endpoint or the device has been reset
#define USB_STATUS_RESET        3
//! @}

// Device State
//! \ingroup S_usb_struc
//! \defgroup S_usb_dev_state USB Device States
//! \brief Constant values used to track which USB state the device is currently
//!        in.
//! @{

//! Attached state
#define USB_STATE_ATTACHED                          (1 << 0)

//! Powered state
#define USB_STATE_POWERED                           (1 << 1)

//! Default state
#define USB_STATE_DEFAULT                           (1 << 2)

//! Address state
#define USB_STATE_ADDRESS                           (1 << 3)

//! Configured state
#define USB_STATE_CONFIGURED                        (1 << 4)

//! Suspended state
#define USB_STATE_SUSPENDED                         (1 << 5)

//! @}

//------------------------------------------------------------------------------
//      Structures
//------------------------------------------------------------------------------

// USB standard structures
//-------------------------
//! \ingroup usb_std_struc
//! \brief This structure represents a standard SETUP request
//! \see usb_20.pdf - Section 9.3 - Table 9.2
struct S_usb_request {
    unsigned char   bmRequestType:8;    //!< Characteristics of the request
    unsigned char   bRequest:8;         //!< Particular request
    unsigned short  wValue:16;          //!< Request-specific parameter
    unsigned short  wIndex:16;          //!< Request-specific parameter
    unsigned short  wLength:16;         //!< Length of data for the data phase
} __attribute__((packed));

//! \ingroup usb_std_struc
//! \brief This descriptor structure is used to provide information on
//!        various parameters of the device
//!
//! Usage example:
//! \include S_usb_device_descriptor_example.c
//! \see usb_20.pdf - Section 9.6.1
struct S_usb_device_descriptor_s {

   unsigned char  bLength;              //!< Size of this descriptor in bytes
   unsigned char  bDescriptorType;      //!< DEVICE descriptor type
   unsigned short bscUSB;               //!< USB specification release number
   unsigned char  bDeviceClass;         //!< Class code
   unsigned char  bDeviceSubClass;      //!< Subclass code
   unsigned char  bDeviceProtocol;      //!< Protocol code
   unsigned char  bMaxPacketSize0;      //!< Control endpoint 0 max. packet size
   unsigned short idVendor;             //!< Vendor ID
   unsigned short idProduct;            //!< Product ID
   unsigned short bcdDevice;            //!< Device release number
   unsigned char  iManufacturer;        //!< Index of manu. string descriptor
   unsigned char  iProduct;             //!< Index of prod. string descriptor
   unsigned char  iSerialNumber;        //!< Index of S.N.  string descriptor
   unsigned char  bNumConfigurations;   //!< Number of possible configurations

} __attribute__((packed));

typedef struct S_usb_device_descriptor_s S_usb_device_descriptor ;

//! \ingroup usb_std_struc
//! \brief This is the standard configuration descriptor structure. It is used
//!        to report the current configuration of the device.
//!
//! Usage example:
//! \include S_usb_configuration_descriptor_example.c
//! \see usb_20.pdf - Section 9.6.3
struct S_usb_configuration_descriptor {
   unsigned char  bLength;              //!< Size of this descriptor in bytes
   unsigned char  bDescriptorType;      //!< CONFIGURATION descriptor type
   unsigned short wTotalLength;         //!< Total length of data returned
                                        //!< for this configuration
   unsigned char  bNumInterfaces;       //!< Number of interfaces for this
                                        //!< configuration
   unsigned char  bConfigurationValue;  //!< Value to use as an argument for
                                        //!< the Set Configuration request to
                                        //!< select this configuration
   unsigned char  iConfiguration;       //!< Index of string descriptor
                                        //!< describing this configuration
   unsigned char  bmAttibutes;          //!< Configuration characteristics
   unsigned char  bMaxPower;            //!< Maximum power consumption of the
                                        //!< device
} __attribute__((packed));


//! \ingroup usb_std_struc
//! \brief Standard interface descriptor. Used to describe a specific interface
//!        of a configuration.
//!
//! Usage example:
//! \include S_usb_interface_descriptor_example.c
//! \see usb_20.pdf - Section 9.6.5
struct S_usb_interface_descriptor {
   unsigned char bLength;               //!< Size of this descriptor in bytes
   unsigned char bDescriptorType;       //!< INTERFACE descriptor type
   unsigned char bInterfaceNumber;      //!< Number of this interface
   unsigned char bAlternateSetting;     //!< Value used to select this alternate
                                        //!< setting
   unsigned char bNumEndpoints;         //!< Number of endpoints used by this
                                        //!< interface (excluding endpoint zero)
   unsigned char bInterfaceClass;       //!< Class code
   unsigned char bInterfaceSubClass;    //!< Sub-class
   unsigned char bInterfaceProtocol;    //!< Protocol code
   unsigned char iInterface;            //!< Index of string descriptor
                                        //!< describing this interface
} __attribute__((packed));

//! \ingroup usb_std_struc
//! \brief This structure is the standard endpoint descriptor. It contains
//!        the necessary information for the host to determine the bandwidth
//!        required by the endpoint.
//!
//! Usage example:
//! \include S_usb_endpoint_descriptor_example.c
//! \see usb_20.pdf - Section 9.6.6
struct S_usb_endpoint_descriptor {
   unsigned char  bLength;              //!< Size of this descriptor in bytes
   unsigned char  bDescriptorType;      //!< ENDPOINT descriptor type
   unsigned char  bEndpointAddress;     //!< Address of the endpoint on the USB
                                        //!< device described by this descriptor
   unsigned char  bmAttributes;         //!< Endpoint attributes when configured
   unsigned short wMaxPacketSize;       //!< Maximum packet size this endpoint
                                        //!< is capable of sending or receiving
   unsigned char  bInterval;            //!< Interval for polling endpoint for
                                        //!< data transfers
} __attribute__((packed));

//! \ingroup usb_std_struc
//! \brief The device qualifier structure provide information on a high-speed
//!        capable device if the device was operating at the other speed.
//!
//! Usage example:
//! \include S_usb_device_qualifier_descriptor_example.c
//! \see usb_20.pdf - Section 9.6.2
struct S_usb_device_qualifier_descriptor {
   unsigned char  bLength;              //!< Size of this descriptor in bytes
   unsigned char  bDescriptorType;      //!< DEVICE_QUALIFIER descriptor type
   unsigned short bscUSB;               //!< USB specification release number
   unsigned char  bDeviceClass;         //!< Class code
   unsigned char  bDeviceSubClass;      //!< Sub-class code
   unsigned char  bDeviceProtocol;      //!< Protocol code
   unsigned char  bMaxPacketSize0;      //!< Control endpoint 0 max. packet size
   unsigned char  bNumConfigurations;   //!< Number of possible configurations
   unsigned char  bReserved;            //!< Reserved for future use, must be 0

} __attribute__((packed));

//! \ingroup usb_std_struc
//! \brief The S_usb_language_id structure represents the string descriptor
//!        zero, used to specify the languages supported by the device. This
//!        structure only define one language ID.
//!
//! Usage example:
//! \include S_usb_language_id_example.c
//! \see usb_20.pdf - Section 9.6.7 - Table 9.15
struct S_usb_language_id {
   unsigned char  bLength;               //!< Size of this descriptor in bytes
   unsigned char  bDescriptorType;       //!< STRING descriptor type
   unsigned short wLANGID;               //!< LANGID code zero
} __attribute__((packed));

#define USB_ENDPOINT0_MAXPACKETSIZE       8

//! \brief 
#define SELF_POWERED              (1<<0)

//! \brief 
#define REMOTE_WAKEUP             (1<<1)

//! @}
//! @}

//------------------------------------------------------------------------------
//      Structures
//------------------------------------------------------------------------------

//! \ingroup usb_std_req_hlr
//! \brief   List of standard descriptors used by the device
struct S_std_descriptors {

    //! Device descriptor
    const struct S_usb_device_descriptor_s         *pDevice;
    //! Configuration descriptor
    const struct S_usb_configuration_descriptor    *pConfiguration;
    //! List of string descriptors
    const char                              **pStrings;
    //! List of endpoint descriptors
    const struct S_usb_endpoint_descriptor         **pEndpoints;
#if defined(HIGHSPEED)
    //! Qualifier descriptor (high-speed only)
    const struct S_usb_device_qualifier_descriptor *pQualifier;
    //! Other speed configuration descriptor (high-speed only)
    const struct S_usb_configuration_descriptor    *pOtherSpeedConfiguration;
#endif

};

struct S_core_configuration_descriptor {
    struct S_usb_configuration_descriptor sConfiguration;
    struct S_usb_interface_descriptor     sInterface;
    struct S_usb_endpoint_descriptor	  sBulkOut;
    struct S_usb_endpoint_descriptor	  sBulkIn;
} __attribute__((packed));

#endif // _USB_H
