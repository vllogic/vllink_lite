#ifndef __BOARD_CFG_VLLINKLITE_GD32F350_H__
#define __BOARD_CFG_VLLINKLITE_GD32F350_H__

#define DMA0_ENABLE             1

#define FLASH0_ENABLE           1

#define GPIOA_ENABLE            1
#define GPIOB_ENABLE            1
#define GPIOC_ENABLE            1
#define GPIOD_ENABLE            1
#define GPIOF_ENABLE            1

#define USART0_ENABLE           0
#   define USART0_DMA_ENABLE    1
#	define USART0_TXD_IO_IDX    GPIO_INVALID_IDX 
#	define USART0_TXD_IO_PIN    9
#	define USART0_TXD_IO_AF     0
#	define USART0_RXD_IO_IDX    GPIO_INVALID_IDX 
#	define USART0_RXD_IO_PIN    10
#	define USART0_RXD_IO_AF     0
#	define USART0_CTS_IO_IDX    GPIO_INVALID_IDX 
#	define USART0_CTS_IO_PIN    11
#	define USART0_CTS_IO_AF     0
#	define USART0_RTS_IO_IDX    GPIO_INVALID_IDX 
#	define USART0_RTS_IO_PIN    12
#	define USART0_RTS_IO_AF     0
#define USART1_ENABLE           0
#   define USART1_DMA_ENABLE    1
#	define USART1_TXD_IO_IDX    GPIO_INVALID_IDX 
#	define USART1_TXD_IO_PIN    2
#	define USART1_TXD_IO_AF     2
#	define USART1_RXD_IO_IDX    GPIO_INVALID_IDX 
#	define USART1_RXD_IO_PIN    3
#	define USART1_RXD_IO_AF     2
#	define USART1_CTS_IO_IDX    GPIO_INVALID_IDX 
#	define USART1_CTS_IO_PIN    1
#	define USART1_CTS_IO_AF     2
#	define USART1_RTS_IO_IDX    GPIO_INVALID_IDX 
#	define USART1_RTS_IO_PIN    1
#	define USART1_RTS_IO_AF     2

#define SPI0_ENABLE             1
#   define SPI0_DMA_ENABLE      1
#	define SPI0_NSS_IO_IDX      GPIO_INVALID_IDX 
#	define SPI0_NSS_IO_PIN      0
#	define SPI0_NSS_IO_AF       0
#	define SPI0_SCK_IO_IDX      GPIO_INVALID_IDX 
#	define SPI0_SCK_IO_PIN      0
#	define SPI0_SCK_IO_AF       0
#	define SPI0_MISO_IO_IDX     GPIO_INVALID_IDX 
#	define SPI0_MISO_IO_PIN     0
#	define SPI0_MISO_IO_AF      0
#	define SPI0_MOSI_IO_IDX     GPIO_INVALID_IDX 
#	define SPI0_MOSI_IO_PIN     0
#	define SPI0_MOSI_IO_AF      0
#define SPI1_ENABLE             1
#   define SPI1_DMA_ENABLE      1
#	define SPI1_NSS_IO_IDX      GPIO_INVALID_IDX 
#	define SPI1_NSS_IO_PIN      0
#	define SPI1_NSS_IO_AF       0
#	define SPI1_SCK_IO_IDX      GPIO_INVALID_IDX 
#	define SPI1_SCK_IO_PIN      0
#	define SPI1_SCK_IO_AF       0
#	define SPI1_MISO_IO_IDX     GPIO_INVALID_IDX 
#	define SPI1_MISO_IO_PIN     0
#	define SPI1_MISO_IO_AF      0
#	define SPI1_MOSI_IO_IDX     GPIO_INVALID_IDX 
#	define SPI1_MOSI_IO_PIN     0
#	define SPI1_MOSI_IO_AF      0


#endif // __BOARD_CFG_VLLINKLITE_GD32F350_H__
