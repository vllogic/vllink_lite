/*
    注意：开启FIFO后，TX完成中断将只在一次发送数据达到(FIFO阈值+2)的情况下生效
*/

/*============================ INCLUDES ======================================*/

#define __VSFSTREAM_CLASS_INHERIT__

#include "usart.h"
#include "io.h"

#if USART_STREAM_ENABLE
#   include "vsf.h" // use eda
#endif

/*============================ MACROS ========================================*/

#define USART_FIFO_SIZE                 16
#define USART_BUFFER_SIZE               (USART_FIFO_SIZE - 2)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef struct usart_control_t {
    uint32_t inited : 1;
    uint32_t fill : 7;
    uint32_t baudrate : 24;
    uint32_t mode;
#if USART_STREAM_ENABLE
    vsf_stream_t *tx;
    vsf_stream_t *rx;
    vsf_eda_t eda;
#endif
    void (*ontx)(void *);
    void (*onrx)(void *);
    void *param;
#if USART_STREAM_ENABLE
    uint8_t stream_rx_buff[USART_BUFFER_SIZE];
    uint8_t stream_rx_size;
#endif
} usart_control_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

#if USART_COUNT > 0

static usart_control_t usart_control[USART_COUNT];

void vsfhal_usart_init(enum usart_idx_t idx)
{
    VSF_HAL_ASSERT(idx < USART_COUNT);

    #if CHIP_CLKEN & MT006_CLKEN_PLL
    RCC->UARTCLKSEL = 1;   // select PLL
    #elif CHIP_CLKEN & MT006_CLKEN_HSE
    RCC->UARTCLKSEL = 2;   // select HSE12M
    #else
    RCC->UARTCLKSEL = 0;   // select HSI12M
    #endif
    RCC->UARTCLKUEN = 0;
    RCC->UARTCLKUEN = 1;

    switch (idx) {
    #if USART0_ENABLE
    case USART0_IDX:
        vsfhal_gpio_config(USART0_TXD_IO_IDX, 0x1 << USART0_TXD_IO_PIN, USART0_TXD_IO_CFG);
        vsfhal_gpio_config(USART0_RXD_IO_IDX, 0x1 << USART0_RXD_IO_PIN, USART0_RXD_IO_CFG);
        RCC->UART0CLKDIV = 1;
        RCC->AHBCLKCTRL0_SET = RCC_AHBCLKCTRL_UART0;
        RCC->PRESETCTRL0_CLR = RCC_PRESETCTRL_UART0;
        RCC->PRESETCTRL0_SET = RCC_PRESETCTRL_UART0;
        break;
    #endif
    }
}

void vsfhal_usart_fini(enum usart_idx_t idx)
{
    VSF_HAL_ASSERT(idx < USART_COUNT);

    switch (idx) {
    #if USART0_ENABLE
    case USART0_IDX:
        RCC->AHBCLKCTRL0_CLR = RCC_AHBCLKCTRL_UART0;
        break;
    #endif
    }
}

uint32_t vsfhal_usart_config(enum usart_idx_t idx, uint32_t baudrate, uint32_t mode)
{
    VSF_HAL_ASSERT(idx < USART_COUNT);

    uint32_t baud;

    #if CHIP_CLKEN & MT006_CLKEN_PLL
    baud = (CHIP_PLL_FREQ_HZ * 4 + baudrate / 2) / baudrate;
    #else
    baud = ((12 * 1000 * 1000) * 4 + baudrate / 2) / baudrate;
    #endif
    
    if (mode == USART_GET_BAUD_ONLY) {
        // NULL
    } else if (mode == USART_RESET_BAUD_ONLY) {
        uint_fast32_t LCRH = UART0->LCRH;
        UART0->CR &= ~UART_CR_UARTEN;
        UART0->IBRD = (baud & 0x3fffc0) >> 6;
        UART0->FBRD = (baud & 0x3f);
        UART0->LCRH = LCRH;
        UART0->CR |= UART_CR_UARTEN;
    } else {
        UART0->CR = UART_CR_TXE | UART_CR_RXE;
        UART0->IBRD = (baud & 0x3fffc0) >> 6;
        UART0->FBRD = (baud & 0x3f);
        UART0->LCRH = (mode & 0xff) | UART_LCRH_FEN;
        UART0->IMSC = UART_IMSC_RXIM | UART_IMSC_RTIM;
        UART0->IFLS = (0x3ul << 3) | (0x0 << 0);    // rx 12/16, tx 2/16
        UART0->CR |= UART_CR_UARTEN;
    }

    #if CHIP_CLKEN & MT006_CLKEN_PLL
    return CHIP_PLL_FREQ_HZ * 4 / baud;
    #else
    return (12 * 1000 * 1000) * 4 / baud;
    #endif
}

void vsfhal_usart_config_cb(enum usart_idx_t idx, int32_t int_priority, void *p, void (*ontx)(void *), void (*onrx)(void *))
{
    VSF_HAL_ASSERT(idx < USART_COUNT);

    usart_control_t *ctrl = &usart_control[idx];
    
    ctrl->ontx = ontx;
    ctrl->onrx = onrx;
    ctrl->param = p;

    if (int_priority >= 0) {
        NVIC_EnableIRQ(UART0_IRQn);
        NVIC_SetPriority(UART0_IRQn, int_priority);
    } else {
        NVIC_DisableIRQ(UART0_IRQn);
    }
}

uint16_t vsfhal_usart_tx_bytes(enum usart_idx_t idx, uint8_t *data, uint16_t size)
{
    VSF_HAL_ASSERT(idx < USART_COUNT);

    uint16_t i;

    UART0->ICR = UART_ICR_TXIC;
    UART0->IMSC |= UART_IMSC_TXIM;
    for (i = 0; i < size; i++) {
        UART0->DR = data[i];
    }
    return size;
}

uint16_t vsfhal_usart_tx_get_free_size(enum usart_idx_t idx)
{
    VSF_HAL_ASSERT(idx < USART_COUNT);

    if (UART0->TFR & UART_FR_TXFE)
        return USART_FIFO_SIZE;
    else if (UART0->TFR & UART_FR_TXFF)
        return 0;
    else
        return 1;
}

uint16_t vsfhal_usart_rx_bytes(enum usart_idx_t idx, uint8_t *data, uint16_t size)
{
    VSF_HAL_ASSERT(idx < USART_COUNT);

    uint16_t i;

    for (i = 0; i < size; i++) {
        if (UART0->TFR & UART_FR_RXFE)
            break;
        data[i] = UART0->DR;
    }
    return i;
}

uint16_t vsfhal_usart_rx_get_data_size(enum usart_idx_t idx)
{
    VSF_HAL_ASSERT(idx < USART_COUNT);

    if (UART0->TFR & UART_FR_RXFE)
        return 0;
    else
        return USART_BUFFER_SIZE;	// Note: vsfhal_usart_rx_bytes will return real rx size
}

#if USART0_ENABLE
ROOT void UART0_IRQHandler(void)
{
    usart_control_t *ctrl = &usart_control[USART0_IDX];

    if (UART0->MIS & UART_MIS_RXMIS) {
        UART0->ICR = UART_ICR_RXIC;
        goto do_rx;
    } else if (UART0->MIS & UART_MIS_RTMIS) {
        UART0->ICR = UART_ICR_RTIC;
    do_rx:
        if (ctrl->onrx)
            ctrl->onrx(ctrl->param);
    }

    if (UART0->MIS & UART_MIS_TXMIS) {
        UART0->IMSC &= ~UART_IMSC_TXIM;
        UART0->ICR = UART_ICR_TXIC;
        if (ctrl->ontx)
            ctrl->ontx(ctrl->param);
    }
}
#endif


#if USART_STREAM_ENABLE

static void stream_dotx(usart_control_t *ctrl, vsf_stream_t *stream)
{
    uint32_t size;
    enum usart_idx_t idx = ((uint32_t)ctrl - (uint32_t)usart_control) / sizeof(usart_control_t);

    size = vsfhal_usart_tx_get_free_size(idx);
    if (size) {
        if (stream->op == &vsf_fifo_stream_op) {
            uint8_t buf[USART_BUFFER_SIZE];
            size = VSF_STREAM_READ(stream, buf, size);
            if (size)
                vsfhal_usart_tx_bytes(idx, buf, size);
        } else if (stream->op == &vsf_block_stream_op) {
            uint8_t *buf;
            size = VSF_STREAM_GET_RBUF(stream, &buf);
            if (size) {
                vsfhal_usart_tx_bytes(idx, buf, size);
                VSF_STREAM_READ(stream, buf, size);
            }
        }
    }
}

static void stream_dorx(usart_control_t *ctrl, vsf_stream_t *stream)
{
    uint32_t size;
    enum usart_idx_t idx = ((uint32_t)ctrl - (uint32_t)usart_control) / sizeof(usart_control_t);

    if (stream->op == &vsf_fifo_stream_op) {
        size = ctrl->stream_rx_size;
        if (size && VSF_STREAM_IS_RX_CONNECTED(stream))
            VSF_STREAM_WRITE(stream, ctrl->stream_rx_buff, size);
    } else if (stream->op == &vsf_block_stream_op) {
        uint8_t *buf;
        size = min(VSF_STREAM_GET_WBUF(stream, &buf), ctrl->stream_rx_size);
        if (size && VSF_STREAM_IS_RX_CONNECTED(stream)) {
            memcpy(buf, ctrl->stream_rx_buff, size);
            VSF_STREAM_WRITE(stream, buf, size);
        }
    }
    ctrl->stream_rx_size = 0;
}

enum {
    VSF_EVT_TX_STREAM_ONRX      = VSF_EVT_USER + 0,
    VSF_EVT_TX_STREAM_ONTX      = VSF_EVT_USER + 1,
    VSF_EVT_RX_STREAM_ONRX      = VSF_EVT_USER + 2,
    VSF_EVT_RX_STREAM_ONTX      = VSF_EVT_USER + 3,
};

static void usart_stream_evthandler(vsf_eda_t *eda, vsf_evt_t evt)
{
    usart_control_t *ctrl = container_of(eda, usart_control_t, eda);
    vsf_stream_t *tx_stream = ctrl->tx;
    vsf_stream_t *rx_stream = ctrl->rx;

    switch (evt) {
    case VSF_EVT_INIT:
        break;
    case VSF_EVT_TX_STREAM_ONRX:
    case VSF_EVT_TX_STREAM_ONTX:
        if (VSF_STREAM_GET_DATA_SIZE(tx_stream)) {
            stream_dotx(ctrl, tx_stream);
        }
        break;
    case VSF_EVT_RX_STREAM_ONRX:
        stream_dorx(ctrl, rx_stream);
        break;
    case VSF_EVT_RX_STREAM_ONTX:
        if (VSF_STREAM_GET_DATA_SIZE(rx_stream)) {
            // used to call __vsf_stream_on_write
            vsf_stream_set_rx_threshold(rx_stream, rx_stream->rx.threshold);
        }
        break;
    }
}

static void tx_stream_rx_evthandler(void *param, vsf_stream_evt_t evt)
{
    usart_control_t *ctrl = param;

    if (evt == VSF_STREAM_ON_RX) {
        vsf_eda_post_evt(&ctrl->eda, VSF_EVT_TX_STREAM_ONRX);
    }
}

static void rx_stream_tx_evthandler(void *param, vsf_stream_evt_t evt)
{
    usart_control_t *ctrl = param;

    if (evt == VSF_STREAM_ON_TX) {
        vsf_eda_post_evt(&ctrl->eda, VSF_EVT_RX_STREAM_ONTX);
    }
}

static void stream_ontx(void *param)
{
    usart_control_t *ctrl = param;
    vsf_eda_post_evt(&ctrl->eda, VSF_EVT_TX_STREAM_ONTX);
}

static void stream_onrx(void *param)
{
    usart_control_t *ctrl = param;
    if (ctrl->stream_rx_size == 0) {
        uint32_t size;
        enum usart_idx_t idx = ((uint32_t)ctrl - (uint32_t)usart_control) / sizeof(usart_control_t);
        size = vsfhal_usart_rx_get_data_size(idx);
        if (size) {
            ctrl->stream_rx_size = vsfhal_usart_rx_bytes(idx, ctrl->stream_rx_buff, size);
            vsf_eda_post_evt(&ctrl->eda, VSF_EVT_RX_STREAM_ONRX);
        }
    }
}

static void vsfhal_usart_stream_fini(enum usart_idx_t idx)
{
    if (usart_control[idx].tx || usart_control[idx].rx)
        vsfhal_usart_config_cb(idx, 0, NULL, NULL, NULL);
    if (usart_control[idx].tx) {
        VSF_STREAM_DISCONNECT_RX(usart_control[idx].tx);
        usart_control[idx].tx->rx.evthandler = NULL;
        usart_control[idx].tx->rx.param = NULL;
    }
    if (usart_control[idx].rx) {
        VSF_STREAM_DISCONNECT_TX(usart_control[idx].rx);
        usart_control[idx].rx->tx.evthandler = NULL;
        usart_control[idx].rx->tx.param = NULL;
    }
}

void vsfhal_usart_stream_config(enum usart_idx_t idx, int32_t eda_priority, int32_t int_priority, vsf_stream_t *tx, vsf_stream_t *rx)
{
    vsfhal_usart_stream_fini(idx);
    
    if (!tx && !rx)
        return;

    const vsf_eda_cfg_t cfg = {
        .fn.evthandler  = usart_stream_evthandler,
        .priority       = eda_priority,
    };
    vsf_eda_start(&usart_control[idx].eda, (vsf_eda_cfg_t *)&cfg);

    usart_control[idx].tx = tx;
    usart_control[idx].rx = rx;
    vsfhal_usart_config_cb(idx, int_priority, &usart_control[idx], stream_ontx, stream_onrx);

    if (tx) {
        tx->rx.evthandler = tx_stream_rx_evthandler;
        tx->rx.param = &usart_control[idx];
        VSF_STREAM_CONNECT_RX(tx);
    }
    if (rx) {
        rx->tx.evthandler = rx_stream_tx_evthandler;
        rx->tx.param = &usart_control[idx];
        VSF_STREAM_CONNECT_TX(rx);
    }
}
#endif

#endif
