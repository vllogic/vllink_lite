/*============================ INCLUDES ======================================*/
#include "usart.h"
#include "io.h"
#include "dma.h"

/*============================ MACROS ========================================*/

#ifndef USART_BUFF_SIZE
#   define USART_BUFF_SIZE              32
#endif

#ifndef VSF_HAL_CFG_USART_PROTECT_LEVEL
#   ifndef VSF_HAL_CFG_PROTECT_LEVEL
#       define VSF_HAL_CFG_USART_PROTECT_LEVEL  interrupt
#   else
#       define VSF_HAL_CFG_USART_PROTECT_LEVEL  VSF_HAL_CFG_PROTECT_LEVEL
#   endif
#endif

#define vsfhal_usart_protect                       vsf_protect(VSF_HAL_CFG_USART_PROTECT_LEVEL)
#define vsfhal_usart_unprotect                     vsf_unprotect(VSF_HAL_CFG_USART_PROTECT_LEVEL)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

struct usart_control_t {
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
};
typedef struct usart_control_t usart_control_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
static void usart_tx_done(void *p);
static void usart_rx_done(void *p);
/*============================ IMPLEMENTATION ================================*/

#if USART_COUNT > 0

static const uint32_t usart_reg_base_list[USART_COUNT] = {
    #if USART0_ENABLE
    USART0,
    #endif  // USART0_ENABLE
    #if USART1_ENABLE
    USART1,
    #endif  // USART1_ENABLE
    #if USART2_ENABLE
    USART2,
    #endif  // USART2_ENABLE
    #if USART3_ENABLE
    UART3,
    #endif  // USART3_ENABLE
    #if USART4_ENABLE
    UART4,
    #endif  // USART4_ENABLE
};

static const IRQn_Type usart_irq_list[USART_COUNT][3] = {   // DMA TX, DMA RX, TX RX / RX OT
    #if USART0_ENABLE
    {DMA0_Channel3_IRQn, DMA0_Channel4_IRQn, USART0_IRQn},
    #endif  // USART0_ENABLE
    #if USART1_ENABLE
    {DMA0_Channel6_IRQn, DMA0_Channel5_IRQn, USART1_IRQn},
    #endif  // USART1_ENABLE
    #if USART2_ENABLE
    {DMA0_Channel1_IRQn, DMA0_Channel2_IRQn, USART2_IRQn},
    #endif  // USART2_ENABLE
    #if USART3_ENABLE
    {DMA1_Channel4_IRQn, DMA1_Channel2_IRQn, UART3_IRQn},
    #endif  // USART3_ENABLE
    #if USART4_ENABLE
    {UART4_IRQn, UART4_IRQn, UART4_IRQn},
    #endif  // USART4_ENABLE
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
        vsfhal_gpio_config(USART0_TXD_IO_IDX, 0x1 << USART0_TXD_IO_PIN, IO_OUTPUT_50M | IO_AFPP_OUT);
        vsfhal_gpio_config(USART0_RXD_IO_IDX, 0x1 << USART0_RXD_IO_PIN, IO_INPUT_FLOAT);
        vsfhal_gpio_config(USART0_CTS_IO_IDX, 0x1 << USART0_CTS_IO_PIN, IO_INPUT_FLOAT);
        vsfhal_gpio_config(USART0_RTS_IO_IDX, 0x1 << USART0_RTS_IO_PIN, IO_OUTPUT_50M | IO_AFPP_OUT);
        break;
    #endif
    #if USART1_ENABLE
    case USART1_IDX:
		RCU_APB1EN |= RCU_APB1EN_USART1EN;
        vsfhal_gpio_init(USART1_TXD_IO_IDX);
        vsfhal_gpio_init(USART1_RXD_IO_IDX);
        vsfhal_gpio_init(USART1_CTS_IO_IDX);
        vsfhal_gpio_init(USART1_RTS_IO_IDX);
        vsfhal_gpio_config(USART1_TXD_IO_IDX, 0x1 << USART1_TXD_IO_PIN, IO_OUTPUT_50M | IO_AFPP_OUT);
        vsfhal_gpio_config(USART1_RXD_IO_IDX, 0x1 << USART1_RXD_IO_PIN, IO_INPUT_FLOAT);
        vsfhal_gpio_config(USART1_CTS_IO_IDX, 0x1 << USART1_CTS_IO_PIN, IO_INPUT_FLOAT);
        vsfhal_gpio_config(USART1_RTS_IO_IDX, 0x1 << USART1_RTS_IO_PIN, IO_OUTPUT_50M | IO_AFPP_OUT);
        break;
    #endif
    #if USART2_ENABLE
    case USART2_IDX:
		RCU_APB1EN |= RCU_APB1EN_USART2EN;
        vsfhal_gpio_init(USART2_TXD_IO_IDX);
        vsfhal_gpio_init(USART2_RXD_IO_IDX);
        vsfhal_gpio_init(USART2_CTS_IO_IDX);
        vsfhal_gpio_init(USART2_RTS_IO_IDX);
        vsfhal_gpio_config(USART2_TXD_IO_IDX, 0x1 << USART2_TXD_IO_PIN, IO_OUTPUT_50M | IO_AFPP_OUT);
        vsfhal_gpio_config(USART2_RXD_IO_IDX, 0x1 << USART2_RXD_IO_PIN, IO_INPUT_FLOAT);
        vsfhal_gpio_config(USART2_CTS_IO_IDX, 0x1 << USART2_CTS_IO_PIN, IO_INPUT_FLOAT);
        vsfhal_gpio_config(USART2_RTS_IO_IDX, 0x1 << USART2_RTS_IO_PIN, IO_OUTPUT_50M | IO_AFPP_OUT);
        break;
    #endif
    #if USART3_ENABLE
    case USART3_IDX:
		RCU_APB1EN |= RCU_APB1EN_UART3EN | RCU_APB1EN_TIMER6EN;
        vsfhal_gpio_init(USART3_TXD_IO_IDX);
        vsfhal_gpio_init(USART3_RXD_IO_IDX);
        vsfhal_gpio_config(USART3_TXD_IO_IDX, 0x1 << USART3_TXD_IO_PIN, IO_OUTPUT_50M | IO_AFPP_OUT);
        vsfhal_gpio_config(USART3_RXD_IO_IDX, 0x1 << USART3_RXD_IO_PIN, IO_INPUT_FLOAT);
        break;
    #endif
    #if USART4_ENABLE
    case USART4_IDX:
		RCU_APB1EN |= RCU_APB1EN_UART4EN | RCU_APB1EN_TIMER6EN;
        vsfhal_gpio_init(USART4_TXD_IO_IDX);
        vsfhal_gpio_init(USART4_RXD_IO_IDX);
        vsfhal_gpio_config(USART4_TXD_IO_IDX, 0x1 << USART4_TXD_IO_PIN, IO_OUTPUT_50M | IO_AFPP_OUT);
        vsfhal_gpio_config(USART4_RXD_IO_IDX, 0x1 << USART4_RXD_IO_PIN, IO_INPUT_FLOAT);
        break;
    #endif
    }
}

void vsfhal_usart_fini(enum usart_idx_t idx)
{
    VSF_HAL_ASSERT(idx < USART_IDX_NUM);

    switch (idx) {
    #if USART0_ENABLE
    case USART0_IDX:
		RCU_APB2EN &= ~RCU_APB2EN_USART0EN;
        break;
    #endif
    #if USART1_ENABLE
    case USART1_IDX:
		RCU_APB1EN &= ~RCU_APB1EN_USART1EN;
        break;
    #endif
    #if USART2_ENABLE
    case USART2_IDX:
		RCU_APB1EN &= ~RCU_APB1EN_USART2EN;
        break;
    #endif
    #if USART3_ENABLE
    case USART3_IDX:
		RCU_APB1EN &= ~RCU_APB1EN_UART3EN;
        break;
    #endif
    #if USART4_ENABLE
    case USART4_IDX:
		RCU_APB1EN &= ~RCU_APB1EN_UART4EN;
        break;
    #endif
    }
}

static void enable_overtimer(uint32_t timer_clk)
{
    if (!(TIMER_CTL0(TIMER6) & TIMER_CTL0_CEN)) {
        TIMER_CTL0(TIMER6) = TIMER_CTL0_ARSE;
        TIMER_DMAINTEN(TIMER6) = TIMER_DMAINTEN_UPIE;
        TIMER_PSC(TIMER6) = timer_clk / 1000000 - 1;    // 1MHz
        TIMER_CNT(TIMER6) = timer_clk / 1000;           // 1KHz
        TIMER_CAR(TIMER6) = timer_clk / 1000;
        TIMER_CTL0(TIMER6) |= TIMER_CTL0_CEN;
    }
}

void vsfhal_usart_config(enum usart_idx_t idx, uint32_t baudrate, uint32_t mode)
{
    VSF_HAL_ASSERT(idx < USART_IDX_NUM);
    
	uint32_t temp;
    uint32_t dmax = 0;
	uint32_t usartx = usart_reg_base_list[idx];
    uint8_t tx_dma_ch, rx_dma_ch;
    struct vsfhal_clk_info_t *info = vsfhal_clk_info_get();

    memset(&usart_control[idx], 0, sizeof(usart_control_t));
    
    USART_CTL0(usartx) = 0;
    USART_STAT0(usartx) = 0;
    USART_CTL0(usartx) = ((mode << 8) & (USART_CTL0_PM | USART_CTL0_PCEN)) |
        USART_CTL0_TEN | USART_CTL0_REN;
    USART_CTL1(usartx) = (mode << 8) & USART_CTL1_STB;
    USART_CTL2(usartx) = (mode >> 4) & (USART_CTL2_CTSEN | USART_CTL2_RTSEN | USART_CTL2_HDEN);
    USART_CTL3(usartx) = ((mode >> 8) & (USART_CTL3_RINV | USART_CTL3_TINV | USART_CTL3_DINV | USART_CTL3_MSBF)) |
        USART_CTL3_RTEN | USART_CTL3_RTIE;
    USART_RT(usartx) = 20;

    switch (idx) {
    #if USART0_ENABLE
    case USART0_IDX:
        temp = info->apb2_freq_hz;
        #if USART0_DMA_ENABLE
        dmax = DMA0;
        tx_dma_ch = 3;
        rx_dma_ch = 4;
        #endif
        break;
    #endif
    #if USART1_ENABLE
    case USART1_IDX:
        temp = info->apb1_freq_hz;
        #if USART1_DMA_ENABLE
        dmax = DMA0;
        tx_dma_ch = 6;
        rx_dma_ch = 5;
        #endif
        break;
    #endif
    #if USART2_ENABLE
    case USART2_IDX:
        temp = info->apb1_freq_hz;
        #if USART2_DMA_ENABLE
        dmax = DMA0;
        tx_dma_ch = 1;
        rx_dma_ch = 2;
        #endif
        break;
    #endif
    #if USART3_ENABLE
    case USART3_IDX:
        temp = info->apb1_freq_hz;
        #if USART3_DMA_ENABLE
        dmax = DMA1;
        tx_dma_ch = 4;
        rx_dma_ch = 2;
        #endif
        enable_overtimer(temp);
        break;
    #endif
    #if USART4_ENABLE
    case USART4_IDX:
        temp = info->apb1_freq_hz;
        enable_overtimer(temp);
        break;
    #endif
    }
    
    if (dmax) {
        usart_control[idx].dma_idx = (dmax - DMA0) / (DMA1 - DMA0);
        usart_control[idx].tx_dma_ch = tx_dma_ch;
        usart_control[idx].rx_dma_ch = rx_dma_ch;
        
        // dma & dma channel config
        DMA_CHxCTL(dmax, tx_dma_ch) = 0;
        DMA_CHxCTL(dmax, tx_dma_ch) = DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_FTFIE;
        DMA_CHxPADDR(dmax, tx_dma_ch) = (uint32_t)USART_DATA(usartx);
        
        DMA_CHxCTL(dmax, rx_dma_ch) = 0;
        DMA_CHxCTL(dmax, rx_dma_ch) = DMA_CHXCTL_CMEN | DMA_CHXCTL_MNAGA | DMA_CHXCTL_FTFIE | DMA_CHXCTL_HTFIE;
        DMA_CHxPADDR(dmax, rx_dma_ch) = (uint32_t)USART_DATA(usartx);
        DMA_CHxMADDR(dmax, rx_dma_ch) = (uint32_t)usart_control[idx].rx_buff;
        DMA_CHxCNT(dmax, rx_dma_ch) = USART_BUFF_SIZE * 2;
        DMA_CHxCTL(dmax, rx_dma_ch) |= DMA_CHXCTL_CHEN;

        USART_CTL2(usartx) |= USART_CTL2_DENR | USART_CTL2_DENT;
    } else {
        usart_control[idx].dma_idx = DMA_INVALID_IDX;
        USART_CTL0(usartx) |= USART_CTL0_RBNEIE | USART_CTL0_TCIE;
    }
    
    USART_BAUD(usartx) = temp / baudrate;
    USART_CTL0(usartx) |= USART_CTL0_UEN;
}

void vsfhal_usart_config_cb(enum usart_idx_t idx, int32_t int_priority, void *p, void (*ontx)(void *), void (*onrx)(void *))
{
    VSF_HAL_ASSERT(idx < USART_IDX_NUM);

    usart_control_t *ctrl = &usart_control[idx];

    ctrl->ontx = ontx;
    ctrl->onrx = onrx;
    ctrl->param = p;
    ctrl->ontx = ontx;

    if (int_priority >= 0) {
        if (ctrl->dma_idx != DMA_INVALID_IDX) {
            vsf_dma_config_channel(ctrl->dma_idx, ctrl->tx_dma_ch, usart_tx_done, ctrl);
            vsf_dma_config_channel(ctrl->dma_idx, ctrl->rx_dma_ch, usart_rx_done, ctrl);
            NVIC_EnableIRQ(usart_irq_list[idx][0]);
            NVIC_EnableIRQ(usart_irq_list[idx][1]);
            NVIC_SetPriority(usart_irq_list[idx][0], int_priority);
            NVIC_SetPriority(usart_irq_list[idx][1], int_priority);
        }
        NVIC_EnableIRQ(usart_irq_list[idx][2]);
        NVIC_SetPriority(usart_irq_list[idx][2], int_priority);
        
        #if USART3_ENABLE || USART4_ENABLE
        #if USART3_ENABLE && USART4_ENABLE
        if ((idx == USART3_IDX) || (idx == USART4_IDX)) {
        #elif USART3_ENABLE
        if (idx == USART3_IDX) {
        #elif USART4_ENABLE
        if (idx == USART4_IDX) {
        #endif
            NVIC_EnableIRQ(TIMER6_IRQn);
            NVIC_SetPriority(TIMER6_IRQn, int_priority);
        }
        #endif
    } else {
        if (ctrl->dma_idx != DMA_INVALID_IDX) {
            vsf_dma_config_channel(ctrl->dma_idx, ctrl->tx_dma_ch, NULL, NULL);
            vsf_dma_config_channel(ctrl->dma_idx, ctrl->rx_dma_ch, NULL, NULL);
            NVIC_DisableIRQ(usart_irq_list[idx][0]);
            NVIC_DisableIRQ(usart_irq_list[idx][1]);
        }
        NVIC_DisableIRQ(usart_irq_list[idx][2]);
    }
}

uint16_t vsfhal_usart_tx_bytes(enum usart_idx_t idx, uint8_t *data, uint16_t size)
{
    VSF_HAL_ASSERT(idx < USART_IDX_NUM);

    uint8_t *tx_buff = usart_control[idx].tx_buff;
    usart_control[idx].tx_size = size;
    
    if (usart_control[idx].dma_idx != DMA_INVALID_IDX) {
        uint32_t dmax = DMA0 + usart_control[idx].dma_idx * (DMA1 - DMA0);
        uint8_t tx_dma_ch = usart_control[idx].tx_dma_ch;

        memcpy(tx_buff, data, size);
        DMA_CHxMADDR(dmax, tx_dma_ch) = (uint32_t)tx_buff;
        DMA_CHxCNT(dmax, tx_dma_ch) = size;
        DMA_CHxCTL(dmax, tx_dma_ch) |= DMA_CHXCTL_CHEN;
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

    uint_fast8_t rx_buff_w_pos, rx_buff_r_pos = usart_control[idx].rx_buff_r_pos;

    if (usart_control[idx].dma_idx != DMA_INVALID_IDX) {
        uint32_t dmax = DMA0 + usart_control[idx].dma_idx * (DMA1 - DMA0);
        uint8_t rx_dma_ch = usart_control[idx].rx_dma_ch;
        rx_buff_w_pos = USART_BUFF_SIZE * 2 - DMA_CHxCNT(dmax, rx_dma_ch);
    } else {
        rx_buff_w_pos = usart_control[idx].rx_buff_w_pos;
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

#if USART3_ENABLE || USART4_ENABLE
ROOT void TIMER6_IRQHandler(void)
{
#if USART3_ENABLE
    if (vsfhal_usart_rx_get_data_size(USART3_IDX))
        usart_rx_done(&usart_control[USART3_IDX]);
#endif  // USART3_ENABLE

#if USART4_ENABLE
    if (vsfhal_usart_rx_get_data_size(USART4_IDX))
        usart_rx_done(&usart_control[USART4_IDX]);
#endif  // USART4_ENABLE
}
#endif  // USART3_ENABLE || USART4_ENABLE

#endif
