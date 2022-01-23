/*============================ INCLUDES ======================================*/
#include "io.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

#if GPIO_COUNT > 0

void vsfhal_gpio_config(enum gpio_idx_t idx, uint32_t pin_mask, uint32_t config)
{
    if (idx == GPIO_INVALID_IDX)
        return;
    VSF_HAL_ASSERT(idx < GPIO_IDX_NUM);
    VSF_HAL_ASSERT(!(pin_mask >> 8));

    IOCON_TypeDef *IOCON;
    uint_fast16_t con = config & 0xffff;
    uint_fast8_t offset = 32, tmp8, out = (config >> 16) & 0x1;

    while (pin_mask) {
        tmp8 = __CLZ(pin_mask) + 1;
        pin_mask <<= tmp8;
        offset -= tmp8;
        
        IOCON = (IOCON_TypeDef *)(IOCON_BASE + (idx * 8 + offset) * 4);
        IOCON->CON = con;

        if (out) {
            GPIOBANK0->DIR_SET = 0x1ul << (idx * 8 + offset);
        } else {
            GPIOBANK0->DIR_CLR = 0x1ul << (idx * 8 + offset);
        }
    }
}

uint32_t vsfhal_gpio_read(enum gpio_idx_t idx, uint32_t pin_mask)
{
    if (idx == GPIO_INVALID_IDX)
        return 0;
    VSF_HAL_ASSERT(idx < GPIO_IDX_NUM);
    VSF_HAL_ASSERT(!(pin_mask >> 8));

    return (GPIODATA0->DT >> (idx * 8)) & pin_mask;
}

void vsfhal_gpio_write(enum gpio_idx_t idx, uint32_t pin_value, uint32_t pin_mask)
{
    if (idx == GPIO_INVALID_IDX)
        return;
    VSF_HAL_ASSERT(idx < GPIO_IDX_NUM);
    VSF_HAL_ASSERT(!(pin_mask >> 8));

    uint_fast32_t set_mask = pin_value & pin_mask;
    uint_fast32_t reset_mask = ~pin_value & pin_mask;

    GPIODATA0->DT_SET = set_mask << (idx * 8);
    GPIODATA0->DT_CLR = reset_mask << (idx * 8);
}

void vsfhal_gpio_set(enum gpio_idx_t idx, uint32_t pin_mask)
{
    vsfhal_gpio_write(idx, pin_mask, pin_mask);
}

void vsfhal_gpio_clear(enum gpio_idx_t idx, uint32_t pin_mask)
{
    vsfhal_gpio_write(idx, 0, pin_mask);
}

#endif
