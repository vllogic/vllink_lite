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

describe_usbd(user_usbd, 0x2348, 0xA7A8, USB_DC_SPEED_FULL)
    usbd_common_desc(user_usbd, u"VSF-USBD-Simplest", u"SimonQian", u"1.0.0", 64, USB_DESC_CDC_ACM_IAD_LEN, USB_CDC_ACM_IFS_NUM, USB_CONFIG_ATT_WAKEUP, 100)
        cdc_acm_desc(user_usbd, 0, 0, 1, 2, 2, 512, 16)
    usbd_func_desc(user_usbd)
        usbd_func_str_desc(user_usbd, 0, u"VSF-CDC")
    usbd_std_desc_table(user_usbd)
        usbd_func_str_desc_table(user_usbd, 0)
    usbd_func(user_usbd)
        cdc_acm_func(user_usbd, 0, 1, 2, 2, NULL, NULL, USB_CDC_ACM_LINECODE(115200, 8, USB_CDC_ACM_PARITY_NONE, USB_CDC_ACM_STOPBIT_1))
    usbd_ifs(user_usbd)
        cdc_acm_ifs(user_usbd, 0)
end_describe_usbd(user_usbd, VSF_USB_DC0)