#ifndef __BOARD_CFG_VLLINKLITE_GD32F350_H__
#define __BOARD_CFG_VLLINKLITE_GD32F350_H__

#define DMA0_ENABLE             1

#define FLASH0_ENABLE           1

#define WDT_FREE_ENABLE         1
#define WDT_WINDOW_ENABLE       0

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

#define I2C0_ENABLE             1
#	define I2C0_SCL_IO_IDX      GPIO_INVALID_IDX 
#	define I2C0_SCL_IO_PIN      0
#	define I2C0_SCL_IO_AF       0
#	define I2C0_SDA_IO_IDX      GPIO_INVALID_IDX 
#	define I2C0_SDA_IO_PIN      0
#	define I2C0_SDA_IO_AF       0
#define I2C1_ENABLE             1
#	define I2C1_SCL_IO_IDX      GPIO_INVALID_IDX 
#	define I2C1_SCL_IO_PIN      0
#	define I2C1_SCL_IO_AF       0
#	define I2C1_SDA_IO_IDX      GPIO_INVALID_IDX 
#	define I2C1_SDA_IO_PIN      0
#	define I2C1_SDA_IO_AF       0

#define PWM_TIMER0_ENABLE           1
#   define PWM_TIMER0_CH0_IO_IDX    GPIO_INVALID_IDX 
#   define PWM_TIMER0_CH0_IO_PIN    0
#   define PWM_TIMER0_CH0_IO_AF     0
#   define PWM_TIMER0_CH1_IO_IDX    GPIO_INVALID_IDX
#   define PWM_TIMER0_CH1_IO_PIN    0
#   define PWM_TIMER0_CH1_IO_AF     0
#   define PWM_TIMER0_CH2_IO_IDX    GPIO_INVALID_IDX
#   define PWM_TIMER0_CH2_IO_PIN    0
#   define PWM_TIMER0_CH2_IO_AF     0
#   define PWM_TIMER0_CH3_IO_IDX    GPIO_INVALID_IDX
#   define PWM_TIMER0_CH3_IO_PIN    0
#   define PWM_TIMER0_CH3_IO_AF     0
#define PWM_TIMER1_ENABLE           1
#   define PWM_TIMER1_CH0_IO_IDX    GPIO_INVALID_IDX 
#   define PWM_TIMER1_CH0_IO_PIN    0
#   define PWM_TIMER1_CH0_IO_AF     0
#   define PWM_TIMER1_CH1_IO_IDX    GPIO_INVALID_IDX
#   define PWM_TIMER1_CH1_IO_PIN    0
#   define PWM_TIMER1_CH1_IO_AF     0
#   define PWM_TIMER1_CH2_IO_IDX    GPIO_INVALID_IDX
#   define PWM_TIMER1_CH2_IO_PIN    0
#   define PWM_TIMER1_CH2_IO_AF     0
#   define PWM_TIMER1_CH3_IO_IDX    GPIO_INVALID_IDX
#   define PWM_TIMER1_CH3_IO_PIN    0
#   define PWM_TIMER1_CH3_IO_AF     0
#define PWM_TIMER2_ENABLE           1
#   define PWM_TIMER2_CH0_IO_IDX    GPIO_INVALID_IDX 
#   define PWM_TIMER2_CH0_IO_PIN    0
#   define PWM_TIMER2_CH0_IO_AF     0
#   define PWM_TIMER2_CH1_IO_IDX    GPIO_INVALID_IDX
#   define PWM_TIMER2_CH1_IO_PIN    0
#   define PWM_TIMER2_CH1_IO_AF     0
#   define PWM_TIMER2_CH2_IO_IDX    GPIO_INVALID_IDX
#   define PWM_TIMER2_CH2_IO_PIN    0
#   define PWM_TIMER2_CH2_IO_AF     0
#   define PWM_TIMER2_CH3_IO_IDX    GPIO_INVALID_IDX
#   define PWM_TIMER2_CH3_IO_PIN    0
#   define PWM_TIMER2_CH3_IO_AF     0
#define PWM_TIMER13_ENABLE          1
#   define PWM_TIMER13_CH0_IO_IDX   GPIO_INVALID_IDX 
#   define PWM_TIMER13_CH0_IO_PIN   0
#   define PWM_TIMER13_CH0_IO_AF    0
#define PWM_TIMER14_ENABLE          1
#   define PWM_TIMER14_CH0_IO_IDX   GPIO_INVALID_IDX 
#   define PWM_TIMER14_CH0_IO_PIN   0
#   define PWM_TIMER14_CH0_IO_AF    0
#   define PWM_TIMER14_CH1_IO_IDX   GPIO_INVALID_IDX 
#   define PWM_TIMER14_CH1_IO_PIN   0
#   define PWM_TIMER14_CH1_IO_AF    0
#define PWM_TIMER15_ENABLE          1
#   define PWM_TIMER15_CH0_IO_IDX   GPIO_INVALID_IDX 
#   define PWM_TIMER15_CH0_IO_PIN   0
#   define PWM_TIMER15_CH0_IO_AF    0
#define PWM_TIMER16_ENABLE          1
#   define PWM_TIMER16_CH0_IO_IDX   GPIO_INVALID_IDX 
#   define PWM_TIMER16_CH0_IO_PIN   0
#   define PWM_TIMER16_CH0_IO_AF    0

#define ADC0_ENABLE                         1
#	define ADC0_AUTO_CALIBRATION_ENABLE     1
#	define ADC0_CH0_ENABLE                  1
#	define ADC0_CH1_ENABLE                  0
#	define ADC0_CH2_ENABLE                  0
#	define ADC0_CH3_ENABLE                  0
#	define ADC0_CH4_ENABLE                  0
#	define ADC0_CH5_ENABLE                  0
#	define ADC0_CH6_ENABLE                  0
#	define ADC0_CH7_ENABLE                  0
#	define ADC0_CH8_ENABLE                  0
#	define ADC0_CH9_ENABLE                  0
#	define ADC0_CH10_ENABLE                 0
#	define ADC0_CH11_ENABLE                 0
#	define ADC0_CH12_ENABLE                 0
#	define ADC0_CH13_ENABLE                 0
#	define ADC0_CH14_ENABLE                 0
#	define ADC0_CH15_ENABLE                 0
#	define ADC0_CH16_ENABLE                 0
#	define ADC0_CH17_ENABLE                 0
#	define ADC0_CH18_ENABLE                 0

#if (VSF_USE_USB_DEVICE == ENABLED) || (VSF_USE_USB_HOST == ENABLED)
#   define APP_CFG_USBD_SPEED               USB_DC_SPEED_FULL
#endif


#endif // __BOARD_CFG_VLLINKLITE_GD32F350_H__
