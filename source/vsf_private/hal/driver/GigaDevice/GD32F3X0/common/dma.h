#ifndef __HAL_DRIVER_GIGADEVICE_GD32F3X0_DMA_H__
#define __HAL_DRIVER_GIGADEVICE_GD32F3X0_DMA_H__

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"
#include "../__device.h"

/*============================ MACROS ========================================*/

#ifndef DMA0_ENABLE
#   define DMA0_ENABLE              0
#endif
#ifndef DMA_CHANNEL_COUNT
#   define DMA_CHANNEL_COUNT        7
#endif

#define DMA_COUNT                   (0 + DMA0_ENABLE)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/


#define DMA_CHxCTL(dmax, ch)                REG32((dmax) + 0x08U + 0x14U * (ch))
#define DMA_CHxCNT(dmax, ch)                REG32((dmax) + 0x0CU + 0x14U * (ch))
#define DMA_CHxPADDR(dmax, ch)              REG32((dmax) + 0x10U + 0x14U * (ch))
#define DMA_CHxMADDR(dmax, ch)              REG32((dmax) + 0x14U + 0x14U * (ch))

enum dma_idx_t {
    #if DMA0_ENABLE
    DMA0_IDX,
    #endif
    DMA_IDX_NUM,
    DMA_INVALID_IDX,
};

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ INCLUDES ======================================*/
/*============================ PROTOTYPES ====================================*/

void vsf_dma_config_channel(enum dma_idx_t idx, uint8_t channel, 
        callback_param_t callback, void *param);

#endif  // __HAL_DRIVER_GIGADEVICE_GD32F3X0_DMA_H__
/* EOF */
