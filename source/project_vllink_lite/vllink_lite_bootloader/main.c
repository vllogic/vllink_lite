/*============================ INCLUDES ======================================*/

#include "vsf.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

int main(void)
{
    vsfhal_gpio_init(GPIOA_IDX);
    vsfhal_gpio_config(GPIOA_IDX, 0x1 << 10, IO_INPUT_PULL_UP);
    vsfhal_gpio_init(GPIOA_IDX);
    vsfhal_gpio_config(GPIOA_IDX, 0x1 << 6, IO_OUTPUT_OD);
    vsfhal_gpio_init(GPIOA_IDX);
    vsfhal_gpio_config(GPIOA_IDX, 0x1 << 7, IO_OUTPUT_OD);
    vsfhal_gpio_init(GPIOB_IDX);
    vsfhal_gpio_config(GPIOB_IDX, 0x1 << 0, IO_OUTPUT_OD);
    vsfhal_gpio_set(GPIOA_IDX, 0x1 << 6);
    vsfhal_gpio_set(GPIOA_IDX, 0x1 << 7);
    vsfhal_gpio_set(GPIOB_IDX, 0x1 << 0);

    
    return 0;
}

/* EOF */
