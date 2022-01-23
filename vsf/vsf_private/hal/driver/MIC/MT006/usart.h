#ifndef __HAL_DRIVER_MIC_MT006_USART_H__
#define __HAL_DRIVER_MIC_MT006_USART_H__

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"
#include "__device.h"
#if USART_STREAM_ENABLE
#   include "service/simple_stream/vsf_simple_stream.h"
#endif

/*============================ MACROS ========================================*/

#ifndef USART0_ENABLE
#   define USART0_ENABLE        0
#endif

#define USART_COUNT             (0 + USART0_ENABLE)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

#if USART_COUNT
enum usart_idx_t {
    #if USART0_ENABLE
    USART0_IDX,
    #endif
    USART_IDX_NUM,
    USART_INVALID_IDX,
};
#endif

enum usart_mode_t{
    USART_BREAK                 = (0x1ul << 0),
    USART_PARITY_NONE           = (0x0ul << 1),
    USART_PARITY_ODD            = (0x1ul << 1),
    USART_PARITY_EVEN           = (0x3ul << 1),
    USART_STOPBITS_1            = (0x0ul << 3),
    USART_STOPBITS_2            = (0x1ul << 3),
    USART_BITLEN_5              = (0x0ul << 5),
    USART_BITLEN_6              = (0x1ul << 5),
    USART_BITLEN_7              = (0x2ul << 5),
    USART_BITLEN_8              = (0x3ul << 5),
    
    USART_GET_BAUD_ONLY         = (0x1ul << 30),
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
void vsfhal_usart_stream_config(enum usart_idx_t idx, int32_t eda_priority, int32_t int_priority, vsf_stream_t *tx, vsf_stream_t *rx);
#endif
#endif

#endif
/* EOF */
