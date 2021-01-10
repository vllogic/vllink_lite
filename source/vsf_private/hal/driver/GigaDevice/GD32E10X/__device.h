/*============================ INCLUDES ======================================*/
#include "hal/vsf_hal_cfg.h"

#undef VSF_DEVICE_HEADER

#if     defined(__GD32E10X__)
#   define  VSF_DEVICE_HEADER       "./device.h"
#else
#   error No supported device found.
#endif

/* include specified device driver header file */
#include VSF_DEVICE_HEADER

#ifdef __VSF_HEADER_ONLY_SHOW_ARCH_INFO__
#   ifndef __CPU_ARM__
#       define __CPU_ARM__
#   endif
#else

#ifndef __HAL_DRIVER_GIGADEVICE_GD32E10X_DEVICE_H__
#define __HAL_DRIVER_GIGADEVICE_GD32E10X_DEVICE_H__

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

#endif      // __HAL_DRIVER_GIGADEVICE_GD32E10X_DEVICE_H__
#endif      // __VSF_HEADER_ONLY_SHOW_ARCH_INFO__
/* EOF */
