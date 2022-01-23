/*****************************************************************************
 *   Copyright(C)2009-2019 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

#ifndef __HAL_DRIVER_MT006_USBH_H__
#define __HAL_DRIVER_MT006_USBH_H__

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"
#include "../../device.h"

#include "../usb.h"

#if VSF_HAL_USE_USBH == ENABLED

/*============================ MACROS ========================================*/

#define mt006_usbh_ep_number            8
#define mt006_usbh_ep_is_dma            false

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ INCLUDES ======================================*/
/*============================ PROTOTYPES ====================================*/

extern vsf_err_t mt006_usbh_init(mt006_usb_t *hc, usb_hc_ip_cfg_t *cfg);
extern void mt006_usbh_get_info(mt006_usb_t *hc, usb_hc_ip_info_t *info);
extern void mt006_usbh_irq(mt006_usb_t *hc);

#endif      // VSF_HAL_USE_USBH
#endif      // __HAL_DRIVER_MT006_USBH_H__
/* EOF */
