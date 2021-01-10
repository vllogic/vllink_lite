/*============================ INCLUDES ======================================*/
#include "spi.h"
#include "io.h"
#include "dma.h"

/*============================ MACROS ========================================*/

#ifndef VSF_HAL_CFG_SPI_PROTECT_LEVEL
#   ifndef VSF_HAL_CFG_PROTECT_LEVEL
#       define VSF_HAL_CFG_SPI_PROTECT_LEVEL  interrupt
#   else
#       define VSF_HAL_CFG_SPI_PROTECT_LEVEL  VSF_HAL_CFG_PROTECT_LEVEL
#   endif
#endif

#define vsfhal_spi_protect                       vsf_protect(VSF_HAL_CFG_SPI_PROTECT_LEVEL)
#define vsfhal_spi_unprotect                     vsf_unprotect(VSF_HAL_CFG_SPI_PROTECT_LEVEL)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

struct spi_control_t {
    void (*callback)(void *);
    void *param;
    uint8_t *out;
    uint8_t *in;
    uint32_t out_pos;
    uint32_t in_pos;
    uint32_t size;
    uint8_t dummy_out;
    uint8_t dummy_in;
    uint8_t dma_idx;
    uint8_t tx_dma_ch;
    uint8_t rx_dma_ch;
};
typedef struct spi_control_t spi_control_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
static void spi_irq(enum spi_idx_t idx);
static void spi_dma_irq(void *param);
/*============================ IMPLEMENTATION ================================*/

#if SPI_COUNT > 0

static const uint32_t spi_reg_base_list[SPI_COUNT] = {
    #if SPI0_ENABLE
    SPI0,
    #endif
    #if SPI1_ENABLE
    SPI1,
    #endif
};

static const IRQn_Type spi_irq_list[SPI_COUNT][2] = {   // DMA, SPI
    #if SPI0_ENABLE
    {DMA_Channel1_2_IRQn, SPI0_IRQn},
    #endif  // SPI0_ENABLE
    #if SPI1_ENABLE
    {DMA_Channel3_4_IRQn, SPI1_IRQn},
    #endif  // SPI1_ENABLE
};

static spi_control_t spi_control[SPI_COUNT];

void vsfhal_spi_init(enum spi_idx_t idx)
{
    VSF_HAL_ASSERT(idx < SPI_IDX_NUM);

    switch (idx) {
    #if SPI0_ENABLE
    case SPI0_IDX:
        RCU_APB2EN |= RCU_APB2EN_SPI0EN;
        RCU_APB2RST |= RCU_APB2RST_SPI0RST;
        RCU_APB2RST &= ~RCU_APB2RST_SPI0RST;
        #if SPI0_NSS_IO_IDX != GPIO_INVALID_IDX
        vsfhal_gpio_init(SPI0_NSS_IO_IDX);
        vsfhal_gpio_config(SPI0_NSS_IO_IDX, 0x1 << SPI0_NSS_IO_PIN, IO_OUTPUT_AFPP);
        #endif
        #if SPI0_SCK_IO_IDX != GPIO_INVALID_IDX
        vsfhal_gpio_init(SPI0_SCK_IO_IDX);
        vsfhal_gpio_config(SPI0_SCK_IO_IDX, 0x1 << SPI0_SCK_IO_PIN, IO_OUTPUT_AFPP);
        #endif
        #if SPI0_MISO_IO_IDX != GPIO_INVALID_IDX
        vsfhal_gpio_init(SPI0_MISO_IO_IDX);
        vsfhal_gpio_config(SPI0_MISO_IO_IDX, 0x1 << SPI0_MISO_IO_PIN, IO_INPUT_FLOAT);
        #endif
        #if SPI0_MOSI_IO_IDX != GPIO_INVALID_IDX
        vsfhal_gpio_init(SPI0_MOSI_IO_IDX);
        vsfhal_gpio_config(SPI0_MOSI_IO_IDX, 0x1 << SPI0_MOSI_IO_PIN, IO_OUTPUT_AFPP);
        #endif
        break;
    #endif
    #if SPI1_ENABLE
    case SPI1_IDX:
        RCU_APB1EN |= RCU_APB1EN_SPI1EN;
        RCU_APB1RST |= RCU_APB1RST_SPI1RST;
        RCU_APB1RST &= ~RCU_APB1RST_SPI1RST;
        #if SPI1_NSS_IO_IDX != GPIO_INVALID_IDX
        vsfhal_gpio_init(SPI1_NSS_IO_IDX);
        vsfhal_gpio_config(SPI1_NSS_IO_IDX, 0x1 << SPI1_NSS_IO_PIN, IO_OUTPUT_AFPP);
        #endif
        #if SPI1_SCK_IO_IDX != GPIO_INVALID_IDX
        vsfhal_gpio_init(SPI1_SCK_IO_IDX);
        vsfhal_gpio_config(SPI1_SCK_IO_IDX, 0x1 << SPI1_SCK_IO_PIN, IO_OUTPUT_AFPP);
        #endif
        #if SPI1_MISO_IO_IDX != GPIO_INVALID_IDX
        vsfhal_gpio_init(SPI1_MISO_IO_IDX);
        vsfhal_gpio_config(SPI1_MISO_IO_IDX, 0x1 << SPI1_MISO_IO_PIN, IO_INPUT_FLOAT);
        #endif
        #if SPI1_MOSI_IO_IDX != GPIO_INVALID_IDX
        vsfhal_gpio_init(SPI1_MOSI_IO_IDX);
        vsfhal_gpio_config(SPI1_MOSI_IO_IDX, 0x1 << SPI1_MOSI_IO_PIN, IO_OUTPUT_AFPP);
        #endif
        break;
    #endif
    }
}

void vsfhal_spi_fini(enum spi_idx_t idx)
{
    VSF_HAL_ASSERT(idx < SPI_IDX_NUM);

    switch (idx) {
    #if SPI0_ENABLE
    case SPI0_IDX:
        RCU_APB2EN &= ~RCU_APB2EN_SPI0EN;
        break;
    #endif
    #if SPI1_ENABLE
    case SPI1_IDX:
        RCU_APB1EN &= ~RCU_APB1EN_SPI1EN;
        break;
    #endif
    }
}

void vsfhal_spi_config(enum spi_idx_t idx, uint32_t kHz, uint32_t config)
{
    VSF_HAL_ASSERT(idx < SPI_IDX_NUM);

    uint32_t temp;
    uint32_t dmax = 0;
    uint32_t spix = spi_reg_base_list[idx];
    uint8_t tx_dma_ch, rx_dma_ch;
    struct vsfhal_clk_info_t *info = vsfhal_clk_info_get();

    memset(&spi_control[idx], 0, sizeof(spi_control_t));

    switch (idx) {
    #if SPI0_ENABLE
    case SPI0_IDX:
        temp = info->apb2_freq_hz;
        #if SPI0_DMA_ENABLE
        dmax = DMA;
        tx_dma_ch = 2;
        rx_dma_ch = 1;
        #endif
        break;
    #endif
    #if SPI1_ENABLE
    case SPI1_IDX:
        temp = info->apb2_freq_hz;
        #if SPI1_DMA_ENABLE
        dmax = DMA;
        tx_dma_ch = 4;
        rx_dma_ch = 3;
        #endif
        break;
    #endif
    }

    temp = (temp / 1000 + kHz - 1) / kHz;
    if (temp <= 2)
        temp = 0;
    else if (temp <= 4)
        temp = 1;
    else if (temp <= 8)
        temp = 2;
    else if (temp <= 16)
        temp = 3;
    else if (temp <= 32)
        temp = 4;
    else if (temp <= 64)
        temp = 5;
    else if (temp <= 128)
        temp = 6;
    else
        temp = 7;

    SPI_CTL0(spix) = 0;
    SPI_CTL0(spix) = config | (temp << 3);

    if (dmax) {
        spi_control[idx].dma_idx = DMA0_IDX;
        spi_control[idx].tx_dma_ch = tx_dma_ch;
        spi_control[idx].rx_dma_ch = rx_dma_ch;
        
        // dma & dma channel config
        DMA_CHxCTL(dmax, tx_dma_ch) = 0;
        DMA_CHxCTL(dmax, tx_dma_ch) = DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA;
        DMA_CHxPADDR(dmax, tx_dma_ch) = (uint32_t)SPI_DATA(spix);
        
        DMA_CHxCTL(dmax, rx_dma_ch) = 0;
        DMA_CHxCTL(dmax, rx_dma_ch) = DMA_CHXCTL_MNAGA | DMA_CHXCTL_FTFIE;
        DMA_CHxPADDR(dmax, rx_dma_ch) = (uint32_t)SPI_DATA(spix);

        SPI_CTL1(spix) |= SPI_CTL1_DMAREN | SPI_CTL1_DMATEN;
    } else {
        spi_control[idx].dma_idx = DMA_INVALID_IDX;
        SPI_CTL1(spix) |= SPI_CTL1_RBNEIE;
    }
}

void vsfhal_spi_config_cb(enum spi_idx_t idx, int32_t int_priority, void *p, void (*callback)(void *))
{
    VSF_HAL_ASSERT(idx < SPI_IDX_NUM);

    spi_control_t *ctrl = &spi_control[idx];

    ctrl->callback = callback;
    ctrl->param = p;

    if (int_priority >= 0) {
        if (ctrl->dma_idx != DMA_INVALID_IDX) {
            vsf_dma_config_channel(ctrl->dma_idx, ctrl->rx_dma_ch, spi_dma_irq, ctrl);
            NVIC_EnableIRQ(spi_irq_list[idx][0]);
            NVIC_SetPriority(spi_irq_list[idx][0], int_priority);
        } else {
            NVIC_EnableIRQ(spi_irq_list[idx][1]);
            NVIC_SetPriority(spi_irq_list[idx][1], int_priority);
        }
    } else {
        if (ctrl->dma_idx != DMA_INVALID_IDX) {
            vsf_dma_config_channel(ctrl->dma_idx, ctrl->rx_dma_ch, NULL, NULL);
            NVIC_DisableIRQ(spi_irq_list[idx][0]);
        } else {
            NVIC_DisableIRQ(spi_irq_list[idx][1]);
        }
    }
}

vsf_err_t vsfhal_spi_start(enum spi_idx_t idx, uint8_t *out, uint8_t *in, uint32_t len)
{
    VSF_HAL_ASSERT(idx < SPI_IDX_NUM);

    // TODO
}

uint32_t vsfhal_spi_stop(enum spi_idx_t idx)
{
    VSF_HAL_ASSERT(idx < SPI_IDX_NUM);
    return 0;
}

bool vsfhal_spi_is_busy(enum spi_idx_t idx)
{
    spi_control_t *ctrl = &spi_control[idx];
    return ctrl->size ? true : false;
}

static void spi_irq(enum spi_idx_t idx)
{
    // TODO
}

static void spi_dma_irq(void *param)
{
    spi_control_t *ctrl = param;
    ctrl->size = 0;
    if (ctrl->callback)
        ctrl->callback(ctrl->param);
}

#if SPI0_ENABLE
ROOT void SPI0_IRQHandler(void) {spi_irq(SPI0_IDX);}
#endif  // SPI0_ENABLE
#if SPI1_ENABLE
ROOT void SPI1_IRQHandler(void) {spi_irq(SPI1_IDX);}
#endif  // SPI1_ENABLE

#endif
