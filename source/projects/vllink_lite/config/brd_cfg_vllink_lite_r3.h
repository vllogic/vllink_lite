#ifndef __BOARD_CFG_VLLINK_LITE_R3_H__
#define __BOARD_CFG_VLLINK_LITE_R3_H__

#define GPIOA_ENABLE            1
#define GPIOB_ENABLE            1
#define GPIOC_ENABLE            1
#define GPIOD_ENABLE            1
#define GPIOF_ENABLE            1

#define USART0_ENABLE           1
#	define USART0_TXD_IO_PORT   GPIOA_PORT
#	define USART0_TXD_IO_PIN    9
#	define USART0_TXD_IO_AF     1
#	define USART0_RXD_IO_PORT   GPIOA_PORT
#	define USART0_RXD_IO_PIN    10
#	define USART0_RXD_IO_AF     1
#	define USART0_CTS_IO_PORT   GPIO_DUMMY_PORT
#	define USART0_CTS_IO_PIN    11
#	define USART0_CTS_IO_AF     1
#	define USART0_RTS_IO_PORT   GPIO_DUMMY_PORT
#	define USART0_RTS_IO_PIN    12
#	define USART0_RTS_IO_AF     1
#define USART1_ENABLE           0
#	define USART1_TXD_IO_PORT   GPIO_DUMMY_PORT // GPIOA_PORT
#	define USART1_TXD_IO_PIN    2
#	define USART1_TXD_IO_AF     1
#	define USART1_RXD_IO_PORT   GPIOA_PORT
#	define USART1_RXD_IO_PIN    3
#	define USART1_RXD_IO_AF     1
#	define USART1_CTS_IO_PORT   GPIO_DUMMY_PORT
#	define USART1_CTS_IO_PIN    1
#	define USART1_CTS_IO_AF     1
#	define USART1_RTS_IO_PORT   GPIO_DUMMY_PORT
#	define USART1_RTS_IO_PIN    1
#	define USART1_RTS_IO_AF     1


#endif // __BOARD_CFG_VLLINK_LITE_R3_H__
