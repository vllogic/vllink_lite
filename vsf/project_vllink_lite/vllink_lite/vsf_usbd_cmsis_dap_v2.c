/*============================ INCLUDES ======================================*/

#include "component/usb/vsf_usb_cfg.h"

#if VSF_USE_USB_DEVICE == ENABLED

#define __VSF_EDA_CLASS_INHERIT__
#define __VSF_USBD_CLASS_INHERIT__
#define __VSF_USBD_CMSIS_DAP_V2_IMPLEMENT

#include "vsf.h"
#include "vsf_usbd_cmsis_dap_v2.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/

static vsf_err_t __vk_usbd_cmsis_dap_v2_class_init(vk_usbd_dev_t *dev, vk_usbd_ifs_t *ifs);

/*============================ GLOBAL VARIABLES ==============================*/

const vk_usbd_class_op_t vk_usbd_cmsis_dap_v2_class = {
    .init =                 __vk_usbd_cmsis_dap_v2_class_init,
};

/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/


static void on_response_finish(void *param)
{
    vk_usbd_cmsis_dap_v2_t *cmsis_dap_v2 = param;
    vsf_sem_post(&cmsis_dap_v2->response_sem);
}

static void on_response_ready(void *p, uint8_t *buf, uint16_t size)
{
    vk_usbd_cmsis_dap_v2_t *cmsis_dap_v2 = p;
    vk_usbd_trans_t *trans = &cmsis_dap_v2->transact_in;
    
    if (size) {
        if (size < sizeof(cmsis_dap_v2->response_buff)) {
            if (!(size % 64))
                size += 1;
        } else {
            size = sizeof(cmsis_dap_v2->response_buff);
        }
        memcpy(cmsis_dap_v2->response_buff, buf, size);
        trans->use_as__vsf_mem_t.size = size;
        vk_usbd_ep_send(cmsis_dap_v2->dev, trans);
    } else {
        on_response_finish(cmsis_dap_v2);
    }
}

static void on_request_finish(void *param)
{
    vk_usbd_cmsis_dap_v2_t *cmsis_dap_v2 = param;
    vk_usbd_trans_t *trans = &cmsis_dap_v2->transact_out;

    vsf_err_t ret = dap_requset(cmsis_dap_v2->dap, &cmsis_dap_v2->response_sem,
                on_response_ready, cmsis_dap_v2,
                trans->use_as__vsf_mem_t.buffer, sizeof(cmsis_dap_v2->request_buff));
    if (ret != VSF_ERR_NONE) {
        if (vsf_eda_sem_pend(&cmsis_dap_v2->response_sem, 0) == VSF_ERR_NONE) {
            uint8_t buf[1] = {ID_DAP_Invalid};
            on_response_ready(cmsis_dap_v2, buf, 1);
        } else {
            // Not Support this case, response will lost
        }
    }

    trans->use_as__vsf_mem_t.size = sizeof(cmsis_dap_v2->request_buff);
    vk_usbd_ep_recv(cmsis_dap_v2->dev, trans);
}

static vsf_err_t __vk_usbd_cmsis_dap_v2_class_init(vk_usbd_dev_t *dev, vk_usbd_ifs_t *ifs)
{
    vk_usbd_cmsis_dap_v2_t *cmsis_dap_v2 = ifs->class_param;
    vk_usbd_trans_t *trans;

    cmsis_dap_v2->dev = dev;
    cmsis_dap_v2->ifs = ifs;
    
    vsf_sem_init(&cmsis_dap_v2->response_sem, 1);
    
    trans = &cmsis_dap_v2->transact_out;
    trans->ep = cmsis_dap_v2->ep_out;
    trans->zlp = false;
    trans->notify_eda = false;
    trans->use_as__vsf_mem_t.buffer = cmsis_dap_v2->request_buff;
    trans->use_as__vsf_mem_t.size = sizeof(cmsis_dap_v2->request_buff);
    trans->on_finish = on_request_finish;
    trans->param = cmsis_dap_v2;
    vk_usbd_ep_recv(dev, trans);

    trans = &cmsis_dap_v2->transact_in;
    trans->ep = cmsis_dap_v2->ep_in;
    trans->zlp = false;
    trans->notify_eda = false;
    trans->use_as__vsf_mem_t.buffer = cmsis_dap_v2->response_buff;
    trans->use_as__vsf_mem_t.size = sizeof(cmsis_dap_v2->response_buff);
    trans->on_finish = on_response_finish;
    trans->param = cmsis_dap_v2;

    return VSF_ERR_NONE;
}

#endif      // VSF_USE_USB_DEVICE
