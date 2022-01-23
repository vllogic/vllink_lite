#ifndef __BOARD_CFG_LVL2_H__
#define __BOARD_CFG_LVL2_H__

#define VSF_SYSTIMER_FREQ       (CHIP_AHB_FREQ_HZ)

#define DMA0_ENABLE             1
#define DMA1_ENABLE             0

#define FLASH0_ENABLE           0

#define GPIOA_ENABLE            1
#define GPIOB_ENABLE            1
#define GPIOC_ENABLE            0
#define GPIOD_ENABLE            1
#define GPIOE_ENABLE            0

#define USART_STREAM_ENABLE     1
#define USART_STREAM_EDA_PRIORITY   vsf_prio_1      // same as VSF_USBD_CFG_EDA_PRIORITY
#define USART_BUFF_SIZE         64
#define USART0_ENABLE           1
#   define USART0_DMA_ENABLE    1
#   define USART0_IO_REMAP      0
#   define USART0_TXD_IO_IDX    GPIOA_IDX 
#   define USART0_TXD_IO_PIN    9
#   define USART0_RXD_IO_IDX    GPIOA_IDX 
#   define USART0_RXD_IO_PIN    10
#   define USART0_CTS_IO_IDX    GPIO_INVALID_IDX 
#   define USART0_CTS_IO_PIN    11
#   define USART0_RTS_IO_IDX    GPIO_INVALID_IDX 
#   define USART0_RTS_IO_PIN    12
#define USART1_ENABLE           1
#   define USART1_DMA_ENABLE    1
#   define USART1_IO_REMAP      0
#   define USART1_TXD_IO_IDX    GPIOA_IDX 
#   define USART1_TXD_IO_PIN    2
#   define USART1_RXD_IO_IDX    GPIOA_IDX 
#   define USART1_RXD_IO_PIN    3
#   define USART1_CTS_IO_IDX    GPIO_INVALID_IDX 
#   define USART1_CTS_IO_PIN    0
#   define USART1_RTS_IO_IDX    GPIO_INVALID_IDX 
#   define USART1_RTS_IO_PIN    1
#define USART2_ENABLE           0
#   define USART2_DMA_ENABLE    1
#   define USART2_IO_REMAP      0
#   define USART2_TXD_IO_IDX    GPIO_INVALID_IDX 
#   define USART2_TXD_IO_PIN    2
#   define USART2_RXD_IO_IDX    GPIO_INVALID_IDX 
#   define USART2_RXD_IO_PIN    3
#   define USART2_CTS_IO_IDX    GPIO_INVALID_IDX 
#   define USART2_CTS_IO_PIN    1
#   define USART2_RTS_IO_IDX    GPIO_INVALID_IDX 
#   define USART2_RTS_IO_PIN    1
#define USART3_ENABLE           0
#   define USART3_DMA_ENABLE    1
#   define USART3_TXD_IO_IDX    GPIO_INVALID_IDX 
#   define USART3_TXD_IO_PIN    2
#   define USART3_RXD_IO_IDX    GPIO_INVALID_IDX 
#   define USART3_RXD_IO_PIN    3
#define USART4_ENABLE           0
#   define USART4_TXD_IO_IDX    GPIO_INVALID_IDX 
#   define USART4_TXD_IO_PIN    2
#   define USART4_RXD_IO_IDX    GPIO_INVALID_IDX 
#   define USART4_RXD_IO_PIN    3

#define SPI0_ENABLE             0
#   define SPI0_DMA_ENABLE      1
#   define SPI0_IO_REMAP        1
#   define SPI0_NSS_IO_IDX      GPIO_INVALID_IDX 
#   define SPI0_NSS_IO_PIN      0
#   define SPI0_SCK_IO_IDX      GPIO_INVALID_IDX 
#   define SPI0_SCK_IO_PIN      0
#   define SPI0_MISO_IO_IDX     GPIO_INVALID_IDX 
#   define SPI0_MISO_IO_PIN     0
#   define SPI0_MOSI_IO_IDX     GPIO_INVALID_IDX 
#   define SPI0_MOSI_IO_PIN     0
#define SPI1_ENABLE             0
#   define SPI1_DMA_ENABLE      1
#   define SPI1_NSS_IO_IDX      GPIO_INVALID_IDX 
#   define SPI1_NSS_IO_PIN      0
#   define SPI1_SCK_IO_IDX      GPIO_INVALID_IDX 
#   define SPI1_SCK_IO_PIN      0
#   define SPI1_MISO_IO_IDX     GPIO_INVALID_IDX 
#   define SPI1_MISO_IO_PIN     0
#   define SPI1_MOSI_IO_IDX     GPIO_INVALID_IDX 
#   define SPI1_MOSI_IO_PIN     0
#define SPI2_ENABLE             0
#   define SPI2_DMA_ENABLE      1
#   define SPI2_IO_REMAP        0
#   define SPI2_NSS_IO_IDX      GPIO_INVALID_IDX 
#   define SPI2_NSS_IO_PIN      0
#   define SPI2_SCK_IO_IDX      GPIO_INVALID_IDX 
#   define SPI2_SCK_IO_PIN      0
#   define SPI2_MISO_IO_IDX     GPIO_INVALID_IDX 
#   define SPI2_MISO_IO_PIN     0
#   define SPI2_MOSI_IO_IDX     GPIO_INVALID_IDX 
#   define SPI2_MOSI_IO_PIN     0

#define SWD0_ENABLE             1
#   define SWD_SPI_BASE         SPI0
#   define SWD_SPI_REMAP        1
#   define SWD_SPI_DMA_IDX      DMA0_IDX
#   define SWD_SPI_DMAX         DMA0
#   define SWD_SPI_TX_DMA_CH    2
#   define SWD_SPI_RX_DMA_CH    1

#define JTAG0_ENABLE            1
#   define JTAG_SPI_BASE        SPI0
#   define JTAG_SPI_REMAP       0
#   define JTAG_SPI_DMA_IDX     DMA0_IDX
#   define JTAG_SPI_DMAX        DMA0
#   define JTAG_SPI_TX_DMA_CH   2
#   define JTAG_SPI_RX_DMA_CH   1

#define PERIPHERAL_JTAG_PRIORITY            vsf_arch_prio_2
#define PERIPHERAL_SWD_PRIORITY             vsf_arch_prio_2
#define PERIPHERAL_TIMESTAMP_PRIORITY       vsf_arch_prio_3

// KEY
#define PERIPHERAL_KEY_IDX                  GPIOA_IDX
#define PERIPHERAL_KEY_PIN                  4
#define PERIPHERAL_KEY_VALID_LVL            1

// LED
#define PERIPHERAL_LED_RED_IDX              GPIOB_IDX
#define PERIPHERAL_LED_RED_PIN              6
#define PERIPHERAL_LED_RED_VALID_LVL        1
#define PERIPHERAL_LED_GREEN_IDX            GPIOB_IDX
#define PERIPHERAL_LED_GREEN_PIN            7
#define PERIPHERAL_LED_GREEN_VALID_LVL      1

// JTAG (SWD: TMS->SWDIO, TCK->SWCLK, RST->SWRST, TDO->SWO)
#define PERIPHERAL_GPIO_TDI_IDX             GPIOA_IDX
#define PERIPHERAL_GPIO_TDI_PIN             7
#define PERIPHERAL_GPIO_TMS_MO_IDX          GPIOB_IDX
#define PERIPHERAL_GPIO_TMS_MO_PIN          5
#define PERIPHERAL_GPIO_TMS_MI_IDX          GPIOB_IDX
#define PERIPHERAL_GPIO_TMS_MI_PIN          4
#define PERIPHERAL_GPIO_TCK_SWD_IDX         GPIOB_IDX
#define PERIPHERAL_GPIO_TCK_SWD_PIN         3
#define PERIPHERAL_GPIO_TCK_JTAG_IDX        GPIOA_IDX
#define PERIPHERAL_GPIO_TCK_JTAG_PIN        5
#define PERIPHERAL_GPIO_SRST_IDX            GPIOA_IDX
#define PERIPHERAL_GPIO_SRST_PIN            0
#define PERIPHERAL_GPIO_TDO_MI_IDX          GPIOA_IDX
#define PERIPHERAL_GPIO_TDO_MI_PIN          6
#define PERIPHERAL_GPIO_TDO_RX1_IDX         GPIOA_IDX
#define PERIPHERAL_GPIO_TDO_RX1_PIN         3
#define PERIPHERAL_GPIO_TRST_MO_IDX         GPIOA_IDX
#define PERIPHERAL_GPIO_TRST_MO_PIN         1
#define PERIPHERAL_GPIO_TRST_TX1_IDX        GPIOA_IDX
#define PERIPHERAL_GPIO_TRST_TX1_PIN        2
#define PERIPHERAL_GPIO_TX0_IDX             GPIOA_IDX
#define PERIPHERAL_GPIO_TX0_PIN             9
#define PERIPHERAL_GPIO_RX0_IDX             GPIOA_IDX
#define PERIPHERAL_GPIO_RX0_PIN             10

// UART, SWO & EXT
#define PERIPHERAL_UART_SWO_IDX             USART1_IDX
#define PERIPHERAL_UART_EXT_IDX             USART0_IDX
#   define PERIPHERAL_UART_EXT_BAUD_MAX     (96000000 / 20)
#   define PERIPHERAL_UART_EXT_BAUD_MIN     (2000)
#define PERIPHERAL_UART_SWO_PRIORITY        vsf_arch_prio_3
#define PERIPHERAL_UART_EXT_PRIORITY        vsf_arch_prio_3
#define PERIPHERAL_UART_PARITY_NONE         USART_PARITY_NONE
#define PERIPHERAL_UART_PARITY_ODD          USART_PARITY_ODD
#define PERIPHERAL_UART_PARITY_EVEN         USART_PARITY_EVEN
#define PERIPHERAL_UART_STOPBITS_1          USART_STOPBITS_1
#define PERIPHERAL_UART_STOPBITS_1P5        USART_STOPBITS_1P5
#define PERIPHERAL_UART_STOPBITS_2          USART_STOPBITS_2
#define PERIPHERAL_UART_BITLEN_8            0
#define PERIPHERAL_UART_MODE_DEFAULT        (PERIPHERAL_UART_PARITY_NONE | PERIPHERAL_UART_STOPBITS_1 | PERIPHERAL_UART_BITLEN_8)
#define PERIPHERAL_UART_BAUD_DEFAULT        115200
#define PERIPHERAL_UART_EXT_DTR_IDX         GPIOD_IDX
#define PERIPHERAL_UART_EXT_DTR_PIN         0
#define PERIPHERAL_UART_EXT_DTR_CONFIG      IO_OUTPUT_PP
#define PERIPHERAL_UART_EXT_RTS_IDX         GPIOD_IDX
#define PERIPHERAL_UART_EXT_RTS_PIN         1
#define PERIPHERAL_UART_EXT_RTS_CONFIG      IO_OUTPUT_PP


#if PERIPHERAL_KEY_VALID_LVL
#   define PERIPHERAL_KEY_INIT()            do {vsfhal_gpio_init(PERIPHERAL_KEY_IDX); vsfhal_gpio_config(PERIPHERAL_KEY_IDX, 0x1 << PERIPHERAL_KEY_PIN, IO_INPUT_PULL_DOWN);} while (0)
#   define PERIPHERAL_KEY_IsPress()         (vsfhal_gpio_read(PERIPHERAL_KEY_IDX, 1 << PERIPHERAL_KEY_PIN) ? true : false)
#else
#   define PERIPHERAL_KEY_INIT()            do {vsfhal_gpio_init(PERIPHERAL_KEY_IDX); vsfhal_gpio_config(PERIPHERAL_KEY_IDX, 0x1 << PERIPHERAL_KEY_PIN, IO_INPUT_PULL_UP);} while (0)
#   define PERIPHERAL_KEY_IsPress()         (vsfhal_gpio_read(PERIPHERAL_KEY_IDX, 1 << PERIPHERAL_KEY_PIN) ? false : true)
#endif

#if PERIPHERAL_LED_RED_VALID_LVL
#   define PERIPHERAL_LED_RED_INIT()        do {vsfhal_gpio_init(PERIPHERAL_LED_RED_IDX); vsfhal_gpio_config(PERIPHERAL_LED_RED_IDX, 0x1 << PERIPHERAL_LED_RED_PIN, IO_OUTPUT_PP);} while (0)
#   define PERIPHERAL_LED_RED_ON()          vsfhal_gpio_set(PERIPHERAL_LED_RED_IDX, 1 << PERIPHERAL_LED_RED_PIN)
#   define PERIPHERAL_LED_RED_OFF()         vsfhal_gpio_clear(PERIPHERAL_LED_RED_IDX, 1 << PERIPHERAL_LED_RED_PIN)
#else
#   define PERIPHERAL_LED_RED_INIT()        do {vsfhal_gpio_init(PERIPHERAL_LED_RED_IDX); vsfhal_gpio_config(PERIPHERAL_LED_RED_IDX, 0x1 << PERIPHERAL_LED_RED_PIN, IO_OUTPUT_OD);} while (0)
#   define PERIPHERAL_LED_RED_ON()          vsfhal_gpio_clear(PERIPHERAL_LED_RED_IDX, 1 << PERIPHERAL_LED_RED_PIN)
#   define PERIPHERAL_LED_RED_OFF()         vsfhal_gpio_set(PERIPHERAL_LED_RED_IDX, 1 << PERIPHERAL_LED_RED_PIN)
#endif

#if PERIPHERAL_LED_GREEN_VALID_LVL
#   define PERIPHERAL_LED_GREEN_INIT()      do {vsfhal_gpio_init(PERIPHERAL_LED_GREEN_IDX); vsfhal_gpio_config(PERIPHERAL_LED_GREEN_IDX, 0x1 << PERIPHERAL_LED_GREEN_PIN, IO_OUTPUT_PP);} while (0)
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
#define PERIPHERAL_GPIO_TDI_SET_OUTPUT()    do {vsfhal_gpio_config(PERIPHERAL_GPIO_TDI_IDX, 0x1 << PERIPHERAL_GPIO_TDI_PIN, IO_OUTPUT_50M | IO_PP_OUT);} while (0)
#define PERIPHERAL_GPIO_TDI_SET()           do {vsfhal_gpio_set(PERIPHERAL_GPIO_TDI_IDX, 1 << PERIPHERAL_GPIO_TDI_PIN);} while (0)
#define PERIPHERAL_GPIO_TDI_CLEAR()         do {vsfhal_gpio_clear(PERIPHERAL_GPIO_TDI_IDX, 1 << PERIPHERAL_GPIO_TDI_PIN);} while (0)
#define PERIPHERAL_GPIO_TDI_READ()          (vsfhal_gpio_read(PERIPHERAL_GPIO_TDI_IDX, 1 << PERIPHERAL_GPIO_TDI_PIN) >> PERIPHERAL_GPIO_TDI_PIN)

#define PERIPHERAL_GPIO_TMS_INIT()          do {vsfhal_gpio_init(PERIPHERAL_GPIO_TMS_MO_IDX);\
                                                vsfhal_gpio_init(PERIPHERAL_GPIO_TMS_MI_IDX);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TMS_MO_IDX, 0x1 << PERIPHERAL_GPIO_TMS_MO_PIN, IO_INPUT_FLOAT);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TMS_MI_IDX, 0x1 << PERIPHERAL_GPIO_TMS_MI_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TMS_FINI()          do {vsfhal_gpio_config(PERIPHERAL_GPIO_TMS_MO_IDX, 0x1 << PERIPHERAL_GPIO_TMS_MO_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TMS_SET_INPUT()     do {vsfhal_gpio_config(PERIPHERAL_GPIO_TMS_MO_IDX, 0x1 << PERIPHERAL_GPIO_TMS_MO_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TMS_SET_OUTPUT()    do {vsfhal_gpio_config(PERIPHERAL_GPIO_TMS_MO_IDX, 0x1 << PERIPHERAL_GPIO_TMS_MO_PIN, IO_OUTPUT_50M | IO_PP_OUT);} while (0)
#define PERIPHERAL_GPIO_TMS_SET()           do {vsfhal_gpio_set(PERIPHERAL_GPIO_TMS_MO_IDX, 1 << PERIPHERAL_GPIO_TMS_MO_PIN);} while (0)
#define PERIPHERAL_GPIO_TMS_CLEAR()         do {vsfhal_gpio_clear(PERIPHERAL_GPIO_TMS_MO_IDX, 1 << PERIPHERAL_GPIO_TMS_MO_PIN);} while (0)
#define PERIPHERAL_GPIO_TMS_READ()          (vsfhal_gpio_read(PERIPHERAL_GPIO_TMS_MI_IDX, 1 << PERIPHERAL_GPIO_TMS_MI_PIN) >> PERIPHERAL_GPIO_TMS_MI_PIN)

#define PERIPHERAL_GPIO_TCK_INIT()          do {vsfhal_gpio_init(PERIPHERAL_GPIO_TCK_SWD_IDX);\
                                                vsfhal_gpio_init(PERIPHERAL_GPIO_TCK_JTAG_IDX);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TCK_SWD_IDX, 0x1 << PERIPHERAL_GPIO_TCK_SWD_PIN, IO_INPUT_FLOAT);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TCK_JTAG_IDX, 0x1 << PERIPHERAL_GPIO_TCK_JTAG_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TCK_FINI()          do {vsfhal_gpio_config(PERIPHERAL_GPIO_TCK_SWD_IDX, 0x1 << PERIPHERAL_GPIO_TCK_SWD_PIN, IO_INPUT_FLOAT);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TCK_JTAG_IDX, 0x1 << PERIPHERAL_GPIO_TCK_JTAG_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TCK_SET_INPUT()     do {vsfhal_gpio_config(PERIPHERAL_GPIO_TCK_SWD_IDX, 0x1 << PERIPHERAL_GPIO_TCK_SWD_PIN, IO_INPUT_FLOAT);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TCK_JTAG_IDX, 0x1 << PERIPHERAL_GPIO_TCK_JTAG_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TCK_SET_OUTPUT()    do {vsfhal_gpio_config(PERIPHERAL_GPIO_TCK_SWD_IDX, 0x1 << PERIPHERAL_GPIO_TCK_SWD_PIN, IO_OUTPUT_PP);} while (0)
#define PERIPHERAL_GPIO_TCK_SET()           do {vsfhal_gpio_set(PERIPHERAL_GPIO_TCK_SWD_IDX, 1 << PERIPHERAL_GPIO_TCK_SWD_PIN);} while (0)
#define PERIPHERAL_GPIO_TCK_CLEAR()         do {vsfhal_gpio_clear(PERIPHERAL_GPIO_TCK_SWD_IDX, 1 << PERIPHERAL_GPIO_TCK_SWD_PIN);} while (0)
#define PERIPHERAL_GPIO_TCK_READ()          (vsfhal_gpio_read(PERIPHERAL_GPIO_TCK_SWD_IDX, 1 << PERIPHERAL_GPIO_TCK_SWD_PIN) >> PERIPHERAL_GPIO_TCK_SWD_PIN)

#define PERIPHERAL_GPIO_SRST_INIT()         do {vsfhal_gpio_init(PERIPHERAL_GPIO_SRST_IDX);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_SRST_IDX, 0x1 << PERIPHERAL_GPIO_SRST_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_SRST_FINI()         do {vsfhal_gpio_config(PERIPHERAL_GPIO_SRST_IDX, 0x1 << PERIPHERAL_GPIO_SRST_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_SRST_SET_INPUT()    do {vsfhal_gpio_config(PERIPHERAL_GPIO_SRST_IDX, 0x1 << PERIPHERAL_GPIO_SRST_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_SRST_SET_OUTPUT()   do {vsfhal_gpio_config(PERIPHERAL_GPIO_SRST_IDX, 0x1 << PERIPHERAL_GPIO_SRST_PIN, IO_OUTPUT_PP);} while (0)
#define PERIPHERAL_GPIO_SRST_SET()          do {vsfhal_gpio_set(PERIPHERAL_GPIO_SRST_IDX, 1 << PERIPHERAL_GPIO_SRST_PIN);} while (0)
#define PERIPHERAL_GPIO_SRST_CLEAR()        do {vsfhal_gpio_clear(PERIPHERAL_GPIO_SRST_IDX, 1 << PERIPHERAL_GPIO_SRST_PIN);} while (0)
#define PERIPHERAL_GPIO_SRST_READ()         (vsfhal_gpio_read(PERIPHERAL_GPIO_SRST_IDX, 1 << PERIPHERAL_GPIO_SRST_PIN) >> PERIPHERAL_GPIO_SRST_PIN)

#define PERIPHERAL_GPIO_TDO_INIT()          do {vsfhal_gpio_init(PERIPHERAL_GPIO_TDO_MI_IDX);\
                                                vsfhal_gpio_init(PERIPHERAL_GPIO_TDO_RX1_IDX);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TDO_MI_IDX, 0x1 << PERIPHERAL_GPIO_TDO_MI_PIN, IO_INPUT_FLOAT);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TDO_RX1_IDX, 0x1 << PERIPHERAL_GPIO_TDO_RX1_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TDO_FINI()          do {vsfhal_gpio_config(PERIPHERAL_GPIO_TDO_MI_IDX, 0x1 << PERIPHERAL_GPIO_TDO_MI_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TDO_SET_INPUT()     do {vsfhal_gpio_config(PERIPHERAL_GPIO_TDO_MI_IDX, 0x1 << PERIPHERAL_GPIO_TDO_MI_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TDO_SET_OUTPUT()    do {vsfhal_gpio_config(PERIPHERAL_GPIO_TDO_MI_IDX, 0x1 << PERIPHERAL_GPIO_TDO_MI_PIN, IO_OUTPUT_PP);} while (0)
#define PERIPHERAL_GPIO_TDO_SET()           do {vsfhal_gpio_set(PERIPHERAL_GPIO_TDO_MI_IDX, 1 << PERIPHERAL_GPIO_TDO_MI_PIN);} while (0)
#define PERIPHERAL_GPIO_TDO_CLEAR()         do {vsfhal_gpio_clear(PERIPHERAL_GPIO_TDO_MI_IDX, 1 << PERIPHERAL_GPIO_TDO_MI_PIN);} while (0)
#define PERIPHERAL_GPIO_TDO_READ()          (vsfhal_gpio_read(PERIPHERAL_GPIO_TDO_MI_IDX, 1 << PERIPHERAL_GPIO_TDO_MI_PIN) >> PERIPHERAL_GPIO_TDO_MI_PIN)

#define PERIPHERAL_GPIO_TRST_INIT()         do {vsfhal_gpio_init(PERIPHERAL_GPIO_TRST_MO_IDX);\
                                                vsfhal_gpio_init(PERIPHERAL_GPIO_TRST_TX1_IDX);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TRST_MO_IDX, 0x1 << PERIPHERAL_GPIO_TRST_MO_PIN, IO_INPUT_FLOAT);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TRST_TX1_IDX, 0x1 << PERIPHERAL_GPIO_TRST_TX1_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TRST_FINI()         do {vsfhal_gpio_config(PERIPHERAL_GPIO_TRST_MO_IDX, 0x1 << PERIPHERAL_GPIO_TRST_MO_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TRST_SET_INPUT()    do {vsfhal_gpio_config(PERIPHERAL_GPIO_TRST_MO_IDX, 0x1 << PERIPHERAL_GPIO_TRST_MO_PIN, IO_INPUT_FLOAT);} while (0)
#define PERIPHERAL_GPIO_TRST_SET_OUTPUT()   do {vsfhal_gpio_config(PERIPHERAL_GPIO_TRST_MO_IDX, 0x1 << PERIPHERAL_GPIO_TRST_MO_PIN, IO_OUTPUT_PP);} while (0)
#define PERIPHERAL_GPIO_TRST_SET()          do {vsfhal_gpio_set(PERIPHERAL_GPIO_TRST_MO_IDX, 1 << PERIPHERAL_GPIO_TRST_MO_PIN);} while (0)
#define PERIPHERAL_GPIO_TRST_CLEAR()        do {vsfhal_gpio_clear(PERIPHERAL_GPIO_TRST_MO_IDX, 1 << PERIPHERAL_GPIO_TRST_MO_PIN);} while (0)
#define PERIPHERAL_GPIO_TRST_READ()         (vsfhal_gpio_read(PERIPHERAL_GPIO_TRST_MO_IDX, 1 << PERIPHERAL_GPIO_TRST_MO_PIN) >> PERIPHERAL_GPIO_TRST_MO_PIN)

#define PERIPHERAL_JTAG_IO_AF_CONFIG()      do {AFIO_PCF0 = ((AFIO_PCF0 & ~AFIO_PCF0_SPI0_REMAP) | JTAG_SPI_REMAP);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TDI_IDX, 0x1 << PERIPHERAL_GPIO_TDI_PIN, IO_OUTPUT_50M | IO_AFPP_OUT);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TCK_JTAG_IDX, 0x1 << PERIPHERAL_GPIO_TCK_JTAG_PIN, IO_OUTPUT_50M | IO_AFPP_OUT);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TDO_MI_IDX, 0x1 << PERIPHERAL_GPIO_TDO_MI_PIN, IO_INPUT_FLOAT);} while (0)

#define PERIPHERAL_SWD_IO_AF_CONFIG()       do {AFIO_PCF0 = ((AFIO_PCF0 & ~AFIO_PCF0_SPI0_REMAP) | SWD_SPI_REMAP);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TMS_MO_IDX, 0x1 << PERIPHERAL_GPIO_TMS_MI_PIN, IO_OUTPUT_50M | IO_AFPP_OUT);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TMS_MI_IDX, 0x1 << PERIPHERAL_GPIO_TMS_MI_PIN, IO_INPUT_FLOAT);\
                                                vsfhal_gpio_config(PERIPHERAL_GPIO_TCK_SWD_IDX, 0x1 << PERIPHERAL_GPIO_TCK_SWD_PIN, IO_OUTPUT_50M | IO_AFPP_OUT);} while (0)

#endif // __BOARD_CFG_LVL2_H__
