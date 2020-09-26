#ifndef __BRD_CFG_H__
#define __BRD_CFG_H__

#if defined(BRD_CFG_VLLINKLITE_GD32E103)
#   define PROJ_CFG_GD32E10X_HSI48M_USB_PLL_128M_OVERCLOCK
//#   define PROJ_CFG_GD32E10X_AHP_APB_UNFIXED
#	include "chip_cfg_gd32e10x.h"
#   include "brd_cfg_vllinklite_gd32e103.h"
#elif defined(BRD_CFG_VLLINKLITE_GD32F350)
#   define PROJ_CFG_GD32F3X0_HSI48M_USB_PLL_128M_OVERCLOCK
//#   define PROJ_CFG_GD32F3X0_AHP_APB_UNFIXED
#	include "chip_cfg_gd32f3x0.h"
#   include "brd_cfg_vllinklite_gd32f350.h"
#endif

#define VSF_SYSTIMER_FREQ                   (CHIP_AHB_FREQ_HZ)

#endif // __BRD_CFG_H__
