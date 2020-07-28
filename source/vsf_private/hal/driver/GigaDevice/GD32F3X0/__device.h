#ifndef __HAL_DRIVER_GIGADEVICE_GD32F3X0_DEVICE_H__
#define __HAL_DRIVER_GIGADEVICE_GD32F3X0_DEVICE_H__

/*============================ INCLUDES ======================================*/
#include "hal/vsf_hal_cfg.h"

#undef VSF_DEVICE_HEADER

#if     defined(__GD32F330__) || defined(__GD32F350__)
#   define  VSF_DEVICE_HEADER       "./device.h"
#else
#   error No supported device found.
#endif

/* include specified device driver header file */
#include VSF_DEVICE_HEADER

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/


#endif
/* EOF */
