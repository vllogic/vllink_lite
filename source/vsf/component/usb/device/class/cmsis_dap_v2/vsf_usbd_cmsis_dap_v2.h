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

#ifndef __VSF_USBD_CMSIS_DAP_V2_H__
#define __VSF_USBD_CMSIS_DAP_V2_H__

/*============================ INCLUDES ======================================*/

#include "component/usb/vsf_usb_cfg.h"

/*============================ MACROS ========================================*/

#define USB_DESC_CMSIS_DAP_V2_IAD_LEN           23

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

declare_simple_class(vk_usbd_cmsis_dap_v2_t)

def_simple_class(vk_usbd_cmsis_dap_v2_t) {
    public_member(
        uint8_t dummy1;
    )

    private_member(
        uint8_t dummy2;
    )
};

/*============================ GLOBAL VARIABLES ==============================*/

extern const vk_usbd_class_op_t vk_usbd_cmsis_dap_v2;

/*============================ PROTOTYPES ====================================*/

#endif      // __VSF_USBD_CMSIS_DAP_V2_H__
