/*
Reference Document:
	"IEEE_1149_JTAG_and_Boundary_Scan_Tutorial"
	https://github.com/ARMmbed/DAPLink/blob/master/source/daplink/cmsis-dap/JTAG_DP.c
*/

#include "jtag.h"
#include "./common/io.h"
#include "./common/dma.h"
#include "vsf.h"

#define DAP_TRANSFER_RnW (1U << 1)
#define DAP_TRANSFER_TIMESTAMP (1U << 7)
#define DAP_TRANSFER_OK (1U << 0)
#define DAP_TRANSFER_WAIT (1U << 1)


#define IO_CFG_INPUT(idx, pin) (((pin) >= 8) ? (GPIO_CTL1(GPIOA + 0x400 * (idx)) = (GPIO_CTL1(GPIOA + 0x400 * (idx)) & ~(0xful << (((pin) - 8) * 4))) | (0x4ul << (((pin) - 8) * 4))) : (GPIO_CTL0(GPIOA + 0x400 * (idx)) = (GPIO_CTL0(GPIOA + 0x400 * (idx)) & ~(0xful << ((pin) * 4))) | (0x4ul << ((pin) * 4))))
#define IO_CFG_OUTPUT(idx, pin) (((pin) >= 8) ? (GPIO_CTL1(GPIOA + 0x400 * (idx)) = (GPIO_CTL1(GPIOA + 0x400 * (idx)) & ~(0xful << (((pin) - 8) * 4))) | (0x3ul << (((pin) - 8) * 4))) : (GPIO_CTL0(GPIOA + 0x400 * (idx)) = (GPIO_CTL0(GPIOA + 0x400 * (idx)) & ~(0xful << ((pin) * 4))) | (0x3ul << ((pin) * 4))))
#define IO_SET(idx, pin) (GPIO_BOP(GPIOA + 0x400 * (idx)) = 0x1ul << (pin))
#define IO_CLEAR(idx, pin) (GPIO_BOP(GPIOA + 0x400 * (idx)) = 0x10000ul << (pin))
#define IO_GET(idx, pin) ((GPIO_ISTAT(GPIOA + 0x400 * (idx)) >> (pin)) & 0x1)


typedef struct jtag_transfer_op_t {
    bool is_spi;
    uint8_t spi_tms_lvl;
    uint8_t spi_bytelen;
    uint8_t io_bitlen;
    uint8_t *io_tms;
    uint8_t *common_tdi;
    uint8_t *common_tdo;
} jtag_transfer_op_t;

typedef struct jtag_transfer_t {
    volatile bool busy;
    uint8_t ack_pos;
    uint8_t ack;
    
    uint16_t retry_cnt;

    uint8_t op_idx;
    uint8_t op_num;
    jtag_transfer_op_t op[8];    // 64Bit Max
} jtag_transfer_t;

typedef struct jtag_control_t {
    uint8_t select;

    uint16_t retry_limit;
    uint16_t delay_tick;

    #if TIMESTAMP_CLOCK
    uint32_t dap_timestamp;
    #endif
    int32_t int_priority;
    callback_param_t dma_callback;
    void (*jtag_delay)(uint16_t delay_tick);
    
    jtag_transfer_t transfer[2];
} jtag_control_t;

#if JTAG_COUNT > 0

static void jtag_transfer_handler_quick(void *param);
static void jtag_transfer_handler_slow(void *param);

static jtag_control_t jtag_control;

void vsfhal_jtag_init(int32_t int_priority)
{
    PERIPHERAL_GPIO_TDI_INIT();
    PERIPHERAL_GPIO_TMS_INIT();
    PERIPHERAL_GPIO_TCK_INIT();
    PERIPHERAL_GPIO_TDO_INIT();
    PERIPHERAL_GPIO_SRST_INIT();
    PERIPHERAL_GPIO_TRST_INIT();
    vsfhal_jtag_io_reconfig();

    switch (JTAG_SPI_BASE) {
    case SPI0:
        RCU_APB2EN |= RCU_APB2EN_SPI0EN;
        RCU_APB2RST |= RCU_APB2RST_SPI0RST;
        RCU_APB2RST &= ~RCU_APB2RST_SPI0RST;
        break;
    case SPI1:
        RCU_APB1EN |= RCU_APB1EN_SPI1EN;
        RCU_APB1RST |= RCU_APB1RST_SPI1RST;
        RCU_APB1RST &= ~RCU_APB1RST_SPI1RST;
        break;
    case SPI2:
        RCU_APB2EN |= RCU_APB1EN_SPI2EN;
        RCU_APB2RST |= RCU_APB1RST_SPI2RST;
        RCU_APB2RST &= ~RCU_APB1RST_SPI2RST;
        break;
    }

    memset(&jtag_control, 0, sizeof(jtag_control_t));
    jtag_control.int_priority = int_priority;
}

void vsfhal_jtag_fini(void)
{
    vsf_dma_config_channel(JTAG_SPI_DMA_IDX, JTAG_SPI_TX_DMA_CH, NULL, NULL, -1);
    vsf_dma_config_channel(JTAG_SPI_DMA_IDX, JTAG_SPI_RX_DMA_CH, NULL, NULL, -1);

    IO_CFG_INPUT(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
    IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    IO_CFG_INPUT(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
    IO_CFG_INPUT(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN);
    IO_CFG_INPUT(PERIPHERAL_GPIO_SRST_IDX, PERIPHERAL_GPIO_SRST_PIN);
    IO_CFG_INPUT(PERIPHERAL_GPIO_TRST_MO_IDX, PERIPHERAL_GPIO_TRST_MO_PIN);

    PERIPHERAL_GPIO_TDI_FINI();
    PERIPHERAL_GPIO_TMS_FINI();
    PERIPHERAL_GPIO_TCK_FINI();
    PERIPHERAL_GPIO_TDO_FINI();
    PERIPHERAL_GPIO_SRST_FINI();
    PERIPHERAL_GPIO_TRST_FINI();
}

void vsfhal_jtag_io_reconfig(void)
{
    PERIPHERAL_JTAG_IO_AF_CONFIG();

    /*
        Default:
        TDI     : OUTPP
        TMS MO  : OUTPP
        TMS MI  : Float Input
        TCK     : OUTPP
        TDO     : Float Input
        SRST    : OUTPP
        TRST    : OUTPP
    */

    IO_SET(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);

    IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);

    //IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN);

    IO_SET(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);

    //IO_CFG_INPUT(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN);

    IO_SET(PERIPHERAL_GPIO_SRST_IDX, PERIPHERAL_GPIO_SRST_PIN);
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_SRST_IDX, PERIPHERAL_GPIO_SRST_PIN);

    IO_SET(PERIPHERAL_GPIO_TRST_MO_IDX, PERIPHERAL_GPIO_TRST_MO_PIN);
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_TRST_MO_IDX, PERIPHERAL_GPIO_TRST_MO_PIN);
}

static inline void jtag_set_spi_mode(void)
{
    VSF_HAL_ASSERT(PERIPHERAL_GPIO_TDI_IDX == PERIPHERAL_GPIO_TCK_JTAG_IDX)
    GPIO_CTL0(GPIOA + 0x400 * PERIPHERAL_GPIO_TDI_IDX) |= (0x8ul << (4 * PERIPHERAL_GPIO_TDI_PIN)) | (0x8ul << (4 * PERIPHERAL_GPIO_TCK_JTAG_PIN));
}

static inline void jtag_set_io_mode(void)
{
    VSF_HAL_ASSERT(PERIPHERAL_GPIO_TDI_IDX == PERIPHERAL_GPIO_TCK_JTAG_IDX)

    if (GPIO_ISTAT(GPIOA + 0x400 * PERIPHERAL_GPIO_TDI_IDX) & (0x1ul << PERIPHERAL_GPIO_TDI_PIN))
        GPIO_BOP(GPIOA + 0x400 * PERIPHERAL_GPIO_TDI_IDX) = 0x1ul << PERIPHERAL_GPIO_TDI_PIN;
    else
        GPIO_BOP(GPIOA + 0x400 * PERIPHERAL_GPIO_TDI_IDX) = 0x10000ul << PERIPHERAL_GPIO_TDI_PIN;
    GPIO_CTL0(GPIOA + 0x400 * PERIPHERAL_GPIO_TDI_IDX) &= ~((0x8ul << (4 * PERIPHERAL_GPIO_TDI_PIN)) | (0x8ul << (4 * PERIPHERAL_GPIO_TCK_JTAG_PIN)));
}

const static uint32_t spi_khz_and_apb_clk_table_list[][2] = {
    {64000000, 32000,}, // 64M / 2 = 32M
    {48000000, 24000,}, // 48M / 2 = 24M
    {64000000, 16000,}, // 64M / 4 = 16M
    {48000000, 12000,}, // 48M / 4 = 12M
    {64000000, 8000,},  // 64M / 8 = 8M
    {48000000, 6000,},  // 48M / 8 = 6M
    {64000000, 4000,},  // 64M / 16 = 4M
    {48000000, 3000,},  // 48M / 16 = 3M
    {64000000, 2000,},  // 64M / 32 = 2M
    {48000000, 1500,},  // 48M / 32 = 1500K
    {64000000, 1000,},  // 64M / 64 = 1M
    {48000000, 750,},   // 48M / 64 = 750K
    {64000000, 500,},   // 64M / 128 = 500K
    {48000000, 375,},   // 48M / 128 = 375K
    {64000000, 250,},   // 64M / 256 = 250K
    {48000000, 188,},   // 48M / 256 = 187.5K
};

#pragma optimize=none
static void delay_jtag_1500khz(uint16_t dummy)
{
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
}

#pragma optimize=none
static void delay_jtag_1000khz(uint16_t dummy)
{
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
}

#pragma optimize=none
static void delay_jtag_ticks(uint16_t delay_tick)
{
    vsf_systimer_cnt_t ticks = vsf_systimer_get() + delay_tick;
    while (ticks >= vsf_systimer_get());
}

void vsfhal_jtag_config(uint16_t kHz, uint16_t retry)
{
    uint32_t temp, apb;
    vsfhal_clk_info_t *info = vsfhal_clk_info_get();
    
    if (kHz < spi_khz_and_apb_clk_table_list[dimof(spi_khz_and_apb_clk_table_list) - 1][1])
        kHz = spi_khz_and_apb_clk_table_list[dimof(spi_khz_and_apb_clk_table_list) - 1][1];
    for (temp = 0; temp < dimof(spi_khz_and_apb_clk_table_list); temp++) {
        if (kHz >= spi_khz_and_apb_clk_table_list[temp][1]) {
            kHz = spi_khz_and_apb_clk_table_list[temp][1];
            apb = spi_khz_and_apb_clk_table_list[temp][0];
            break;
        }
    }
    temp = temp / 2;
    // TODO
    // vsfhal_clk_reconfig_apb(apb);

    info = vsfhal_clk_info_get();

    jtag_control.retry_limit = retry;
    jtag_control.delay_tick = info->ahb_freq_hz / (kHz * 2000);

    if (kHz >= 3000) {
        jtag_control.dma_callback= jtag_transfer_handler_quick;
        jtag_control.jtag_delay = NULL;
    } else if (kHz >= 2000) {
        jtag_control.dma_callback= jtag_transfer_handler_slow;
        jtag_control.jtag_delay = NULL;
    } else if (kHz >= 1500) {
        jtag_control.dma_callback= jtag_transfer_handler_slow;
        jtag_control.jtag_delay = delay_jtag_1500khz;
    } else if (kHz >= 1000) {
        jtag_control.dma_callback= jtag_transfer_handler_slow;
        jtag_control.jtag_delay = delay_jtag_1000khz;
    } else {
        jtag_control.dma_callback= jtag_transfer_handler_slow;
        jtag_control.jtag_delay = delay_jtag_ticks;
    }

    // SPI config
    SPI_CTL0(JTAG_SPI_BASE) &= ~(SPI_CTL0_SPIEN | SPI_CTL0_PSC | SPI_CTL0_BDEN | SPI_CTL0_CRCEN | SPI_CTL0_FF16);
    SPI_CTL0(JTAG_SPI_BASE) |= temp << 3;
    SPI_CTL0(JTAG_SPI_BASE) |= SPI_CTL0_MSTMOD | SPI_CTL0_SWNSSEN | SPI_CTL0_SWNSS | SPI_CTL0_LF | SPI_CTL0_CKPL | SPI_CTL0_CKPH;
    SPI_CTL1(JTAG_SPI_BASE) = SPI_CTL1_DMAREN | SPI_CTL1_DMATEN;
    SPI_CTL0(JTAG_SPI_BASE) |= SPI_CTL0_SPIEN;

    // DMA TX RX channel config
    DMA_CHxCTL(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH) = DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
    DMA_CHxCTL(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = DMA_CHXCTL_FTFIE | DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO; 
    DMA_CHxPADDR(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH) = JTAG_SPI_BASE + 0xc;
    DMA_CHxPADDR(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = JTAG_SPI_BASE + 0xc;

    vsf_dma_config_channel(JTAG_SPI_DMA_IDX, JTAG_SPI_TX_DMA_CH, jtag_control.dma_callback, &jtag_control, jtag_control.int_priority);
    //vsf_dma_config_channel(JTAG_SPI_DMA_IDX, JTAG_SPI_RX_DMA_CH, NULL, NULL, -1);

    jtag_set_io_mode();
}

uint32_t vsfhal_jtag_raw(uint8_t ack_pos, uint8_t bitlen, uint8_t *tms, uint8_t *tdi, uint8_t *tdo)
{
    uint32_t ret;

    VSF_HAL_ASSERT(bitlen <= 64);

    jtag_transfer_t *previous_transfer = &jtag_control.transfer[jtag_control.select];
    jtag_transfer_t *next_transfer = &jtag_control.transfer[jtag_control.select ? 0 : 1];

    // prepare next transfer
    {
        uint8_t bitpos = 0;
        uint8_t op_num = 0;
        jtag_transfer_op_t *op = &next_transfer->op[0];

        if (bitlen >= 8) {
            if (tms[0] == 0) {
                op->spi_tms_lvl = tms[0];
            } else if (tms[0] == 0xff) {
                op->spi_tms_lvl = tms[0];
            } else {
                goto FIRST_NOT_SPI;
            }
            op->is_spi = true;
            op->spi_bytelen = 1;
            op->common_tdi = tdi;
            op->common_tdo = tdo;
            bitpos = 8;
            op_num = 1;
        }
        else 
        {
        FIRST_NOT_SPI:
            op->is_spi = false;
            op->io_tms = tms;
            op->common_tdi = tdi;
            op->common_tdo = tdo;
            bitpos = 8;
            op_num = 1;
            if (bitlen <= 8) {
                op->io_bitlen = bitlen;
                goto PREPARE_DONE;
            } else {
                op->io_bitlen = 8;
            }
        }

        while (bitpos < bitlen) {
            uint8_t bytepos = bitpos >> 3;
            if (op->is_spi) {
                if ((bitpos + 8) <= bitlen) {
                    if (tms[bytepos] == op->spi_tms_lvl) {
                        op->spi_bytelen++;
                    } else if ((tms[bytepos] == 0) || (tms[bytepos] == 0xff)) {
                        op++;
                        op->is_spi = true;
                        op->spi_bytelen = 1;
                        op->spi_tms_lvl = tms[bytepos];
                        op->common_tdi = &tdi[bytepos];
                        op->common_tdo = &tdo[bytepos];
                        op_num++;
                    } else {
                        op++;
                        op->is_spi = false;
                        op->io_bitlen = 8;
                        op->io_tms = &tms[bytepos];
                        op->common_tdi = &tdi[bytepos];
                        op->common_tdo = &tdo[bytepos];
                        op_num++;
                    }
                    bitpos += 8;
                } else {
                    op++;
                    op->is_spi = false;
                    op->io_bitlen = bitlen - bitpos;
                    op->io_tms = &tms[bytepos];
                    op->common_tdi = &tdi[bytepos];
                    op->common_tdo = &tdo[bytepos];
                    op_num++;
                    break;
                }
            } else {
                if ((bitpos + 8) > bitlen) {
                    op->io_bitlen += bitlen - bitpos;
                    break;
                } else {
                    if ((tms[bytepos] == 0) || (tms[bytepos] == 0xff)) {
                        op++;
                        op->is_spi = true;
                        op->spi_bytelen = 1;
                        op->spi_tms_lvl = tms[bytepos];
                        op->common_tdi = &tdi[bytepos];
                        op->common_tdo = &tdo[bytepos];
                        op_num++;
                    } else {
                        op->io_bitlen += 8;
                    }
                    bitpos += 8;
                }
            }
        }

    PREPARE_DONE:
        next_transfer->ack_pos = ack_pos;
        next_transfer->op_idx = 0;
        next_transfer->op_num = op_num;
    }
    
    while (previous_transfer->busy);
    ret = previous_transfer->ack;

    if (ret == DAP_TRANSFER_OK) {
        next_transfer->busy = true;
        jtag_control.select = jtag_control.select ? 0 : 1;
        
        #if TIMESTAMP_CLOCK
        // dr only
        // TODO: USE independent timer
        jtag_control.dap_timestamp = vsf_systimer_get_us();
        #endif
        
        // activer interrupt transfer
        NVIC_SetPendingIRQ(JTAG_SPI_IRQ);
    }
    return ret;
}

uint32_t vsfhal_jtag_raw_less_8bit(uint32_t bitlen, uint32_t tms, uint32_t tdi)
{
    uint_fast32_t tdo = 0, pos = 8 - bitlen;
    jtag_transfer_t *previous_transfer = &jtag_control.transfer[jtag_control.select];
    while (previous_transfer->busy);

    do {
        if (tdi & 0x1)
            IO_SET(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
        else
            IO_CLEAR(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
        if (tms & 0x1)
            IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        else
            IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        IO_CLEAR(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
        tms >>= 1;
        tdi >>= 1;
        tdo >>= 1;
        bitlen--;
        //if (jtag_control.jtag_delay)
        //    jtag_control.jtag_delay(jtag_control.delay_tick);
        IO_SET(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
        tdo |= IO_GET(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN) ? 0x80 : 0;
        //if (jtag_control.jtag_delay)
        //    jtag_control.jtag_delay(jtag_control.delay_tick);
    } while (bitlen);

    return tdo >> pos;
}

uint32_t vsfhal_jtag_raw_1bit(uint32_t tms, uint32_t tdi)
{
    jtag_transfer_t *previous_transfer = &jtag_control.transfer[jtag_control.select];
    while (previous_transfer->busy);

    if (tdi)
        IO_SET(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
    else
        IO_CLEAR(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
    if (tms)
        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    else
        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    IO_CLEAR(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
    if (jtag_control.jtag_delay)
        jtag_control.jtag_delay(jtag_control.delay_tick);
    IO_SET(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
    return IO_GET(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN) ? 0x01 : 0;
}

uint32_t vsfhal_jtag_wait(void)
{
    jtag_transfer_t *previous_transfer = &jtag_control.transfer[jtag_control.select];
    while (previous_transfer->busy);
    return previous_transfer->ack;
}

void vsfhal_jtag_clear(void)
{
    jtag_control.transfer[0].ack = DAP_TRANSFER_OK;
    jtag_control.transfer[1].ack = DAP_TRANSFER_OK;
}

#pragma optimize=size
static void jtag_transfer_handler_quick(void *param)
{
    struct jtag_transfer_op_t *op;
    jtag_control_t *control = param;
    jtag_transfer_t *transfer = &control->transfer[control->select];

    // wait spi tx/rx finish
    //while (SPI_STAT(JTAG_SPI_BASE) & (SPI_STAT_RBNE | SPI_STAT_TRANS));
    while (SPI_STAT(JTAG_SPI_BASE) & SPI_STAT_TRANS);

TRANSFER_RESTART:
    while (transfer->op_idx < transfer->op_num) {
        op = &transfer->op[transfer->op_idx];
        transfer->op_idx++;

        if (op->is_spi) {
            jtag_set_spi_mode();
            if (op->spi_tms_lvl)
                IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            else
                IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            DMA_CHxCTL(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH) = DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
            DMA_CHxCTL(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = DMA_CHXCTL_FTFIE | DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO; 
            DMA_CHxMADDR(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH) = (uint32_t)op->common_tdo;
            DMA_CHxCNT(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH) = op->spi_bytelen;
            DMA_CHxCTL(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
            DMA_CHxMADDR(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = (uint32_t)op->common_tdi;
            DMA_CHxCNT(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = op->spi_bytelen;
            DMA_CHxCTL(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_FTFIE | DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
            return; // wait DMA SPI op finish
        } else {
            uint_fast32_t bits, bitlen, bytepos = 0, bytelen, tms_last, tdi_last, tdo_last;
            jtag_set_io_mode();

            bitlen = op->io_bitlen;

            while (bitlen > 8) {
                bits = 8;
                bitlen -= 8;
                tms_last = op->io_tms[bytepos];
                tdi_last = op->common_tdi[bytepos];
                tdo_last = 0;
                do {
                    if (tdi_last & 0x1)
                        IO_SET(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
                    else
                        IO_CLEAR(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
                    if (tms_last & 0x1)
                        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                    else
                        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                    IO_CLEAR(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
                    tms_last >>= 1;
                    tdi_last >>= 1;
                    tdo_last >>= 1;
                    bits--;
                    IO_SET(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
                    tdo_last |= IO_GET(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN) ? 0x80 : 0;
                } while (bits);
                op->common_tdo[bytepos] = tdo_last;
                bytepos++;
            }
            if (bitlen) {
                bits = 8 - bitlen;
                tms_last = op->io_tms[bytepos];
                tdi_last = op->common_tdi[bytepos];
                tdo_last = 0;
                do {
                    if (tdi_last & 0x1)
                        IO_SET(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
                    else
                        IO_CLEAR(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
                    if (tms_last & 0x1)
                        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                    else
                        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                    IO_CLEAR(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
                    tms_last >>= 1;
                    tdi_last >>= 1;
                    tdo_last >>= 1;
                    bitlen--;
                    IO_SET(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
                    tdo_last |= IO_GET(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN) ? 0x80 : 0;
                } while (bitlen);
                op->common_tdo[bytepos] = tdo_last >> bits;
            }
        }
    }

    if (transfer->ack_pos) {
        uint_fast32_t ack = (get_unaligned_le32(transfer->op[0].common_tdo + 4) >> transfer->ack_pos) & 0x7;
		ack = (ack & 0x4) | ((ack & 0x2) >> 1) | ((ack & 0x1) << 1);
        if ((ack == DAP_TRANSFER_WAIT) && (transfer->retry_cnt++ < control->retry_limit)) {
            transfer->op_idx = 0;
            goto TRANSFER_RESTART;
        }
        transfer->ack = ack;
    } else {
        transfer->ack = DAP_TRANSFER_OK;
    }

    jtag_set_io_mode();

    transfer->busy = false;
}

#pragma optimize=size
static void jtag_transfer_handler_slow(void *param)
{
    struct jtag_transfer_op_t *op;
    jtag_control_t *control = param;
    jtag_transfer_t *transfer = &control->transfer[control->select];

    // wait spi tx/rx finish
    //while (SPI_STAT(JTAG_SPI_BASE) & (SPI_STAT_RBNE | SPI_STAT_TRANS));
    while (SPI_STAT(JTAG_SPI_BASE) & SPI_STAT_TRANS);

TRANSFER_RESTART:
    while (transfer->op_idx < transfer->op_num) {
        op = &transfer->op[transfer->op_idx];
        transfer->op_idx++;

        if (op->is_spi) {
            jtag_set_spi_mode();
            if (op->spi_tms_lvl)
                IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            else
                IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            DMA_CHxCTL(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH) = DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
            DMA_CHxCTL(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = DMA_CHXCTL_FTFIE | DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO; 
            DMA_CHxMADDR(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH) = (uint32_t)op->common_tdo;
            DMA_CHxCNT(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH) = op->spi_bytelen;
            DMA_CHxCTL(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
            DMA_CHxMADDR(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = (uint32_t)op->common_tdi;
            DMA_CHxCNT(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = op->spi_bytelen;
            DMA_CHxCTL(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_FTFIE | DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
            return; // wait DMA SPI op finish
        } else {
            uint_fast32_t bits, bitlen, bytepos = 0, bytelen, tms_last, tdi_last, tdo_last;
            jtag_set_io_mode();

            bitlen = op->io_bitlen;

            while (bitlen > 8) {
                bits = 8;
                bitlen -= 8;
                tms_last = op->io_tms[bytepos];
                tdi_last = op->common_tdi[bytepos];
                tdo_last = 0;
                do {
                    if (tdi_last & 0x1)
                        IO_SET(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
                    else
                        IO_CLEAR(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
                    if (tms_last & 0x1)
                        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                    else
                        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                    IO_CLEAR(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
                    tms_last >>= 1;
                    tdi_last >>= 1;
                    tdo_last >>= 1;
                    bits--;
                    if (control->jtag_delay)
                        control->jtag_delay(control->delay_tick);
                    IO_SET(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
                    tdo_last |= IO_GET(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN) ? 0x80 : 0;
                    if (control->jtag_delay)
                        control->jtag_delay(control->delay_tick);
                } while (bits);
                op->common_tdo[bytepos] = tdo_last;
                bytepos++;
            }
            if (bitlen) {
                bits = 8 - bitlen;
                tms_last = op->io_tms[bytepos];
                tdi_last = op->common_tdi[bytepos];
                tdo_last = 0;
                do {
                    if (tdi_last & 0x1)
                        IO_SET(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
                    else
                        IO_CLEAR(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
                    if (tms_last & 0x1)
                        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                    else
                        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                    IO_CLEAR(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
                    tms_last >>= 1;
                    tdi_last >>= 1;
                    tdo_last >>= 1;
                    bitlen--;
                    if (control->jtag_delay)
                        control->jtag_delay(control->delay_tick);
                    IO_SET(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
                    tdo_last |= IO_GET(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN) ? 0x80 : 0;
                    if (control->jtag_delay)
                        control->jtag_delay(control->delay_tick);
                } while (bitlen);
                op->common_tdo[bytepos] = tdo_last >> bits;
            }
        }
    }

    if (transfer->ack_pos) {
        uint_fast32_t ack = (get_unaligned_le32(transfer->op[0].common_tdo + 4) >> transfer->ack_pos) & 0x7;
		ack = (ack & 0x4) | ((ack & 0x2) >> 1) | ((ack & 0x1) << 1);
        if ((ack == DAP_TRANSFER_WAIT) && (transfer->retry_cnt++ < control->retry_limit)) {
            transfer->op_idx = 0;
            goto TRANSFER_RESTART;
        }
        transfer->ack = ack;
    } else {
        transfer->ack = DAP_TRANSFER_OK;
    }

    jtag_set_io_mode();
    
    transfer->busy = false;
}

#if TIMESTAMP_CLOCK
uint32_t vsfhal_jtag_get_timestamp(void)
{
    return jtag_control.dap_timestamp;
}
#endif

#endif
