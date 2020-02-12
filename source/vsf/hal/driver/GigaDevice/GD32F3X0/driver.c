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
#include "hal/vsf_hal_cfg.h"
#include "./device.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

/*! \note initialize device driver
 *  \param none
 *  \retval true initialization succeeded.
 *  \retval false initialization failed
 */
bool vsf_driver_init(void)
{
    uint32_t tmp32;

    // select irc8m
    RCU_CTL0 |= RCU_CTL0_IRC8MEN;
    while(!(RCU_CTL0 & RCU_CTL0_IRC8MSTB));
    RCU_CFG0 &= ~RCU_CFG0_SCS;

    // enable hsi48m
    RCU_ADDCTL |= RCU_ADDCTL_IRC48MEN;
    while(!(RCU_ADDCTL & RCU_ADDCTL_IRC48MSTB));

    // not enable hse
    //RCU_CTL0 |= RCU_CTL0_HXTALEN;
    //while(!(RCU_CTL0 & RCU_CTL0_HXTALSTB));

    // config pll
    tmp32 = 48000000 / 4000000 - 1;
    RCU_CTL0 &= ~RCU_CTL0_PLLEN;
    RCU_CFG1 &= RCU_CFG1_PLLPRESEL | RCU_CFG1_PREDV;
    RCU_CFG1 |= RCU_CFG1_PLLPRESEL | tmp32;
    RCU_CFG0 |= RCU_CFG0_PLLSEL;
    tmp32 = 128000000 / 4000000 - 1;
    RCU_CFG0 &= ~RCU_CFG0_PLLMF;
    RCU_CFG1 &= ~RCU_CFG1_PLLMF5;
    RCU_CFG0 |= ((tmp32 & 0xf) << 18) | ((tmp32 & 0x10) << 23);
    RCU_CFG1 |= ((tmp32 & 0x20) << 26);

    RCU_CTL0 |= RCU_CTL0_PLLEN;
    while(!(RCU_CTL0 & RCU_CTL0_PLLSTB));

    // config ahb apb1 apb2: apb1 == apb2 == ahb / 2 == pll / 2
    RCU_CFG0 &= ~(RCU_CFG0_AHBPSC | RCU_CFG0_APB1PSC | RCU_CFG0_APB2PSC);
    RCU_CFG0 |= RCU_APB1_CKAHB_DIV2 | RCU_APB2_CKAHB_DIV2 | RCU_AHB_CKSYS_DIV1;

    RCU_CFG0 |= RCU_CKSYSSRC_PLL;
    while((RCU_CFG0 & RCU_SCSS_PLL) != RCU_SCSS_PLL);

    RCU_CTL0 &= ~RCU_CTL0_IRC8MEN;

    //SCB->VTOR = vsfhal_info.vector_table;
    //SCB->AIRCR = 0x05FA0000 | vsfhal_info.priority_group;

    return true;
}


/* EOF */
