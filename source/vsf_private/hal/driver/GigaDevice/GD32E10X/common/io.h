#ifndef __HAL_DRIVER_GIGADEVICE_GD32E10X_IO_H__
#define __HAL_DRIVER_GIGADEVICE_GD32E10X_IO_H__

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
#ifndef GPIOE_ENABLE
#   define GPIOE_ENABLE             0
#endif

#define GPIO_COUNT                  (0 + GPIOA_ENABLE + GPIOB_ENABLE + GPIOC_ENABLE +   \
                                        GPIOD_ENABLE + GPIOE_ENABLE)

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
    #if GPIOE_ENABLE
    GPIOE_IDX = 4,
    #endif
    GPIOE_IDX_NUM,
    GPIO_INVALID_IDX,
};

enum gpio_mode_t{
    IO_INPUT                = (0x0 << 0),
    IO_OUTPUT_10M           = (0x1 << 0),
    IO_OUTPUT_2M            = (0x2 << 0),
    IO_OUTPUT_50M           = (0x3 << 0),

    IO_ANALOG_IN            = (0x0 << 2),
    IO_FLOAT_IN             = (0x1 << 2),
    IO_PULL_IN              = (0x2 << 2),
    IO_PP_OUT               = (0x0 << 2),
    IO_OD_OUT               = (0x1 << 2),
    IO_AFPP_OUT             = (0x2 << 2),
    IO_AFOD_OUT             = (0x3 << 2),

    IO_OUTPUT_PP            = IO_OUTPUT_10M | IO_PP_OUT,
    IO_OUTPUT_OD            = IO_OUTPUT_10M | IO_OD_OUT,
    IO_OUTPUT_AFPP          = IO_OUTPUT_10M | IO_AFPP_OUT,
    IO_OUTPUT_AFOD          = IO_OUTPUT_10M | IO_AFOD_OUT,

    IO_PULL_DOWN            = (0x0 << 4),
    IO_PULL_UP              = (0x1 << 4),

    IO_INPUT_ANALOG         = IO_INPUT | IO_ANALOG_IN,
    IO_INPUT_FLOAT          = IO_INPUT | IO_FLOAT_IN,
    IO_INPUT_PULL_DOWN      = IO_INPUT | IO_PULL_IN | IO_PULL_DOWN,
    IO_INPUT_PULL_UP        = IO_INPUT | IO_PULL_IN | IO_PULL_UP,

    IO_OUTPUT_SPEED_HIGH    = (0x1 << 5),
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
