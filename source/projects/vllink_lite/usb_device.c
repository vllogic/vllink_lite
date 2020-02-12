/***************************************************************************
 *   Copyright (C) 2018 - 2020 by Chen Le <talpachen@gmail.com>            *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

/*============================ INCLUDES ======================================*/

#include "vsf.h"
#include "vsf_private.h"

#if VSF_USE_USB_DEVICE == ENABLED

/*============================ MACROS ========================================*/

#define USRAPP_CFG_USBD_INTERFACE_NUM       (1 + 1 + 2 * 2)

#define WINUSB_BOS_DESC_LENGTH              24
#define WEBUSB_BOS_DESC_LENGTH              28
#define BOS_DESC_LENGTH		                (5 + WINUSB_BOS_DESC_LENGTH + WEBUSB_BOS_DESC_LENGTH)

#define FUNCTION_WEBUSB_SUBSET_LEN			160
#define FUNCTION_CMSIS_DAP_V2_SUBSET_LEN    160
#define WINUSB_DESC_LENGTH	                (10 + FUNCTION_WEBUSB_SUBSET_LEN + FUNCTION_CMSIS_DAP_V2_SUBSET_LEN)

#define VSF_USBD_DESC_BOS(__LANID, __INDEX, __DESC, __SIZE)                  \
    {USB_DT_BOS, (__INDEX), (__LANID), (__SIZE), (uint8_t*)(__DESC)}

#if USRAPP_CFG_USBD_SPEED == USB_SPEED_HIGH
#   define USRAPP_DESC_CDC_UART_IAD          USB_DESC_CDC_UART_HS_IAD
#elif USRAPP_CFG_USBD_SPEED == USB_SPEED_FULL
#   define USRAPP_DESC_CDC_UART_IAD          USB_DESC_CDC_UART_FS_IAD
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/

#define USRAPP_CDC_INSTANCE(__NAME, __CDC, __CTRL, __DATA, __NOTIFY, __OUT, __IN) \
    .__NAME                     = {                                             \
        .param                  = {                                             \
            .ep                 = {                                             \
                .notify         = __NOTIFY,                                     \
                .out            = __OUT,                                        \
                .in             = __IN,                                         \
            },                                                                  \
            .line_coding        = {                                             \
                .bitrate        = 115200,                                       \
                .stop           = 0,                                            \
                .parity         = 0,                                            \
                .datalen        = 8,                                            \
            },                                                                  \
            .stream.tx.stream   = (vsf_stream_t *)&__CDC.stream.tx,             \
            .stream.rx.stream   = (vsf_stream_t *)&__CDC.stream.rx,             \
        },                                                                      \
        .stream                 = {                                             \
            .tx.op              = &vsf_mem_stream_op,                           \
            .tx.pchBuffer       = (uint8_t *)&__CDC.stream.tx_buffer,           \
            .tx.nSize           = sizeof(__CDC.stream.tx_buffer),               \
            .tx.align           = USRAPP_CFG_STREAM_ALIGN,                      \
            .rx.op              = &vsf_mem_stream_op,                           \
            .rx.pchBuffer       = (uint8_t *)&__CDC.stream.rx_buffer,           \
            .rx.nSize           = sizeof(__CDC.stream.rx_buffer),               \
            .rx.align           = USRAPP_CFG_STREAM_ALIGN,                      \
        },                                                                      \
    },                                                                          \
    .ifs[__CTRL].class_op       = &vk_usbd_cdcacm_control,                      \
    .ifs[__CTRL].class_param    = &__CDC.param,                                 \
    .ifs[__DATA].class_op       = &vk_usbd_cdcacm_data,                         \
    .ifs[__DATA].class_param    = &__CDC.param,

/*============================ TYPES =========================================*/

#if VSF_USE_USB_DEVICE
struct usrapp_usb_device_const_t {
    struct {
#if     USRAPP_CFG_DCD_TYPE == USRAPP_CFG_DCD_TYPE_DWCOTG                       \
    ||  USRAPP_CFG_DCD_TYPE == USRAPP_CFG_DCD_TYPE_MUSB_FDRC
        struct {
#   if USRAPP_CFG_DCD_TYPE == USRAPP_CFG_DCD_TYPE_DWCOTG
            vk_dwcotg_dcd_param_t dwcotg_param;
#   elif USRAPP_CFG_DCD_TYPE == USRAPP_CFG_DCD_TYPE_MUSB_FDRC
            vk_musb_fdrc_dcd_param_t musb_fdrc_param;
#   endif
        } dcd;
#endif

        uint8_t dev_desc[18];
        uint8_t config_desc[9 + USB_DESC_CMSIS_DAP_V2_IAD_LEN + USB_DESC_WEBUSB_IAD_LEN + USB_DESC_CDC_ACM_IAD_LEN * 2];
        uint8_t bos_desc[BOS_DESC_LENGTH];
        uint8_t winusb_desc[WINUSB_DESC_LENGTH];
        uint8_t str_lanid[4];
        uint8_t str_vendor[8];
        uint8_t str_product[36];
        //uint8_t str_serial[50];
        uint8_t str_cmsis_dap_v2[26];
        uint8_t str_webusb[36];
        uint8_t str_cdcuart[28];
        uint8_t str_cdcshell[32];
        vk_usbd_desc_t std_desc[12];
    } usbd;
};
typedef struct usrapp_usb_device_const_t usrapp_usb_device_const_t;

struct usrapp_usb_device_unconst_t {
    uint8_t str_serial[50];
};
typedef struct usrapp_usb_device_unconst_t usrapp_usb_device_unconst_t;

struct usrapp_usb_usb_device_t {
    struct {
#if     USRAPP_CFG_DCD_TYPE == USRAPP_CFG_DCD_TYPE_DWCOTG                       \
    ||  USRAPP_CFG_DCD_TYPE == USRAPP_CFG_DCD_TYPE_MUSB_FDRC
        struct {
#   if USRAPP_CFG_DCD_TYPE == USRAPP_CFG_DCD_TYPE_DWCOTG
            vk_dwcotg_dcd_t dwcotg_dcd;
#   elif USRAPP_CFG_DCD_TYPE == USRAPP_CFG_DCD_TYPE_MUSB_FDRC
            vk_musb_fdrc_dcd_t musb_fdrc_dcd;
#   endif
        } dcd;
#endif

        struct {
            vk_usbd_cmsis_dap_v2_t param;
        } cmsis_dap_v2;

        struct {
            vk_usbd_webusb_t param;
        } webusb;

        struct {
            vk_usbd_cdcacm_t param;
            struct {
                vsf_mem_stream_t tx;
                vsf_mem_stream_t rx;
                uint8_t tx_buffer[USRAPP_CFG_CDCUART_TX_STREAM_SIZE];
                uint8_t rx_buffer[USRAPP_CFG_CDCUART_RX_STREAM_SIZE];
            } stream;
        } cdc_uart;

        struct {
            vk_usbd_cdcacm_t param;
            struct {
                vsf_mem_stream_t tx;
                vsf_mem_stream_t rx;
                uint8_t tx_buffer[USRAPP_CFG_CDCSHELL_TX_STREAM_SIZE];
                uint8_t rx_buffer[USRAPP_CFG_CDCSHELL_RX_STREAM_SIZE];
            } stream;
        } cdc_shell;

        vk_usbd_ifs_t ifs[USRAPP_CFG_USBD_INTERFACE_NUM];
        vk_usbd_cfg_t config[1];
        vk_usbd_dev_t dev;

        vsf_callback_timer_t connect_timer;
    } usbd;
};
typedef struct usrapp_usb_usb_device_t usrapp_usb_usb_device_t;
#endif

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
static usrapp_usb_device_unconst_t __usrapp_usb_device_unconst;
static const usrapp_usb_device_const_t __usrapp_usb_device_const = {
    .usbd                       = {
#if     USRAPP_CFG_DCD_TYPE == USRAPP_CFG_DCD_TYPE_DWCOTG                       \
    ||  USRAPP_CFG_DCD_TYPE == USRAPP_CFG_DCD_TYPE_MUSB_FDRC
        .dcd                    = {
#   if USRAPP_CFG_DCD_TYPE == USRAPP_CFG_DCD_TYPE_DWCOTG
            .dwcotg_param       = {
                .op             = &VSF_USB_DC0_IP,
                .speed          = USRAPP_CFG_USBD_SPEED,
            },
#   elif USRAPP_CFG_DCD_TYPE == USRAPP_CFG_DCD_TYPE_MUSB_FDRC
            musb_fdrc_param     = {
                .op             = &VSF_USB_DC0_IP,
            },
#   endif
        },
#endif
        .dev_desc               = {
            USB_DT_DEVICE_SIZE,                                                 \
            USB_DT_DEVICE,                                                      \
            0x10, 0x02,                         /* bcdUSB */                    \
            0xEF,                               /* device class: IAD */         \
            0x02,                               /* device sub class */          \
            0x01,                               /* device protocol */           \
            64,                                 /* max packet size */           \
            USB_DESC_WORD(APPCFG_USBD_VID),     /* vendor */                    \
            USB_DESC_WORD(APPCFG_USBD_PID),     /* product */                   \
            USB_DESC_WORD(APPCFG_USBD_BCD),     /* bcdDevice */                 \
            1,                                  /* manufacturer */              \
            2,                                  /* product */                   \
            3,                                  /* serial number */             \
            1,                                  /* number of configuration */
        },
        .config_desc            = {
            USB_DESC_CFG(sizeof(__usrapp_usb_device_const.usbd.config_desc), USRAPP_CFG_USBD_INTERFACE_NUM, 1, 0, 0x80, 250)

            /* CMSIS-DAP V2, Length: 23 */ 
            0x09,        //   bLength
            0x04,        //   bDescriptorType (Interface)
            0x00,        //   bInterfaceNumber 0
            0x00,        //   bAlternateSetting
            0x02,        //   bNumEndpoints 2
            0xFF,        //   bInterfaceClass
            0x00,        //   bInterfaceSubClass
            0x00,        //   bInterfaceProtocol
            0x04,        //   iInterface (String Index)

            0x07,        //   bLength
            0x05,        //   bDescriptorType (Endpoint)
            0x01,        //   bEndpointAddress (OUT/H2D)
            0x02,        //   bmAttributes (Bulk)
            0x40, 0x00,  //   wMaxPacketSize 64
            0x00,        //   bInterval 0 (unit depends on device speed)

            0x07,        //   bLength
            0x05,        //   bDescriptorType (Endpoint)
            0x81,        //   bEndpointAddress (IN/D2H)
            0x02,        //   bmAttributes (Bulk)
            0x40, 0x00,  //   wMaxPacketSize 64
            0x00,        //   bInterval 0 (unit depends on device speed)
            
            /* WEBUSB, Length: 9 */ 
            0x09,        //   bLength
            0x04,        //   bDescriptorType (Interface)
            0x01,        //   bInterfaceNumber 1
            0x00,        //   bAlternateSetting
            0x00,        //   bNumEndpoints 0
            0xFF,        //   bInterfaceClass
            0x03,        //   bInterfaceSubClass
            0x00,        //   bInterfaceProtocol
            0x05,        //   iInterface (String Index)

            USRAPP_DESC_CDC_UART_IAD(2, 6, 4, 2, 2)
            USRAPP_DESC_CDC_UART_IAD(4, 7, 5, 3, 3)
        },
        .bos_desc              = {
            0x05,
            0x0F,
            (BOS_DESC_LENGTH >> 0) & 0xFF,
            (BOS_DESC_LENGTH >> 8) & 0xFF,
            2,
            
            WEBUSB_BOS_DESC_LENGTH,
            0x10,
            0x05,
            0x00,
            0x38, 0xB6, 0x08, 0x34,
            0xA9, 0x09, 0xA0, 0x47,
            0x8B, 0xFD, 0xA0, 0x76,
            0x88, 0x15, 0xB6, 0x65,
            (0x0100 >> 0) & 0xFF,
            (0x0100 >> 8) & 0xFF,
            0x21,
            0x00,
            
            WINUSB_BOS_DESC_LENGTH,
            0x10,
            0x05,
            0x00,
            0xDF, 0x60, 0xDD, 0xD8,
            0x89, 0x45, 0xC7, 0x4C,
            0x9C, 0xD2, 0x65, 0x9D,
            0x9E, 0x64, 0x8A, 0x9F,
            0x00, 0x00, 0x03, 0x06,
            (sizeof(__usrapp_usb_device_const.usbd.winusb_desc) >> 0) & 0xFF,
            (sizeof(__usrapp_usb_device_const.usbd.winusb_desc) >> 8) & 0xFF,
            0x20,
            0x00,
        },
        .winusb_desc            = {
            (10 >> 0) & 0xFF,
            (10 >> 8) & 0xFF,
            0x00, 0x00,
            0x00, 0x00, 0x03, 0x06,
            (WINUSB_DESC_LENGTH >> 0) & 0xFF,
            (WINUSB_DESC_LENGTH >> 8) & 0xFF,

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
        },
        .str_lanid              = {
            USB_DESC_STRING(2, 0x09, 0x04)
        },
        .str_vendor             = {
            USB_DESC_STRING(6,
                'A', 0, 'R', 0, 'M', 0
            )
        },
        .str_product            = {
            USB_DESC_STRING(34,
                'D', 0, 'A', 0, 'P', 0, 'L', 0, 'i', 0, 'n', 0, 'k', 0, ' ', 0,
                'C', 0, 'M', 0, 'S', 0, 'I', 0,	'S', 0, '-', 0, 'D', 0, 'A', 0,
                'P', 0
            )
        },
        .str_cmsis_dap_v2       = {
            USB_DESC_STRING(24,
                'C', 0, 'M', 0, 'S', 0, 'I', 0, 'S', 0, '-', 0, 'D', 0, 'A', 0,
                'P', 0, ' ', 0, 'v', 0, '2', 0
            )
        },
        .str_webusb             = {
            USB_DESC_STRING(34,
                'W', 0, 'e', 0, 'b', 0, 'U', 0, 'S', 0, 'B', 0, ':', 0,	' ', 0,
                'C', 0, 'M', 0, 'S', 0, 'I', 0, 'S', 0, '-', 0, 'D', 0, 'A', 0,
                'P', 0
            )
        },
        .str_cdcuart            = {
            USB_DESC_STRING(26,
                'V', 0, 'l', 0, 'l', 0, 'i', 0, 'n', 0, 'k', 0, '-', 0, 'C', 0,
                'D', 0, 'C', 0, 'E', 0, 'x', 0, 't', 0
            )
        },
        .str_cdcshell           = {
            USB_DESC_STRING(30,
                'V', 0, 'l', 0, 'l', 0, 'i', 0, 'n', 0, 'k', 0, '-', 0, 'C', 0,
                'D', 0, 'C', 0, 'S', 0, 'h', 0, 'e', 0, 'l', 0, 'l', 0
            )
        },
        .std_desc               = {
            VSF_USBD_DESC_DEVICE(0, __usrapp_usb_device_const.usbd.dev_desc, sizeof(__usrapp_usb_device_const.usbd.dev_desc)),
            VSF_USBD_DESC_CONFIG(0, 0, __usrapp_usb_device_const.usbd.config_desc, sizeof(__usrapp_usb_device_const.usbd.config_desc)),
            VSF_USBD_DESC_BOS(0, 0, __usrapp_usb_device_const.usbd.bos_desc, sizeof(__usrapp_usb_device_const.usbd.bos_desc)),
            VSF_USBD_DESC_STRING(0, 0, __usrapp_usb_device_const.usbd.str_lanid, sizeof(__usrapp_usb_device_const.usbd.str_lanid)),
            VSF_USBD_DESC_STRING(0x0409, 1, __usrapp_usb_device_const.usbd.str_vendor, sizeof(__usrapp_usb_device_const.usbd.str_vendor)),
            VSF_USBD_DESC_STRING(0x0409, 2, __usrapp_usb_device_const.usbd.str_product, sizeof(__usrapp_usb_device_const.usbd.str_product)),
            VSF_USBD_DESC_STRING(0x0409, 3, __usrapp_usb_device_unconst.str_serial, sizeof(__usrapp_usb_device_unconst.str_serial)),
            VSF_USBD_DESC_STRING(0x0000, 4, __usrapp_usb_device_const.usbd.str_cmsis_dap_v2, sizeof(__usrapp_usb_device_const.usbd.str_cmsis_dap_v2)),
            VSF_USBD_DESC_STRING(0x0409, 4, __usrapp_usb_device_const.usbd.str_cmsis_dap_v2, sizeof(__usrapp_usb_device_const.usbd.str_cmsis_dap_v2)),
            VSF_USBD_DESC_STRING(0x0409, 5, __usrapp_usb_device_const.usbd.str_webusb, sizeof(__usrapp_usb_device_const.usbd.str_webusb)),
            VSF_USBD_DESC_STRING(0x0409, 6, __usrapp_usb_device_const.usbd.str_cdcuart, sizeof(__usrapp_usb_device_const.usbd.str_cdcuart)),
            VSF_USBD_DESC_STRING(0x0409, 7, __usrapp_usb_device_const.usbd.str_cdcshell, sizeof(__usrapp_usb_device_const.usbd.str_cdcshell)),
        },
    },
};

static usrapp_usb_usb_device_t __usrapp_usb_device;
#if USRAPP_CFG_DCD_TYPE == USRAPP_CFG_DCD_TYPE_DWCOTG
VSF_USB_DC_FROM_DWCOTG_IP(0, __usrapp_usb_device.usbd.dcd.dwcotg_dcd, VSF_USB_DC0)
#elif USRAPP_CFG_DCD_TYPE == USRAPP_CFG_DCD_TYPE_MUSB_FDRC
VSF_USB_DC_FROM_MUSB_FDRC_IP(0, __usrapp_usb_device.usbd.dcd.musb_fdrc_dcd, VSF_USB_DC0)
#endif

static usrapp_usb_usb_device_t __usrapp_usb_device = {
    .usbd                       = {
#if     USRAPP_CFG_DCD_TYPE == USRAPP_CFG_DCD_TYPE_DWCOTG                       \
    ||  USRAPP_CFG_DCD_TYPE == USRAPP_CFG_DCD_TYPE_MUSB_FDRC
        .dcd                    = {
#   if USRAPP_CFG_DCD_TYPE == USRAPP_CFG_DCD_TYPE_DWCOTG
            .dwcotg_dcd.param   = &__usrapp_usb_device_const.usbd.dcd.dwcotg_param,
#   elif USRAPP_CFG_DCD_TYPE == USRAPP_CFG_DCD_TYPE_MUSB_FDRC
            .musb_fdrc_dcd.param= &__usrapp_usb_device_const.usbd.musb_fdrc_param,
#   endif
        },
#endif

        // .cmsis_dap_v2 =
        .ifs[0].class_op        = &vk_usbd_cmsis_dap_v2,
        .ifs[0].class_param     = &__usrapp_usb_device.usbd.cmsis_dap_v2.param,

        // .webusb = 
        .ifs[1].class_op        = &vk_usbd_webusb,
        .ifs[1].class_param     = &__usrapp_usb_device.usbd.webusb.param,

        USRAPP_CDC_INSTANCE(cdc_uart, __usrapp_usb_device.usbd.cdc_uart, 2, 3, 4, 2, 2)
        USRAPP_CDC_INSTANCE(cdc_shell, __usrapp_usb_device.usbd.cdc_shell, 4, 5, 5, 3, 3)

        .config[0].num_of_ifs   = dimof(__usrapp_usb_device.usbd.ifs),
        .config[0].ifs          = __usrapp_usb_device.usbd.ifs,
        .dev.num_of_config      = dimof(__usrapp_usb_device.usbd.config),
        .dev.config             = __usrapp_usb_device.usbd.config,
        .dev.num_of_desc        = dimof(__usrapp_usb_device_const.usbd.std_desc),
        .dev.desc               = (vk_usbd_desc_t *)__usrapp_usb_device_const.usbd.std_desc,

#if USRAPP_CFG_USBD_SPEED == USB_SPEED_HIGH
        .dev.speed              = USB_DC_SPEED_HIGH,
#elif USRAPP_CFG_USBD_SPEED == USB_SPEED_FULL
        .dev.speed              = USB_DC_SPEED_FULL,
#endif
        .dev.drv                = &VSF_USB_DC0,//&VSF_USB.DC[0],
    },
};

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

static void __usrapp_usbd_on_timer(vsf_callback_timer_t *timer)
{
    vk_usbd_connect(&__usrapp_usb_device.usbd.dev);
}

void usrapp_usbd_init(void)
{
    vsf_stream_init(&__usrapp_usb_device.usbd.cdc_uart.stream.tx.use_as__vsf_stream_t);
    vsf_stream_init(&__usrapp_usb_device.usbd.cdc_uart.stream.rx.use_as__vsf_stream_t);
    vsf_stream_init(&__usrapp_usb_device.usbd.cdc_shell.stream.tx.use_as__vsf_stream_t);
    vsf_stream_init(&__usrapp_usb_device.usbd.cdc_shell.stream.rx.use_as__vsf_stream_t);

    vk_usbd_init(&__usrapp_usb_device.usbd.dev);
    vk_usbd_disconnect(&__usrapp_usb_device.usbd.dev);
    __usrapp_usb_device.usbd.connect_timer.on_timer = __usrapp_usbd_on_timer;
    vsf_callback_timer_add_ms(&__usrapp_usb_device.usbd.connect_timer, 200);
}

void usrapp_usbd_cdcuart_get_stream(vsf_stream_t **stream_tx, vsf_stream_t **stream_rx)
{
    if(stream_tx != NULL) {
            *stream_tx = &__usrapp_usb_device.usbd.cdc_uart.stream.tx.use_as__vsf_stream_t;
    }
    if(stream_rx != NULL) {
        *stream_rx = &__usrapp_usb_device.usbd.cdc_uart.stream.rx.use_as__vsf_stream_t;
    }
}

void usrapp_usbd_cdcshell_get_stream(vsf_stream_t **stream_tx, vsf_stream_t **stream_rx)
{
    if(stream_tx != NULL) {
            *stream_tx = &__usrapp_usb_device.usbd.cdc_shell.stream.tx.use_as__vsf_stream_t;
    }
    if(stream_rx != NULL) {
        *stream_rx = &__usrapp_usb_device.usbd.cdc_shell.stream.rx.use_as__vsf_stream_t;
    }
}

#endif      // VSF_USE_USB_DEVICE

/* EOF */
