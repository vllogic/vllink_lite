/*****************************************************************************
 *   Copyright(C)2009-2019 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

#ifndef __HAL_DRIVER_GIGADEVICE_GD32F3X0_USART_H__
#define __HAL_DRIVER_GIGADEVICE_GD32F3X0_USART_H__

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"
#include "../__device.h"

/*============================ MACROS ========================================*/

#define USART_COUNT         (0 + USART0_ENABLE + USART1_ENABLE)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

enum usart_idx_t {
    #if USART0_ENABLE
    USART0_IDX,
    #endif
    #if USART1_ENABLE
    USART1_IDX,
    #endif
};

enum usart_mode_t{
    USART_PARITY_NONE           = (0x0ul << 9),
    USART_PARITY_ODD            = (0x3ul << 9),
    USART_PARITY_EVEN           = (0x2ul << 9),
    USART_STOPBITS_0P5          = (0x1ul << (8 + 12)),
    USART_STOPBITS_1            = (0x0ul << (8 + 12)),
    USART_STOPBITS_1P5          = (0x3ul << (8 + 12)),
    USART_STOPBITS_2            = (0x2ul << (8 + 12)),
    USART_CTS                   = (0x3ul << (16 + 9)),
    USART_RTS                   = (0x2ul << (16 + 8)),
};

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ INCLUDES ======================================*/
/*============================ PROTOTYPES ====================================*/

void vsf_usart_init(enum usart_idx_t idx);
void vsf_usart_fini(enum usart_idx_t idx);
void vsf_usart_config(enum usart_idx_t idx, uint32_t baudrate, uint32_t mode);
void vsf_usart_config_cb(enum usart_idx_t idx, int32_t int_priority, void *p, void (*ontx)(void *), void (*onrx)(void *));
uint16_t vsfhal_usart_tx_bytes(enum usart_idx_t idx, uint8_t *data, uint16_t size);
uint16_t vsfhal_usart_tx_get_free_size(enum usart_idx_t idx);
uint16_t vsfhal_usart_rx_bytes(enum usart_idx_t idx, uint8_t *data, uint16_t size);
uint16_t vsfhal_usart_rx_get_data_size(enum usart_idx_t idx);

#endif
/* EOF */
