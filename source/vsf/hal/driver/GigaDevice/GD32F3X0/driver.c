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

static vsfhal_clk_info_t vsfhal_clk_info = {
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

static void clk_init(vsfhal_clk_info_t *info)
{
    uint32_t tmp32;

    VSF_HAL_ASSERT(info && (info->pllsrc <= GD32F3X0_PLLSRC_HSI48M));
 
    RCU_CTL0 |= RCU_CTL0_IRC8MEN;
    while(!(RCU_CTL0 & RCU_CTL0_IRC8MSTB));
    RCU_CFG0 &= ~RCU_CFG0_SCS;

    if (info->clken & GD32F3X0_CLKEN_HSI48M) {
        RCU_ADDCTL |= RCU_ADDCTL_IRC48MEN;
        while(!(RCU_ADDCTL & RCU_ADDCTL_IRC48MSTB));
    }

    if (info->clken & GD32F3X0_CLKEN_HSE) {
        RCU_ADDCTL |= RCU_CTL0_HXTALEN;
        while(!(RCU_ADDCTL & RCU_CTL0_HXTALSTB));
    }

    RCU_CTL0 &= ~RCU_CTL0_PLLEN;
    if (info->clken & GD32F3X0_CLKEN_PLL) {
        
		if (info->pllsrc == GD32F3X0_PLLSRC_HSI8M_D2) {
			RCU_CFG0 &= RCU_CFG0_PLLSEL;
		} else if (info->pllsrc == GD32F3X0_PLLSRC_HSE) {
			RCU_CFG1 &= RCU_CFG1_PLLPRESEL | RCU_CFG1_PREDV;
			RCU_CFG1 |= info->hse_freq_hz / 2000000 - 1;
			RCU_CFG0 |= RCU_CFG0_PLLSEL;
		} else if (info->pllsrc == GD32F3X0_PLLSRC_HSI48M) {
			RCU_CFG1 &= RCU_CFG1_PLLPRESEL | RCU_CFG1_PREDV;
			RCU_CFG1 |= RCU_CFG1_PLLPRESEL | (48000000 / 2000000 - 1);
			RCU_CFG0 |= RCU_CFG0_PLLSEL;
		}
        
        tmp32 = info->pll_freq_hz / 2000000 - 1;
        RCU_CFG0 &= ~RCU_CFG0_PLLMF;
        RCU_CFG1 &= ~RCU_CFG1_PLLMF5;
        RCU_CFG0 |= ((tmp32 & 0xf) << 18) | ((tmp32 & 0x10) << 23);
        RCU_CFG1 |= ((tmp32 & 0x20) << 26);

        RCU_CTL0 |= RCU_CTL0_PLLEN;
        while(!(RCU_CTL0 & RCU_CTL0_PLLSTB));
    }

	RCU_CFG0 &= ~(RCU_CFG0_AHBPSC | RCU_CFG0_APB1PSC | RCU_CFG0_APB2PSC);
	tmp32 = info->ahb_freq_hz / info->apb1_freq_hz;
	if (tmp32 == 2)
		RCU_CFG0 |= BIT(10);
	else if (tmp32 == 4)
		RCU_CFG0 |= BIT(10) | BIT(8);
	else if (tmp32 == 8)
		RCU_CFG0 |= BITS(9, 10);
	else
		RCU_CFG0 |= BITS(8, 10);
	tmp32 = info->ahb_freq_hz / info->apb2_freq_hz;
	if (tmp32 == 2)
		RCU_CFG0 |= BIT(13);
	else if (tmp32 == 4)
		RCU_CFG0 |= BIT(13) | BIT(11);
	else if (tmp32 == 8)
		RCU_CFG0 |= BITS(12, 13);
	else
		RCU_CFG0 |= BITS(11, 13);
	if (info->hclksrc == GD32F3X0_HCLKSRC_HSE)
		tmp32 = info->hse_freq_hz;
	else if (info->hclksrc == GD32F3X0_HCLKSRC_PLL)
		tmp32 = info->pll_freq_hz;
	else
		tmp32 = 8000000;
	tmp32 = tmp32 / info->ahb_freq_hz;
	if (tmp32 == 1)
		RCU_CFG0 |= RCU_AHB_CKSYS_DIV1;
	else if (tmp32 == 2)
		RCU_CFG0 |= RCU_AHB_CKSYS_DIV2;
	else if (tmp32 == 4)
		RCU_CFG0 |= RCU_AHB_CKSYS_DIV4;
	else if (tmp32 == 8)
		RCU_CFG0 |= RCU_AHB_CKSYS_DIV8;
	else if (tmp32 == 16)
		RCU_CFG0 |= RCU_AHB_CKSYS_DIV16;
	else if (tmp32 == 64)
		RCU_CFG0 |= RCU_AHB_CKSYS_DIV64;
	else if (tmp32 == 128)
		RCU_CFG0 |= RCU_AHB_CKSYS_DIV128;
	else if (tmp32 == 256)
		RCU_CFG0 |= RCU_AHB_CKSYS_DIV256;
	else
		RCU_CFG0 |= RCU_AHB_CKSYS_DIV512;

    if (info->hclksrc == GD32F3X0_HCLKSRC_HSE) {
        RCU_CFG0 |= RCU_CKSYSSRC_HXTAL;
        while((RCU_CFG0 & RCU_SCSS_HXTAL) != RCU_SCSS_HXTAL);
    } else if (info->hclksrc == GD32F3X0_HCLKSRC_PLL) {
        RCU_CFG0 |= RCU_CKSYSSRC_PLL;
        while((RCU_CFG0 & RCU_SCSS_PLL) != RCU_SCSS_PLL);
    } else {
        RCU_CFG0 &= ~RCU_CFG0_SCS;
        while((RCU_CFG0 & RCU_SCSS_IRC8M) != RCU_SCSS_IRC8M);
    }
    
    if (!(info->clken & GD32F3X0_CLKEN_HSI))
        RCU_CTL0 &= ~RCU_CTL0_IRC8MEN;
}

bool vsf_driver_init(void)
{
	NVIC_SetPriorityGrouping(3);
    SCB->VTOR = (uint32_t)__VECTOR_TABLE;

	clk_init(&vsfhal_clk_info);
    return true;
}

vsfhal_clk_info_t *vsfhal_clk_info_get(void)
{
	return &vsfhal_clk_info;
}

#if DMA_COUNT > 0

static callback_param_t dma_stream_callback[DMA_COUNT][DMA_STREAM_COUNT];
static void *dma_stream_callback_param[DMA_COUNT][DMA_STREAM_COUNT];

void vsf_config_dma_stream_callback(uint8_t dma, uint8_t stream, callback_param_t callback, void *param)
{
    VSF_HAL_ASSERT(dma < DMA_COUNT);
    VSF_HAL_ASSERT(stream < DMA_STREAM_COUNT);

    dma_stream_callback[dma][stream] = callback;
    dma_stream_callback_param[dma][stream] = param;
}

#if DMA_STREAM_COUNT > 0
ROOT void DMA_Channel0_IRQHandler(void)
{
    if (dma_stream_callback[0][0]) {
        dma_stream_callback[0][0](dma_stream_callback_param[0][0]);
    }
}
#endif

#if DMA_STREAM_COUNT > 1
ROOT void DMA_Channel1_2_IRQHandler(void)
{
    if (dma_stream_callback[0][1]) {
        dma_stream_callback[0][1](dma_stream_callback_param[0][1]);
    }
	#if DMA_STREAM_COUNT > 2
    if (dma_stream_callback[0][2]) {
        dma_stream_callback[0][2](dma_stream_callback_param[0][2]);
    }
	#endif
}
#endif

#if DMA_STREAM_COUNT > 3
ROOT void DMA_Channel3_4_IRQHandler(void)
{
    if (dma_stream_callback[0][3]) {
        dma_stream_callback[0][3](dma_stream_callback_param[0][3]);
    }
	#if DMA_STREAM_COUNT > 4
    if (dma_stream_callback[0][4]) {
        dma_stream_callback[0][4](dma_stream_callback_param[0][4]);
    }
	#endif
}
#endif

#if DMA_STREAM_COUNT > 5
ROOT void DMA_Channel5_6_IRQHandler(void)
{
    if (dma_stream_callback[0][5]) {
        dma_stream_callback[0][5](dma_stream_callback_param[0][5]);
    }
	#if DMA_STREAM_COUNT > 6
    if (dma_stream_callback[0][6]) {
        dma_stream_callback[0][6](dma_stream_callback_param[0][6]);
    }
	#endif
}
#endif
#endif

/* EOF */
