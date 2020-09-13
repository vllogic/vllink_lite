/*============================ INCLUDES ======================================*/

#include "component/usb/vsf_usb_cfg.h"

#if VSF_USE_USB_DEVICE == ENABLED

#define __VSF_EDA_CLASS_INHERIT__
#define __VSF_USBD_CLASS_INHERIT__
#define __VSF_USBD_WEBUSB_USR_IMPLEMENT

#include "vsf.h"
#include "vsf_usbd_webusb_usr.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/

static vsf_err_t __vk_usbd_webusb_request_prepare(vk_usbd_dev_t *dev, vk_usbd_ifs_t *ifs);
static vsf_err_t __vk_usbd_webusb_request_process(vk_usbd_dev_t *dev, vk_usbd_ifs_t *ifs);
static vsf_err_t __vk_usbd_webusb_class_init(vk_usbd_dev_t *dev, vk_usbd_ifs_t *ifs);

/*============================ GLOBAL VARIABLES ==============================*/

const vk_usbd_class_op_t vk_usbd_webusb_class = {
    .request_prepare =      __vk_usbd_webusb_request_prepare,
    .request_process =      __vk_usbd_webusb_request_process,
    .init =                 __vk_usbd_webusb_class_init,
};

/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

static vsf_err_t __vk_usbd_webusb_request_prepare(vk_usbd_dev_t *dev, vk_usbd_ifs_t *ifs)
{
    vk_usbd_webusb_t *webusb = ifs->class_param;
    vk_usbd_ctrl_handler_t *ctrl_handler = &dev->ctrl_handler;
    struct usb_ctrlrequest_t *request = &ctrl_handler->request;

    // TODO
    return VSF_ERR_NONE;
}

static vsf_err_t __vk_usbd_webusb_request_process(vk_usbd_dev_t *dev, vk_usbd_ifs_t *ifs)
{
    vk_usbd_webusb_t *webusb = ifs->class_param;
    vk_usbd_ctrl_handler_t *ctrl_handler = &dev->ctrl_handler;
    struct usb_ctrlrequest_t *request = &ctrl_handler->request;

    // TODO
    return VSF_ERR_NONE;
}

static vsf_err_t __vk_usbd_webusb_class_init(vk_usbd_dev_t *dev, vk_usbd_ifs_t *ifs)
{
    vk_usbd_webusb_t *webusb = ifs->class_param;

    vsf_sem_init(&webusb->response_sem, 1);
    return VSF_ERR_NONE;
}

#endif      // VSF_USE_USB_DEVICE
