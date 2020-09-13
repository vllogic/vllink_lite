/*============================ INCLUDES ======================================*/
#include "./dma.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

#if DMA_COUNT > 0

static const IRQn_Type dma0_irqn_list[DMA_CHANNEL_COUNT] = {
    DMA_Channel0_IRQn,
    DMA_Channel1_2_IRQn, DMA_Channel1_2_IRQn,
    DMA_Channel3_4_IRQn, DMA_Channel3_4_IRQn,
    DMA_Channel5_6_IRQn, DMA_Channel5_6_IRQn,
};
static callback_param_t dma0_channel_callback[DMA_CHANNEL_COUNT];
static void *dma0_channel_callback_param[DMA_CHANNEL_COUNT];

void vsf_dma_config_channel(enum dma_idx_t idx, uint8_t channel, 
        callback_param_t callback, void *param, int32_t int_priority)
{
    if (idx == DMA0_IDX) {
        if (channel < DMA_CHANNEL_COUNT) {
            IRQn_Type irqn = dma0_irqn_list[channel];

            dma0_channel_callback[channel] = callback;
            dma0_channel_callback_param[channel] = param;

            if (int_priority >= 0) {
                NVIC_EnableIRQ(irqn);
                NVIC_SetPriority(irqn, int_priority);
            } else {
                NVIC_DisableIRQ(irqn);
            }
        }
    }
}

static void dma_irqhandler(enum dma_idx_t idx, uint8_t channel)
{
    if (dma0_channel_callback[channel])
        dma0_channel_callback[channel](dma0_channel_callback_param[channel]);
}

ROOT void DMA_Channel0_IRQHandler(void)
{
    DMA_INTC = DMA_INTC_GIFC;
    dma_irqhandler(DMA0_IDX, 0);
}

ROOT void DMA_Channel1_2_IRQHandler(void)
{
    if (DMA_INTF & (DMA_INTF_GIF << (4 * 1))) {
        DMA_INTC = DMA_INTC_GIFC << (4 * 1);
        dma_irqhandler(DMA0_IDX, 1);
    }
    if (DMA_INTF & (DMA_INTF_GIF << (4 * 2))) {
        DMA_INTC = DMA_INTC_GIFC << (4 * 2);
        dma_irqhandler(DMA0_IDX, 2);
    }
}

ROOT void DMA_Channel3_4_IRQHandler(void)
{
    if (DMA_INTF & (DMA_INTF_GIF << (4 * 3))) {
        DMA_INTC = DMA_INTC_GIFC << (4 * 3);
        dma_irqhandler(DMA0_IDX, 3);
    }
    if (DMA_INTF & (DMA_INTF_GIF << (4 * 4))) {
        DMA_INTC = DMA_INTC_GIFC << (4 * 4);
        dma_irqhandler(DMA0_IDX, 4);
    }
}

ROOT void DMA_Channel5_6_IRQHandler(void)
{
    if (DMA_INTF & (DMA_INTF_GIF << (4 * 5)))
        dma_irqhandler(DMA0_IDX, 5);
    if (DMA_INTF & (DMA_INTF_GIF << (4 * 6)))
        dma_irqhandler(DMA0_IDX, 6);
}

#endif
