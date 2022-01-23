#ifndef __BRD_CFG_H__
#define __BRD_CFG_H__

#if defined(BRD_CFG_VLLINKLITE_GD32E103)
#	include "chip_cfg_gd32e10x.h"
#   include "brd_cfg_vllinklite_gd32e103.h"
#elif defined(BRD_CFG_VLLINKLITE_GD32F350)
#	include "chip_cfg_gd32f3x0.h"
#   include "brd_cfg_vllinklite_gd32f350.h"
#elif defined(BRD_CFG_VLLINKLITE_MT006)
#	include "chip_cfg_mt006.h"
#   include "brd_cfg_vllinklite_mt006.h"
#endif

#endif // __BRD_CFG_H__


