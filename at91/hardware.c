#include "hardware.h"
#include "usbEndpoints.h"

unsigned const endpointMaxSize[UDP_MAXENDPOINTS] = {
	USBMAX_CONTROL_DATA,
	USBMAX_BULK_DATA,
	USBMAX_BULK_DATA,
	USBMAX_BULK_DATA,
	256,
	256
};

