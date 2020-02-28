#include "vsf_usbd_DFU.h"

struct usrapp_usbd_common_const_t {
#if VSF_USE_USB_DEVICE_DCD_USBIP == ENABLED
    vk_usbip_dcd_param_t usbip_dcd_param;
#endif
#if VSF_USE_USB_DEVICE_DCD_MUSB_FDRC == ENABLED
    vk_musb_fdrc_dcd_param_t musb_fdrc_dcd_param;
#endif
#if VSF_USE_USB_DEVICE_DCD_DWCOTG == ENABLED
    vk_dwcotg_dcd_param_t dwcotg_dcd_param;
#endif
};
typedef struct usrapp_usbd_common_const_t usrapp_usbd_common_const_t;

struct usrapp_usbd_common_t {
#if VSF_USE_USB_DEVICE_DCD_USBIP == ENABLED
    vk_usbip_dcd_t usbip_dcd;
#endif
#if VSF_USE_USB_DEVICE_DCD_MUSB_FDRC == ENABLED
    vk_musb_fdrc_dcd_t musb_fdrc_dcd;
#endif
#if VSF_USE_USB_DEVICE_DCD_DWCOTG == ENABLED
    vk_dwcotg_dcd_t dwcotg_dcd;
#endif
};
typedef struct usrapp_usbd_common_t usrapp_usbd_common_t;

const usrapp_usbd_common_const_t usrapp_usbd_common_const = {
#if VSF_USE_USB_DEVICE_DCD_MUSB_FDRC == ENABLED
    .musb_fdrc_dcd_param    = {
        .op                 = &VSF_USB_DC0_IP,
    },
#endif
#if VSF_USE_USB_DEVICE_DCD_DWCOTG == ENABLED
    .dwcotg_dcd_param       = {
        .op                 = &VSF_USB_DC0_IP,
        .speed              = VSF_USBD_CFG_SPEED,
    },
#endif
};

usrapp_usbd_common_t usrapp_usbd_common = {
#if VSF_USE_USB_DEVICE_DCD_USBIP == ENABLED
    .usbip_dcd.param        = &usrapp_usbd_common_const.usbip_dcd_param,
#endif
#if VSF_USE_USB_DEVICE_DCD_MUSB_FDRC == ENABLED
    .musb_fdrc_dcd.param    = &usrapp_usbd_common_const.musb_fdrc_dcd_param,
#endif
#if VSF_USE_USB_DEVICE_DCD_DWCOTG == ENABLED
    .dwcotg_dcd.param       = &usrapp_usbd_common_const.dwcotg_dcd_param,
#endif
};

#if VSF_USE_USB_DEVICE_DCD_USBIP == ENABLED
VSF_USB_DC_FROM_USBIP_IP(0, usrapp_usbd_common.usbip_dcd, VSF_USB_DC0)
#elif VSF_USE_USB_DEVICE_DCD_MUSB_FDRC == ENABLED
VSF_USB_DC_FROM_MUSB_FDRC_IP(0, usrapp_usbd_common.musb_fdrc_dcd, VSF_USB_DC0)
#elif VSF_USE_USB_DEVICE_DCD_DWCOTG == ENABLED
VSF_USB_DC_FROM_DWCOTG_IP(0, usrapp_usbd_common.dwcotg_dcd, VSF_USB_DC0)
#endif

struct usrapp_usbd_unconst_t {
    uint8_t str_serial[50];
};
typedef struct usrapp_usbd_unconst_t usrapp_usbd_unconst_t;

struct usrapp_usbd_const_t {
    uint8_t dev_desc[18];
    uint8_t config_desc[9 + 9 + 9];
    uint8_t winusb_desc[40];
    uint8_t bos_desc[5 + 24];
    uint8_t webusb_url_desc[33];
    uint8_t str_lanid[4];
    uint8_t str_vendor[16];
    uint8_t str_product[32];
    //uint8_t str_serial[50];
    uint8_t str_dfu[8];
    uint8_t str_bos[18];
    vk_usbd_desc_t std_desc[9];
};
typedef struct usrapp_usbd_const_t usrapp_usbd_const_t;

struct usrapp_usbd_t {
    vk_usbd_dev_t dev;
    vk_usbd_cfg_t config[1];
    vk_usbd_ifs_t ifs[1];

    struct vk_usbd_dfu_t dfu;
};
typedef struct usrapp_usbd_t usrapp_usbd_t;

static usrapp_usbd_unconst_t usrapp_usbd_unconst = {
    .str_serial[0] = 50,
    .str_serial[1] = USB_DT_STRING,
};

static const usrapp_usbd_const_t usrapp_usbd_const = {
    .dev_desc = {
        USB_DT_DEVICE_SIZE,
        USB_DT_DEVICE,
        0x10, 0x02,     // bcdUSB
        0x00,           // device class: IAD
        0x00,           // device sub class
        0x00,           // device protocol
        64,             // max packet size
        USB_DESC_WORD(APP_CFG_USBD_VID),
                        // vendor
        USB_DESC_WORD(APP_CFG_USBD_PID),
                        // product
        USB_DESC_WORD(APP_CFG_USBD_BCD),
                        // bcdDevice
        1,              // manu facturer
        2,              // product
        3,              // serial number
        1,              // number of configuration
    },
    .config_desc = {
        USB_DT_CONFIG_SIZE,
        USB_DT_CONFIG,
        USB_DESC_WORD(sizeof(usrapp_usbd_const.config_desc)),
                        // wTotalLength
        0x01,           // bNumInterfaces: 1 interfaces
        0x01,           // bConfigurationValue: Configuration value
        0x00,           // iConfiguration: Index of string descriptor describing the configuration
        0xc0,           // bmAttributes: bus powered
        0x32,           // MaxPower

        // DFU
        USB_DT_INTERFACE_SIZE,
        USB_DT_INTERFACE,
        0x00,           // bInterfaceNumber: Number of Interface
        0x00,           // bAlternateSetting: Alternate setting
        0x00,           // bNumEndpoints
        0xfe,           // bInterfaceClass:
        0x01,           // bInterfaceSubClass:
        0x02,           // nInterfaceProtocol:
        0x04,           // iInterface:

        0x09, 
        0x21, 
        0x0b, 
        0xff, 
        0x00,
        0x00, 0x04,     // block size
        0x10,
        0x01,
    },
    .winusb_desc = {
        (40 >> 0) & 0xFF,
        (40 >> 8) & 0xFF,
        0x00, 0x00,
        0x00, 0x01,
        0x04, 0x00,
        0x1,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 

        0x0,
        0x1, 
        'W', 'I', 'N', 'U', 'S', 'B', 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
    },
    .bos_desc = {
        0x05,
        0x0F,
        USB_DESC_WORD(29),
        1,

        24,
        0x10,
        0x05,
        0x00,
        0x38, 0xB6, 0x08, 0x34,
        0xA9, 0x09, 0xA0, 0x47,
        0x8B, 0xFD, 0xA0, 0x76,
        0x88, 0x15, 0xB6, 0x65,
        (0x0100 >> 0) & 0xFF,
        (0x0100 >> 8) & 0xFF,
        0x01,
        0x01,
    },
    .webusb_url_desc = {
        3 + sizeof("vllogic.github.io/vllink_lite/") - 1,
        3,	// dt url
        1,	// https
        0x76, 0x6C, 0x6C, 0x6F, 0x67, 0x69, 0x63, 0x2E, 0x67, 0x69, 0x74, 0x68,
        0x75, 0x62, 0x2E, 0x69, 0x6F, 0x2F, 0x76, 0x6C, 0x6C, 0x69, 0x6E, 0x6B,
        0x5F, 0x6C, 0x69, 0x74, 0x65, 0x2F,
    },
    .str_lanid = {
        4,
        USB_DT_STRING,
        0x09,
        0x04,
    },
    .str_vendor = {
        16,
        USB_DT_STRING,
        'V', 0, 'l', 0, 'l', 0, 'o', 0, 'g', 0, 'i', 0, 'c', 0,
    },
    .str_product = {
        32,
        USB_DT_STRING,
        'V', 0, 'l', 0, 'l', 0, 'i', 0, 'n', 0, 'k', 0, ' ', 0, 'L', 0,
        'i', 0, 't', 0, 'e', 0, ' ', 0,	'D', 0, 'F', 0, 'U', 0,
    },
    .str_dfu = {
        8,
        USB_DT_STRING,
        'D', 0, 'F', 0, 'U', 0,
    },
    .str_bos = {
        18,
        USB_DT_STRING,
        'M', 0, 'S', 0, 'F', 0, 'T', 0, '1', 0, '0', 0, '0', 0, 0x21, 0,
    },
    .std_desc = {
        VSF_USBD_DESC_DEVICE(usrapp_usbd_const.dev_desc, sizeof(usrapp_usbd_const.dev_desc)),
        VSF_USBD_DESC_CONFIG(0, usrapp_usbd_const.config_desc, sizeof(usrapp_usbd_const.config_desc)),
        {USB_DT_BOS, 0, 0, (sizeof(usrapp_usbd_const.bos_desc)), ((uint8_t*)(usrapp_usbd_const.bos_desc))},
        VSF_USBD_DESC_STRING(0, 0, usrapp_usbd_const.str_lanid, sizeof(usrapp_usbd_const.str_lanid)),
        VSF_USBD_DESC_STRING(0x0409, 1, usrapp_usbd_const.str_vendor, sizeof(usrapp_usbd_const.str_vendor)),
        VSF_USBD_DESC_STRING(0x0409, 2, usrapp_usbd_const.str_product, sizeof(usrapp_usbd_const.str_product)),
        VSF_USBD_DESC_STRING(0x0409, 3, usrapp_usbd_unconst.str_serial, sizeof(usrapp_usbd_unconst.str_serial)),
        VSF_USBD_DESC_STRING(0x0409, 4, usrapp_usbd_const.str_dfu, sizeof(usrapp_usbd_const.str_dfu)),
        VSF_USBD_DESC_STRING(0x0000, 0xee, usrapp_usbd_const.str_bos, sizeof(usrapp_usbd_const.str_bos)),
    },
};

static usrapp_usbd_t usrapp_usbd = {
    .dev.drv                = &VSF_USB_DC0,
    .dev.speed              = USB_DC_SPEED_FULL,
    .dev.num_of_config      = dimof(usrapp_usbd.config),
    .dev.config             = usrapp_usbd.config,
    .dev.num_of_desc        = dimof(usrapp_usbd_const.std_desc),
    .dev.desc               = (vk_usbd_desc_t *)usrapp_usbd_const.std_desc,
    
    .config[0].num_of_ifs   = dimof(usrapp_usbd.ifs),
    .config[0].ifs          = usrapp_usbd.ifs,

    .ifs[0].class_op        = &vk_usbd_dfu_class,
    .ifs[0].class_param     = &usrapp_usbd.dfu,
};
