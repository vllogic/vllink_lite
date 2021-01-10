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
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

#if GPIO_COUNT > 0

static const uint32_t gpio_reg_base_list[GPIO_COUNT] = {
    #if GPIOA_ENABLE
    GPIOA_BASE,
    #endif
    #if GPIOB_ENABLE
    GPIOB_BASE,
    #endif
    #if GPIOC_ENABLE
    GPIOC_BASE,
    #endif
    #if GPIOD_ENABLE
    GPIOD_BASE,
    #endif
    #if GPIOF_ENABLE
    GPIOF_BASE,
    #endif
};

void vsfhal_gpio_init(enum gpio_idx_t idx)
{
    if (idx == GPIO_INVALID_IDX)
        return;
    VSF_HAL_ASSERT(idx < GPIO_IDX_NUM);

    RCU_AHBEN |= RCU_AHBEN_PAEN << idx;
}

void vsfhal_gpio_fini(enum gpio_idx_t idx)
{
    if (idx == GPIO_INVALID_IDX)
        return;
    VSF_HAL_ASSERT(idx < GPIO_IDX_NUM);

    RCU_AHBEN &= ~(RCU_AHBEN_PAEN << idx);
}

void vsfhal_gpio_config(enum gpio_idx_t idx, uint32_t pin_mask, uint32_t config)
{
    if (idx == GPIO_INVALID_IDX)
        return;
    VSF_HAL_ASSERT(idx < GPIO_IDX_NUM);
    VSF_HAL_ASSERT(!(pin_mask >> 16));

    uint32_t gpiox = gpio_reg_base_list[idx];

    uint_fast8_t offset = 32, tmp8;
    uint_fast32_t ctl = config & 0x3;
    uint_fast32_t omode = (config >> 2) & 0x1;
    uint_fast32_t speed = (config >> 3) & 0x3;
    uint_fast32_t pull = (config >> 5) & 0x3;
    uint_fast32_t af = (config >> 7) & 0xf;
    
    while (pin_mask) {
        tmp8 = __CLZ(pin_mask) + 1;
        pin_mask <<= tmp8;
        offset -= tmp8;

        GPIO_CTL(gpiox) = (GPIO_CTL(gpiox) & ~(GPIO_CTL_CTL0 << (offset * 2))) | (ctl << (offset * 2));
        GPIO_OMODE(gpiox) = (GPIO_OMODE(gpiox) & ~(GPIO_OMODE_OM0 << offset)) | (omode << offset);
        GPIO_OSPD0(gpiox) = (GPIO_OSPD0(gpiox) & ~(GPIO_OSPD0_OSPD0 << (offset * 2))) | (speed << (offset * 2));
        GPIO_PUD(gpiox) = (GPIO_PUD(gpiox) & ~(GPIO_PUD_PUD0 << (offset * 2))) | (pull << (offset * 2));
        if (offset <= 8) {
            GPIO_AFSEL0(gpiox) = (GPIO_AFSEL0(gpiox) & ~(GPIO_AFSEL0_SEL0 << (offset * 4))) | (af << (offset * 4));
        } else {
            GPIO_AFSEL1(gpiox) = (GPIO_AFSEL1(gpiox) & ~(GPIO_AFSEL1_SEL8 << ((offset - 8) * 4))) | (af << ((offset - 8) * 4));
        }
    }
}

uint32_t vsfhal_gpio_read(enum gpio_idx_t idx, uint32_t pin_mask)
{
    if (idx == GPIO_INVALID_IDX)
        return 0;
    VSF_HAL_ASSERT(idx < GPIO_IDX_NUM);
    VSF_HAL_ASSERT(!(pin_mask >> 16));

    uint32_t gpiox = gpio_reg_base_list[idx];
    return GPIO_ISTAT(gpiox) & pin_mask;
}

void vsfhal_gpio_write(enum gpio_idx_t idx, uint32_t pin_value, uint32_t pin_mask)
{
    if (idx == GPIO_INVALID_IDX)
        return;
    VSF_HAL_ASSERT(idx < GPIO_IDX_NUM);
    VSF_HAL_ASSERT(!(pin_mask >> 16));

    uint32_t gpiox = gpio_reg_base_list[idx];
    uint_fast32_t set_mask, reset_mask;

    set_mask = pin_value & pin_mask;
    reset_mask = ~pin_value & pin_mask;

    GPIO_BOP(gpiox) = (reset_mask << 16) | set_mask;
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
