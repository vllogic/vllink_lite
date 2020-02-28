#include "vsf.h"

static vsf_err_t __vk_usbd_dfu_class_init(vk_usbd_dev_t *dev, vk_usbd_ifs_t *ifs)
{

}

static vsf_err_t __vk_usbd_dfu_request_prepare(vk_usbd_dev_t *dev, vk_usbd_ifs_t *ifs)
{

}

static vsf_err_t __vk_usbd_dfu_request_process(vk_usbd_dev_t *dev, vk_usbd_ifs_t *ifs)
{

}

const vk_usbd_class_op_t vk_usbd_dfu_class = {
    .request_prepare =      __vk_usbd_dfu_request_prepare,
    .request_process =      __vk_usbd_dfu_request_process,
    .init =                 __vk_usbd_dfu_class_init,
};
