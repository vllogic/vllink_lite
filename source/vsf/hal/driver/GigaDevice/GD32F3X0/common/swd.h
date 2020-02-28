#ifndef __HAL_DRIVER_GIGADEVICE_GD32F3X0_SWD_H__
#define __HAL_DRIVER_GIGADEVICE_GD32F3X0_SWD_H__

#include "hal/vsf_hal_cfg.h"
#include "../__device.h"

void vsfhal_swd_init(int32_t int_priority);
void vsfhal_swd_fini(void);
void vsfhal_swd_config(uint16_t kHz, uint8_t idle, uint8_t trn,
		bool data_force, uint16_t retry);
void vsfhal_swd_seqout(uint8_t *data, uint16_t bitlen);
void vsfhal_swd_seqin(uint8_t *data, uint16_t bitlen);
int vsfhal_swd_read(uint8_t request, uint32_t *r_data);
int vsfhal_swd_write(uint8_t request, uint32_t w_data);

#endif

