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

/*============================ INCLUDES ======================================*/

#include "../../common.h"
#include "./usbd.h"

#if VSF_HAL_USE_USBD == ENABLED

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

extern vsf_err_t __mt006_usb_init_interrupt(mt006_usb_t *usb, vsf_arch_prio_t priority);
extern void __mt006_usb_irq(mt006_usb_t *usb);

/*============================ IMPLEMENTATION ================================*/

vsf_err_t mt006_usbd_init(mt006_usb_t *dc, usb_dc_ip_cfg_t *cfg)
{
#if VSF_HAL_USE_USBH == ENABLED
    // hc->is_host only exists when both host and device modes are enabled
    dc->is_host = false;
#endif
    dc->callback.irq_handler = cfg->irq_handler;
    dc->callback.param = cfg->param;

    RCC->PDRUNCFG &= ~RCC_PDRUNCFG_USB;
    RCC->AHBCLKCTRL0_SET = RCC_AHBCLKCTRL_USB;

    __mt006_usb_init_interrupt(dc, cfg->priority);
    return VSF_ERR_NONE;
}

void mt006_usbd_fini(mt006_usb_t *dc)
{
    NVIC_DisableIRQ(dc->param->irq);
    RCC->AHBCLKCTRL0_CLR = RCC_AHBCLKCTRL_USB;
    RCC->PDRUNCFG |= RCC_PDRUNCFG_USB;
}

void mt006_usbd_get_info(mt006_usb_t *dc, usb_dc_ip_info_t *info)
{
    VSF_HAL_ASSERT(info != NULL);
    info->regbase = dc->param->reg;
    info->ep_num = mt006_usbd_ep_number;
    info->is_dma = mt006_usbd_ep_is_dma;
}

void mt006_usbd_connect(mt006_usb_t *dc)
{
    RCC->USBCTRL |= 0xF0;
    //    RCC->PRESETCTRL0_SET = AHBCLK_USB_msk;
}

void mt006_usbd_disconnect(mt006_usb_t *dc)
{
    RCC->USBCTRL &= 0xFFFFFF0F;
    RCC->USBCTRL |= 0x100;
    //    RCC->PRESETCTRL0_CLR = AHBCLK_USB_msk;
}

void mt006_usbd_irq(mt006_usb_t *dc)
{
    __mt006_usb_irq(dc);
}

#endif      // VSF_HAL_USE_USBD
