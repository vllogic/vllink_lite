#ifndef __VSF_USBD_CMSIS_DAP_V2_H__
#define __VSF_USBD_CMSIS_DAP_V2_H__

/*============================ INCLUDES ======================================*/

#include "component/usb/vsf_usb_cfg.h"
#include "dap/dap.h"

#if VSF_USE_USB_DEVICE == ENABLED

#if     defined(__VSF_USBD_CMSIS_DAP_V2_IMPLEMENT)
#   define __PLOOC_CLASS_IMPLEMENT__
#elif   defined(__VSF_USBD_CMSIS_DAP_V2_INHERIT__)
#   define __PLOOC_CLASS_INHERIT__
#endif
#include "utilities/ooc_class.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

declare_simple_class(vk_usbd_cmsis_dap_v2_t)

def_simple_class(vk_usbd_cmsis_dap_v2_t) {

    public_member(
        uint8_t ep_out;
        uint8_t ep_in;

        dap_t *dap;
    )

    private_member(
        vsf_sem_t response_sem;

        uint8_t request_buff[CMSIS_DAP_V2_PACKET_SIZE];
        uint8_t response_buff[CMSIS_DAP_V2_PACKET_SIZE];

        vk_usbd_trans_t transact_in;
        vk_usbd_trans_t transact_out;
        vk_usbd_dev_t *dev;
        vk_usbd_ifs_t *ifs;
    )
};

/*============================ GLOBAL VARIABLES ==============================*/

extern const vk_usbd_class_op_t vk_usbd_cmsis_dap_v2_class;

/*============================ PROTOTYPES ====================================*/

#ifdef __cplusplus
}
#endif

#endif      // VSF_USE_USB_DEVICE
#endif      // __VSF_USBD_CMSIS_DAP_V2_H__
