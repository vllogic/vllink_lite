/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"

/*============================ MACROS ========================================*/

#if defined(__VSF_HEADER_ONLY_SHOW_ARCH_INFO__)

/*\note first define basic info for arch. */
#define VSF_ARCH_PRI_NUM            4
#define VSF_ARCH_PRI_BIT            2

// software interrupt provided by a dedicated device
#define VSF_DEV_SWI_NUM             3

#else

#ifndef __HAL_DEVICE_MIC_MT006_H__
#define __HAL_DEVICE_MIC_MT006_H__

// software interrupt provided by a dedicated device
#define SWI0_IRQHandler                                 CRS_IRQHandler
#define SWI1_IRQHandler                                 BOD_IRQHandler
#define SWI2_IRQHandler                                 BOR_IRQHandler
#define VSF_DEV_SWI_LIST                                CRS_IRQn, BOD_IRQn, BOR_IRQn

/*============================ INCLUDES ======================================*/

#include "./common.h"
#include "utilities/compiler/compiler.h"
#include "utilities/vsf_utilities.h"

/*============================ MACROS ========================================*/

#if     defined(__MT006__)
#define USB_OTG_COUNT               1
#define USB_OTG0_IRQHandler         USB_IRQHandler
#define USB_OTG0_EP_NUMBER          4

#define USB_OTG0_CONFIG                                                         \
    .reg                = (void *)USB_Common_Base,                              \
    .irq                = USB_IRQn,
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

#ifndef CHIP_CLKEN
#WARNING "Using Default Chip CLK Config"
#define MT006_CLKEN_LSI25K              (1UL << 0)
#define MT006_CLKEN_HSI12M              (1UL << 1)
#define MT006_CLKEN_LSE                 (1UL << 2)
#define MT006_CLKEN_HSE                 (1UL << 3)
#define MT006_CLKEN_PLL                 (1UL << 4)

#define MT006_HCLKSRC_HSI12M            0
#define MT006_HCLKSRC_PLL               1
#define MT006_HCLKSRC_HSE               2
#define MT006_HCLKSRC_PLSI25K           3
#define MT006_HCLKSRC_LSE               4

#define MT006_PLLSRC_HSI12M             0
#define MT006_PLLSRC_HSE                1

#define CHIP_CLKEN                      (MT006_CLKEN_HSI12M | MT006_CLKEN_PLL)
#define CHIP_HCLKSRC                    MT006_HCLKSRC_PLL
#define CHIP_PLLSRC                     MT006_PLLSRC_HSI12M
#define CHIP_LSE_FREQ_HZ                (32768)
#define CHIP_HSE_FREQ_HZ                (12 * 1000 * 1000)
#define CHIP_PLL_FREQ_HZ                (96 * 1000 * 1000)
#define CHIP_MAINCLK_FREQ_HZ            (CHIP_PLL_FREQ_HZ)
#define CHIP_AHB_APB_FREQ_HZ            (CHIP_MAINCLK_FREQ_HZ)
#endif  // CHIP_CLKEN

struct vsfhal_clk_info_t {
    uint32_t clken;

    uint8_t hclksrc;
    uint8_t pllsrc;

    uint32_t lse_freq_hz;
    uint32_t hse_freq_hz;
    uint32_t pll_freq_hz;

    uint32_t mainclk_freq_hz;
    uint32_t ahb_apb_freq_hz;
};
typedef struct vsfhal_clk_info_t vsfhal_clk_info_t;

typedef void(*callback_param_t)(void *param);

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

uint32_t vsfhal_tickcnt_get_us(void);
uint64_t vsfhal_tickcnt_get_us_64(void);
uint32_t vsfhal_tickcnt_get_ms(void);
uint64_t vsfhal_tickcnt_get_ms_64(void);

const vsfhal_clk_info_t *vsfhal_clk_info_get(void);
void vsfhal_core_reset(void *p);
void vsfhal_core_delay(uint32_t t);

#endif      // __HAL_DEVICE_MIC_MT006_H__
#endif      // __VSF_HEADER_ONLY_SHOW_ARCH_INFO__
/* EOF */
