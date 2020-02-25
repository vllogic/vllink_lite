/*****************************************************************************
 *   Copyright(C)2009-2019 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

/*============================ INCLUDES ======================================*/
#include "usart.h"
#include "io.h"

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

#define vsf_usart_protect                       vsf_protect(VSF_HAL_CFG_USART_PROTECT_LEVEL)
#define vsf_usart_unprotect                     vsf_unprotect(VSF_HAL_CFG_USART_PROTECT_LEVEL)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

struct usart_control_t {
    void (*ontx)(void *);
    void (*onrx)(void *);
    void *param;
    uint8_t tx_buff[USART_BUFF_SIZE];
    uint8_t rx_buff[USART_BUFF_SIZE * 2];
    uint8_t rx_buff_pos;
};
typedef struct usart_control_t usart_control_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
static void usart_tx_irq(void *p);
static void usart_rx_irq(void *p);
/*============================ IMPLEMENTATION ================================*/

#if USART_COUNT > 0

static USART_TypeDef * const usart_list[USART_COUNT] = {
    #if USART0_ENABLE
    USART0,
    #endif  // USART0_ENABLE
    #if USART1_ENABLE
    USART1,
    #endif  // USART1_ENABLE
};

static const uint8_t usart_txrx_dma_stream_list[USART_COUNT][2] = { // TX, RX
    #if USART0_ENABLE
    {2, 1},
    #endif  // USART0_ENABLE
    #if USART1_ENABLE
    {4, 3},
    #endif  // USART1_ENABLE
};

static const IRQn_Type usart_irq_list[USART_COUNT][3] = {   // TX, RX, RX overtime
    #if USART0_ENABLE
    {DMA_Channel1_2_IRQn, DMA_Channel1_2_IRQn, USART0_IRQn},
    #endif  // USART0_ENABLE
    #if USART1_ENABLE
    {DMA_Channel3_4_IRQn, DMA_Channel3_4_IRQn, TIMER13_IRQn},
    #endif  // USART1_ENABLE
};

static usart_control_t usart_control[USART_COUNT];

#if USART1_ENABLE
static uint8_t usart1_rx_cnt;
#endif  // USART1_ENABLE

void vsf_usart_init(enum usart_idx_t idx)
{
    VSF_HAL_ASSERT(idx < USART_COUNT);

    switch (idx) {
    #if USART0_ENABLE
    case USART0_IDX:
		RCU_AHBEN |= RCU_AHBEN_DMAEN;
		RCU_APB2EN |= RCU_APB2EN_USART0EN;
        vsf_gpio_init(USART0_TXD_IO_PORT);
        vsf_gpio_init(USART0_RXD_IO_PORT);
        vsf_gpio_init(USART0_CTS_IO_PORT);
        vsf_gpio_init(USART0_RTS_IO_PORT);
        vsf_gpio_config(USART0_TXD_IO_PORT, USART0_TXD_IO_PIN, IO_AF | USART0_TXD_IO_AF);
        vsf_gpio_config(USART0_RXD_IO_PORT, USART0_RXD_IO_PIN, IO_AF | USART0_RXD_IO_AF);
        vsf_gpio_config(USART0_CTS_IO_PORT, USART0_CTS_IO_PIN, IO_AF | USART0_CTS_IO_AF);
        vsf_gpio_config(USART0_RTS_IO_PORT, USART0_RTS_IO_PIN, IO_AF | USART0_RTS_IO_AF);
        break;
    #endif
    #if USART1_ENABLE
    case USART1_IDX:
		RCU_AHBEN |= RCU_AHBEN_DMAEN;
		RCU_APB1EN |= RCU_APB1EN_USART1EN | RCU_APB1EN_TIMER13EN;
        vsf_gpio_init(USART1_TXD_IO_PORT);
        vsf_gpio_init(USART1_RXD_IO_PORT);
        vsf_gpio_init(USART1_CTS_IO_PORT);
        vsf_gpio_init(USART1_RTS_IO_PORT);
        vsf_gpio_config(USART1_TXD_IO_PORT, USART1_TXD_IO_PIN, IO_AF | USART1_TXD_IO_AF);
        vsf_gpio_config(USART1_RXD_IO_PORT, USART1_RXD_IO_PIN, IO_AF | USART1_RXD_IO_AF);
        vsf_gpio_config(USART1_CTS_IO_PORT, USART1_CTS_IO_PIN, IO_AF | USART1_CTS_IO_AF);
        vsf_gpio_config(USART1_RTS_IO_PORT, USART1_RTS_IO_PIN, IO_AF | USART1_RTS_IO_AF);
        break;
    #endif
    }
}

void vsf_usart_fini(enum usart_idx_t idx)
{
    VSF_HAL_ASSERT(idx < USART_COUNT);

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
    }
}

void vsf_usart_config(enum usart_idx_t idx, uint32_t baudrate, uint32_t mode)
{
    VSF_HAL_ASSERT(idx < USART_COUNT);
    
	uint_fast32_t temp;
	USART_TypeDef *usart = usart_list[idx];
    struct vsf_clk_info_t *info = vsf_clk_info_get();

    usart->CR1 = 0;

    switch (idx) {
    #if USART0_ENABLE
    case USART0_IDX:
		temp = info->apb2_freq_hz;

        DMA_CH1CTL = 0;
        DMA_CH1CTL = DMA_CHXCTL_MNAGA | DMA_CHXCTL_DIR | DMA_CHXCTL_FTFIE;
        DMA_CH1PADDR = (uint32_t)&(usart->TDR);
        
        DMA_CH2CTL = 0;
        DMA_CH2PADDR = (uint32_t)&(usart->RDR);
        DMA_CH2MADDR = (uint32_t)&(usart_control[idx].rx_buff[0]);
        usart_control[idx].rx_buff_pos = 0;
        DMA_CH2CNT = USART_BUFF_SIZE * 2;
        DMA_CH2CTL = DMA_CHXCTL_MNAGA | DMA_CHXCTL_CMEN | DMA_CHXCTL_HTFIE | DMA_CHXCTL_FTFIE;
        DMA_CH2CTL |= DMA_CHXCTL_CHEN;
        break;
    #endif
    #if USART1_ENABLE
    case USART1_IDX:
		temp = info->apb1_freq_hz;
		
        DMA_CH3CTL = 0;
        DMA_CH3CTL = DMA_CHXCTL_MNAGA | DMA_CHXCTL_DIR | DMA_CHXCTL_FTFIE;
        DMA_CH3PADDR = (uint32_t)&(usart->TDR);
        
        DMA_CH4CTL = 0;
        DMA_CH4PADDR = (uint32_t)&(usart->RDR);
        DMA_CH4MADDR = usart_control[idx].rx_buff;
        usart_control[idx].rx_buff_pos = 0;
        DMA_CH4CNT = USART_BUFF_SIZE * 2;
        DMA_CH4CTL = DMA_CHXCTL_MNAGA | DMA_CHXCTL_CMEN | DMA_CHXCTL_HTFIE | DMA_CHXCTL_FTFIE;
        DMA_CH4CTL |= DMA_CHXCTL_CHEN;

        // overtime check timer
        usart1_rx_cnt = 0;
        TIMER_CTL0(TIMER13) = TIMER_CTL0_ARSE;
        TIMER_DMAINTEN(TIMER13) = TIMER_DMAINTEN_UPIE;
        TIMER_PSC(TIMER13) = temp / 1000000 - 1;    // 1M
        TIMER_CNT(TIMER13) = temp / 1000;
        TIMER_CAR(TIMER13) = temp / 1000;
        TIMER_CTL0(TIMER13) |= TIMER_CTL0_CEN;
        break;
    #endif
    }

    if (mode & USART_CR1_PCE) {
        mode |= USART_CR1_M0;
    }

    usart->CR1 = (mode & (USART_CR1_M0 | USART_CR1_PCE | USART_CR1_PS)) |
        USART_CR1_RTOIE | USART_CR1_TE | USART_CR1_RE;
    usart->CR2 = ((mode >> 8) & USART_CR2_STOP) | USART_CR2_RTOEN;
    usart->CR3 = ((mode >> 16) & (USART_CR3_RTSE | USART_CR3_CTSE)) |
        USART_CR3_DMAR | USART_CR3_DMAT;
    usart->RTOR = 30;
    usart->BRR = temp / baudrate;
    usart->RQR = USART_RQR_ABRRQ | USART_RQR_SBKRQ | USART_RQR_MMRQ |
        USART_RQR_RXFRQ | USART_RQR_TXFRQ;
    usart->CR1 |= USART_CR1_UE;
}

void vsf_usart_config_cb(enum usart_idx_t idx, int32_t int_priority, void *p, void (*ontx)(void *), void (*onrx)(void *))
{
    VSF_HAL_ASSERT(idx < USART_COUNT);
    
    usart_control[idx].ontx = ontx;
    usart_control[idx].onrx = onrx;
    usart_control[idx].param = p;
    usart_control[idx].ontx = ontx;

    if (int_priority >= 0) {
        vsf_config_dma_stream_callback(0, usart_txrx_dma_stream_list[idx][0],
            usart_tx_irq, &usart_control[idx]);
        vsf_config_dma_stream_callback(0, usart_txrx_dma_stream_list[idx][1],
            usart_rx_irq, &usart_control[idx]);
        NVIC_EnableIRQ(usart_irq_list[idx][0]);
        NVIC_EnableIRQ(usart_irq_list[idx][1]);
        NVIC_EnableIRQ(usart_irq_list[idx][2]);
        NVIC_SetPriority(usart_irq_list[idx][0], int_priority);
        NVIC_SetPriority(usart_irq_list[idx][1], int_priority);
        NVIC_SetPriority(usart_irq_list[idx][2], int_priority);
    } else {
        vsf_config_dma_stream_callback(0, usart_txrx_dma_stream_list[idx][0],
            NULL, NULL);
        vsf_config_dma_stream_callback(0, usart_txrx_dma_stream_list[idx][1],
            NULL, NULL);
        NVIC_DisableIRQ(usart_irq_list[idx][0]);
        NVIC_DisableIRQ(usart_irq_list[idx][1]);
        NVIC_DisableIRQ(usart_irq_list[idx][2]);
    }
}

uint16_t vsfhal_usart_tx_bytes(enum usart_idx_t idx, uint8_t *data, uint16_t size)
{
    VSF_HAL_ASSERT(idx < USART_COUNT);

    uint8_t *tx_buff =  usart_control[idx].tx_buff;
    
    switch (idx) {
    #if USART0_ENABLE
    case USART0_IDX:
        DMA_CH1CTL &= ~DMA_CHXCTL_CHEN;
        memcpy(tx_buff, data, size);
        DMA_CH1MADDR = (uint32_t)tx_buff;
        DMA_CH1CNT = size;
		DMA_CH1CTL |= DMA_CHXCTL_CHEN;
        break;
    #endif
    #if USART1_ENABLE
    case USART1_IDX:
        DMA_CH3CTL &= ~DMA_CHXCTL_CHEN;
        memcpy(tx_buff, data, size);
        DMA_CH3MADDR = (uint32_t)tx_buff;
        DMA_CH3CNT = size;
		DMA_CH3CTL |= DMA_CHXCTL_CHEN;
        break;
    #endif
    }

    return size;
}

uint16_t vsfhal_usart_tx_get_free_size(enum usart_idx_t idx)
{
    VSF_HAL_ASSERT(idx < USART_COUNT);

    switch (idx) {
    #if USART0_ENABLE
    case USART0_IDX:
        return DMA_CH1CNT ? 0 : USART_BUFF_SIZE;
    #endif
    #if USART1_ENABLE
    case USART1_IDX:
        return DMA_CH3CNT ? 0 : USART_BUFF_SIZE;
    #endif
    }

    return 0;
}

uint16_t vsfhal_usart_rx_bytes(enum usart_idx_t idx, uint8_t *data, uint16_t size)
{
    VSF_HAL_ASSERT(idx < USART_COUNT);

    uint_fast16_t end;
    usart_control_t *ctrl = &usart_control[idx];
    size = min(vsfhal_usart_rx_get_data_size(idx), size);
    if (!size)
        return 0;

    end = min((USART_BUFF_SIZE * 2) - ctrl->rx_buff_pos, size);
    size = size - end;

    if (end) {
        memcpy(data, ctrl->rx_buff + ctrl->rx_buff_pos, end);
        ctrl->rx_buff_pos = ctrl->rx_buff_pos + end;
        if (ctrl->rx_buff_pos == (USART_BUFF_SIZE * 2)) {
            ctrl->rx_buff_pos = 0;
        }
    }
    if (size) {
        memcpy(data + end, ctrl->rx_buff, size);
        ctrl->rx_buff_pos = size;
    }
    
    return end + size;
}

uint16_t vsfhal_usart_rx_get_data_size(enum usart_idx_t idx)
{
    VSF_HAL_ASSERT(idx < USART_COUNT);

    uint_fast32_t dma_pos, rx_pos = usart_control[idx].rx_buff_pos;

    switch (idx) {
    #if USART0_ENABLE
    case USART0_IDX:
        dma_pos = USART_BUFF_SIZE * 2 - DMA_CH2CNT;
        break;
    #endif
    #if USART1_ENABLE
    case USART1_IDX:
        dma_pos = USART_BUFF_SIZE * 2 - DMA_CH4CNT;
        break;
    #endif
    default:
        return 0;
    }

    if (dma_pos >= rx_pos) {
        return dma_pos - rx_pos;
    } else {
        return dma_pos + USART_BUFF_SIZE * 2 - rx_pos;
    }
}

static void usart_tx_irq(void *p)
{
    usart_control_t *ctrl = p;
    if (ctrl->ontx) {
        ctrl->ontx(ctrl->param);
    }
}

static void usart_rx_irq(void *p)
{
    usart_control_t *ctrl = p;
    if (ctrl->onrx) {
        ctrl->onrx(ctrl->param);
    }
}

#if USART1_ENABLE
void TIMER13_IRQHandler(void)
{
    uint_fast8_t cnt = DMA_CH4CNT;
    if ((cnt == usart1_rx_cnt) {
        if (cnt != 0) {
            cnt = USART_BUFF_SIZE * 2 - cnt;
        }
        if (cnt != usart_control[USART1_IDX].rx_buff_pos) {
            usart_rx_irq(&usart_control[USART1_IDX]);
        }
    }
}
#endif  // USART1_ENABLE

#endif
