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

#define DMA1_CHANNEL_LIMIT  5

static callback_param_t dma0_channel_callback[DMA_CHANNEL_COUNT];
static void *dma0_channel_callback_param[DMA_CHANNEL_COUNT];
static const IRQn_Type dma0_irqn_list[DMA_CHANNEL_COUNT] = {
    DMA0_Channel0_IRQn,
    DMA0_Channel1_IRQn,
    DMA0_Channel2_IRQn,
    DMA0_Channel3_IRQn,
    DMA0_Channel4_IRQn,
    DMA0_Channel5_IRQn,
    DMA0_Channel6_IRQn,
};
#if DMA_COUNT > 1
static callback_param_t dma1_channel_callback[DMA1_CHANNEL_LIMIT];
static void *dma1_channel_callback_param[DMA1_CHANNEL_LIMIT];
static const IRQn_Type dma1_irqn_list[DMA1_CHANNEL_LIMIT] = {
    DMA1_Channel0_IRQn,
    DMA1_Channel1_IRQn,
    DMA1_Channel2_IRQn,
    DMA1_Channel3_IRQn,
    DMA1_Channel4_IRQn,
};
#endif

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
#if DMA_COUNT > 1
    else if (idx == DMA1_IDX) {
        if (channel < DMA1_CHANNEL_LIMIT) {
            IRQn_Type irqn = dma1_irqn_list[channel];

            dma1_channel_callback[channel] = callback;
            dma1_channel_callback_param[channel] = param;

            if (int_priority >= 0) {
                NVIC_EnableIRQ(irqn);
                NVIC_SetPriority(irqn, int_priority);
            } else {
                NVIC_DisableIRQ(irqn);
            }
        }
    }
#endif
}

static void dma_irqhandler(enum dma_idx_t idx, uint8_t channel)
{
#if DMA_COUNT > 1
    if (idx == DMA0_IDX)
#endif
    {
        DMA_INTC(DMA0) = DMA_INTC_GIFC << (4 * channel);
        if (dma0_channel_callback[channel])
            dma0_channel_callback[channel](dma0_channel_callback_param[channel]);
    }
#if DMA_COUNT > 1
    else if (idx == DMA1_IDX) {
        DMA_INTC(DMA1) = DMA_INTC_GIFC << (4 * channel);
        if (dma1_channel_callback[channel])
            dma1_channel_callback[channel](dma1_channel_callback_param[channel]);
    }
#endif
}

ROOT void DMA0_Channel0_IRQHandler(void)    {dma_irqhandler(DMA0_IDX, 0);}
ROOT void DMA0_Channel1_IRQHandler(void)    {dma_irqhandler(DMA0_IDX, 1);}
ROOT void DMA0_Channel2_IRQHandler(void)    {dma_irqhandler(DMA0_IDX, 2);}
ROOT void DMA0_Channel3_IRQHandler(void)    {dma_irqhandler(DMA0_IDX, 3);}
ROOT void DMA0_Channel4_IRQHandler(void)    {dma_irqhandler(DMA0_IDX, 4);}
ROOT void DMA0_Channel5_IRQHandler(void)    {dma_irqhandler(DMA0_IDX, 5);}
ROOT void DMA0_Channel6_IRQHandler(void)    {dma_irqhandler(DMA0_IDX, 6);}
#if DMA_COUNT > 1
ROOT void DMA1_Channel0_IRQHandler(void)    {dma_irqhandler(DMA1_IDX, 0);}
ROOT void DMA1_Channel1_IRQHandler(void)    {dma_irqhandler(DMA1_IDX, 1);}
ROOT void DMA1_Channel2_IRQHandler(void)    {dma_irqhandler(DMA1_IDX, 2);}
ROOT void DMA1_Channel3_IRQHandler(void)    {dma_irqhandler(DMA1_IDX, 3);}
ROOT void DMA1_Channel4_IRQHandler(void)    {dma_irqhandler(DMA1_IDX, 4);}
#endif

#endif
