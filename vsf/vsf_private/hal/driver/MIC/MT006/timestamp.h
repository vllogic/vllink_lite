#ifndef __HAL_DRIVER_GIGADEVICE_GD32F3X0_TIMESTAMP_H__
#define __HAL_DRIVER_GIGADEVICE_GD32F3X0_TIMESTAMP_H__

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"
#include "__device.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ INCLUDES ======================================*/
/*============================ PROTOTYPES ====================================*/

void vsfhal_timestamp_init(uint32_t timestamp, int32_t int_priority);
uint32_t vsfhal_timestamp_get(void);

#endif
/* EOF */
