#ifndef __HAL_DRIVER_GIGADEVICE_GD32F3X0_SPI_H__
#define __HAL_DRIVER_GIGADEVICE_GD32F3X0_SPI_H__

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"
#include "../__device.h"

/*============================ MACROS ========================================*/

#ifndef SPI0_ENABLE
#   define SPI0_ENABLE              0
#endif
#ifndef SPI1_ENABLE
#   define SPI1_ENABLE              0
#endif

#define SPI_COUNT                   (0 + SPI0_ENABLE + SPI1_ENABLE)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

enum spi_idx_t {
    #if SPI0_ENABLE
    SPI0_IDX,
    #endif
    #if SPI1_ENABLE
    SPI1_IDX,
    #endif
    SPI_IDX_NUM,
    SPI_INVALID_IDX,
};

enum spi_config_t {
    SPI_CFG_SLAVE                       = (0x0ul << 2),
    SPI_CFG_MASTER                      = (0x1ul << 2),
    SPI_CFG_CKPL_0                      = (0x0ul << 1),
    SPI_CFG_CKPL_1                      = (0x1ul << 1),
    SPI_CFG_CKPH_0                      = (0x0ul << 0),
    SPI_CFG_CKPH_1                      = (0x1ul << 0),
    SPI_CFG_MSB                         = (0x0ul << 7),
    SPI_CFG_LSB                         = (0x1ul << 7),
    SPI_CFG_WIDTH_8                     = (0x0ul << 11),
    SPI_CFG_WIDTH_16                    = (0x1ul << 11),
};

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ INCLUDES ======================================*/
/*============================ PROTOTYPES ====================================*/

#if SPI_COUNT
void vsfhal_spi_init(enum spi_idx_t idx);
void vsfhal_spi_fini(enum spi_idx_t idx);
void vsfhal_spi_config(enum spi_idx_t idx, uint32_t kHz, uint32_t config);
void vsfhal_spi_config_cb(enum spi_idx_t idx, int32_t int_priority, void *p, void (*callback)(void *));
vsf_err_t vsfhal_spi_start(enum spi_idx_t idx, uint8_t *out, uint8_t *in, uint32_t len);
uint32_t vsfhal_spi_stop(enum spi_idx_t idx);
bool vsfhal_spi_is_busy(enum spi_idx_t idx);
#endif

#endif
/* EOF */
