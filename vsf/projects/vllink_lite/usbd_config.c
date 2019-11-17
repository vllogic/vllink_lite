static const uint8_t USB_DeviceDescriptor[] =
{
	0x12,        // bLength
	0x01,        // bDescriptorType (Device)
	0x10, 0x02,  // bcdUSB 2.10
	0xEF,        // bDeviceClass 
	0x02,        // bDeviceSubClass 
	0x01,        // bDeviceProtocol 
	0x40,        // bMaxPacketSize0 64
	(APPCFG_USBD_VID >> 0) & 0xFF,
	(APPCFG_USBD_VID >> 8) & 0xFF,
			// vendor
	(APPCFG_USBD_PID >> 0) & 0xFF,
	(APPCFG_USBD_PID >> 8) & 0xFF,
			// product
	0x00, 0x10,	// bcdDevice
	1,			// manu facturer
	2,			// product
	3,			// serial number
	0x01		// number of configuration 
};

static const uint8_t USB_ConfigDescriptor[] =
{
	
	USB_DT_CONFIG_SIZE,
	USB_DT_CONFIG,
				// wTotalLength
	(USBD_CONFIGDESC_LENGTH >> 0) & 0xFF,
	(USBD_CONFIGDESC_LENGTH >> 8) & 0xFF,
	USBD_INTERFACE_COUNT,		// bNumInterfaces
	0x01,		// bConfigurationValue: Configuration value
	0x00,		// iConfiguration: Index of string descriptor describing the configuration
	0x80,		// bmAttributes: bus powered
	0xfa,	// MaxPower 500 mA

#ifdef PROJC_CFG_CMSIS_DAP_V1_SUPPORT
	/* CMSIS-DAP V1, Length: 32 */ 
	0x09,        //   bLength
	0x04,        //   bDescriptorType (Interface)
	0x00,        //   bInterfaceNumber 0
	0x00,        //   bAlternateSetting
	0x02,        //   bNumEndpoints 2
	0x03,        //   bInterfaceClass
	0x00,        //   bInterfaceSubClass
	0x00,        //   bInterfaceProtocol
	0x04,        //   iInterface (String Index)
	
	0x09,        //   bLength
	0x21,        //   bDescriptorType (HID)
	0x00, 0x01,  //   bcdHID 1.00
	0x00,        //   bCountryCode
	0x01,        //   bNumDescriptors
	0x22,        //   bDescriptorType[0] (HID)
	(sizeof(vsfusbd_CMSIS_DAP_HIDReportDesc) >> 0) & 0xFF,
	(sizeof(vsfusbd_CMSIS_DAP_HIDReportDesc) >> 8) & 0xFF,
				//   wDescriptorLength[0]

	0x07,        //   bLength
	0x05,        //   bDescriptorType (Endpoint)
	0x81,        //   bEndpointAddress (IN/D2H)
	0x03,        //   bmAttributes (Interrupt)
	0x40, 0x00,  //   wMaxPacketSize 64
	0x01,        //   bInterval 1 (unit depends on device speed)

	0x07,        //   bLength
	0x05,        //   bDescriptorType (Endpoint)
	0x01,        //   bEndpointAddress (OUT/H2D)
	0x03,        //   bmAttributes (Interrupt)
	0x40, 0x00,  //   wMaxPacketSize 64
	0x01,        //   bInterval 1 (unit depends on device speed)
#endif	// PROJC_CFG_CMSIS_DAP_V1_SUPPORT
	
#ifdef PROJC_CFG_CMSIS_DAP_V2_SUPPORT
	/* CMSIS-DAP V2, Length: 23 */ 
	0x09,        //   bLength
	0x04,        //   bDescriptorType (Interface)
	0x01,        //   bInterfaceNumber 1
	0x00,        //   bAlternateSetting
	0x02,        //   bNumEndpoints 2
	0xFF,        //   bInterfaceClass
	0x00,        //   bInterfaceSubClass
	0x00,        //   bInterfaceProtocol
	0x05,        //   iInterface (String Index)

	0x07,        //   bLength
	0x05,        //   bDescriptorType (Endpoint)
	0x02,        //   bEndpointAddress (OUT/H2D)
	0x02,        //   bmAttributes (Bulk)
	0x40, 0x00,  //   wMaxPacketSize 64
	0x00,        //   bInterval 0 (unit depends on device speed)

	0x07,        //   bLength
	0x05,        //   bDescriptorType (Endpoint)
	0x82,        //   bEndpointAddress (IN/D2H)
	0x02,        //   bmAttributes (Bulk)
	0x40, 0x00,  //   wMaxPacketSize 64
	0x00,        //   bInterval 0 (unit depends on device speed)
#endif	// PROJC_CFG_CMSIS_DAP_V2_SUPPORT

#ifdef PROJC_CFG_CDCEXT_SUPPORT
	/* IAD CDC EXT, Length: 66 */ 
	0x08,	// bLength: IAD Descriptor size
	USB_DT_INTERFACE_ASSOCIATION,	// bDescriptorType: IAD
	2,		// bFirstInterface
	2,		// bInterfaceCount
	0x02,	// bFunctionClass
	0x02,	// bFunctionSubClass
	0x01,	// bFunctionProtocol
	0x06,	// iFunction

	// Data class interface descriptor
	0x09,	// bLength: Endpoint Descriptor size
	USB_DT_INTERFACE,
			// bDescriptorType: Interface
	0x02,	// bInterfaceNumber: Number of Interface
	0x00,	// bAlternateSetting: Alternate setting
	0x02,	// bNumEndpoints: Two endpoints used
	0x0A,	// bInterfaceClass: CDC
	0x00,	// bInterfaceSubClass:
	0x00,	// bInterfaceProtocol:
	0x00,	// iInterface:
	
	// Endpoint Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x83,	// bEndpointAddress:
	0x02,	// bmAttributes: Bulk
	64,		// wMaxPacketSize:
	0x00,
	0x00,	// bInterval
	
	// Endpoint Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x03,	// bEndpointAddress:
	0x02,	// bmAttributes: Bulk
	64,		// wMaxPacketSize:
	0x00,
	0x00,	// bInterval: ignore for Bulk transfer
	
	// Interface Descriptor for CDC
	0x09,	// bLength: Interface Descriptor size
	USB_DT_INTERFACE,
			// bDescriptorType: Interface
	0x03,	// bInterfaceNumber: Number of Interface
	0x00,	// bAlternateSetting: Alternate setting
	0x01,	// bNumEndpoints: One endpoints used
	0x02,	// bInterfaceClass: Communication Interface Class
	0x02,	// bInterfaceSubClass: Abstract Control Model
	0x01,	// bInterfaceProtocol: Common AT commands
	0x00,	// iInterface:
	
	// Header Functional Descriptor
	0x05,	// bLength: Endpoint Descriptor size
	0x24,	// bDescriptorType: CS_INTERFACE
	0x00,	// bDescriptorSubtype: Header Func Desc
	0x10,	// bcdCDC: spec release number
	0x01,
	
	// Call Managment Functional Descriptor
	0x05,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x01,	// bDescriptorSubtype: Call Management Func Desc
	0x00,	// bmCapabilities: D0+D1
	0x01,	// bDataInterface: 1
	
	// ACM Functional Descriptor
	0x04,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x02,	// bDescriptorSubtype: Abstract Control Management desc
	0x02,	// bmCapabilities
	
	// Union Functional Descriptor
	0x05,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x06,	// bDescriptorSubtype: Union func desc
	3,		// bMasterInterface: Communication class interface
	2,		// bSlaveInterface0: Data Class Interface
	
	// Endpoint Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x85,	// bEndpointAddress:
	0x03,	// bmAttributes: Interrupt
	8,		// wMaxPacketSize:
	0x00,
	0xFF,	// bInterval:
#endif // PROJC_CFG_CDCEXT_SUPPORT

#ifdef PROJC_CFG_CDCSHELL_SUPPORT
	/* IAD CDC Shell, Length: 66 */ 
	0x08,	// bLength: IAD Descriptor size
	USB_DT_INTERFACE_ASSOCIATION,	// bDescriptorType: IAD
	4,		// bFirstInterface
	2,		// bInterfaceCount
	0x02,	// bFunctionClass
	0x02,	// bFunctionSubClass
	0x01,	// bFunctionProtocol
	0x07,	// iFunction

	// Data class interface descriptor
	0x09,	// bLength: Endpoint Descriptor size
	USB_DT_INTERFACE,
			// bDescriptorType: Interface
	0x04,	// bInterfaceNumber: Number of Interface
	0x00,	// bAlternateSetting: Alternate setting
	0x02,	// bNumEndpoints: Two endpoints used
	0x0A,	// bInterfaceClass: CDC
	0x00,	// bInterfaceSubClass:
	0x00,	// bInterfaceProtocol:
	0x00,	// iInterface:
	
	// Endpoint Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x84,	// bEndpointAddress:
	0x02,	// bmAttributes: Bulk
	64,		// wMaxPacketSize:
	0x00,
	0x00,	// bInterval
	
	// Endpoint Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x04,	// bEndpointAddress:
	0x02,	// bmAttributes: Bulk
	64,		// wMaxPacketSize:
	0x00,
	0x00,	// bInterval: ignore for Bulk transfer
	
	// Interface Descriptor for CDC
	0x09,	// bLength: Interface Descriptor size
	USB_DT_INTERFACE,
			// bDescriptorType: Interface
	0x05,	// bInterfaceNumber: Number of Interface
	0x00,	// bAlternateSetting: Alternate setting
	0x01,	// bNumEndpoints: One endpoints used
	0x02,	// bInterfaceClass: Communication Interface Class
	0x02,	// bInterfaceSubClass: Abstract Control Model
	0x01,	// bInterfaceProtocol: Common AT commands
	0x00,	// iInterface:
	
	// Header Functional Descriptor
	0x05,	// bLength: Endpoint Descriptor size
	0x24,	// bDescriptorType: CS_INTERFACE
	0x00,	// bDescriptorSubtype: Header Func Desc
	0x10,	// bcdCDC: spec release number
	0x01,
	
	// Call Managment Functional Descriptor
	0x05,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x01,	// bDescriptorSubtype: Call Management Func Desc
	0x00,	// bmCapabilities: D0+D1
	0x01,	// bDataInterface: 1
	
	// ACM Functional Descriptor
	0x04,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x02,	// bDescriptorSubtype: Abstract Control Management desc
	0x02,	// bmCapabilities
	
	// Union Functional Descriptor
	0x05,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x06,	// bDescriptorSubtype: Union func desc
	3,		// bMasterInterface: Communication class interface
	2,		// bSlaveInterface0: Data Class Interface
	
	// Endpoint Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x86,	// bEndpointAddress:
	0x03,	// bmAttributes: Interrupt
	8,		// wMaxPacketSize:
	0x00,
	0xFF,	// bInterval:
#endif	// PROJC_CFG_CDCSHELL_SUPPORT
};

static const uint8_t USB_StringLangID[] =
{
	4,
	USB_DT_STRING,
	0x09,
	0x04
};

static const uint8_t USB_StringVendor[] =
{
	24,
	USB_DT_STRING,
	'V', 0, 'l', 0, 'l', 0, 'o', 0, 'g', 0, 'i', 0, 'c', 0, '.', 0,
	'c', 0, 'o', 0, 'm', 0,
};

static const uint8_t USB_StringProduct[] =
{
	46,
	USB_DT_STRING,
	'V', 0, 'l', 0, 'l', 0, 'i', 0, 'n', 0, 'k', 0, ' ', 0, 'L', 0,
	'i', 0, 't', 0, 'e', 0, '(', 0, 'C', 0, 'M', 0, 'S', 0, 'I', 0,
	'S', 0, '-', 0, 'D', 0, 'A', 0, 'P', 0, ')', 0,
};

static const uint8_t USB_StringSerial[50] =
{
	50,
	USB_DT_STRING,
	'0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0,
	'0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0,
	'0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0,
};

static const uint8_t CMSIS_DAP_V1_StringFunc[] =
{
	26,
	USB_DT_STRING,
	'C', 0, 'M', 0, 'S', 0, 'I', 0, 'S', 0, '-', 0, 'D', 0, 'A', 0, 'P', 0, ' ', 0, 'V', 0, '1', 0,
};

static const uint8_t CMSIS_DAP_V2_StringFunc[] =
{
	26,
	USB_DT_STRING,
	'C', 0, 'M', 0, 'S', 0, 'I', 0, 'S', 0, '-', 0, 'D', 0, 'A', 0, 'P', 0, ' ', 0, 'V', 0, '2', 0,
};

static const uint8_t CDCEXT_StringFunc[] =
{
	28,
	USB_DT_STRING,
	'V', 0, 'l', 0, 'l', 0, 'i', 0, 'n', 0, 'k', 0, '-', 0, 'C', 0, 'D', 0, 'C', 0, 'E', 0, 'x', 0, 't', 0,
};

static const uint8_t CDCSHELL_StringFunc[] =
{
	32,
	USB_DT_STRING,
	'V', 0, 'l', 0, 'l', 0, 'i', 0, 'n', 0, 'k', 0, '-', 0, 'C', 0, 'D', 0, 'C', 0, 'S', 0, 'h', 0, 'e', 0, 'l', 0, 'l', 0,
};

static const struct vsfusbd_desc_filter_t USB_descriptors[] =
{
	VSFUSBD_DESC_DEVICE(0, USB_DeviceDescriptor, sizeof(USB_DeviceDescriptor)),
	VSFUSBD_DESC_CONFIG(0, 0, USB_ConfigDescriptor, sizeof(USB_ConfigDescriptor)),
	VSFUSBD_DESC_STRING(0, 0, USB_StringLangID, sizeof(USB_StringLangID)),
	VSFUSBD_DESC_STRING(0x0409, 1, USB_StringVendor, sizeof(USB_StringVendor)),
	VSFUSBD_DESC_STRING(0x0409, 2, USB_StringProduct, sizeof(USB_StringProduct)),
	VSFUSBD_DESC_STRING(0x0409, 3, USB_StringSerial, sizeof(USB_StringSerial)),
	VSFUSBD_DESC_STRING(0x0409, 4, CMSIS_DAP_V1_StringFunc, sizeof(CMSIS_DAP_V1_StringFunc)),
	VSFUSBD_DESC_STRING(0x0409, 5, CMSIS_DAP_V2_StringFunc, sizeof(CMSIS_DAP_V2_StringFunc)),
	VSFUSBD_DESC_STRING(0x0409, 6, CDCEXT_StringFunc, sizeof(CDCEXT_StringFunc)),
	VSFUSBD_DESC_STRING(0x0409, 7, CDCSHELL_StringFunc, sizeof(CDCSHELL_StringFunc)),
	VSFUSBD_DESC_NULL
};

