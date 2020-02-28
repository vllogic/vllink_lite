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

/*============================ INCLUDES ======================================*/
#include "io.h"

/*============================ MACROS ========================================*/

#ifndef VSF_HAL_CFG_GPIO_PROTECT_LEVEL
#   ifndef VSF_HAL_CFG_PROTECT_LEVEL
#       define VSF_HAL_CFG_GPIO_PROTECT_LEVEL   interrupt
#   else
#       define VSF_HAL_CFG_GPIO_PROTECT_LEVEL   VSF_HAL_CFG_PROTECT_LEVEL
#   endif
#endif

#define vsfhal_gpio_protect                        vsf_protect(VSF_HAL_CFG_GPIO_PROTECT_LEVEL)
#define vsfhal_gpio_unprotect                      vsf_unprotect(VSF_HAL_CFG_GPIO_PROTECT_LEVEL)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef struct
{
  __IO uint32_t MODER;    /*!< GPIO port mode register,               Address offset: 0x00      */
  __IO uint32_t OTYPER;   /*!< GPIO port output type register,        Address offset: 0x04      */
  __IO uint32_t OSPEEDR;  /*!< GPIO port output speed register,       Address offset: 0x08      */
  __IO uint32_t PUPDR;    /*!< GPIO port pull-up/pull-down register,  Address offset: 0x0C      */
  __IO uint32_t IDR;      /*!< GPIO port input data register,         Address offset: 0x10      */
  __IO uint32_t ODR;      /*!< GPIO port output data register,        Address offset: 0x14      */
  __IO uint32_t BSRR;     /*!< GPIO port bit set/reset register,      Address offset: 0x18      */
  __IO uint32_t LCKR;     /*!< GPIO port configuration lock register, Address offset: 0x1C      */
  __IO uint32_t AFR[2];   /*!< GPIO alternate function registers,     Address offset: 0x20-0x24 */
  __IO uint32_t CLEAR;
  __IO uint32_t TOGGLE;
  __IO uint32_t SPEED2;
} GPIO_TypeDef;

#define GPIOA               ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB               ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC               ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD               ((GPIO_TypeDef *) GPIOD_BASE)
#define GPIOF               ((GPIO_TypeDef *) GPIOF_BASE)

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

#if GPIO_PORT_COUNT > 0

static GPIO_TypeDef * const gpio_reg_table[GPIO_PORT_COUNT] = {
    #if GPIOA_ENABLE
    GPIOA,
    #endif
    #if GPIOB_ENABLE
    GPIOB,
    #endif
    #if GPIOC_ENABLE
    GPIOC,
    #endif
    #if GPIOD_ENABLE
    GPIOD,
    #endif
    #if GPIOF_ENABLE
    GPIOF,
    #endif
};


void vsfhal_gpio_init(enum gpio_port_t port)
{
    if (port == GPIO_DUMMY_PORT)
        return;
    VSF_HAL_ASSERT(port < GPIO_PORT_COUNT);

    uint_fast32_t hw_idx = ((uint32_t)gpio_reg_table[port] - (uint32_t)GPIOA) / 0x400ul;
    RCU_AHBEN |= RCU_AHBEN_PAEN << hw_idx;
}

void vsfhal_gpio_fini(enum gpio_port_t port)
{
    if (port == GPIO_DUMMY_PORT)
        return;
    VSF_HAL_ASSERT(port < GPIO_PORT_COUNT);

    uint_fast32_t hw_idx = ((uint32_t)gpio_reg_table[port] - (uint32_t)GPIOA) / 0x400ul;
    RCU_AHBEN &= ~(RCU_AHBEN_PAEN << hw_idx);
}

void vsfhal_gpio_config(enum gpio_port_t port, uint32_t pin_mask, uint32_t config)
{
    if (port == GPIO_DUMMY_PORT)
        return;
    VSF_HAL_ASSERT(port < GPIO_PORT_COUNT);
    VSF_HAL_ASSERT(!(pin_mask >> 16));

    GPIO_TypeDef *reg = gpio_reg_table[port];
    uint_fast32_t mode_reg = 0, out_type_reg = 0, speed_reg = 0,
        pull_reg = 0, af_reg0 = 0, af_reg1 = 0;
    uint_fast32_t mask1 = 0, mask2 = 0, mask_af0 = 0, mask_af1 = 0;
    uint_fast8_t offset = 32, tmp8;

    uint_fast8_t mode = (config >> 0) & 0x03;
    uint_fast8_t out_type = (config >> 2) & 0x01;
    uint_fast8_t speed = (config >> 3) & 0x03;
    uint_fast8_t pull = (config >> 5) & 0x03;
    uint_fast8_t af = (config >> 7) & 0x0f;

    while (pin_mask) {
        tmp8 = __CLZ(pin_mask) + 1;
        pin_mask <<= tmp8;
        offset -= tmp8;

        mode_reg |= mode << (offset * 2);
        out_type_reg |= out_type << offset;
        speed_reg |= speed << (offset * 2);
        pull_reg |= pull << (offset * 2);
        if (offset < 8) {
            af_reg0 |= af << (offset * 4);
            mask_af0 |= 0xf << (offset * 4);
        } else {
            af_reg1 |= af << ((offset - 8) * 4);
            mask_af1 |= 0xf << (offset * 4);
        }
        mask1 |= 0x1 << offset;
        mask2 |= 0x3 << (offset * 2);
    }
    
    vsf_protect_t state = vsfhal_gpio_protect();
	reg->MODER  = (reg->MODER & ~mask2) | mode_reg;
	reg->OTYPER  = (reg->OTYPER & ~mask1) | out_type_reg;
	reg->OSPEEDR  = (reg->OSPEEDR & ~mask2) | speed_reg;
	reg->PUPDR  = (reg->PUPDR & ~mask2) | pull_reg;
	reg->AFR[0]  = (reg->AFR[0] & ~mask_af0) | af_reg0;
	reg->AFR[1]  = (reg->AFR[1] & ~mask_af1) | af_reg1;
    vsfhal_gpio_unprotect(state);
}

uint32_t vsfhal_gpio_read(enum gpio_port_t port, uint32_t pin_mask)
{
    if (port == GPIO_DUMMY_PORT)
        return 0;
    VSF_HAL_ASSERT(port < GPIO_PORT_COUNT);
    VSF_HAL_ASSERT(!(pin_mask >> 16));

    GPIO_TypeDef *reg = gpio_reg_table[port];
    return reg->IDR & pin_mask;
}

void vsfhal_gpio_write(enum gpio_port_t port, uint32_t pin_value, uint32_t pin_mask)
{
    if (port == GPIO_DUMMY_PORT)
        return;
    VSF_HAL_ASSERT(port < GPIO_PORT_COUNT);
    VSF_HAL_ASSERT(!(pin_mask >> 16));

    GPIO_TypeDef *reg = gpio_reg_table[port];
    uint_fast32_t set_mask, reset_mask;

    set_mask = pin_value & pin_mask;
    reset_mask = ~pin_value & pin_mask;

    reg->BSRR = (reset_mask << 16) | set_mask;
}

void vsfhal_gpio_set(enum gpio_port_t port, uint32_t pin_mask)
{
    vsfhal_gpio_write(port, pin_mask, pin_mask);
}

void vsfhal_gpio_clear(enum gpio_port_t port, uint32_t pin_mask)
{
    vsfhal_gpio_write(port, 0, pin_mask);
}

#endif
