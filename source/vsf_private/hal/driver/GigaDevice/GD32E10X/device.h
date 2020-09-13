#ifndef __HAL_DEVICE_GIGADEVICE_GD32E10X_H__
#define __HAL_DEVICE_GIGADEVICE_GD32E10X_H__

/*============================ INCLUDES ======================================*/
#include "hal/vsf_hal_cfg.h"

/*============================ MACROS ========================================*/

/*\note first define basic info for arch. */
#define VSF_ARCH_PRI_NUM            16
#define VSF_ARCH_PRI_BIT            4

// software interrupt provided by a dedicated device
#define VSF_DEV_SWI_NUM             5
#define VSF_DEV_SWI_LIST            SWI0_IRQn,SWI1_IRQn,SWI2_IRQn,SWI3_IRQn,SWI4_IRQn

/*============================ INCLUDES ======================================*/

/*\note this is should be the only place where __common.h is included.*/
#include "./common/__common.h"

/*============================ MACROS ========================================*/

#if     defined(__GD32E10X__)
#define USB_OTG_COUNT               1
#define USB_OTG0_IRQHandler         USBFS_IRQHandler
// required by dwcotg, define the max ep number of dwcotg
#define USB_DWCOTG_MAX_EP_NUM       4

#define USB_OTG0_CONFIG                                                         \
    .ep_num = 8,                                                                \
    .irq = USBFS_IRQn,                                                          \
    .reg = (void *)USBFS_BASE,                                                  \
    .buffer_word_size = 0x500 >> 2,                                             \
    .speed = USB_SPEED_FULL,                                                    \
	.dma_en = false,                                                            \
	.ulpi_en = false,                                                           \
	.utmi_en = false,                                                           \
	.vbus_en = false,
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

#ifndef CHIP_CLKEN
#WARNING "Using Default Chip CLK Config"
#define GD32E10X_CLKEN_LSI              (1UL << 0)
#define GD32E10X_CLKEN_HSI              (1UL << 1)
#define GD32E10X_CLKEN_HSI48M           (1UL << 2)
#define GD32E10X_CLKEN_LSE              (1UL << 3)
#define GD32E10X_CLKEN_HSE              (1UL << 4)
#define GD32E10X_CLKEN_PLL0             (1UL << 5)
#define GD32E10X_CLKEN_PLL1             (1UL << 6)
#define GD32E10X_CLKEN_PLL2             (1UL << 7)

#define GD32E10X_HCLKSRC_HSI8M          0
#define GD32E10X_HCLKSRC_HSE            1
#define GD32E10X_HCLKSRC_PLL            2

#define GD32E10X_PLLSRC_HSI8M_D2        0
#define GD32E10X_PLLSRC_HSE             1
#define GD32E10X_PLLSRC_HSI48M          2

#define GD32E10X_USBSRC_PLL             0
#define GD32E10X_USBSRC_HSI48M          1


#define CHIP_CLKEN                      (GD32E10X_CLKEN_HSI48M | GD32E10X_CLKEN_PLL0)
#define CHIP_HCLKSRC                    GD32E10X_HCLKSRC_PLL
#define CHIP_PLLSRC                     GD32E10X_PLLSRC_HSI48M
#define CHIP_USBSRC                     GD32E10X_USBSRC_HSI48M
#define CHIP_LSE_FREQ_HZ                (32768)
#define CHIP_HSE_FREQ_HZ                (12 * 1000 * 1000)
#define CHIP_PLL_FREQ_HZ                (120 * 1000 * 1000)
#define CHIP_AHB_FREQ_HZ                (CHIP_PLL_FREQ_HZ)
#define CHIP_APB1_FREQ_HZ               (CHIP_AHB_FREQ_HZ / 2)
#define CHIP_APB2_FREQ_HZ               (CHIP_AHB_FREQ_HZ / 2)
#endif  // CHIP_CLKEN

struct vsfhal_clk_info_t {
	uint32_t clken;
	
    uint8_t hclksrc;
    uint8_t pllsrc;
    uint8_t usbsrc;
	
	uint32_t lse_freq_hz;
	uint32_t hse_freq_hz;
	uint32_t pll_freq_hz;

	uint32_t ahb_freq_hz;
	uint32_t apb1_freq_hz;
	uint32_t apb2_freq_hz;
};
typedef struct vsfhal_clk_info_t vsfhal_clk_info_t;

typedef void(*callback_param_t)(void *param);

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
extern vsfhal_clk_info_t *vsfhal_clk_info_get(void);
extern uint32_t vsfhal_uid_read(uint8_t *buffer, uint32_t size);

#endif
/* EOF */
