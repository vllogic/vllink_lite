#ifndef __HAL_DRIVER_MIC_MT006_FLASH_H__
#define __HAL_DRIVER_MIC_MT006_FLASH_H__

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"
#include "__device.h"

/*============================ MACROS ========================================*/

#ifndef FLASH0_ENABLE
#   define FLASH0_ENABLE        0
#endif

#define FLASH_COUNT             (0 + FLASH0_ENABLE)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

#if FLASH_COUNT
enum flash_idx_t {
    #if FLASH0_ENABLE
    FLASH0_IDX,
    #endif
    FLASH_IDX_NUM,
    FLASH_INVALID_IDX,
};
#endif

enum flash_config_t {
    FLASH_SECURITY_NULL             = 0,
    FLASH_SECURITY_LOW              = 1,
    FLASH_SECURITY_HIGH             = 2,
};

enum flash_op_t {
    FLASH_READ,
    FLASH_WRITE,
    FLASH_ERASE,
};

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ INCLUDES ======================================*/
/*============================ PROTOTYPES ====================================*/

#if FLASH_COUNT
vsf_err_t vsfhal_flash_security_config(enum flash_idx_t idx, uint32_t config);
uint32_t vsfhal_flash_opsize(enum flash_idx_t idx, uint32_t addr, enum flash_op_t op);
uint32_t vsfhal_flash_read(enum flash_idx_t idx, uint32_t addr, uint32_t size, uint8_t *buff);
uint32_t vsfhal_flash_write(enum flash_idx_t idx, uint32_t addr, uint32_t size, uint8_t *buff);
uint32_t vsfhal_flash_erase(enum flash_idx_t idx, uint32_t addr, uint32_t size);
#endif

#endif
/* EOF */
