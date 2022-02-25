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
#define USART_TX_FIFO_SIZE              USART_FIFO_SIZE
#define USART_RX_FIFO_SIZE              USART_FIFO_SIZE
#define USART_RX_BUFFER_NUM             4

#define USART_TX_IRQ_USE_TIMER

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef struct usart_control_t {
    uint32_t inited : 1;
    uint32_t fill : 7;
    uint32_t baudrate : 24;
    uint32_t mode;
#ifdef USART_TX_IRQ_USE_TIMER
    uint32_t byte_ticks;
#endif
#if USART_STREAM_ENABLE
    vsf_stream_t *tx;
    uint8_t *tx_block_buf;
    uint16_t tx_block_buf_pos;
    uint16_t tx_block_buf_size;
    vsf_stream_t *rx;
    uint8_t rx_buffer[USART_RX_BUFFER_NUM][USART_RX_FIFO_SIZE];
    uint8_t rx_buffer_pos[USART_RX_BUFFER_NUM];
    uint8_t rx_buffer_w_select;
    uint8_t rx_buffer_r_select;
    bool rx_onrx;
    vsf_eda_t eda;
#endif
    void (*ontx)(void *);
    void (*onrx)(void *);
    void *param;
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
    memset(&usart_control[idx], 0, sizeof(usart_control_t));
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
    usart_control_t *ctrl = &usart_control[idx];

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
    } else if (ctrl->mode != mode){
        ctrl->mode = mode;
        UART0->CR &= ~UART_CR_UARTEN;
        UART0->IBRD = (baud & 0x3fffc0) >> 6;
        UART0->FBRD = (baud & 0x3f);
        UART0->LCRH = (mode & 0xff) | UART_LCRH_FEN;
        UART0->IMSC = UART_IMSC_RXIM | UART_IMSC_RTIM;
        UART0->IFLS = (0x3ul << 3) | (0x0 << 0);    // rx 12/16, tx 2/16
        UART0->CR |= UART_CR_UARTEN;
    }
    
    #if CHIP_CLKEN & MT006_CLKEN_PLL
    baudrate = CHIP_PLL_FREQ_HZ * 4 / baud;
    #else
    baudrate = (12 * 1000 * 1000) * 4 / baud;
    #endif
    
    ctrl->baudrate = baudrate;

    #ifdef USART_TX_IRQ_USE_TIMER
    #if CHIP_CLKEN & MT006_CLKEN_PLL
    RCC->OUTCLKSEL = RCC_OUTCLKSEL_SEL_SYSTEM_PLL;
    RCC->OUTCLKDIV = CHIP_PLL_FREQ_HZ / 12000000;
    RCC->OUTCLKUEN = 0;
    RCC->OUTCLKUEN = 1;
    #else
    #   error "Not Support other clock source"
    #endif
    RCC->AHBCLKCTRL0_SET = RCC_AHBCLKCTRL_STIMER;
    RCC->PRESETCTRL0_CLR = RCC_PRESETCTRL_STIMER;
    RCC->PRESETCTRL0_SET = RCC_PRESETCTRL_STIMER;

    ST->CONTROLREG1 = 0;
    ST->CONTROLREG1 = ST_CTRL_MODE;
    //ST->LOADCOUNT1 = 1000 - 1;
    //ST->CONTROLREG1 |= ST_CTRL_ENABLE;
    ctrl->byte_ticks = 12000000 * 10 / baudrate;
    //ctrl->byte_ticks += ctrl->byte_ticks / 16;
    #endif
    
    return baudrate;
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
        

        #ifdef USART_TX_IRQ_USE_TIMER
        NVIC_EnableIRQ(STIMER_IRQn);
        NVIC_SetPriority(STIMER_IRQn, int_priority);
        #endif
    } else {
        NVIC_DisableIRQ(UART0_IRQn);
        NVIC_DisableIRQ(STIMER_IRQn);
    }
}

uint16_t vsfhal_usart_tx_bytes(enum usart_idx_t idx, uint8_t *data, uint16_t size)
{
    VSF_HAL_ASSERT(idx < USART_COUNT);

    if (UART0->TFR & UART_FR_TXFF) {
        return 0;
    } else {
        uint16_t i;
        UART0->ICR = UART_ICR_TXIC;
        for (i = 0; i < size; i++) {
            if (UART0->TFR & UART_FR_TXFF)
                break;
            UART0->DR = data[i];
        }
        

        #ifndef USART_TX_IRQ_USE_TIMER
        UART0->IMSC |= UART_IMSC_TXIM;
        #else
        uint_fast32_t load = usart_control[idx].byte_ticks * i;
        ST->LOADCOUNT1 = load - 1;
        ST->CONTROLREG1 |= ST_CTRL_ENABLE;
        #endif
        return i;
    }
}

uint16_t vsfhal_usart_tx_get_free_size(enum usart_idx_t idx)
{
    VSF_HAL_ASSERT(idx < USART_COUNT);

    if (UART0->TFR & UART_FR_TXFE)
        return USART_TX_FIFO_SIZE;
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
        return USART_RX_FIFO_SIZE;  // Note: vsfhal_usart_rx_bytes will return real rx size
}

#if USART0_ENABLE
ROOT void UART0_IRQHandler(void)
{
    uint_fast32_t mis = UART0->MIS;
    usart_control_t *ctrl = &usart_control[USART0_IDX];

    if (mis & (UART_MIS_RXMIS | UART_MIS_RTMIS)) {
        UART0->ICR = UART_ICR_RXIC | UART_ICR_RTIC;
        if (ctrl->onrx)
            ctrl->onrx(ctrl->param);
    }

    #ifndef USART_TX_IRQ_USE_TIMER
    /*
    注意：开启FIFO后，TX完成中断将只在一次发送数据达到(FIFO阈值+2)的情况下生效
    */
    if (mis & UART_MIS_TXMIS) {
        UART0->IMSC &= ~UART_IMSC_TXIM;
        UART0->ICR = UART_ICR_TXIC;
        if (ctrl->ontx)
            ctrl->ontx(ctrl->param);
    }
    #endif
}

#ifdef USART_TX_IRQ_USE_TIMER
#if TICKCNT_ENABLE
bool usart_tx_callback(void)
{
    uint_fast32_t status;
    usart_control_t *ctrl = &usart_control[USART0_IDX];
    
    if (!ctrl->ontx) {
        return false;
    }

    ST->CONTROLREG1 &= ~ST_CTRL_ENABLE;
    status = ST->EOI;
    if (ctrl->ontx)
        ctrl->ontx(ctrl->param);
    else if (status)    // dummy read
        NOP();
    return true;
}
#else
ROOT void STIMER_IRQHandler(void)
{
    uint_fast32_t status;
    usart_control_t *ctrl = &usart_control[USART0_IDX];
    ST->CONTROLREG1 &= ~ST_CTRL_ENABLE;
    status = ST->EOI;
    if (ctrl->ontx)
        ctrl->ontx(ctrl->param);
    else if (status)    // dummy read
        NOP();
}
#endif

#endif
#endif


#if USART_STREAM_ENABLE

static enum usart_idx_t get_idx_by_control(void *ctrl)
{
    return ((uint32_t)ctrl - (uint32_t)&usart_control[0]) / sizeof(usart_control_t);
}

static void stream_dotx(usart_control_t *ctrl, vsf_stream_t *stream)
{
    uint32_t size;
    enum usart_idx_t idx = get_idx_by_control(ctrl);

    size = vsfhal_usart_tx_get_free_size(idx);
    if (size) {
        if (stream->op == &vsf_fifo_stream_op) {
            uint8_t buf[USART_BUFFER_SIZE];
            size = VSF_STREAM_READ(stream, buf, size);
            if (size)
                vsfhal_usart_tx_bytes(idx, buf, size);
        } else if (stream->op == &vsf_block_stream_op) {
            uint8_t *buf;
            uint16_t buf_size = VSF_STREAM_GET_RBUF(stream, &buf);
            if (buf_size) {
                uint16_t buf_pos = vsfhal_usart_tx_bytes(idx, buf, buf_size);
                if (buf_pos < buf_size) {
                    ctrl->tx_block_buf_pos = buf_pos;
                    ctrl->tx_block_buf_size = buf_size;
                    ctrl->tx_block_buf = buf;
                } else {
                    VSF_STREAM_READ(stream, buf, buf_size);
                }
            }
        }
    }
}

static void stream_dorx(usart_control_t *ctrl, vsf_stream_t *stream)
{
    uint32_t size;
    enum usart_idx_t idx = ((uint32_t)ctrl - (uint32_t)usart_control) / sizeof(usart_control_t);

    if (stream->op == &vsf_fifo_stream_op) {
        uint8_t buf[USART_RX_FIFO_SIZE];
        size = vsfhal_usart_rx_bytes(idx, buf, USART_RX_FIFO_SIZE);
        if (size && VSF_STREAM_IS_RX_CONNECTED(stream))
            VSF_STREAM_WRITE(stream, buf, size);
    } else if (stream->op == &vsf_block_stream_op) {
        // NULL
    }
}

enum {
    VSF_EVT_TX_STREAM_ONRX      = VSF_EVT_USER + 0,
    VSF_EVT_TX_STREAM_ONTX      = VSF_EVT_USER + 1,
    VSF_EVT_RX_STREAM_ONRX      = VSF_EVT_USER + 2,
    VSF_EVT_RX_STREAM_ONTX      = VSF_EVT_USER + 3,
};

static uint8_t get_next_select(uint8_t cur)
{
    if (++cur >= USART_RX_BUFFER_NUM)
        cur = 0;
    return cur;
}

static void usart_stream_evthandler(vsf_eda_t *eda, vsf_evt_t evt)
{
    usart_control_t *ctrl = container_of(eda, usart_control_t, eda);
    vsf_stream_t *tx_stream = ctrl->tx;
    vsf_stream_t *rx_stream = ctrl->rx;

    switch (evt) {
    case VSF_EVT_INIT:
        break;
    case VSF_EVT_TX_STREAM_ONRX:    // new data need to send
    case VSF_EVT_TX_STREAM_ONTX:    // data send finish
    tx_ontx:
        if (ctrl->tx_block_buf) {
            enum usart_idx_t idx = get_idx_by_control(ctrl);
            ctrl->tx_block_buf_pos += vsfhal_usart_tx_bytes(idx, ctrl->tx_block_buf + ctrl->tx_block_buf_pos, ctrl->tx_block_buf_size - ctrl->tx_block_buf_pos);
            if (ctrl->tx_block_buf_pos == ctrl->tx_block_buf_size) {
                VSF_STREAM_READ(tx_stream, ctrl->tx_block_buf, ctrl->tx_block_buf_size);
                ctrl->tx_block_buf = NULL;
                ctrl->tx_block_buf_pos = 0;
                ctrl->tx_block_buf_size = 0;
            }
        } else if (VSF_STREAM_GET_DATA_SIZE(tx_stream)) {
            stream_dotx(ctrl, tx_stream);
        }
        break;
    case VSF_EVT_RX_STREAM_ONRX:
        ctrl->rx_onrx = false;
        {
            uint8_t *w_buf = NULL;
            uint32_t w_buf_size, w_size = 0;
            uint8_t rx_pos;
            
            if (VSF_STREAM_IS_RX_CONNECTED(rx_stream) && VSF_STREAM_GET_FREE_SIZE(rx_stream)) {
                w_buf_size = VSF_STREAM_GET_WBUF(rx_stream, &w_buf);
            }
            if (w_buf) {
                for (rx_pos = ctrl->rx_buffer_pos[ctrl->rx_buffer_r_select]; (rx_pos != 0) && (rx_pos <= w_buf_size);) {
                    memcpy(w_buf + w_size, ctrl->rx_buffer[ctrl->rx_buffer_r_select], rx_pos);
                    w_size += rx_pos;
                    w_buf_size -= rx_pos;
                    ctrl->rx_buffer_pos[ctrl->rx_buffer_r_select] = 0;
                    ctrl->rx_buffer_r_select = get_next_select(ctrl->rx_buffer_r_select);
                    rx_pos = ctrl->rx_buffer_pos[ctrl->rx_buffer_r_select];
                }
                if (w_size) {
                    VSF_STREAM_WRITE(rx_stream, NULL, w_size);
                }
            } else {
                for (rx_pos = ctrl->rx_buffer_pos[ctrl->rx_buffer_r_select]; rx_pos != 0;) {
                    ctrl->rx_buffer_pos[ctrl->rx_buffer_r_select] = 0;
                    ctrl->rx_buffer_r_select = get_next_select(ctrl->rx_buffer_r_select);
                    rx_pos = ctrl->rx_buffer_pos[ctrl->rx_buffer_r_select];
                }
            }
        }
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
    enum usart_idx_t idx = get_idx_by_control(ctrl);
    uint8_t next_w = get_next_select(ctrl->rx_buffer_w_select);
    
    if (next_w == ctrl->rx_buffer_r_select) {
        uint8_t buf[USART_RX_FIFO_SIZE];
        vsfhal_usart_rx_bytes(get_idx_by_control(ctrl), buf, USART_RX_FIFO_SIZE);
        return;
    }

    uint8_t *rx_buffer = ctrl->rx_buffer[ctrl->rx_buffer_w_select];
    uint8_t rx_pos;

    rx_pos = vsfhal_usart_rx_bytes(idx, rx_buffer, USART_RX_FIFO_SIZE);
    if (rx_pos) {
        ctrl->rx_buffer_pos[ctrl->rx_buffer_w_select] = rx_pos;
        ctrl->rx_buffer_w_select = next_w;
        
        if (!ctrl->rx_onrx) {
            ctrl->rx_onrx = true;
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

    if (usart_control[idx].inited == false) {
        const vsf_eda_cfg_t cfg = {
            .fn.evthandler  = usart_stream_evthandler,
            .priority       = eda_priority,
        };
        vsf_eda_start(&usart_control[idx].eda, (vsf_eda_cfg_t *)&cfg);

        usart_control[idx].inited = true;
    }

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
