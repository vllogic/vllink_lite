#ifndef __HAL_DRIVER_GIGADEVICE_GD32E10X_SWD_H__
#define __HAL_DRIVER_GIGADEVICE_GD32E10X_SWD_H__

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"
#include "__device.h"

/*============================ MACROS ========================================*/

#ifndef SWD0_ENABLE
#   define SWD0_ENABLE             0
#endif

#define SWD_COUNT                  (0 + SWD0_ENABLE)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ INCLUDES ======================================*/
/*============================ PROTOTYPES ====================================*/

#if SWD_COUNT
void vsfhal_swd_init(int32_t int_priority);
void vsfhal_swd_fini(void);
void vsfhal_swd_io_reconfig(void);
void vsfhal_swd_config(uint16_t kHz, uint16_t retry, uint8_t idle, uint8_t trn, bool data_force);
void vsfhal_swd_seqout(uint8_t *data, uint32_t bitlen);
void vsfhal_swd_seqin(uint8_t *data, uint32_t bitlen);
uint32_t vsfhal_swd_read(uint32_t request, uint8_t *r_data);
uint32_t vsfhal_swd_write(uint32_t request, uint8_t *w_data);
uint32_t vsfhal_swd_wait(void);
void vsfhal_swd_clear(void);
#if TIMESTAMP_CLOCK
uint32_t vsfhal_swd_get_timestamp(void);
#endif
#endif

#endif
/* EOF */
