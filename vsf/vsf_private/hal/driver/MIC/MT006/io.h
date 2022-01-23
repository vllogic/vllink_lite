#ifndef __HAL_DRIVER_MIC_MT006_IO_H__
#define __HAL_DRIVER_MIC_MT006_IO_H__

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"
#include "__device.h"

/*============================ MACROS ========================================*/

#ifndef GPIO0_ENABLE
#   define GPIO0_ENABLE             0
#endif
#ifndef GPIO1_ENABLE
#   define GPIO1_ENABLE             0
#endif
#ifndef GPIO2_ENABLE
#   define GPIO2_ENABLE             0
#endif

#define GPIO_COUNT                  (0 + GPIO0_ENABLE + GPIO1_ENABLE + GPIO2_ENABLE)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

enum gpio_idx_t {
    #if GPIO0_ENABLE
    GPIO0_IDX = 0,
    #endif
    #if GPIO1_ENABLE
    GPIO1_IDX = 1,
    #endif
    #if GPIO2_ENABLE
    GPIO2_IDX = 2,
    #endif
    GPIO_IDX_NUM,
    GPIO_INVALID_IDX,
};

enum gpio_mode_t{
    IO_FUNC_GPIO            = (0x0 << 0),
    IO_FUNC_1               = (0x1 << 0),
    IO_FUNC_2               = (0x2 << 0),
    IO_FUNC_3               = (0x3 << 0),
    IO_FUNC_4               = (0x4 << 0),
    IO_FUNC_5               = (0x5 << 0),
    IO_FUNC_6               = (0x6 << 0),
    IO_FUNC_7               = (0x7 << 0),

    IO_FLOAT                = (0x0 << 3),
    IO_PULL_DOWN            = (0x1 << 3),
    IO_PULL_UP              = (0x2 << 3),

    IO_ANALOG_ONLY          = (0x1 << 5),

    IO_GPIO_IN              = (0x1 << 7),

    IO_GPIO_OUT_FAST        = (0x1 << 9),

    IO_GPIO_OD              = (0x1 << 12),
    IO_GPIO_OS              = (0x1 << 13),

    IO_GPIO_OUT             = (0x1 << 16),


    IO_INPUT_FLOAT          = (IO_FUNC_GPIO | IO_FLOAT | IO_GPIO_IN),
    IO_INPUT_PULL_DOWN      = (IO_FUNC_GPIO | IO_PULL_DOWN | IO_GPIO_IN),
    IO_INPUT_PULL_UP        = (IO_FUNC_GPIO | IO_PULL_UP | IO_GPIO_IN),
    IO_OUTPUT               = (IO_FUNC_GPIO | IO_GPIO_IN | IO_GPIO_OUT),
    IO_ANALOG               = (IO_FUNC_GPIO | IO_ANALOG_ONLY | IO_GPIO_IN),
    IO_SELECT_FUNC_1        = IO_FUNC_1,
    IO_SELECT_FUNC_2        = IO_FUNC_2,
    IO_SELECT_FUNC_3        = IO_FUNC_3,
    IO_SELECT_FUNC_4        = IO_FUNC_4,
    IO_SELECT_FUNC_5        = IO_FUNC_5,
    IO_SELECT_FUNC_6        = IO_FUNC_6,
    IO_SELECT_FUNC_7        = IO_FUNC_7,
};

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ INCLUDES ======================================*/
/*============================ PROTOTYPES ====================================*/

//void vsfhal_gpio_init(enum gpio_idx_t idx);
#define vsfhal_gpio_init(idx)
//void vsfhal_gpio_fini(enum gpio_idx_t idx);
#define vsfhal_gpio_init(idx)
void vsfhal_gpio_config(enum gpio_idx_t idx, uint32_t pin_mask, uint32_t config);
uint32_t vsfhal_gpio_read(enum gpio_idx_t idx, uint32_t pin_mask);
void vsfhal_gpio_write(enum gpio_idx_t idx, uint32_t pin_value, uint32_t pin_mask);
void vsfhal_gpio_set(enum gpio_idx_t idx, uint32_t pin_mask);
void vsfhal_gpio_clear(enum gpio_idx_t idx, uint32_t pin_mask);

#endif
/* EOF */
