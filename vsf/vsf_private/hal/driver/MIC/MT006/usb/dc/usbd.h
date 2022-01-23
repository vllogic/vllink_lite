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

#ifndef __HAL_DRIVER_MT006_USBD_H__
#define __HAL_DRIVER_MT006_USBD_H__

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"
#include "../../device.h"

#include "../usb.h"

#if VSF_HAL_USE_USBD == ENABLED

/*============================ MACROS ========================================*/

#define mt006_usbd_ep_number            16
#define mt006_usbd_ep_is_dma            false

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ INCLUDES ======================================*/
/*============================ PROTOTYPES ====================================*/

extern vsf_err_t mt006_usbd_init(mt006_usb_t *dc, usb_dc_ip_cfg_t *cfg);
extern void mt006_usbd_fini(mt006_usb_t *dc);
extern void mt006_usbd_get_info(mt006_usb_t *dc, usb_dc_ip_info_t *info);
extern void mt006_usbd_connect(mt006_usb_t *dc);
extern void mt006_usbd_disconnect(mt006_usb_t *dc);
extern void mt006_usbd_irq(mt006_usb_t *dc);

#endif      // VSF_HAL_USE_USBD
#endif      // __HAL_DRIVER_MT006_USBD_H__
/* EOF */
