#ifndef __BOARD_CFG_VLLINKLITE_GD32E103_H__
#define __BOARD_CFG_VLLINKLITE_GD32E103_H__

#define DMA0_ENABLE             0
#define DMA1_ENABLE             0

#define FLASH0_ENABLE           1

#define GPIOA_ENABLE            1
#define GPIOB_ENABLE            1
#define GPIOC_ENABLE            0
#define GPIOD_ENABLE            0
#define GPIOE_ENABLE            0

#define USART0_ENABLE           0
#   define USART0_DMA_ENABLE    1
#   define USART0_IO_REMAP      0
#	define USART0_TXD_IO_IDX    GPIOA_IDX 
#	define USART0_TXD_IO_PIN    9
#	define USART0_RXD_IO_IDX    GPIO_INVALID_IDX 
#	define USART0_RXD_IO_PIN    10
#	define USART0_CTS_IO_IDX    GPIO_INVALID_IDX 
#	define USART0_CTS_IO_PIN    11
#	define USART0_RTS_IO_IDX    GPIO_INVALID_IDX 
#	define USART0_RTS_IO_PIN    12
#define USART1_ENABLE           0
#   define USART1_DMA_ENABLE    1
#   define USART1_IO_REMAP      0
#	define USART1_TXD_IO_IDX    GPIOA_IDX 
#	define USART1_TXD_IO_PIN    2
#	define USART1_RXD_IO_IDX    GPIOA_IDX 
#	define USART1_RXD_IO_PIN    3
#	define USART1_CTS_IO_IDX    GPIO_INVALID_IDX 
#	define USART1_CTS_IO_PIN    0
#	define USART1_RTS_IO_IDX    GPIOA_IDX 
#	define USART1_RTS_IO_PIN    1
#define USART2_ENABLE           0
#   define USART2_DMA_ENABLE    1
#   define USART2_IO_REMAP      0
#	define USART2_TXD_IO_IDX    GPIO_INVALID_IDX 
#	define USART2_TXD_IO_PIN    2
#	define USART2_RXD_IO_IDX    GPIO_INVALID_IDX 
#	define USART2_RXD_IO_PIN    3
#	define USART2_CTS_IO_IDX    GPIO_INVALID_IDX 
#	define USART2_CTS_IO_PIN    1
#	define USART2_RTS_IO_IDX    GPIO_INVALID_IDX 
#	define USART2_RTS_IO_PIN    1
#define USART3_ENABLE           0
#   define USART3_DMA_ENABLE    1
#	define USART3_TXD_IO_IDX    GPIO_INVALID_IDX 
#	define USART3_TXD_IO_PIN    2
#	define USART3_RXD_IO_IDX    GPIO_INVALID_IDX 
#	define USART3_RXD_IO_PIN    3
#define USART4_ENABLE           0
#	define USART4_TXD_IO_IDX    GPIO_INVALID_IDX 
#	define USART4_TXD_IO_PIN    2
#	define USART4_RXD_IO_IDX    GPIO_INVALID_IDX 
#	define USART4_RXD_IO_PIN    3

#define SPI0_ENABLE             0
#   define SPI0_DMA_ENABLE      1
#   define SPI0_IO_REMAP        1
#	define SPI0_NSS_IO_IDX      GPIO_INVALID_IDX 
#	define SPI0_NSS_IO_PIN      15
#	define SPI0_SCK_IO_IDX      GPIOB_IDX 
#	define SPI0_SCK_IO_PIN      3
#	define SPI0_MISO_IO_IDX     GPIOB_IDX 
#	define SPI0_MISO_IO_PIN     4
#	define SPI0_MOSI_IO_IDX     GPIOB_IDX 
#	define SPI0_MOSI_IO_PIN     5
#define SPI1_ENABLE             0
#   define SPI1_DMA_ENABLE      1
#	define SPI1_NSS_IO_IDX      GPIO_INVALID_IDX 
#	define SPI1_NSS_IO_PIN      0
#	define SPI1_SCK_IO_IDX      GPIO_INVALID_IDX 
#	define SPI1_SCK_IO_PIN      0
#	define SPI1_MISO_IO_IDX     GPIO_INVALID_IDX 
#	define SPI1_MISO_IO_PIN     0
#	define SPI1_MOSI_IO_IDX     GPIO_INVALID_IDX 
#	define SPI1_MOSI_IO_PIN     0
#define SPI2_ENABLE             0
#   define SPI2_DMA_ENABLE      1
#   define SPI2_IO_REMAP        0
#	define SPI2_NSS_IO_IDX      GPIO_INVALID_IDX 
#	define SPI2_NSS_IO_PIN      0
#	define SPI2_SCK_IO_IDX      GPIO_INVALID_IDX 
#	define SPI2_SCK_IO_PIN      0
#	define SPI2_MISO_IO_IDX     GPIO_INVALID_IDX 
#	define SPI2_MISO_IO_PIN     0
#	define SPI2_MOSI_IO_IDX     GPIO_INVALID_IDX 
#	define SPI2_MOSI_IO_PIN     0

#if (VSF_USE_USB_DEVICE == ENABLED) || (VSF_USE_USB_HOST == ENABLED)
#   define APP_CFG_USBD_SPEED               USB_DC_SPEED_FULL
#endif


#endif // __BOARD_CFG_VLLINKLITE_GD32F350_H__
