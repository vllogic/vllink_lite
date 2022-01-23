#ifndef __CHIP_CFG_XXX_H__
#define __CHIP_CFG_XXX_H__

#define __MIC__
#define __MT006__

#define VSF_DRIVER_HEADER               "../../../vsf_private/hal/driver/MIC/driver.h"

#define MT006_CLKEN_LSI25K              (1UL << 0)
#define MT006_CLKEN_HSI12M              (1UL << 1)
#define MT006_CLKEN_LSE                 (1UL << 2)
#define MT006_CLKEN_HSE                 (1UL << 3)
#define MT006_CLKEN_PLL                 (1UL << 4)
#define MT006_CLKEN_CRS_USBDSOF         (1UL << 5)

#define MT006_HCLKSRC_HSI12M            0
#define MT006_HCLKSRC_PLL               1
#define MT006_HCLKSRC_HSE               2
#define MT006_HCLKSRC_PLSI25K           3
#define MT006_HCLKSRC_LSE               4

#define MT006_PLLSRC_HSI12M             0
#define MT006_PLLSRC_HSE                1


#if defined(PROJ_CFG_MT006_HSI12M_PLL_96M)
#	define CHIP_CLKEN                      (MT006_CLKEN_HSI12M | MT006_CLKEN_PLL | MT006_CLKEN_CRS_USBDSOF)
#	define CHIP_HCLKSRC                    MT006_HCLKSRC_PLL
#	define CHIP_PLLSRC                     MT006_PLLSRC_HSI12M
#	define CHIP_LSE_FREQ_HZ                (32768)
#	define CHIP_HSE_FREQ_HZ                (12 * 1000 * 1000)
#	define CHIP_PLL_FREQ_HZ                (96 * 1000 * 1000)
#   define CHIP_MAINCLK_FREQ_HZ            (CHIP_PLL_FREQ_HZ)
#   define CHIP_AHB_APB_FREQ_HZ            (CHIP_MAINCLK_FREQ_HZ)
#elif defined(PROJ_CFG_MT006_HSI12M_PLL_48M)
#	define CHIP_CLKEN                      (MT006_CLKEN_HSI12M | MT006_CLKEN_PLL | MT006_CLKEN_CRS_USBDSOF)
#	define CHIP_HCLKSRC                    MT006_HCLKSRC_PLL
#	define CHIP_PLLSRC                     MT006_PLLSRC_HSI12M
#	define CHIP_LSE_FREQ_HZ                (32768)
#	define CHIP_HSE_FREQ_HZ                (12 * 1000 * 1000)
#	define CHIP_PLL_FREQ_HZ                (48 * 1000 * 1000)
#   define CHIP_MAINCLK_FREQ_HZ            (CHIP_PLL_FREQ_HZ)
#   define CHIP_AHB_APB_FREQ_HZ            (CHIP_MAINCLK_FREQ_HZ)
#elif defined(PROJ_CFG_MT006_HSI12M_NOPLL_12M)
#	define CHIP_CLKEN                      (MT006_CLKEN_HSI12M)
#	define CHIP_HCLKSRC                    MT006_HCLKSRC_HSI12M
#	define CHIP_PLLSRC                     MT006_PLLSRC_HSI12M
#	define CHIP_LSE_FREQ_HZ                (32768)
#	define CHIP_HSE_FREQ_HZ                (12 * 1000 * 1000)
#	define CHIP_PLL_FREQ_HZ                (12 * 1000 * 1000)
#   define CHIP_MAINCLK_FREQ_HZ            (12 * 1000 * 1000)
#   define CHIP_AHB_APB_FREQ_HZ            (CHIP_MAINCLK_FREQ_HZ)
#else
#   error "Need Config Clock"
#endif

#if (VSF_USE_USB_DEVICE == ENABLED) || (VSF_USE_USB_HOST == ENABLED)
#   if (VSF_USE_USB_DEVICE == ENABLED)
#       if (VSF_USBD_CFG_USE_EDA == DISABLED)
#           undef VSF_USBD_CFG_USE_EDA
#           define  VSF_USBD_CFG_USE_EDA                                ENABLED
#       endif
#       define VSF_USBD_USE_DCD_MUSB_FDRC       ENABLED
#   endif
#   if (VSF_USE_USB_HOST == ENABLED)
#       define VSF_USBH_USE_HCD_MUSB_FDRC       ENABLED
#   endif
#   if (VSF_KERNEL_CFG_ALLOW_KERNEL_BEING_PREEMPTED == DISABLED)
#       undef VSF_KERNEL_CFG_ALLOW_KERNEL_BEING_PREEMPTED
#       define  VSF_KERNEL_CFG_ALLOW_KERNEL_BEING_PREEMPTED         ENABLED
#   endif
#endif

#endif // __CHIP_CFG_XXX_H__