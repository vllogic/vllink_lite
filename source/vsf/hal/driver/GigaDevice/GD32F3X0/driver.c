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
typedef void(*pFunc)(void);
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
extern const pFunc __VECTOR_TABLE[];
/*============================ IMPLEMENTATION ================================*/

static vsf_clk_info_t vsf_clk_info = {
	.clken = CHIP_CLKEN,
	.hclksrc = CHIP_HCLKSRC,
	.pllsrc = CHIP_PLLSRC,
	.usbsrc = CHIP_USBSRC,
	.lse_freq_hz = CHIP_LSE_FREQ_HZ,
	.hse_freq_hz = CHIP_HSE_FREQ_HZ,
	.pll_freq_hz = CHIP_PLL_FREQ_HZ,
	.ahb_freq_hz = CHIP_AHB_FREQ_HZ,
	.apb1_freq_hz = CHIP_APB1_FREQ_HZ,
	.apb2_freq_hz = CHIP_APB2_FREQ_HZ,
};

static void vsf_clk_init(vsf_clk_info_t *info)
{
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


}

bool vsf_driver_init(void)
{
    uint32_t tmp32;

	NVIC_SetPriorityGrouping(3);
    SCB->VTOR = (uint32_t)__VECTOR_TABLE;

	vsf_clk_init(&vsf_clk_info);
	
    return true;
}

vsf_clk_info_t *vsf_clk_info_get(void)
{
	return &vsf_clk_info;
}

static callback_param_t dma_stream_callback[DMA_COUNT][DMA_STREAM_COUNT];
static void *dma_stream_callback_param[DMA_COUNT][DMA_STREAM_COUNT];

void vsf_config_dma_stream_callback(uint8_t dma, uint8_t stream, callback_param_t callback, void *param)
{
    VSF_HAL_ASSERT(dma < DMA_COUNT);
    VSF_HAL_ASSERT(stream < DMA_STREAM_COUNT);

    dma_stream_callback[dma][stream] = callback;
    dma_stream_callback_param[dma][stream] = param;
}

ROOT void DMA_Channel0_IRQHandler(void)
{
    if (dma_stream_callback[0][0]) {
        dma_stream_callback[0][0](dma_stream_callback_param[0][0]);
    }
}

ROOT void DMA_Channel1_2_IRQHandler(void)
{
    if (dma_stream_callback[0][1]) {
        dma_stream_callback[0][1](dma_stream_callback_param[0][1]);
    }
    if (dma_stream_callback[0][2]) {
        dma_stream_callback[0][2](dma_stream_callback_param[0][2]);
    }
}

ROOT void DMA_Channel3_4_IRQHandler(void)
{
    if (dma_stream_callback[0][3]) {
        dma_stream_callback[0][3](dma_stream_callback_param[0][3]);
    }
    if (dma_stream_callback[0][4]) {
        dma_stream_callback[0][4](dma_stream_callback_param[0][4]);
    }
}

ROOT void DMA_Channel5_6_IRQHandler(void)
{
    if (dma_stream_callback[0][5]) {
        dma_stream_callback[0][5](dma_stream_callback_param[0][5]);
    }
    if (dma_stream_callback[0][6]) {
        dma_stream_callback[0][6](dma_stream_callback_param[0][6]);
    }
}

/* EOF */
