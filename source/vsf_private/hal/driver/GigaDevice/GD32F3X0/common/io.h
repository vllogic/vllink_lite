#ifndef __HAL_DRIVER_GIGADEVICE_GD32F3X0_IO_H__
#define __HAL_DRIVER_GIGADEVICE_GD32F3X0_IO_H__

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"
#include "../__device.h"

/*============================ MACROS ========================================*/

#ifndef GPIOA_ENABLE
#   define GPIOA_ENABLE             0
#endif
#ifndef GPIOB_ENABLE
#   define GPIOB_ENABLE             0
#endif
#ifndef GPIOC_ENABLE
#   define GPIOC_ENABLE             0
#endif
#ifndef GPIOD_ENABLE
#   define GPIOD_ENABLE             0
#endif
#ifndef GPIOF_ENABLE
#   define GPIOF_ENABLE             0
#endif

#define GPIO_COUNT                  (0 + GPIOA_ENABLE + GPIOB_ENABLE + GPIOC_ENABLE +   \
                                        GPIOD_ENABLE + GPIOF_ENABLE)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

enum gpio_idx_t {
    #if GPIOA_ENABLE
    GPIOA_IDX = 0,
    #endif
    #if GPIOB_ENABLE
    GPIOB_IDX = 1,
    #endif
    #if GPIOC_ENABLE
    GPIOC_IDX = 2,
    #endif
    #if GPIOD_ENABLE
    GPIOD_IDX = 3,
    #endif
    #if GPIOF_ENABLE
    GPIOF_IDX = 5,
    #endif
    GPIO_IDX_NUM,
    GPIO_INVALID_IDX,
};

enum gpio_mode_t{
    IO_INPUT                = (0x0 << 0),
    IO_OUTPUT               = (0x1 << 0),
    IO_AF                   = (0x2 << 0),
    IO_ANALOG               = (0x3 << 0),
    
    IO_PP_OUT               = (0x0 << 2),
    IO_OD_OUT               = (0x1 << 2),
    
    IO_SPEED_2M             = (0x0 << 3),
    IO_SPEED_10M            = (0x1 << 3),
    IO_SPEED_50M            = (0x3 << 3),
    
    IO_FLOAT_IN             = (0x0 << 5),
    IO_PULL_UP              = (0x1 << 5),
    IO_PULL_DOWN            = (0x2 << 5),
    
    IO_AF_0                 = (0x0 << 7),
    IO_AF_1                 = (0x1 << 7),
    IO_AF_2                 = (0x2 << 7),
    IO_AF_3                 = (0x3 << 7),
    IO_AF_4                 = (0x4 << 7),
    IO_AF_5                 = (0x5 << 7),
    IO_AF_6                 = (0x6 << 7),
    IO_AF_7                 = (0x7 << 7),
    
    IO_INPUT_ANALOG         = IO_ANALOG,
    IO_INPUT_FLOAT          = IO_INPUT | IO_FLOAT_IN,
    IO_INPUT_PULL_DOWN      = IO_INPUT | IO_PULL_DOWN,
    IO_INPUT_PULL_UP        = IO_INPUT | IO_PULL_UP,

    IO_OUTPUT_PP            = IO_OUTPUT | IO_PP_OUT,
    IO_OUTPUT_OD            = IO_OUTPUT | IO_OD_OUT,

};

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ INCLUDES ======================================*/
/*============================ PROTOTYPES ====================================*/

#if GPIO_COUNT
void vsfhal_gpio_init(enum gpio_idx_t idx);
void vsfhal_gpio_fini(enum gpio_idx_t idx);
void vsfhal_gpio_config(enum gpio_idx_t idx, uint32_t pin_mask, uint32_t config);
uint32_t vsfhal_gpio_read(enum gpio_idx_t idx, uint32_t pin_mask);
void vsfhal_gpio_write(enum gpio_idx_t idx, uint32_t pin_value, uint32_t pin_mask);
void vsfhal_gpio_set(enum gpio_idx_t idx, uint32_t pin_mask);
void vsfhal_gpio_clear(enum gpio_idx_t idx, uint32_t pin_mask);
#endif

#endif
/* EOF */
