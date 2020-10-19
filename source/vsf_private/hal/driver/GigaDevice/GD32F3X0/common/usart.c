/*============================ INCLUDES ======================================*/

#define __VSFSTREAM_CLASS_INHERIT__

#include "usart.h"
#include "io.h"
#include "dma.h"

#if USART_STREAM_ENABLE
#   include "vsf.h" // use eda
#endif

/*============================ MACROS ========================================*/

#ifndef USART_BUFF_SIZE
#   define USART_BUFF_SIZE              32
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef struct usart_control_t {
#if USART_STREAM_ENABLE
    vsf_stream_t *tx;
    vsf_stream_t *rx;
    vsf_eda_t eda;
#endif
    void (*ontx)(void *);
    void (*onrx)(void *);
    void *param;
    uint8_t tx_buff[USART_BUFF_SIZE];
    uint8_t rx_buff[USART_BUFF_SIZE * 2];
    uint8_t rx_buff_w_pos;  // Non-DMA RX
    uint8_t rx_buff_r_pos;
    uint8_t tx_size;
    uint8_t dma_idx;
    uint8_t tx_dma_ch;
    uint8_t rx_dma_ch;
} usart_control_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

static void usart_tx_done(void *p);
static void usart_rx_done(void *p);

/*============================ IMPLEMENTATION ================================*/

#if USART_COUNT > 0

static const uint32_t usart_reg_base_list[USART_COUNT] = {
    #if USART0_ENABLE
    USART0_BASE,
    #endif  // USART0_ENABLE
    #if USART1_ENABLE
    USART1_BASE,
    #endif  // USART1_ENABLE
};

static uint8_t usart_dma_channel[USART_COUNT][2] = {   // DMA, TX RX
    #if USART0_ENABLE
    {1, 2},
    #endif  // USART0_ENABLE
    #if USART1_ENABLE
    {3, 4},
    #endif  // USART1_ENABLE
};

static const IRQn_Type usart_irqn_list[USART_COUNT] = {
    #if USART0_ENABLE
    USART0_IRQn,
    #endif  // USART0_ENABLE
    #if USART1_ENABLE
    TIMER5_DAC_IRQn,
    #endif  // USART1_ENABLE
};

static usart_control_t usart_control[USART_COUNT];

void vsfhal_usart_init(enum usart_idx_t idx)
{
    VSF_HAL_ASSERT(idx < USART_IDX_NUM);

    switch (idx) {
    #if USART0_ENABLE
    case USART0_IDX:
		RCU_APB2EN |= RCU_APB2EN_USART0EN;
        vsfhal_gpio_init(USART0_TXD_IO_IDX);
        vsfhal_gpio_init(USART0_RXD_IO_IDX);
        vsfhal_gpio_init(USART0_CTS_IO_IDX);
        vsfhal_gpio_init(USART0_RTS_IO_IDX);
        vsfhal_gpio_config(USART0_TXD_IO_IDX, 0x1 << USART0_TXD_IO_PIN, USART0_TXD_IO_AF);
        vsfhal_gpio_config(USART0_RXD_IO_IDX, 0x1 << USART0_RXD_IO_PIN, USART0_RXD_IO_AF);
        vsfhal_gpio_config(USART0_CTS_IO_IDX, 0x1 << USART0_CTS_IO_PIN, USART0_CTS_IO_AF);
        vsfhal_gpio_config(USART0_RTS_IO_IDX, 0x1 << USART0_RTS_IO_PIN, USART0_RTS_IO_AF);
        break;
    #endif
    #if USART1_ENABLE
    case USART1_IDX:
		RCU_APB1EN |= RCU_APB1EN_USART1EN;
        vsfhal_gpio_init(USART1_TXD_IO_IDX);
        vsfhal_gpio_init(USART1_RXD_IO_IDX);
        vsfhal_gpio_init(USART1_CTS_IO_IDX);
        vsfhal_gpio_init(USART1_RTS_IO_IDX);
        vsfhal_gpio_config(USART1_TXD_IO_IDX, 0x1 << USART1_TXD_IO_PIN, USART1_TXD_IO_AF);
        vsfhal_gpio_config(USART1_RXD_IO_IDX, 0x1 << USART1_RXD_IO_PIN, USART1_RXD_IO_AF);
        vsfhal_gpio_config(USART1_CTS_IO_IDX, 0x1 << USART1_CTS_IO_PIN, USART1_CTS_IO_AF);
        vsfhal_gpio_config(USART1_RTS_IO_IDX, 0x1 << USART1_RTS_IO_PIN, USART1_RTS_IO_AF);
        break;
    #endif
    }
    memset(&usart_control[idx], 0, sizeof(usart_control_t));
}

void vsfhal_usart_fini(enum usart_idx_t idx)
{
    VSF_HAL_ASSERT(idx < USART_IDX_NUM);

    switch (idx) {
    #if USART0_ENABLE
    case USART0_IDX:
        vsfhal_gpio_config(USART0_TXD_IO_IDX, 0x1 << USART0_TXD_IO_PIN, IO_INPUT_FLOAT);
        vsfhal_gpio_config(USART0_RXD_IO_IDX, 0x1 << USART0_RXD_IO_PIN, IO_INPUT_FLOAT);
        vsfhal_gpio_config(USART0_CTS_IO_IDX, 0x1 << USART0_CTS_IO_PIN, IO_INPUT_FLOAT);
        vsfhal_gpio_config(USART0_RTS_IO_IDX, 0x1 << USART0_RTS_IO_PIN, IO_INPUT_FLOAT);
		RCU_APB2EN &= ~RCU_APB2EN_USART0EN;
        break;
    #endif
    #if USART1_ENABLE
    case USART1_IDX:
        vsfhal_gpio_config(USART1_TXD_IO_IDX, 0x1 << USART1_TXD_IO_PIN, IO_INPUT_FLOAT);
        vsfhal_gpio_config(USART1_RXD_IO_IDX, 0x1 << USART1_RXD_IO_PIN, IO_INPUT_FLOAT);
        vsfhal_gpio_config(USART1_CTS_IO_IDX, 0x1 << USART1_CTS_IO_PIN, IO_INPUT_FLOAT);
        vsfhal_gpio_config(USART1_RTS_IO_IDX, 0x1 << USART1_RTS_IO_PIN, IO_INPUT_FLOAT);
		RCU_APB1EN &= ~RCU_APB1EN_USART1EN;
        break;
    #endif
    }
}

#if USART1_ENABLE
static void enable_overtimer(uint32_t timer_clk)
{
    if (!(RCU_APB1EN & RCU_APB1EN_TIMER5EN)) {
        RCU_APB1EN |= RCU_APB1EN_TIMER5EN;
        TIMER_CTL0(TIMER5) = TIMER_CTL0_ARSE;
        TIMER_DMAINTEN(TIMER5) = TIMER_DMAINTEN_UPIE;
        TIMER_PSC(TIMER5) = timer_clk / 1000000 - 1;    // 1MHz
        TIMER_CNT(TIMER5) = 1000;                       // 1KHz
        TIMER_CAR(TIMER5) = 1000;
        TIMER_CTL0(TIMER5) |= TIMER_CTL0_CEN;
    }
}
#endif

uint32_t vsfhal_usart_config(enum usart_idx_t idx, uint32_t baudrate, uint32_t mode)
{
    VSF_HAL_ASSERT(idx < USART_IDX_NUM);
    
	uint32_t temp;
	uint32_t usartx = usart_reg_base_list[idx];
    uint8_t dma_idx = DMA_INVALID_IDX;
    struct vsfhal_clk_info_t *info = vsfhal_clk_info_get();

    if (mode == USART_RESET_BAUD_ONLY) {
        switch (idx) {
        #if USART0_ENABLE
        case USART0_IDX:
            temp = info->apb2_freq_hz;
            break;
        #endif
        #if USART1_ENABLE
        case USART1_IDX:
            temp = info->apb1_freq_hz;
            break;
        #endif
        }

        USART_CTL0(usartx) &= ~USART_CTL0_UEN;
        USART_BAUD(usartx) = (temp + baudrate - 1) / baudrate;
        USART_CTL0(usartx) |= USART_CTL0_UEN;
    } else {
        USART_CTL0(usartx) = 0;
        USART_STAT(usartx) = 0;
        USART_CTL0(usartx) = ((mode << 8) & (USART_CTL0_PM | USART_CTL0_PCEN)) |
                USART_CTL0_TEN | USART_CTL0_REN | USART_CTL0_RTIE;
        USART_CTL1(usartx) = ((mode << 8) & USART_CTL1_STB) | USART_CTL1_RTEN;
        USART_CTL1(usartx) |= mode & (USART_CTL1_MSBF | USART_CTL1_DINV | USART_CTL1_TINV | USART_CTL1_RINV);
        USART_CTL2(usartx) = (mode >> 4) & (USART_CTL2_CTSEN | USART_CTL2_RTSEN | USART_CTL2_HDEN);
        USART_RT(usartx) = 20;

        switch (idx) {
        #if USART0_ENABLE
        case USART0_IDX:
            temp = info->apb2_freq_hz;
            #if USART0_DMA_ENABLE
            dma_idx = DMA0_IDX;
            #endif
            break;
        #endif
        #if USART1_ENABLE
        case USART1_IDX:
            temp = info->apb1_freq_hz;
            #if USART1_DMA_ENABLE
            dma_idx = DMA0_IDX;
            #endif
            enable_overtimer(temp);
            break;
        #endif
        }
        
        if (dma_idx != DMA_INVALID_IDX) {
            uint8_t tx_dma_ch = usart_dma_channel[idx][0];
            uint8_t rx_dma_ch = usart_dma_channel[idx][1];

            usart_control[idx].dma_idx = dma_idx;
            usart_control[idx].tx_dma_ch = tx_dma_ch;
            usart_control[idx].rx_dma_ch = rx_dma_ch;
            
            // dma & dma channel config
            DMA_CHxCTL(DMA, tx_dma_ch) = 0;
            DMA_CHxCTL(DMA, tx_dma_ch) = DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_FTFIE;
            DMA_CHxPADDR(DMA, tx_dma_ch) = (uint32_t)(usartx + 0x28U);
            
            DMA_CHxCTL(DMA, rx_dma_ch) = 0;
            DMA_CHxCTL(DMA, rx_dma_ch) = DMA_CHXCTL_CMEN | DMA_CHXCTL_MNAGA | DMA_CHXCTL_FTFIE | DMA_CHXCTL_HTFIE;
            DMA_CHxPADDR(DMA, rx_dma_ch) = (uint32_t)(usartx + 0x24U);
            DMA_CHxMADDR(DMA, rx_dma_ch) = (uint32_t)usart_control[idx].rx_buff;
            DMA_CHxCNT(DMA, rx_dma_ch) = USART_BUFF_SIZE * 2;
            DMA_CHxCTL(DMA, rx_dma_ch) |= DMA_CHXCTL_CHEN;

            USART_CTL2(usartx) |= USART_CTL2_DENR | USART_CTL2_DENT;
        } else {
            usart_control[idx].dma_idx = DMA_INVALID_IDX;
            USART_CTL0(usartx) |= USART_CTL0_RBNEIE | USART_CTL0_TCIE;
        }

        USART_BAUD(usartx) = (temp + baudrate - 1) / baudrate;
        USART_CMD(usartx) = USART_CMD_ABDCMD | USART_CMD_SBKCMD | USART_CMD_MMCMD | USART_CMD_RXFCMD | USART_CMD_TXFCMD;
        USART_RFCS(usartx) |= USART_RFCS_RFEN;
        USART_CTL0(usartx) |= USART_CTL0_UEN;
    }

    return temp / USART_BAUD(usartx);
}

void vsfhal_usart_config_cb(enum usart_idx_t idx, int32_t int_priority, void *p, void (*ontx)(void *), void (*onrx)(void *))
{
    VSF_HAL_ASSERT(idx < USART_IDX_NUM);

    usart_control_t *ctrl = &usart_control[idx];

    ctrl->ontx = ontx;
    ctrl->onrx = onrx;
    ctrl->param = p;

    if (int_priority >= 0) {
        if (ctrl->dma_idx != DMA_INVALID_IDX) {
            vsf_dma_config_channel(ctrl->dma_idx, ctrl->tx_dma_ch, usart_tx_done, ctrl, int_priority);
            vsf_dma_config_channel(ctrl->dma_idx, ctrl->rx_dma_ch, usart_rx_done, ctrl, int_priority);
        }
        NVIC_EnableIRQ(usart_irqn_list[idx]);
        NVIC_SetPriority(usart_irqn_list[idx], int_priority);
    } else {
        if (ctrl->dma_idx != DMA_INVALID_IDX) {
            vsf_dma_config_channel(ctrl->dma_idx, ctrl->tx_dma_ch, NULL, NULL, -1);
            vsf_dma_config_channel(ctrl->dma_idx, ctrl->rx_dma_ch, NULL, NULL, -1);
        }
        NVIC_DisableIRQ(usart_irqn_list[idx]);
    }
}

uint16_t vsfhal_usart_tx_bytes(enum usart_idx_t idx, uint8_t *data, uint16_t size)
{
    VSF_HAL_ASSERT(idx < USART_IDX_NUM);

    uint8_t *tx_buff = usart_control[idx].tx_buff;
    usart_control[idx].tx_size = size;
    
    if (usart_control[idx].dma_idx != DMA_INVALID_IDX) {
        uint8_t tx_dma_ch = usart_control[idx].tx_dma_ch;

        if (data)
            memcpy(tx_buff, data, size);

        DMA_CHxCTL(DMA, tx_dma_ch) &= ~DMA_CHXCTL_CHEN;
        DMA_CHxMADDR(DMA, tx_dma_ch) = (uint32_t)tx_buff;
        DMA_CHxCNT(DMA, tx_dma_ch) = size;
        DMA_CHxCTL(DMA, tx_dma_ch) |= DMA_CHXCTL_CHEN;
    } else {
        #warning "TODO"
    }

    return size;
}

uint16_t vsfhal_usart_tx_get_free_size(enum usart_idx_t idx)
{
    VSF_HAL_ASSERT(idx < USART_IDX_NUM);

    return usart_control[idx].tx_size ? 0 : USART_BUFF_SIZE;
}

uint16_t vsfhal_usart_rx_bytes(enum usart_idx_t idx, uint8_t *data, uint16_t size)
{
    VSF_HAL_ASSERT(idx < USART_IDX_NUM);

    uint_fast16_t end;
    usart_control_t *ctrl = &usart_control[idx];
    size = min(vsfhal_usart_rx_get_data_size(idx), size);
    if (!size)
        return 0;

    end = min((USART_BUFF_SIZE * 2) - ctrl->rx_buff_r_pos, size);
    size = size - end;

    if (end) {
        memcpy(data, ctrl->rx_buff + ctrl->rx_buff_r_pos, end);
        ctrl->rx_buff_r_pos = ctrl->rx_buff_r_pos + end;
        if (ctrl->rx_buff_r_pos == (USART_BUFF_SIZE * 2)) {
            ctrl->rx_buff_r_pos = 0;
        }
    }
    if (size) {
        memcpy(data + end, ctrl->rx_buff, size);
        ctrl->rx_buff_r_pos = size;
    }

    return end + size;
}

uint16_t vsfhal_usart_rx_get_data_size(enum usart_idx_t idx)
{
    VSF_HAL_ASSERT(idx < USART_IDX_NUM);

    usart_control_t *ctrl = &usart_control[idx];
    uint_fast8_t rx_buff_w_pos, rx_buff_r_pos = ctrl->rx_buff_r_pos;

    if (ctrl->dma_idx != DMA_INVALID_IDX) {
        rx_buff_w_pos = USART_BUFF_SIZE * 2 - DMA_CHxCNT(DMA, ctrl->rx_dma_ch);
    } else {
        rx_buff_w_pos = ctrl->rx_buff_w_pos;
    }

    if (rx_buff_w_pos >= rx_buff_r_pos) {
        return rx_buff_w_pos - rx_buff_r_pos;
    } else {
        return rx_buff_w_pos + USART_BUFF_SIZE * 2 - rx_buff_r_pos;
    }
}

static void usart_tx_done(void *p)
{
    usart_control_t *ctrl = p;

    ctrl->tx_size = 0;
    if (ctrl->ontx) {
        ctrl->ontx(ctrl->param);
    }
}

static void usart_rx_done(void *p)
{
    usart_control_t *ctrl = p;
    if (ctrl->onrx) {
        ctrl->onrx(ctrl->param);
    }
}

#if USART0_ENABLE
ROOT void USART0_IRQHandler(void)
{
    if (USART_STAT(USART0_BASE) & USART_STAT_RTF) {
        USART_INTC(USART0_BASE) = USART_INTC_RTC;
        usart_rx_done(&usart_control[USART0_IDX]);
    }
}
#endif

#if USART1_ENABLE
ROOT void TIMER5_DAC_IRQHandler(void)
{
    TIMER_INTF(TIMER5) = 0;
    if (vsfhal_usart_rx_get_data_size(USART1_IDX))
        usart_rx_done(&usart_control[USART1_IDX]);
}
#endif  // USART1_ENABLE

#if USART_STREAM_ENABLE

static void stream_dotx(usart_control_t *ctrl, vsf_stream_t *stream)
{
    uint32_t size;
    enum usart_idx_t idx = ((uint32_t)ctrl - (uint32_t)usart_control) / sizeof(usart_control_t);

    size = vsfhal_usart_tx_get_free_size(idx);
    if (size) {
        if (stream->op == &vsf_fifo_stream_op) {
            uint8_t buf[USART_BUFF_SIZE];
            size = VSF_STREAM_READ(stream, buf, size);
            if (size)
                vsfhal_usart_tx_bytes(idx, buf, size);
        } else if (stream->op == &vsf_block_stream_op) {
            uint8_t *buf;
            size = VSF_STREAM_GET_RBUF(stream, &buf);
            if (size) {
                memcpy(usart_control[idx].tx_buff, buf, size);
                VSF_STREAM_READ(stream, buf, size);
                vsfhal_usart_tx_bytes(idx, NULL, size);
            }
        }
    }
}

static void stream_dorx(usart_control_t *ctrl, vsf_stream_t *stream)
{
    uint32_t size;
    enum usart_idx_t idx = ((uint32_t)ctrl - (uint32_t)usart_control) / sizeof(usart_control_t);

    if (stream->op == &vsf_fifo_stream_op) {
        uint8_t buf[USART_BUFF_SIZE];
        size = min(vsfhal_usart_rx_get_data_size(idx), USART_BUFF_SIZE);
        if (size) {
            size = vsfhal_usart_rx_bytes(idx, buf, size);
            if (size && VSF_STREAM_IS_RX_CONNECTED(stream))
                VSF_STREAM_WRITE(stream, buf, size);
        }
    } else if (stream->op == &vsf_block_stream_op) {
        uint8_t *buf;
        size = VSF_STREAM_GET_WBUF(stream, &buf);
        if (size) {
            size = vsfhal_usart_rx_bytes(idx, buf, size);
            if (size && VSF_STREAM_IS_RX_CONNECTED(stream))
                VSF_STREAM_WRITE(stream, buf, size);
        } else {
            uint8_t discard[USART_BUFF_SIZE];
            vsfhal_usart_rx_bytes(idx, discard, USART_BUFF_SIZE);
        }
    }
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
    vsf_eda_post_evt(&ctrl->eda, VSF_EVT_RX_STREAM_ONRX);
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
        usart_control[idx].rx->rx.evthandler = NULL;
        usart_control[idx].rx->rx.param = NULL;
    }
}

void vsfhal_usart_stream_init(enum usart_idx_t idx, int32_t eda_priority, int32_t int_priority, vsf_stream_t *tx, vsf_stream_t *rx)
{
    vsfhal_usart_stream_fini(idx);
    
    if (!tx && !rx)
        return;

    const vsf_eda_cfg_t cfg = {
        .fn.evthandler  = usart_stream_evthandler,
        .priority       = eda_priority,
    };
    vsf_eda_init_ex(&usart_control[idx].eda, (vsf_eda_cfg_t *)&cfg);

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
