#ifndef __BOARD_CFG_LVL2_H__
#define __BOARD_CFG_LVL2_H__

#define VSF_SYSTIMER_FREQ           (CHIP_AHB_APB_FREQ_HZ)

#define FLASH0_ENABLE               0

#define GPIO0_ENABLE                1
#define GPIO1_ENABLE                1
#define GPIO2_ENABLE                1

#define USART_STREAM_ENABLE         1
#define USART_STREAM_EDA_PRIORITY   vsf_prio_1      // same as VSF_USBD_CFG_EDA_PRIORITY
#define USART0_ENABLE               1
#   define USART0_TXD_IO_IDX        GPIO0_IDX
#   define USART0_TXD_IO_PIN        5
#   define USART0_TXD_IO_CFG        IO_SELECT_FUNC_6
#   define USART0_RXD_IO_IDX        GPIO0_IDX
#   define USART0_RXD_IO_PIN        4
#   define USART0_RXD_IO_CFG        IO_SELECT_FUNC_6

#define SWD0_ENABLE                 1

#define JTAG0_ENABLE                0

#define PERIPHERAL_JTAG_PRIORITY            vsf_arch_prio_2
#define PERIPHERAL_SWD_PRIORITY             vsf_arch_prio_2
#define PERIPHERAL_TIMESTAMP_PRIORITY       vsf_arch_prio_3

// KEY
#if 0
#define PERIPHERAL_KEY_IDX                  GPIO1_IDX
#define PERIPHERAL_KEY_PIN                  6
#define PERIPHERAL_KEY_VALID_LVL            0
#endif

// LED
#if 0
#define PERIPHERAL_LED_RED_IDX              GPIO_INVALID_IDX    // P12
#define PERIPHERAL_LED_RED_PIN              2
#define PERIPHERAL_LED_RED_VALID_LVL        1
#define PERIPHERAL_LED_GREEN_IDX            GPIO_INVALID_IDX    // P13
#define PERIPHERAL_LED_GREEN_PIN            3
#define PERIPHERAL_LED_GREEN_VALID_LVL      1
#define PERIPHERAL_LED_BLUE_IDX             GPIO_INVALID_IDX    // P00
#define PERIPHERAL_LED_BLUE_PIN             0
#define PERIPHERAL_LED_BLUE_VALID_LVL       1
#endif

// JTAG (SWD: TMS->SWDIO, TCK->SWCLK, RST->SWRST, TDO->SWO)
#define PERIPHERAL_GPIO_TDI_IDX             GPIO1_IDX
#define PERIPHERAL_GPIO_TDI_PIN             1
#define PERIPHERAL_GPIO_TMS_MO_IDX          GPIO2_IDX
#define PERIPHERAL_GPIO_TMS_MO_PIN          3
#define PERIPHERAL_GPIO_TMS_MO_AF           IO_FUNC_4
#define PERIPHERAL_GPIO_TMS_MI_IDX          GPIO2_IDX
#define PERIPHERAL_GPIO_TMS_MI_PIN          4
#define PERIPHERAL_GPIO_TMS_MI_AF           IO_FUNC_4
#define PERIPHERAL_GPIO_TCK_SWD_IDX         GPIO1_IDX
#define PERIPHERAL_GPIO_TCK_SWD_PIN         0
#define PERIPHERAL_GPIO_TCK_SWD_AF          IO_FUNC_1
#define PERIPHERAL_GPIO_TCK_JTAG_IDX        PERIPHERAL_GPIO_TCK_SWD_IDX
#define PERIPHERAL_GPIO_TCK_JTAG_PIN        PERIPHERAL_GPIO_TCK_SWD_PIN
#define PERIPHERAL_GPIO_TCK_JTAG_AF         PERIPHERAL_GPIO_TCK_SWD_AF
#define PERIPHERAL_GPIO_SRST_IDX            GPIO0_IDX
#define PERIPHERAL_GPIO_SRST_PIN            6
#define PERIPHERAL_GPIO_TDO_IDX             GPIO1_IDX
#define PERIPHERAL_GPIO_TDO_PIN             7
#define PERIPHERAL_GPIO_TRST_IDX            GPIO0_IDX
#define PERIPHERAL_GPIO_TRST_PIN            5
#define PERIPHERAL_GPIO_TX0_IDX             GPIO0_IDX
#define PERIPHERAL_GPIO_TX0_PIN             5
#define PERIPHERAL_GPIO_RX0_IDX             GPIO0_IDX
#define PERIPHERAL_GPIO_RX0_PIN             4

// UART, SWO & EXT
#define PERIPHERAL_UART_EXT_IDX             USART0_IDX
//#define PERIPHERAL_UART_EXT_IDX             USART0_IDX
#   define PERIPHERAL_UART_EXT_BAUD_MAX     (96000000 / 20)
#   define PERIPHERAL_UART_EXT_BAUD_MIN     (2000)
#define PERIPHERAL_UART_EXT_PRIORITY        vsf_arch_prio_3
//#define PERIPHERAL_UART_EXT_PRIORITY        vsf_arch_prio_3
#define PERIPHERAL_UART_PARITY_NONE         USART_PARITY_NONE
#define PERIPHERAL_UART_PARITY_ODD          USART_PARITY_ODD
#define PERIPHERAL_UART_PARITY_EVEN         USART_PARITY_EVEN
#define PERIPHERAL_UART_STOPBITS_1          USART_STOPBITS_1
#define PERIPHERAL_UART_STOPBITS_1P5        USART_STOPBITS_1
#define PERIPHERAL_UART_STOPBITS_2          USART_STOPBITS_2
#define PERIPHERAL_UART_BITLEN_8            0
#define PERIPHERAL_UART_MODE_DEFAULT        (PERIPHERAL_UART_PARITY_NONE | PERIPHERAL_UART_STOPBITS_1 | PERIPHERAL_UART_BITLEN_8)
#define PERIPHERAL_UART_BAUD_DEFAULT        115200
#define PERIPHERAL_UART_EXT_DTR_IDX         GPIO_INVALID_IDX
#define PERIPHERAL_UART_EXT_DTR_PIN         2
#define PERIPHERAL_UART_EXT_DTR_CONFIG      IO_OUTPUT
#define PERIPHERAL_UART_EXT_RTS_IDX         GPIO_INVALID_IDX
#define PERIPHERAL_UART_EXT_RTS_PIN         3
#define PERIPHERAL_UART_EXT_RTS_CONFIG      IO_OUTPUT

#ifndef PERIPHERAL_KEY_IDX
#   define PERIPHERAL_KEY_INIT()
#   define PERIPHERAL_KEY_IsPress()
#elif PERIPHERAL_KEY_VALID_LVL
#   define PERIPHERAL_KEY_INIT()            do {vsfhal_gpio_init(PERIPHERAL_KEY_IDX); vsfhal_gpio_config(PERIPHERAL_KEY_IDX, 0x1 << PERIPHERAL_KEY_PIN, IO_INPUT_PULL_DOWN);} while (0)
#   define PERIPHERAL_KEY_IsPress()         (vsfhal_gpio_read(PERIPHERAL_KEY_IDX, 1 << PERIPHERAL_KEY_PIN) ? true : false)
#else
#   define PERIPHERAL_KEY_INIT()            do {vsfhal_gpio_init(PERIPHERAL_KEY_IDX); vsfhal_gpio_config(PERIPHERAL_KEY_IDX, 0x1 << PERIPHERAL_KEY_PIN, IO_INPUT_PULL_UP);} while (0)
#   define PERIPHERAL_KEY_IsPress()         (vsfhal_gpio_read(PERIPHERAL_KEY_IDX, 1 << PERIPHERAL_KEY_PIN) ? false : true)
#endif

#ifndef PERIPHERAL_LED_RED_IDX
#   define PERIPHERAL_LED_RED_INIT()        
#   define PERIPHERAL_LED_RED_ON()          
#   define PERIPHERAL_LED_RED_OFF()         
#elif PERIPHERAL_LED_RED_VALID_LVL
#   define PERIPHERAL_LED_RED_INIT()        do {vsfhal_gpio_init(PERIPHERAL_LED_RED_IDX); vsfhal_gpio_config(PERIPHERAL_LED_RED_IDX, 0x1 << PERIPHERAL_LED_RED_PIN, IO_OUTPUT);} while (0)
#   define PERIPHERAL_LED_RED_ON()          vsfhal_gpio_set(PERIPHERAL_LED_RED_IDX, 1 << PERIPHERAL_LED_RED_PIN)
#   define PERIPHERAL_LED_RED_OFF()         vsfhal_gpio_clear(PERIPHERAL_LED_RED_IDX, 1 << PERIPHERAL_LED_RED_PIN)
#else
#   define PERIPHERAL_LED_RED_INIT()        do {vsfhal_gpio_init(PERIPHERAL_LED_RED_IDX); vsfhal_gpio_config(PERIPHERAL_LED_RED_IDX, 0x1 << PERIPHERAL_LED_RED_PIN, IO_OUTPUT_OD);} while (0)
#   define PERIPHERAL_LED_RED_ON()          vsfhal_gpio_clear(PERIPHERAL_LED_RED_IDX, 1 << PERIPHERAL_LED_RED_PIN)
#   define PERIPHERAL_LED_RED_OFF()         vsfhal_gpio_set(PERIPHERAL_LED_RED_IDX, 1 << PERIPHERAL_LED_RED_PIN)
#endif

#ifndef PERIPHERAL_LED_GREEN_IDX
#   define PERIPHERAL_LED_GREEN_INIT()        
#   define PERIPHERAL_LED_GREEN_ON()          
#   define PERIPHERAL_LED_GREEN_OFF()         
#elif PERIPHERAL_LED_GREEN_VALID_LVL
#   define PERIPHERAL_LED_GREEN_INIT()      do {vsfhal_gpio_init(PERIPHERAL_LED_GREEN_IDX); vsfhal_gpio_config(PERIPHERAL_LED_GREEN_IDX, 0x1 << PERIPHERAL_LED_GREEN_PIN, IO_OUTPUT);} while (0)
#   define PERIPHERAL_LED_GREEN_ON()        vsfhal_gpio_set(PERIPHERAL_LED_GREEN_IDX, 1 << PERIPHERAL_LED_GREEN_PIN)
#   define PERIPHERAL_LED_GREEN_OFF()       vsfhal_gpio_clear(PERIPHERAL_LED_GREEN_IDX, 1 << PERIPHERAL_LED_GREEN_PIN)
#else
#   define PERIPHERAL_LED_GREEN_INIT()      do {vsfhal_gpio_init(PERIPHERAL_LED_GREEN_IDX); vsfhal_gpio_config(PERIPHERAL_LED_GREEN_IDX, 0x1 << PERIPHERAL_LED_GREEN_PIN, IO_OUTPUT_OD);} while (0)
#   define PERIPHERAL_LED_GREEN_ON()        vsfhal_gpio_clear(PERIPHERAL_LED_GREEN_IDX, 1 << PERIPHERAL_LED_GREEN_PIN)
#   define PERIPHERAL_LED_GREEN_OFF()       vsfhal_gpio_set(PERIPHERAL_LED_GREEN_IDX, 1 << PERIPHERAL_LED_GREEN_PIN)
#endif

#define PERIPHERAL_GPIO_TDI_INIT()          do {vsfhal_gpio_init(PERIPHERAL_GPIO_TDI_IDX); vsfhal_gpio_config(PERIPHERAL_GPIO_TDI_IDX, 0x1 << PERIPHERAL_GPIO_TDI_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TDI_FINI()          do {vsfhal_gpio_config(PERIPHERAL_GPIO_TDI_IDX, 0x1 << PERIPHERAL_GPIO_TDI_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TDI_SET_INPUT()     do {vsfhal_gpio_config(PERIPHERAL_GPIO_TDI_IDX, 0x1 << PERIPHERAL_GPIO_TDI_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TDI_SET_OUTPUT()    do {vsfhal_gpio_config(PERIPHERAL_GPIO_TDI_IDX, 0x1 << PERIPHERAL_GPIO_TDI_PIN, IO_OUTPUT);} while (0)
#define PERIPHERAL_GPIO_TDI_SET()           do {vsfhal_gpio_set(PERIPHERAL_GPIO_TDI_IDX, 1 << PERIPHERAL_GPIO_TDI_PIN);} while (0)
#define PERIPHERAL_GPIO_TDI_CLEAR()         do {vsfhal_gpio_clear(PERIPHERAL_GPIO_TDI_IDX, 1 << PERIPHERAL_GPIO_TDI_PIN);} while (0)
#define PERIPHERAL_GPIO_TDI_READ()          (vsfhal_gpio_read(PERIPHERAL_GPIO_TDI_IDX, 1 << PERIPHERAL_GPIO_TDI_PIN) >> PERIPHERAL_GPIO_TDI_PIN)

#define PERIPHERAL_GPIO_TMS_INIT()          do {vsfhal_gpio_init(PERIPHERAL_GPIO_TMS_MO_IDX);\
                                                vsfhal_gpio_init(PERIPHERAL_GPIO_TMS_MI_IDX);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TMS_MO_IDX, 0x1 << PERIPHERAL_GPIO_TMS_MO_PIN, IO_INPUT_FLOAT);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TMS_MI_IDX, 0x1 << PERIPHERAL_GPIO_TMS_MI_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TMS_FINI()          do {vsfhal_gpio_config(PERIPHERAL_GPIO_TMS_MO_IDX, 0x1 << PERIPHERAL_GPIO_TMS_MO_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TMS_SET_INPUT()     do {vsfhal_gpio_config(PERIPHERAL_GPIO_TMS_MO_IDX, 0x1 << PERIPHERAL_GPIO_TMS_MO_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TMS_SET_OUTPUT()    do {vsfhal_gpio_config(PERIPHERAL_GPIO_TMS_MO_IDX, 0x1 << PERIPHERAL_GPIO_TMS_MO_PIN, IO_OUTPUT);} while (0)
#define PERIPHERAL_GPIO_TMS_SET()           do {vsfhal_gpio_set(PERIPHERAL_GPIO_TMS_MO_IDX, 1 << PERIPHERAL_GPIO_TMS_MO_PIN);} while (0)
#define PERIPHERAL_GPIO_TMS_CLEAR()         do {vsfhal_gpio_clear(PERIPHERAL_GPIO_TMS_MO_IDX, 1 << PERIPHERAL_GPIO_TMS_MO_PIN);} while (0)
#define PERIPHERAL_GPIO_TMS_READ()          (vsfhal_gpio_read(PERIPHERAL_GPIO_TMS_MI_IDX, 1 << PERIPHERAL_GPIO_TMS_MI_PIN) >> PERIPHERAL_GPIO_TMS_MI_PIN)

#define PERIPHERAL_GPIO_TCK_INIT()          do {vsfhal_gpio_init(PERIPHERAL_GPIO_TCK_SWD_IDX);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TCK_SWD_IDX, 0x1 << PERIPHERAL_GPIO_TCK_SWD_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TCK_FINI()          do {vsfhal_gpio_config(PERIPHERAL_GPIO_TCK_SWD_IDX, 0x1 << PERIPHERAL_GPIO_TCK_SWD_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TCK_SET_INPUT()     do {vsfhal_gpio_config(PERIPHERAL_GPIO_TCK_SWD_IDX, 0x1 << PERIPHERAL_GPIO_TCK_SWD_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TCK_SET_OUTPUT()    do {vsfhal_gpio_config(PERIPHERAL_GPIO_TCK_SWD_IDX, 0x1 << PERIPHERAL_GPIO_TCK_SWD_PIN, IO_OUTPUT);} while (0)
#define PERIPHERAL_GPIO_TCK_SET()           do {vsfhal_gpio_set(PERIPHERAL_GPIO_TCK_SWD_IDX, 1 << PERIPHERAL_GPIO_TCK_SWD_PIN);} while (0)
#define PERIPHERAL_GPIO_TCK_CLEAR()         do {vsfhal_gpio_clear(PERIPHERAL_GPIO_TCK_SWD_IDX, 1 << PERIPHERAL_GPIO_TCK_SWD_PIN);} while (0)
#define PERIPHERAL_GPIO_TCK_READ()          (vsfhal_gpio_read(PERIPHERAL_GPIO_TCK_SWD_IDX, 1 << PERIPHERAL_GPIO_TCK_SWD_PIN) >> PERIPHERAL_GPIO_TCK_SWD_PIN)

#define PERIPHERAL_GPIO_SRST_INIT()         do {vsfhal_gpio_init(PERIPHERAL_GPIO_SRST_IDX);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_SRST_IDX, 0x1 << PERIPHERAL_GPIO_SRST_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_SRST_FINI()         do {vsfhal_gpio_config(PERIPHERAL_GPIO_SRST_IDX, 0x1 << PERIPHERAL_GPIO_SRST_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_SRST_SET_INPUT()    do {vsfhal_gpio_config(PERIPHERAL_GPIO_SRST_IDX, 0x1 << PERIPHERAL_GPIO_SRST_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_SRST_SET_OUTPUT()   do {vsfhal_gpio_config(PERIPHERAL_GPIO_SRST_IDX, 0x1 << PERIPHERAL_GPIO_SRST_PIN, IO_OUTPUT);} while (0)
#define PERIPHERAL_GPIO_SRST_SET()          do {vsfhal_gpio_set(PERIPHERAL_GPIO_SRST_IDX, 1 << PERIPHERAL_GPIO_SRST_PIN);} while (0)
#define PERIPHERAL_GPIO_SRST_CLEAR()        do {vsfhal_gpio_clear(PERIPHERAL_GPIO_SRST_IDX, 1 << PERIPHERAL_GPIO_SRST_PIN);} while (0)
#define PERIPHERAL_GPIO_SRST_READ()         (vsfhal_gpio_read(PERIPHERAL_GPIO_SRST_IDX, 1 << PERIPHERAL_GPIO_SRST_PIN) >> PERIPHERAL_GPIO_SRST_PIN)

#define PERIPHERAL_GPIO_TDO_INIT()          do {vsfhal_gpio_init(PERIPHERAL_GPIO_TDO_IDX);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TDO_IDX, 0x1 << PERIPHERAL_GPIO_TDO_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TDO_FINI()          do {vsfhal_gpio_config(PERIPHERAL_GPIO_TDO_IDX, 0x1 << PERIPHERAL_GPIO_TDO_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TDO_SET_INPUT()     do {vsfhal_gpio_config(PERIPHERAL_GPIO_TDO_IDX, 0x1 << PERIPHERAL_GPIO_TDO_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TDO_SET_OUTPUT()    do {vsfhal_gpio_config(PERIPHERAL_GPIO_TDO_IDX, 0x1 << PERIPHERAL_GPIO_TDO_PIN, IO_OUTPUT);} while (0)
#define PERIPHERAL_GPIO_TDO_SET()           do {vsfhal_gpio_set(PERIPHERAL_GPIO_TDO_IDX, 1 << PERIPHERAL_GPIO_TDO_PIN);} while (0)
#define PERIPHERAL_GPIO_TDO_CLEAR()         do {vsfhal_gpio_clear(PERIPHERAL_GPIO_TDO_IDX, 1 << PERIPHERAL_GPIO_TDO_PIN);} while (0)
#define PERIPHERAL_GPIO_TDO_READ()          (vsfhal_gpio_read(PERIPHERAL_GPIO_TDO_IDX, 1 << PERIPHERAL_GPIO_TDO_PIN) >> PERIPHERAL_GPIO_TDO_PIN)

#define PERIPHERAL_GPIO_TRST_INIT()         do {vsfhal_gpio_init(PERIPHERAL_GPIO_TRST_IDX);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TRST_IDX, 0x1 << PERIPHERAL_GPIO_TRST_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TRST_FINI()         do {vsfhal_gpio_config(PERIPHERAL_GPIO_TRST_IDX, 0x1 << PERIPHERAL_GPIO_TRST_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TRST_SET_INPUT()    do {vsfhal_gpio_config(PERIPHERAL_GPIO_TRST_IDX, 0x1 << PERIPHERAL_GPIO_TRST_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TRST_SET_OUTPUT()   do {vsfhal_gpio_config(PERIPHERAL_GPIO_TRST_IDX, 0x1 << PERIPHERAL_GPIO_TRST_PIN, IO_OUTPUT);} while (0)
#define PERIPHERAL_GPIO_TRST_SET()          do {vsfhal_gpio_set(PERIPHERAL_GPIO_TRST_IDX, 1 << PERIPHERAL_GPIO_TRST_PIN);} while (0)
#define PERIPHERAL_GPIO_TRST_CLEAR()        do {vsfhal_gpio_clear(PERIPHERAL_GPIO_TRST_IDX, 1 << PERIPHERAL_GPIO_TRST_PIN);} while (0)
#define PERIPHERAL_GPIO_TRST_READ()         (vsfhal_gpio_read(PERIPHERAL_GPIO_TRST_IDX, 1 << PERIPHERAL_GPIO_TRST_PIN) >> PERIPHERAL_GPIO_TRST_PIN)

#endif // __BOARD_CFG_LVL2_H__
