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

#include "component/usb/vsf_usb_cfg.h"
#include "vsf.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/

static vsf_err_t vk_usbd_cmsis_dap_v2_init(vk_usbd_dev_t *dev, vk_usbd_ifs_t *ifs);
static vsf_err_t vk_usbd_cmsis_dap_v2_request_prepare(vk_usbd_dev_t *dev, vk_usbd_ifs_t *ifs);
static vsf_err_t vk_usbd_cmsis_dap_v2_request_process(vk_usbd_dev_t *dev, vk_usbd_ifs_t *ifs);

/*============================ GLOBAL VARIABLES ==============================*/

const vk_usbd_class_op_t vk_usbd_cmsis_dap_v2 = {
    .request_prepare =      vk_usbd_cmsis_dap_v2_request_prepare,
    .request_process =      vk_usbd_cmsis_dap_v2_request_process,
    .init =                 vk_usbd_cmsis_dap_v2_init,
};

/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

static vsf_err_t vk_usbd_cmsis_dap_v2_request_prepare(vk_usbd_dev_t *dev, vk_usbd_ifs_t *ifs)
{
    // TODO
    return VSF_ERR_NONE;
}

static vsf_err_t vk_usbd_cmsis_dap_v2_request_process(vk_usbd_dev_t *dev, vk_usbd_ifs_t *ifs)
{
    // TODO
    return VSF_ERR_NONE;
}

static vsf_err_t vk_usbd_cmsis_dap_v2_init(vk_usbd_dev_t *dev, vk_usbd_ifs_t *ifs)
{
    // TODO
    return VSF_ERR_NONE;
}
