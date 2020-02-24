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

#ifndef __HAL_DEVICE_GIGADEVICE_GD32F3X0_H__
#define __HAL_DEVICE_GIGADEVICE_GD32F3X0_H__

/*============================ INCLUDES ======================================*/
#include "hal/vsf_hal_cfg.h"

/*============================ MACROS ========================================*/

/*\note first define basic info for arch. */
#define VSF_ARCH_PRI_NUM            16
#define VSF_ARCH_PRI_BIT            4

// software interrupt provided by a dedicated device
#define VSF_DEV_SWI_NUM             9
#define VSF_DEV_SWI_LIST            70, 71, 72, 73, 74, 75, 76, 77, 78

/*============================ INCLUDES ======================================*/

/*\note this is should be the only place where __common.h is included.*/
#include "./common/__common.h"

/*============================ MACROS ========================================*/

#define GPIO_PORT_COUNT     (0 + GPIOA_ENABLE + GPIOB_ENABLE + GPIOC_ENABLE +   \
                                GPIOD_ENABLE + GPIOF_ENABLE)
#define USART_COUNT         (0 + USART0_ENABLE + USART1_ENABLE)


#if     defined(__GD32F350__)
#define USB_OTG_COUNT               1
#define USB_OTG0_IRQHandler         USBFS_IRQHandler
// required by dwcotg, define the max ep number of dwcotg
#define USB_DWCOTG_MAX_EP_NUM       4

#define USB_OTG0_CONFIG                                                         \
    .ep_num = 8,                                                                \
    .irq = USBFS_IRQn,                                                          \
    .reg = USBFS_BASE,                                                          \
    .buffer_word_size = 0x500 >> 2,                                             \
    .speed = USB_SPEED_FULL,                                                    \
	.dma_en = false,                                                            \
	.ulpi_en = false,                                                           \
	.utmi_en = false,                                                           \
	.vbus_en = false,
#endif

#define DMA_COUNT           		2
#define DMA_STREAM_COUNT    		7

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

#ifndef CHIP_CLKEN
#define GD32F3X0_CLKEN_LSI              (1UL << 0)
#define GD32F3X0_CLKEN_HSI              (1UL << 1)
#define GD32F3X0_CLKEN_HSI48M           (1UL << 2)
#define GD32F3X0_CLKEN_LSE              (1UL << 3)
#define GD32F3X0_CLKEN_HSE              (1UL << 4)
#define GD32F3X0_CLKEN_PLL              (1UL << 5)

enum gd32f3x0_hclksrc_t
{
	GD32F3X0_HCLKSRC_HSI8M      = 0,
	GD32F3X0_HCLKSRC_HSE        = 1,
	GD32F3X0_HCLKSRC_PLL        = 2,
};

enum gd32f3x0_pllsrc_t
{
	GD32F3X0_PLLSRC_HSI8M_D2    = 0,
	GD32F3X0_PLLSRC_HSE         = 1,
	GD32F3X0_PLLSRC_HSI48M      = 2,
};

enum gd32f3x0_usbsrc_t
{
	GD32F3X0_USBSRC_PLL         = 0,
	GD32F3X0_USBSRC_HSI48M      = 1,
};

#define CHIP_CLKEN                      (GD32F3X0_CLKEN_HSI48M | GD32F3X0_CLKEN_PLL)
#define CHIP_HCLKSRC                    GD32F3X0_HCLKSRC_PLL
#define CHIP_PLLSRC                     GD32F3X0_PLLSRC_HSI48M
#define CHIP_USBSRC                     GD32F3X0_USBSRC_HSI48M
#define CHIP_LSE_FREQ_HZ                (32768)
#define CHIP_HSE_FREQ_HZ                (12 * 1000 * 1000)
#define CHIP_PLL_FREQ_HZ                (128 * 1000 * 1000)
#define CHIP_AHB_FREQ_HZ                (CHIP_PLL_FREQ_HZ)
#define CHIP_APB1_FREQ_HZ               (CHIP_AHB_FREQ_HZ / 2)
#define CHIP_APB2_FREQ_HZ               (CHIP_AHB_FREQ_HZ / 2)
#endif

struct vsf_clk_info_t {
	uint32_t clken;
	
	enum gd32f3x0_hclksrc_t hclksrc;
	enum gd32f3x0_pllsrc_t pllsrc;
    enum gd32f3x0_usbsrc_t usbsrc;
	
	uint32_t lse_freq_hz;
	uint32_t hse_freq_hz;
	uint32_t pll_freq_hz;

	uint32_t ahb_freq_hz;
	uint32_t apb1_freq_hz;
	uint32_t apb2_freq_hz;
};
typedef struct vsf_clk_info_t vsf_clk_info_t;

typedef void(*callback_param_t)(void *param);

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
extern vsf_clk_info_t *vsf_clk_info_get(void);
extern void vsf_config_dma_stream_callback(uint8_t dma, uint8_t stream, callback_param_t callback, void *param);

#endif
/* EOF */
