#ifndef __CHIP_CFG_XXX_H__
#define __CHIP_CFG_XXX_H__

#define __GigaDevice__
//#define __GD32F330__
#define __GD32F350__

#define VSF_DRIVER_HEADER               "../../../vsf_private/hal/driver/GigaDevice/driver.h"

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

#if defined(PROJ_CFG_GD32F3X0_HSI48M_USB_PLL_128M_OVERCLOCK)
#   define CHIP_CLKEN                      (GD32F3X0_CLKEN_HSI48M | GD32F3X0_CLKEN_PLL)
#   define CHIP_HCLKSRC                    GD32F3X0_HCLKSRC_PLL
#   define CHIP_PLLSRC                     GD32F3X0_PLLSRC_HSI48M
#   define CHIP_USBSRC                     GD32F3X0_USBSRC_HSI48M
#   define CHIP_LSE_FREQ_HZ                (32768)
#   define CHIP_HSE_FREQ_HZ                (12 * 1000 * 1000)
#   define CHIP_PLL_FREQ_HZ                (128 * 1000 * 1000)
#   define CHIP_AHB_FREQ_HZ                (CHIP_PLL_FREQ_HZ)
#   define CHIP_APB1_FREQ_HZ               (CHIP_AHB_FREQ_HZ / 2)
#   define CHIP_APB2_FREQ_HZ               (CHIP_AHB_FREQ_HZ / 2)
#elif defined(PROJ_CFG_GD32F3X0_HSI48M_USB_PLL_108M)
#   define CHIP_CLKEN                      (GD32F3X0_CLKEN_HSI48M | GD32F3X0_CLKEN_PLL)
#   define CHIP_HCLKSRC                    GD32F3X0_HCLKSRC_PLL
#   define CHIP_PLLSRC                     GD32F3X0_PLLSRC_HSI48M
#   define CHIP_USBSRC                     GD32F3X0_USBSRC_HSI48M
#   define CHIP_LSE_FREQ_HZ                (32768)
#   define CHIP_HSE_FREQ_HZ                (12 * 1000 * 1000)
#   define CHIP_PLL_FREQ_HZ                (108 * 1000 * 1000)
#   define CHIP_AHB_FREQ_HZ                (CHIP_PLL_FREQ_HZ)
#   define CHIP_APB1_FREQ_HZ               (CHIP_AHB_FREQ_HZ / 2)
#   define CHIP_APB2_FREQ_HZ               (CHIP_AHB_FREQ_HZ / 2)
#elif defined(PROJ_CFG_GD32F3X0_HSI48M_USB_PLL_72M)
#   define CHIP_CLKEN                      (GD32F3X0_CLKEN_HSI48M | GD32F3X0_CLKEN_PLL)
#   define CHIP_HCLKSRC                    GD32F3X0_HCLKSRC_PLL
#   define CHIP_PLLSRC                     GD32F3X0_PLLSRC_HSI48M
#   define CHIP_USBSRC                     GD32F3X0_USBSRC_HSI48M
#   define CHIP_LSE_FREQ_HZ                (32768)
#   define CHIP_HSE_FREQ_HZ                (12 * 1000 * 1000)
#   define CHIP_PLL_FREQ_HZ                (72 * 1000 * 1000)
#   define CHIP_AHB_FREQ_HZ                (CHIP_PLL_FREQ_HZ)
#   define CHIP_APB1_FREQ_HZ               (CHIP_AHB_FREQ_HZ / 2)
#   define CHIP_APB2_FREQ_HZ               (CHIP_AHB_FREQ_HZ / 2)
#endif


#endif // __CHIP_CFG_XXX_H__