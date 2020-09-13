#ifndef __HAL_DRIVER_GIGADEVICE_GD32F3X0_JTAG_H__
#define __HAL_DRIVER_GIGADEVICE_GD32F3X0_JTAG_H__

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"
#include "__device.h"

/*============================ MACROS ========================================*/

#ifndef JTAG0_ENABLE
#   define JTAG0_ENABLE             0
#endif

#define JTAG_COUNT                  (0 + JTAG0_ENABLE)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ INCLUDES ======================================*/
/*============================ PROTOTYPES ====================================*/

#if JTAG_COUNT
void vsfhal_jtag_init(int32_t int_priority);
void vsfhal_jtag_fini(void);
void vsfhal_jtag_config(uint16_t kHz, uint16_t retry);
void vsfhal_jtag_raw(uint16_t bitlen, uint8_t *tms, uint8_t *tdi, uint8_t *tdo);
void vsfhal_jtag_ir(uint32_t ir, uint8_t lr_length, uint16_t ir_before, uint16_t ir_after);
uint8_t vsfhal_jtag_dr(uint8_t request, uint32_t dr, uint16_t dr_before, uint16_t dr_after, uint32_t *data);
#endif

#endif
/* EOF */
