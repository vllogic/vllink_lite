typedef struct usbd_vllinklite_nonconst_t {
    struct {
        uint8_t str_serial[50];
    } usbd;
} usbd_vllinklite_nonconst_t;

typedef struct usbd_vllinklite_const_t {
    struct {
        uint8_t dev_desc[18];
        uint8_t config_desc[APP_CFG_USBD_CONFIGDESC_LENGTH];
        uint8_t winusb_desc[APP_CFG_USBD_WINUSB_DESC_LENGTH];
        uint8_t bos_desc[APP_CFG_USBD_BOS_DESC_LENGTH];
        uint8_t str_lanid[4];
        uint8_t str_vendor[8];
        uint8_t str_product[36];
        uint8_t str_cmsis_dap_v2[26];
        uint8_t str_webusb[36];
        uint8_t str_cdcext[28];
        #ifdef APP_CFG_CDCSHELL_SUPPORT
        uint8_t str_cdcshell[32];
        #endif
        vk_usbd_desc_t std_desc[APP_CFG_USBD_DESC_COUNT];
    } usbd;
} usbd_vllinklite_const_t;

typedef struct usbd_vllinklite_t {
    struct {
        vk_usbd_dev_t dev;
        vk_usbd_cfg_t config[1];
        vk_usbd_ifs_t ifs[APP_CFG_USBD_INTERFACE_COUNT];

        #ifdef APP_CFG_CMSIS_DAP_V2_SUPPORT
        vk_usbd_cmsis_dap_v2_t cmsis_dap_v2;
        #endif
        #ifdef APP_CFG_WEBUSB_SUPPORT
        vk_usbd_webusb_t webusb;
        #endif
        #ifdef APP_CFG_CDCEXT_SUPPORT
        struct {
            vk_usbd_cdcacm_t param;
            vsf_block_stream_t ext2usb;
            vsf_block_stream_t usb2ext;
            uint8_t ext2usb_buf[(APP_CFG_CDCEXT_PKT_SIZE + 4) * 4];
            uint8_t usb2ext_buf[(APP_CFG_CDCEXT_PKT_SIZE + 4) * 3];
        } cdcext;
        #endif
        #ifdef APP_CFG_CDCSHELL_SUPPORT
        struct {
            vk_usbd_cdcacm_t param;
            vsf_block_stream_t shell2usb;
            vsf_block_stream_t usb2shell;
            uint8_t shell2usb_buf[(APP_CFG_CDCSHELL_PKT_SIZE + 4) * 8];
            uint8_t usb2shell_buf[(APP_CFG_CDCSHELL_PKT_SIZE + 4) * 3];
        } cdcshell;
        #endif
    } usbd;
} usbd_vllinklite_t;

static usbd_vllinklite_nonconst_t __usrapp_usbd_vllinklite_nonconst;

static const usbd_vllinklite_const_t __usrapp_usbd_vllinklite_const = {
    .usbd                       = {
        .dev_desc               = {
            USB_DT_DEVICE_SIZE,
            USB_DT_DEVICE,
            0x10, 0x02,             // bcdUSB
            0xEF,                   // device class: IAD
            0x02,                   // device sub class
            0x01,                   // device protocol
            64,                     // max packet size
            USB_DESC_WORD(APP_CFG_USBD_VID),
                                    // vendor
            USB_DESC_WORD(APP_CFG_USBD_PID),
                                    // product
            USB_DESC_WORD(APP_CFG_USBD_BCD),  
                                    // bcdDevice
            1,                      // manu facturer
            2,                      // product
            3,                      // serial number
            1,                      // number of configuration
        },
        .config_desc            = {
            USB_DT_CONFIG_SIZE,
            USB_DT_CONFIG,
            USB_DESC_WORD(sizeof(__usrapp_usbd_vllinklite_const.usbd.config_desc)),
                                    // wTotalLength
            APP_CFG_USBD_INTERFACE_COUNT,
                                    // bNumInterfaces
            0x01,                   // bConfigurationValue: Configuration value
            0x00,                   // iConfiguration: Index of string descriptor describing the configuration
            0x80,                   // bmAttributes: bus powered
            0xFA,                   // MaxPower
            
            #ifdef APP_CFG_CMSIS_DAP_V2_SUPPORT
            /* CMSIS-DAP V2, Length: 23 */ 
            0x09,        // bLength
            0x04,        // bDescriptorType (Interface)
            0x00,        // bInterfaceNumber 0
            0x00,        // bAlternateSetting
            0x02,        // bNumEndpoints 2
            0xFF,        // bInterfaceClass
            0x00,        // bInterfaceSubClass
            0x00,        // bInterfaceProtocol
            0x04,        // iInterface (String Index)

            0x07,        // bLength
            0x05,        // bDescriptorType (Endpoint)
            0x01,        // bEndpointAddress (OUT/H2D)
            0x02,        // bmAttributes (Bulk)
            0x40, 0x00,  // wMaxPacketSize 64
            0x00,        // bInterval 0 (unit depends on device speed)

            0x07,        // bLength
            0x05,        // bDescriptorType (Endpoint)
            0x81,        // bEndpointAddress (IN/D2H)
            0x02,        // bmAttributes (Bulk)
            0x40, 0x00,  // wMaxPacketSize 64
            0x00,        // bInterval 0 (unit depends on device speed)
            #endif	// APP_CFG_CMSIS_DAP_V2_SUPPORT
            
            #ifdef APP_CFG_WEBUSB_SUPPORT
            /* WEBUSB, Length: 9 */ 
            0x09,        // bLength
            0x04,        // bDescriptorType (Interface)
            0x01,        // bInterfaceNumber 1
            0x00,        // bAlternateSetting
            0x00,        // bNumEndpoints 0
            0xFF,        // bInterfaceClass
            0x03,        // bInterfaceSubClass
            0x00,        // bInterfaceProtocol
            0x05,        // iInterface (String Index)
            #endif	// APP_CFG_WEBUSB_SUPPORT

            #ifdef APP_CFG_CDCEXT_SUPPORT
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
            0x80 | APP_CFG_CDCEXT_DATA_IN_EP,	// bEndpointAddress:
            0x02,	// bmAttributes: Bulk
            APP_CFG_CDCEXT_PKT_SIZE,		// wMaxPacketSize:
            0x00,
            0x00,	// bInterval

            // Endpoint Descriptor
            0x07,	// bLength: Endpoint Descriptor size
            USB_DT_ENDPOINT,
                    // bDescriptorType: Endpoint
            APP_CFG_CDCEXT_DATA_OUT_EP,	// bEndpointAddress:
            0x02,	// bmAttributes: Bulk
            APP_CFG_CDCEXT_PKT_SIZE,		// wMaxPacketSize:
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
            0x00,	// bInterfaceProtocol: 
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
            0x01,	// bmCapabilities: 
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
            0x03,	// bMasterInterface: Communication class interface
            0x02,	// bSlaveInterface0: Data Class Interface

            // Endpoint Descriptor
            0x07,	// bLength: Endpoint Descriptor size
            USB_DT_ENDPOINT,
                    // bDescriptorType: Endpoint
            0x80 | APP_CFG_CDCEXT_NOTIFY_EP,	// bEndpointAddress:
            0x03,	// bmAttributes: Interrupt
            8,		// wMaxPacketSize:
            0x00,
            0xFF,	// bInterval:
            #endif // APP_CFG_CDCEXT_SUPPORT

            #ifdef APP_CFG_CDCSHELL_SUPPORT
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
            0x80 | APP_CFG_CDCSHELL_DATA_IN_EP,	// bEndpointAddress:
            0x02,	// bmAttributes: Bulk
            APP_CFG_CDCSHELL_PKT_SIZE,		// wMaxPacketSize:
            0x00,
            0x00,	// bInterval

            // Endpoint Descriptor
            0x07,	// bLength: Endpoint Descriptor size
            USB_DT_ENDPOINT,
                    // bDescriptorType: Endpoint
            APP_CFG_CDCSHELL_DATA_OUT_EP,	// bEndpointAddress:
            0x02,	// bmAttributes: Bulk
            APP_CFG_CDCSHELL_PKT_SIZE,		// wMaxPacketSize:
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
            0x00,	// bInterfaceProtocol:
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
            0x01,	// bmCapabilities: 
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
            5,		// bMasterInterface: Communication class interface
            4,		// bSlaveInterface0: Data Class Interface

            // Endpoint Descriptor
            0x07,	// bLength: Endpoint Descriptor size
            USB_DT_ENDPOINT,
                    // bDescriptorType: Endpoint
            0x80 | APP_CFG_CDCSHELL_NOTIFY_EP,	// bEndpointAddress:
            0x03,	// bmAttributes: Interrupt
            8,		// wMaxPacketSize:
            0x00,
            0xFF,	// bInterval:
            #endif	// APP_CFG_CDCSHELL_SUPPORT
        },
        .winusb_desc            = {
            USB_DESC_WORD(10),
            0x00, 0x00,
            0x00, 0x00, 0x03, 0x06,
            USB_DESC_WORD(APP_CFG_USBD_WINUSB_DESC_LENGTH),
            
            #ifdef APP_CFG_WEBUSB_SUPPORT
            (8 >> 0) & 0xFF,
            (8 >> 8) & 0xFF,
            (2 >> 0) & 0xFF,
            (2 >> 8) & 0xFF,
            0x01, // webusb interface
            0x00,
            (FUNCTION_WEBUSB_SUBSET_LEN >> 0) & 0xFF,
            (FUNCTION_WEBUSB_SUBSET_LEN >> 8) & 0xFF,

            (20 >> 0) & 0xFF,
            (20 >> 8) & 0xFF,
            (3 >> 0) & 0xFF,
            (3 >> 8) & 0xFF,
            'W', 'I', 'N', 'U', 'S', 'B', 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 
            (132 >> 0) & 0xFF,
            (132 >> 8) & 0xFF,
            (4 >> 0) & 0xFF,
            (4 >> 8) & 0xFF,
            (7 >> 0) & 0xFF,
            (7 >> 8) & 0xFF,
            (42 >> 0) & 0xFF,
            (42 >> 8) & 0xFF,
            'D',0,'e',0,'v',0,'i',0,'c',0,'e',0,
            'I',0,'n',0,'t',0,'e',0,'r',0,'f',0,'a',0,'c',0,'e',0,
            'G',0,'U',0,'I',0,'D',0,'s',0,0,0,
            (80 >> 0) & 0xFF,
            (80 >> 8) & 0xFF,
            '{',0,
            '9',0,'2',0,'C',0,'E',0,'6',0,'4',0,'6',0,'2',0,'-',0,
            '9',0,'C',0,'7',0,'7',0,'-',0,
            '4',0,'6',0,'F',0,'E',0,'-',0,
            '9',0,'3',0,'3',0,'B',0,'-',
            0,'3',0,'1',0,'C',0,'B',0,'9',0,'C',0,'5',0,'A',0,'A',0,'3',0,'B',0,'9',0,
            '}',0,0,0,0,0,
            #endif  // APP_CFG_WEBUSB_SUPPORT

            #ifdef APP_CFG_CMSIS_DAP_V2_SUPPORT
            (8 >> 0) & 0xFF,
            (8 >> 8) & 0xFF,
            (2 >> 0) & 0xFF,
            (2 >> 8) & 0xFF,
            0x00, // cmsis-dap v2 interface
            0x00,
            (FUNCTION_CMSIS_DAP_V2_SUBSET_LEN >> 0) & 0xFF,
            (FUNCTION_CMSIS_DAP_V2_SUBSET_LEN >> 8) & 0xFF,

            (20 >> 0) & 0xFF,
            (20 >> 8) & 0xFF,
            (3 >> 0) & 0xFF,
            (3 >> 8) & 0xFF,
            'W', 'I', 'N', 'U', 'S', 'B', 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 
            (132 >> 0) & 0xFF,
            (132 >> 8) & 0xFF,
            (4 >> 0) & 0xFF,
            (4 >> 8) & 0xFF,
            (7 >> 0) & 0xFF,
            (7 >> 8) & 0xFF,
            (42 >> 0) & 0xFF,
            (42 >> 8) & 0xFF,
            'D',0,'e',0,'v',0,'i',0,'c',0,'e',0,
            'I',0,'n',0,'t',0,'e',0,'r',0,'f',0,'a',0,'c',0,'e',0,
            'G',0,'U',0,'I',0,'D',0,'s',0,0,0,
            (80 >> 0) & 0xFF,
            (80 >> 8) & 0xFF,
            '{',0,
            'C',0,'D',0,'B',0,'3',0,'B',0,'5',0,'A',0,'D',0,'-',0,
            '2',0,'9',0,'3',0,'B',0,'-',0,
            '4',0,'6',0,'6',0,'3',0,'-',0,
            'A',0,'A',0,'3',0,'6',0,'-',
            0,'1',0,'A',0,'A',0,'E',0,'4',0,'6',0,'4',0,'6',0,'3',0,'7',0,'7',0,'6',0,
            '}',0,0,0,0,0,
            #endif  // APP_CFG_CMSIS_DAP_V2_SUPPORT
        },
        .bos_desc              = {
            0x05,
            0x0F,
            USB_DESC_WORD(APP_CFG_USBD_BOS_DESC_LENGTH),
            APP_CFG_USBD_BOS_NUMBER,

            #if WEBUSB_BOS_DESC_LENGTH
            WEBUSB_BOS_DESC_LENGTH,
            0x10,
            0x05,
            0x00,
            0x38, 0xB6, 0x08, 0x34,
            0xA9, 0x09, 0xA0, 0x47,
            0x8B, 0xFD, 0xA0, 0x76,
            0x88, 0x15, 0xB6, 0x65,
            USB_DESC_WORD(0x0100),
            0x21,
            0x00,
            #endif

            #if WINUSB_BOS_DESC_LENGTH
            WINUSB_BOS_DESC_LENGTH,
            0x10,
            0x05,
            0x00,
            0xDF, 0x60, 0xDD, 0xD8,
            0x89, 0x45, 0xC7, 0x4C,
            0x9C, 0xD2, 0x65, 0x9D,
            0x9E, 0x64, 0x8A, 0x9F,
            0x00, 0x00, 0x03, 0x06,
            USB_DESC_WORD(sizeof(__usrapp_usbd_vllinklite_const.usbd.winusb_desc)),
            0x20,
            0x00,
            #endif
        },
        .str_lanid              = {
            4,
            USB_DT_STRING,
            0x09,
            0x04,
        },
        .str_vendor             = {
            8,
            USB_DT_STRING,
            'A', 0, 'R', 0, 'M', 0,
        },
        .str_product            = {
            36,
            USB_DT_STRING,
            'D', 0, 'A', 0, 'P', 0, 'L', 0, 'i', 0, 'n', 0, 'k', 0, ' ', 0,
            'C', 0, 'M', 0, 'S', 0, 'I', 0,	'S', 0, '-', 0, 'D', 0, 'A', 0,
            'P', 0,
        },
        .str_cmsis_dap_v2            = {
            26,
            USB_DT_STRING,
            'C', 0, 'M', 0, 'S', 0, 'I', 0, 'S', 0, '-', 0, 'D', 0, 'A', 0,
            'P', 0, ' ', 0, 'v', 0, '2', 0,
        },
        .str_webusb            = {
            36,
            USB_DT_STRING,
            'W', 0, 'e', 0, 'b', 0, 'U', 0, 'S', 0, 'B', 0, ':', 0,	' ', 0,
            'C', 0, 'M', 0, 'S', 0, 'I', 0, 'S', 0, '-', 0, 'D', 0, 'A', 0,
            'P', 0,
        },
        .str_cdcext            = {
            28,
            USB_DT_STRING,
            'V', 0, 'l', 0, 'l', 0, 'i', 0, 'n', 0, 'k', 0, '-', 0, 'C', 0,
            'D', 0, 'C', 0, 'E', 0, 'x', 0, 't', 0,
        },
        #ifdef APP_CFG_CDCSHELL_SUPPORT
        .str_cdcshell            = {
            32,
            USB_DT_STRING,
           'V', 0, 'l', 0, 'l', 0, 'i', 0, 'n', 0, 'k', 0, '-', 0, 'C', 0,
           'D', 0, 'C', 0, 'S', 0, 'h', 0, 'e', 0, 'l', 0, 'l', 0,
        },
        #endif
        .std_desc               = {
            VSF_USBD_DESC_DEVICE(__usrapp_usbd_vllinklite_const.usbd.dev_desc, sizeof(__usrapp_usbd_vllinklite_const.usbd.dev_desc)),
            VSF_USBD_DESC_CONFIG(0, __usrapp_usbd_vllinklite_const.usbd.config_desc, sizeof(__usrapp_usbd_vllinklite_const.usbd.config_desc)),
            {USB_DT_BOS, 0, 0, sizeof(__usrapp_usbd_vllinklite_const.usbd.bos_desc), (uint8_t*)__usrapp_usbd_vllinklite_const.usbd.bos_desc},
            VSF_USBD_DESC_STRING(0, 0, __usrapp_usbd_vllinklite_const.usbd.str_lanid, sizeof(__usrapp_usbd_vllinklite_const.usbd.str_lanid)),
            VSF_USBD_DESC_STRING(0x0409, 1, __usrapp_usbd_vllinklite_const.usbd.str_vendor, sizeof(__usrapp_usbd_vllinklite_const.usbd.str_vendor)),
            VSF_USBD_DESC_STRING(0x0409, 2, __usrapp_usbd_vllinklite_const.usbd.str_product, sizeof(__usrapp_usbd_vllinklite_const.usbd.str_product)),
            VSF_USBD_DESC_STRING(0x0409, 3, __usrapp_usbd_vllinklite_nonconst.usbd.str_serial, sizeof(__usrapp_usbd_vllinklite_nonconst.usbd.str_serial)),
            #ifdef APP_CFG_CMSIS_DAP_V2_SUPPORT
            VSF_USBD_DESC_STRING(0x0000, 4, __usrapp_usbd_vllinklite_const.usbd.str_cmsis_dap_v2, sizeof(__usrapp_usbd_vllinklite_const.usbd.str_cmsis_dap_v2)),
            VSF_USBD_DESC_STRING(0x0409, 4, __usrapp_usbd_vllinklite_const.usbd.str_cmsis_dap_v2, sizeof(__usrapp_usbd_vllinklite_const.usbd.str_cmsis_dap_v2)),
            #endif
            #ifdef APP_CFG_WEBUSB_SUPPORT
            VSF_USBD_DESC_STRING(0x0409, 5, __usrapp_usbd_vllinklite_const.usbd.str_webusb, sizeof(__usrapp_usbd_vllinklite_const.usbd.str_webusb)),
            #endif
            #ifdef APP_CFG_CDCEXT_SUPPORT
            VSF_USBD_DESC_STRING(0x0409, 6, __usrapp_usbd_vllinklite_const.usbd.str_cdcext, sizeof(__usrapp_usbd_vllinklite_const.usbd.str_cdcext)),
            #endif
            #ifdef APP_CFG_CDCSHELL_SUPPORT
            VSF_USBD_DESC_STRING(0x0409, 7, __usrapp_usbd_vllinklite_const.usbd.str_cdcshell, sizeof(__usrapp_usbd_vllinklite_const.usbd.str_cdcshell)),
            #endif
        },
    },
};

usbd_vllinklite_t __usrapp_usbd_vllinklite = {
    .usbd                       = {
        .dev.speed              = USB_DC_SPEED_HIGH,
        .dev.num_of_config      = dimof(__usrapp_usbd_vllinklite.usbd.config),
        .dev.num_of_desc        = dimof(__usrapp_usbd_vllinklite_const.usbd.std_desc),
        .dev.desc               = (vk_usbd_desc_t *)__usrapp_usbd_vllinklite_const.usbd.std_desc,
        .dev.config             = __usrapp_usbd_vllinklite.usbd.config,
        .dev.drv                = &VSF_USB_DC0,
        
        .config[0].num_of_ifs   = dimof(__usrapp_usbd_vllinklite.usbd.ifs),
        .config[0].ifs          = __usrapp_usbd_vllinklite.usbd.ifs,

        #ifdef APP_CFG_CMSIS_DAP_V2_SUPPORT
        .ifs[0].class_op        = &vk_usbd_cmsis_dap_v2_class,
        .ifs[0].class_param     = &__usrapp_usbd_vllinklite.usbd.cmsis_dap_v2,
        #endif
        #ifdef APP_CFG_WEBUSB_SUPPORT
        .ifs[0 + CMSIS_DAP_V2_INTERFACE_COUNT].\
                class_op        = &vk_usbd_webusb_class,
        .ifs[0 + CMSIS_DAP_V2_INTERFACE_COUNT].\
                class_param     = &__usrapp_usbd_vllinklite.usbd.webusb,
        #endif
        #ifdef APP_CFG_CDCEXT_SUPPORT
        .ifs[0 + CMSIS_DAP_V2_INTERFACE_COUNT + WEBUSB_INTERFACE_COUNT].\
                class_op        = &vk_usbd_cdcacm_data,
        .ifs[0 + CMSIS_DAP_V2_INTERFACE_COUNT + WEBUSB_INTERFACE_COUNT].\
                class_param     = &__usrapp_usbd_vllinklite.usbd.cdcext.param,
        .ifs[1 + CMSIS_DAP_V2_INTERFACE_COUNT + WEBUSB_INTERFACE_COUNT].\
                class_op        = &vk_usbd_cdcacm_control,
        .ifs[1 + CMSIS_DAP_V2_INTERFACE_COUNT + WEBUSB_INTERFACE_COUNT].\
                class_param     = &__usrapp_usbd_vllinklite.usbd.cdcext.param,
        #endif
        #ifdef APP_CFG_CDCSHELL_SUPPORT
        .ifs[0 + CMSIS_DAP_V2_INTERFACE_COUNT + WEBUSB_INTERFACE_COUNT + CDCEXT_INTERFACE_COUNT].\
                class_op        = &vk_usbd_cdcacm_data,
        .ifs[0 + CMSIS_DAP_V2_INTERFACE_COUNT + WEBUSB_INTERFACE_COUNT + CDCEXT_INTERFACE_COUNT].\
                class_param     = &__usrapp_usbd_vllinklite.usbd.cdcshell.param,
        .ifs[1 + CMSIS_DAP_V2_INTERFACE_COUNT + WEBUSB_INTERFACE_COUNT + CDCEXT_INTERFACE_COUNT].\
                class_op        = &vk_usbd_cdcacm_control,
        .ifs[1 + CMSIS_DAP_V2_INTERFACE_COUNT + WEBUSB_INTERFACE_COUNT + CDCEXT_INTERFACE_COUNT].\
                class_param     = &__usrapp_usbd_vllinklite.usbd.cdcshell.param,
        #endif

        #ifdef APP_CFG_CMSIS_DAP_V2_SUPPORT
        .cmsis_dap_v2           = {
            .ep_out             = 1,
            .ep_in              = 1,
            .dap                = &usrapp.dap,
        },
        #endif
        #ifdef APP_CFG_WEBUSB_SUPPORT
        .webusb                 = {
            .dap                = &usrapp.dap,
        },
        #endif
        #ifdef APP_CFG_CDCEXT_SUPPORT
        .cdcext.param           = {
            .ep                 = {
                .notify         = APP_CFG_CDCEXT_NOTIFY_EP,        // wake ep
                .out            = APP_CFG_CDCEXT_DATA_OUT_EP,
                .in             = APP_CFG_CDCEXT_DATA_IN_EP,
            },
            .line_coding        = {
                .bitrate        = PERIPHERAL_UART_BAUD_DEFAULT,
                .stop           = USB_CDC_ACM_STOPBIT_1,
                .parity         = USB_CDC_ACM_PARITY_NONE,
                .datalen        = 8,
            },
            .callback.set_line_coding   = usrapp_cdcext_set_line_coding,
            .callback.set_control_line  = usrapp_cdcext_set_control_line,
            .stream.tx.stream   = (vsf_stream_t *)&__usrapp_usbd_vllinklite.usbd.cdcext.ext2usb,
            .stream.rx.stream   = (vsf_stream_t *)&__usrapp_usbd_vllinklite.usbd.cdcext.usb2ext,
        },
        .cdcext.ext2usb         = {
            .op                 = &vsf_block_stream_op,
            .buffer             = __usrapp_usbd_vllinklite.usbd.cdcext.ext2usb_buf,
            .size               = sizeof(__usrapp_usbd_vllinklite.usbd.cdcext.ext2usb_buf),
            .block_size         = APP_CFG_CDCEXT_PKT_SIZE,
        },
        .cdcext.usb2ext         = {
            .op                 = &vsf_block_stream_op,
            .buffer             = __usrapp_usbd_vllinklite.usbd.cdcext.usb2ext_buf,
            .size               = sizeof(__usrapp_usbd_vllinklite.usbd.cdcext.usb2ext_buf),
            .block_size         = APP_CFG_CDCEXT_PKT_SIZE,
        },
        #endif
        #ifdef APP_CFG_CDCSHELL_SUPPORT
        .cdcshell.param         = {
            .ep                 = {
                .notify         = APP_CFG_CDCSHELL_NOTIFY_EP,        // wake ep
                .out            = APP_CFG_CDCSHELL_DATA_OUT_EP,
                .in             = APP_CFG_CDCSHELL_DATA_IN_EP,
            },
            .line_coding        = {
                .bitrate        = PERIPHERAL_UART_BAUD_DEFAULT,
                .stop           = USB_CDC_ACM_STOPBIT_1,
                .parity         = USB_CDC_ACM_PARITY_NONE,
                .datalen        = 8,
            },
            .callback.set_line_coding   = usrapp_cdcshell_set_line_coding,
            .stream.tx.stream   = (vsf_stream_t *)&__usrapp_usbd_vllinklite.usbd.cdcshell.shell2usb,
            .stream.rx.stream   = (vsf_stream_t *)&__usrapp_usbd_vllinklite.usbd.cdcshell.usb2shell,
        },
        .cdcshell.shell2usb     = {
            .op                 = &vsf_block_stream_op,
            .buffer             = __usrapp_usbd_vllinklite.usbd.cdcshell.shell2usb_buf,
            .size               = sizeof(__usrapp_usbd_vllinklite.usbd.cdcshell.shell2usb_buf),
            .block_size         = APP_CFG_CDCSHELL_PKT_SIZE,
        },
        .cdcshell.usb2shell     = {
            .op                 = &vsf_block_stream_op,
            .buffer             = __usrapp_usbd_vllinklite.usbd.cdcshell.usb2shell_buf,
            .size               = sizeof(__usrapp_usbd_vllinklite.usbd.cdcshell.usb2shell_buf),
            .block_size         = APP_CFG_CDCSHELL_PKT_SIZE,
        },
        #endif
    },
};

vsf_err_t vsf_usbd_vendor_prepare(vk_usbd_dev_t *dev)
{
    vk_usbd_ctrl_handler_t *ctrl_handler = &dev->ctrl_handler;
    struct usb_ctrlrequest_t *request = &ctrl_handler->request;

    if (request->bRequest == 0x20) {    // USBD_WINUSB_VENDOR_CODE
        if (request->wIndex == 0x07) {	// WINUSB_REQUEST_GET_DESCRIPTOR_SET
            ctrl_handler->trans.buffer = (uint8_t *)__usrapp_usbd_vllinklite_const.usbd.winusb_desc;
            ctrl_handler->trans.size = sizeof(__usrapp_usbd_vllinklite_const.usbd.winusb_desc);
            return VSF_ERR_NONE;
        }
    }
    return VSF_ERR_FAIL;
}

// double tx ep fifo size
uint_fast16_t vsf_dwcotg_dcd_get_fifo_size(uint_fast8_t ep, usb_ep_type_t type, uint_fast16_t size)
{
    return size * 2;
}
