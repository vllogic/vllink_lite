#ifndef __VSF_USBD_WEBUSB_USR_H__
#define __VSF_USBD_WEBUSB_USR_H__

/*============================ INCLUDES ======================================*/

#include "component/usb/vsf_usb_cfg.h"
#include "dap.h"

#if VSF_USE_USB_DEVICE == ENABLED

#if     defined(__VSF_USBD_WEBUSB_USR_IMPLEMENT)
#   define __PLOOC_CLASS_IMPLEMENT__
#elif   defined(__VSF_USBD_WEBUSB_USR_INHERIT__)
#   define __PLOOC_CLASS_INHERIT__
#endif
#include "utilities/ooc_class.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

declare_simple_class(vk_usbd_webusb_t)

def_simple_class(vk_usbd_webusb_t) {

    public_member(
        dap_t *dap;
    )

    private_member(
        vsf_sem_t response_sem;

        uint8_t request_buff[WEBUSB_DAP_PACKET_SIZE];
        uint8_t response_buff[WEBUSB_DAP_PACKET_SIZE];

        vk_usbd_dev_t *dev;
    )
};

/*============================ GLOBAL VARIABLES ==============================*/

extern const vk_usbd_class_op_t vk_usbd_webusb_class;

/*============================ PROTOTYPES ====================================*/

#ifdef __cplusplus
}
#endif

#endif      // VSF_USE_USB_DEVICE
#endif      // __VSF_USBD_WEBUSB_TEMP_H__
