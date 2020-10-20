#ifndef __HAL_DRIVER_GIGADEVICE_GD32E10X_USART_H__
#define __HAL_DRIVER_GIGADEVICE_GD32E10X_USART_H__

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"
#include "../__device.h"
#if USART_STREAM_ENABLE
#   include "service/vsfstream/vsfstream.h"
#endif

/*============================ MACROS ========================================*/

#ifndef USART0_ENABLE
#   define USART0_ENABLE        0
#endif
#ifndef USART1_ENABLE
#   define USART1_ENABLE        0
#endif
#ifndef USART2_ENABLE
#   define USART2_ENABLE        0
#endif
#ifndef USART3_ENABLE
#   define USART3_ENABLE        0
#endif
#ifndef USART4_ENABLE
#   define USART4_ENABLE        0
#endif

#define USART_COUNT             (0 + USART0_ENABLE + USART1_ENABLE + USART2_ENABLE + USART3_ENABLE + USART4_ENABLE)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

#if USART_COUNT
enum usart_idx_t {
    #if USART0_ENABLE
    USART0_IDX,
    #endif
    #if USART1_ENABLE
    USART1_IDX,
    #endif
    #if USART2_ENABLE
    USART2_IDX,
    #endif
    #if USART3_ENABLE
    USART3_IDX,
    #endif
    #if USART4_ENABLE
    USART4_IDX,
    #endif
    USART_IDX_NUM,
    USART_INVALID_IDX,
};
#endif

enum usart_mode_t{
    USART_PARITY_NONE           = (0x0ul << (-8 + 9)),
    USART_PARITY_ODD            = (0x3ul << (-8 + 9)),
    USART_PARITY_EVEN           = (0x2ul << (-8 + 9)),
    USART_STOPBITS_0P5          = (0x1ul << (-8 + 12)),
    USART_STOPBITS_1            = (0x0ul << (-8 + 12)),
    USART_STOPBITS_1P5          = (0x3ul << (-8 + 12)),
    USART_STOPBITS_2            = (0x2ul << (-8 + 12)),
    USART_HALF_DUPLEX           = (0x1ul << (4 + 3)),
    USART_CTS                   = (0x3ul << (4 + 9)),
    USART_RTS                   = (0x2ul << (4 + 8)),
    USART_MSB_FIRST             = (0x1ul << (8 + 11)),
    USART_DATA_INV              = (0x1ul << (8 + 10)),
    USART_TX_INV                = (0x1ul << (8 + 9)),
    USART_RX_INV                = (0x1ul << (8 + 8)),
    USART_RESET_BAUD_ONLY       = (0x1ul << 31),
};

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ INCLUDES ======================================*/
/*============================ PROTOTYPES ====================================*/

#if USART_COUNT
void vsfhal_usart_init(enum usart_idx_t idx);
void vsfhal_usart_fini(enum usart_idx_t idx);
uint32_t vsfhal_usart_config(enum usart_idx_t idx, uint32_t baudrate, uint32_t mode);
void vsfhal_usart_config_cb(enum usart_idx_t idx, int32_t int_priority, void *p, void (*ontx)(void *), void (*onrx)(void *));
uint16_t vsfhal_usart_tx_bytes(enum usart_idx_t idx, uint8_t *data, uint16_t size);
uint16_t vsfhal_usart_tx_get_free_size(enum usart_idx_t idx);
uint16_t vsfhal_usart_rx_bytes(enum usart_idx_t idx, uint8_t *data, uint16_t size);
uint16_t vsfhal_usart_rx_get_data_size(enum usart_idx_t idx);
#if USART_STREAM_ENABLE
void vsfhal_usart_stream_init(enum usart_idx_t idx, int32_t eda_priority, int32_t int_priority, vsf_stream_t *tx, vsf_stream_t *rx);
#endif
#endif

#endif
/* EOF */
