#ifndef __HAL_DRIVER_GIGADEVICE_GD32F3X0_DELAY_H__
#define __HAL_DRIVER_GIGADEVICE_GD32F3X0_DELAY_H__

#include "hal/vsf_hal_cfg.h"
#include "../__device.h"

void vsfhal_delay_jtag_125ns(uint16_t dummy);
void vsfhal_delay_jtag_250ns(uint16_t dummy);
void vsfhal_delay_swd_125ns(uint16_t dummy);
void vsfhal_delay_swd_250ns(uint16_t dummy);

#endif
