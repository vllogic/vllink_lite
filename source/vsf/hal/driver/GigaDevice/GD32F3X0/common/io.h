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

#ifndef __HAL_DRIVER_GIGADEVICE_GD32F3X0_IO_H__
#define __HAL_DRIVER_GIGADEVICE_GD32F3X0_IO_H__

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"
#include "../__device.h"

/*============================ MACROS ========================================*/

#define GPIO_PORT_COUNT     (0 + GPIOA_ENABLE + GPIOB_ENABLE + GPIOC_ENABLE +   \
                                GPIOD_ENABLE + GPIOF_ENABLE)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

enum gpio_port_t {
    #if GPIOA_ENABLE
    GPIOA_PORT,
    #endif
    #if GPIOB_ENABLE
    GPIOB_PORT,
    #endif
    #if GPIOC_ENABLE
    GPIOC_PORT,
    #endif
    #if GPIOD_ENABLE
    GPIOD_PORT,
    #endif
    #if GPIOF_ENABLE
    GPIOF_PORT,
    #endif
    GPIO_DUMMY_PORT = 0xff,
};

enum gpio_mode_t{
    IO_INPUT                = (0x0 << 0),
    IO_OUTPUT               = (0x1 << 0),
    IO_AF                   = (0x2 << 0),
    IO_ANALOG               = (0x3 << 0),

    IO_OUTPUT_PP            = IO_OUTPUT | (0x0 << 2),
    IO_OUTPUT_OD            = IO_OUTPUT | (0x1 << 2),

    IO_SPEED_0              = 0x0 << 3,
    IO_SPEED_1              = 0x1 << 3,
    IO_SPEED_2              = 0x1 << 3,     // same as IO_SPEED_1
    IO_SPEED_3              = 0x3 << 3,

    IO_INPUT_FLOAT          = IO_INPUT | (0x0 << 5),
    IO_INPUT_PU             = IO_INPUT | (0x1 << 5),
    IO_INPUT_PD             = IO_INPUT | (0x2 << 5),

    IO_AF_0                 = IO_AF | (0x0 << 7),
    IO_AF_1                 = IO_AF | (0x1 << 7),
    IO_AF_2                 = IO_AF | (0x2 << 7),
    IO_AF_3                 = IO_AF | (0x3 << 7),
    IO_AF_4                 = IO_AF | (0x4 << 7),
    IO_AF_5                 = IO_AF | (0x5 << 7),
    IO_AF_6                 = IO_AF | (0x6 << 7),
    IO_AF_7                 = IO_AF | (0x7 << 7),
    IO_AF_8                 = IO_AF | (0x8 << 7),
    IO_AF_9                 = IO_AF | (0x9 << 7),
    IO_AF_10                = IO_AF | (0xa << 7),
    IO_AF_11                = IO_AF | (0xb << 7),
    IO_AF_12                = IO_AF | (0xc << 7),
    IO_AF_13                = IO_AF | (0xd << 7),
    IO_AF_14                = IO_AF | (0xe << 7),
    IO_AF_15                = IO_AF | (0xf << 7),
};

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ INCLUDES ======================================*/
/*============================ PROTOTYPES ====================================*/

void vsfhal_gpio_init(enum gpio_port_t port);
void vsfhal_gpio_fini(enum gpio_port_t port);
void vsfhal_gpio_config(enum gpio_port_t port, uint32_t pin_mask, uint32_t config);
uint32_t vsfhal_gpio_read(enum gpio_port_t port, uint32_t pin_mask);
void vsfhal_gpio_write(enum gpio_port_t port, uint32_t pin_value, uint32_t pin_mask);
void vsfhal_gpio_set(enum gpio_port_t port, uint32_t pin_mask);
void vsfhal_gpio_clear(enum gpio_port_t port, uint32_t pin_mask);

#endif
/* EOF */
