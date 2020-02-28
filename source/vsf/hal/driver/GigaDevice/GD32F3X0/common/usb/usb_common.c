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

//#include "../common.h"
#include "./usb.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

vsf_err_t gd32f3x0_usb_init(gd32f3x0_usb_t *usb, vsf_arch_prio_t priority)
{
    const gd32f3x0_usb_const_t *param = usb->param;
    struct dwcotg_core_global_regs_t *global_regs = param->reg;
    struct vsfhal_clk_info_t *info = vsfhal_clk_info_get();
    
    if (info->usbsrc == GD32F3X0_USBSRC_PLL) {
        RCU_ADDAPB1EN |= RCU_ADDAPB1EN_CTCEN;
        CTC_CTL1 = (0x2ul << 28) | (0x1cul << 16) | (48000 - 1);
        CTC_CTL0 |= CTC_CTL0_AUTOTRIM | CTC_CTL0_CNTEN;
        RCU_ADDCTL |= RCU_ADDCTL_CK48MSEL;
    } else {
        RCU_CFG0 &= ~RCU_CFG0_USBFSPSC;
        if (info->pll_freq_hz == 48000000) {
            RCU_CFG0 |= BIT(22);
        } else if (info->pll_freq_hz == 72000000) {
            NOP();
        } else if (info->pll_freq_hz == 96000000) {
            RCU_CFG0 |= BITS(22,23);
        } else if (info->pll_freq_hz == 120000000) {
            RCU_CFG0 |= BIT(23);
        } else {
            VSF_HAL_ASSERT(0);
        }
        RCU_ADDCTL &= ~RCU_ADDCTL_CK48MSEL;
    }

    RCU_AHBEN |= RCU_AHBEN_USBFS;

    global_regs->gahbcfg &= ~USB_OTG_GAHBCFG_GINT;

    if (priority >= 0) {
        NVIC_SetPriority(param->irq, priority);
        NVIC_EnableIRQ(param->irq);
    } else {
        NVIC_DisableIRQ(param->irq);
    }
    return VSF_ERR_NONE;
}

void gd32f3x0_usb_irq(gd32f3x0_usb_t *usb)
{
    if (usb->callback.irq_handler != NULL) {
        usb->callback.irq_handler(usb->callback.param);
    }
}
