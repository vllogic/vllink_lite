/*============================ INCLUDES ======================================*/

#include "vsf.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

void usrapp_led_ctrl(bool led_r, bool led_g);
uint32_t usrapp_flash_erase_write(uint8_t *buf, uint32_t addr, uint32_t size);
uint32_t usrapp_flash_read(uint8_t *buf, uint32_t addr, uint32_t size);

/*============================ IMPLEMENTATION ================================*/

#include "usrapp_usbd_common.c"
#include "vsf_dfu.c"

void usrapp_led_ctrl(bool led_r, bool led_g)
{
#if PERIPHERAL_LED_RED_VALID_LVL
    if (led_r)
#else
    if (!led_r)
#endif
        vsfhal_gpio_set(PERIPHERAL_LED_RED_IDX, 0x1 << PERIPHERAL_LED_RED_PIN);
    else
        vsfhal_gpio_clear(PERIPHERAL_LED_RED_IDX, 0x1 << PERIPHERAL_LED_RED_PIN);

#if PERIPHERAL_LED_GREEN_VALID_LVL
    if (led_r)
#else
    if (!led_r)
#endif
        vsfhal_gpio_set(PERIPHERAL_LED_GREEN_IDX, 0x1 << PERIPHERAL_LED_GREEN_PIN);
    else
        vsfhal_gpio_clear(PERIPHERAL_LED_GREEN_IDX, 0x1 << PERIPHERAL_LED_GREEN_PIN);
}

uint32_t usrapp_flash_erase_write(uint8_t *buf, uint32_t addr, uint32_t size)
{
    if ((addr >= FIRMWARE_AREA_ADDR) && ((addr + size) <= (FIRMWARE_AREA_ADDR + FIRMWARE_AREA_SIZE_MAX))) {
        uint32_t erase_op = vsfhal_flash_opsize(FLASH0_IDX, addr, FLASH_ERASE);
        uint32_t write_op = vsfhal_flash_opsize(FLASH0_IDX, addr, FLASH_WRITE);
        
        if (!(addr % erase_op))
            vsfhal_flash_erase(FLASH0_IDX, addr, max(erase_op, size));
        vsfhal_flash_write(FLASH0_IDX, addr, max(write_op, size), buf);
    } else {
        return 0;
    }
}

uint32_t usrapp_flash_read(uint8_t *buf, uint32_t addr, uint32_t size)
{
    if ((addr >= FIRMWARE_AREA_ADDR) && ((addr + size) <= (FIRMWARE_AREA_ADDR + FIRMWARE_AREA_SIZE_MAX)))
        return vsfhal_flash_read(FLASH0_IDX, addr, size, buf);
    else
        return 0;
}

int main(void)
{
#if PERIPHERAL_LED_RED_IDX == PERIPHERAL_LED_GREEN_IDX
    vsfhal_gpio_init(PERIPHERAL_LED_RED_IDX);
    vsfhal_gpio_config(PERIPHERAL_LED_RED_IDX, (0x1 << PERIPHERAL_LED_RED_PIN) | (0x1 << PERIPHERAL_LED_GREEN_PIN), IO_OUTPUT_PP);
#else
    vsfhal_gpio_init(PERIPHERAL_LED_RED_IDX);
    vsfhal_gpio_config(PERIPHERAL_LED_RED_IDX, 0x1 << PERIPHERAL_LED_RED_PIN, IO_OUTPUT_PP);
    vsfhal_gpio_init(PERIPHERAL_LED_GREEN_IDX);
    vsfhal_gpio_config(PERIPHERAL_LED_GREEN_IDX, 0x1 << PERIPHERAL_LED_GREEN_PIN, IO_OUTPUT_PP);
#endif

    vsfhal_gpio_init(PERIPHERAL_KEY_IDX);
#if PERIPHERAL_KEY_VALID_LVL
    vsfhal_gpio_config(PERIPHERAL_KEY_IDX, 0x1 << PERIPHERAL_KEY_PIN, IO_INPUT_PULL_DOWN);
    if (vsfhal_gpio_read(PERIPHERAL_KEY_IDX, 0x1 << PERIPHERAL_KEY_PIN) == 0)
#else
    vsfhal_gpio_config(PERIPHERAL_KEY_IDX, 0x1 << PERIPHERAL_KEY_PIN, IO_INPUT_PULL_UP);
    if (vsfhal_gpio_read(PERIPHERAL_KEY_IDX, 0x1 << PERIPHERAL_KEY_PIN) != 0)
#endif
    {
        uint32_t app_main_addr = *(uint32_t *)(FIRMWARE_AREA_ADDR + 4);
        uint32_t sp_addr = *(uint32_t *)(FIRMWARE_AREA_ADDR);
        
        if ((app_main_addr >= FIRMWARE_AREA_ADDR) && (app_main_addr < (FIRMWARE_AREA_ADDR + FIRMWARE_AREA_SIZE_MAX)) && 
                (sp_addr >= FIRMWARE_SP_ADDR) && (sp_addr < (FIRMWARE_SP_ADDR + FIRMWARE_SP_SIZE_MAX))) {
            uint32_t (*app_main)(void) = (uint32_t(*)(void))app_main_addr;
            __set_MSP(sp_addr);
            app_main();
        }
    }

    usrapp_led_ctrl(true, true);

    vsf_dfu_start();
    return 0;
}

/* EOF */
