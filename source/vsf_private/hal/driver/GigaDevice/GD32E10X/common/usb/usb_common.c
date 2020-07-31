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

#if VSF_USE_USB_DEVICE == ENABLED || VSF_USE_USB_HOST == ENABLED

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

vsf_err_t gd32e10x_usb_init(gd32e10x_usb_t *usb, vsf_arch_prio_t priority)
{
    const gd32e10x_usb_const_t *param = usb->param;
    struct dwcotg_core_global_regs_t *global_regs = param->reg;

#if CHIP_USBSRC == GD32E10X_USBSRC_HSI48M
    RCU_ADDAPB1EN |= RCU_ADDAPB1EN_CTCEN;
    CTC_CTL1 = CTC_REFSOURCE_USBSOF | CTL1_CKLIM(28) | (48000 - 1);
    CTC_CTL0 |= CTC_CTL0_AUTOTRIM | CTC_CTL0_CNTEN;
    RCU_ADDCTL |= RCU_ADDCTL_CK48MSEL;
#elif CHIP_USBSRC == GD32E10X_USBSRC_PLL
    RCU_CFG0 &= ~(RCU_CFG0_USBFSPSC | RCU_CFG0_USBFSPSC_2);
#   if CHIP_PLL_FREQ_HZ == 48000000
    RCU_CFG0 |= RCU_CKUSB_CKPLL_DIV1;
#   elif CHIP_PLL_FREQ_HZ == 72000000
    RCU_CFG0 |= RCU_CKUSB_CKPLL_DIV1_5;
#   elif CHIP_PLL_FREQ_HZ == 96000000
    RCU_CFG0 |= RCU_CKUSB_CKPLL_DIV2;
#   elif CHIP_PLL_FREQ_HZ == 120000000
    RCU_CFG0 |= RCU_CKUSB_CKPLL_DIV2_5;
#   elif CHIP_PLL_FREQ_HZ == 144000000
    RCU_CFG0 |= RCU_CKUSB_CKPLL_DIV3;
#   elif CHIP_PLL_FREQ_HZ == 168000000
    RCU_CFG0 |= RCU_CKUSB_CKPLL_DIV3_5;
#   elif CHIP_PLL_FREQ_HZ == 192000000
    RCU_CFG0 |= RCU_CKUSB_CKPLL_DIV4;
#   else
#       error "Invaild CHIP_PLL_FREQ_HZ"
#   endif
    RCU_ADDCTL &= ~RCU_ADDCTL_CK48MSEL;
#else
#   error "Invaild CHIP_USBSRC"
#endif

    RCU_AHBEN |= RCU_AHBEN_USBFSEN;

    global_regs->gahbcfg &= ~USB_OTG_GAHBCFG_GINT;

#if CHIP_USBSRC == GD32E10X_USBSRC_HSI48M
#   define GCCFG_SOFOEN     (0x1ul << 20)
    global_regs->gccfg = GCCFG_SOFOEN;
#endif

    if (priority >= 0) {
        NVIC_SetPriority(param->irq, priority);
        NVIC_EnableIRQ(param->irq);
    } else {
        NVIC_DisableIRQ(param->irq);
    }
    return VSF_ERR_NONE;
}

void gd32e10x_usb_irq(gd32e10x_usb_t *usb)
{
    if (usb->callback.irq_handler != NULL) {
        usb->callback.irq_handler(usb->callback.param);
    }
}

#endif
