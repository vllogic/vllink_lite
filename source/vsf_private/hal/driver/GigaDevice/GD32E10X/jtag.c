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
#define IO_GET_80_or_00(idx, pin) (*(uint8_t *)(0x42000000U + (((GPIOA + 0x400 * (idx)) + 0x8) - 0x40000000) * 32 + (pin) * 4) << 7)

typedef struct jtag_control_t {
	uint8_t idle;
    uint16_t retry_limit;
    uint16_t delay_tick;

    #if TIMESTAMP_CLOCK
    uint32_t dap_timestamp;
    #endif
    void (*jtag_rw)(uint32_t bits, uint8_t *tms, uint8_t *tdi, uint8_t *tdo);
    void (*jtag_rw_dr)(uint32_t dma_bytes, uint32_t bits_tail, uint8_t *tms, uint8_t *tdi, uint8_t *tdo);
    void (*jtag_delay)(uint16_t delay_tick);
} jtag_control_t;

#if JTAG_COUNT > 0

static jtag_control_t jtag_control;

static void jtag_rw_quick(uint32_t bitlen, uint8_t *tms, uint8_t *tdi, uint8_t *tdo);
static void jtag_rw_slow(uint32_t bitlen, uint8_t *tms, uint8_t *tdi, uint8_t *tdo);
static void jtag_rw_dr_quick(uint32_t bytelen_dma, uint32_t bitlen_tail, uint8_t *tms, uint8_t *tdi, uint8_t *tdo);
static void jtag_rw_dr_slow(uint32_t bytelen_dma, uint32_t bitlen_tail, uint8_t *tms, uint8_t *tdi, uint8_t *tdo);

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
}

void vsfhal_jtag_fini(void)
{
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
    GPIO_CTL0(GPIOA + 0x400 * PERIPHERAL_GPIO_TDI_IDX) &= ~((0x8ul << (4 * PERIPHERAL_GPIO_TDI_PIN)) | (0x8ul << (4 * PERIPHERAL_GPIO_TCK_JTAG_PIN)));
}

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

void vsfhal_jtag_config(uint16_t kHz, uint16_t retry, uint8_t idle)
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

    vsfhal_clk_reconfig_apb(apb);

    info = vsfhal_clk_info_get();

    jtag_control.idle = idle;
    jtag_control.retry_limit = retry;
    jtag_control.delay_tick = info->ahb_freq_hz / (kHz * 2000);

    if (kHz >= 3000) {
        jtag_control.jtag_rw = jtag_rw_quick;
        jtag_control.jtag_rw_dr = jtag_rw_dr_quick;
        jtag_control.jtag_delay = NULL;
    } else if (kHz >= 2000) {
        jtag_control.jtag_rw = jtag_rw_slow;
        jtag_control.jtag_rw_dr = jtag_rw_dr_slow;
        jtag_control.jtag_delay = NULL;
    } else if (kHz >= 1500) {
        jtag_control.jtag_rw = jtag_rw_slow;
        jtag_control.jtag_rw_dr = jtag_rw_dr_slow;
        jtag_control.jtag_delay = delay_jtag_1500khz;
    } else if (kHz >= 1000) {
        jtag_control.jtag_rw = jtag_rw_slow;
        jtag_control.jtag_rw_dr = jtag_rw_dr_slow;
        jtag_control.jtag_delay = delay_jtag_1000khz;
    } else {
        jtag_control.jtag_rw = jtag_rw_slow;
        jtag_control.jtag_rw_dr = jtag_rw_dr_slow;
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
    DMA_CHxCTL(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO; 
    DMA_CHxPADDR(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH) = JTAG_SPI_BASE + 0xc;
    DMA_CHxPADDR(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = JTAG_SPI_BASE + 0xc;
}

void vsfhal_jtag_raw(uint32_t bitlen, uint8_t *tms, uint8_t *tdi, uint8_t *tdo)
{
    jtag_control.jtag_rw(bitlen, tms, tdi, tdo);
}

void vsfhal_jtag_ir(uint32_t ir, uint32_t lr_length, uint32_t ir_before, uint32_t ir_after)
{
    uint_fast32_t bitlen;
    uint64_t buf_tms, buf_tdi, buf_tdo;

    buf_tdi = 0;
    lr_length--;

    // Select-DR-Scan, Select-IR-Scan, Capture-IR, Shift-IR
    buf_tms = 0x3;
    bitlen = 4;

    // Bypass before data
    if (ir_before) {
        buf_tdi |= (((uint64_t)0x1 << ir_before) - 1) << bitlen;
        bitlen += ir_before;
    }

    // Set IR bitlen
    if (lr_length) {
        buf_tdi |= (ir & (((uint64_t)0x1 << lr_length) - 1)) << bitlen;
        bitlen += lr_length;
    }

    // Bypass after data
    if (ir_after) {
        buf_tdi |= ((ir >> lr_length) & 0x1) << bitlen;
        bitlen++;
        ir_after--;
        if (ir_after) {
            buf_tdi |= (((uint64_t)0x1 << ir_after) - 1) << bitlen;
            bitlen += ir_after;
        }
        buf_tms |= (uint64_t)0x1 << bitlen;
        buf_tdi |= (uint64_t)0x1 << bitlen;
        bitlen++;
    } else {
        buf_tms |= (uint64_t)0x1 << bitlen;
        buf_tdi |= ((ir >> lr_length) & 0x1) << bitlen;
        bitlen++;
    }

    // Exit1-IR, Update-IR
    buf_tms |= (uint64_t)0x1 << bitlen;
    bitlen++;
    // idle
    buf_tdi |= (uint64_t)0x1 << bitlen;	// keep tdi high
    bitlen++;

    jtag_control.jtag_rw(bitlen, (uint8_t *)&buf_tms, (uint8_t *)&buf_tdi, (uint8_t *)&buf_tdo);
}

/*
Read:	vsfhal_jtag_dr(request, 0, dr_before, dr_after, *read_buf)
Write:	vsfhal_jtag_dr(request, write_value, dr_before, dr_after, NULL)
*/
uint32_t vsfhal_jtag_dr(uint32_t request, uint32_t dr, uint32_t dr_before, uint32_t dr_after, uint8_t *data)
{
    uint_fast32_t ack, retry, dma_bytes, bits_tail, bitlen;
    uint64_t buf_tms, buf_tdi, buf_tdo;

    retry = 0;
    buf_tdi = 0;

    // Select-DR-Scan, Capture-DR, Shift-DR
    buf_tms = 0x1;
    bitlen = 3;

    // Bypass before data
    bitlen += dr_before;

    // RnW, A2, A3
    buf_tdi |= (uint64_t)((request >> 1) & 0x7) << bitlen;
    bitlen += 3;

    // Data Transfer
    if (!(request & DAP_TRANSFER_RnW))
        buf_tdi |= (uint64_t)dr << bitlen;
    bitlen += 31 + dr_after;
    dma_bytes = (bitlen - 8) >> 3;
    buf_tms |= (uint64_t)0x1 << bitlen;
    bitlen++;

    // Update-DR, Idle
    buf_tms |= (uint64_t)0x1 << bitlen;
    bitlen += 1 + jtag_control.idle;
    buf_tdi |= (uint64_t)0x1 << bitlen;	// keep tdi high
    bitlen++;

    bits_tail = bitlen - 8 - (dma_bytes << 3);

    do
    {
        jtag_control.jtag_rw_dr(dma_bytes, bits_tail, (uint8_t *)&buf_tms, (uint8_t *)&buf_tdi, (uint8_t *)&buf_tdo);
        ack = (buf_tdo >> (dr_before + 3)) & 0x7;
        ack = (ack & 0x4) | ((ack & 0x2) >> 1) | ((ack & 0x1) << 1);
        if (ack != DAP_TRANSFER_WAIT)
            break;
    } while (retry++ < jtag_control.retry_limit);

    if (data)
        put_unaligned_le32(buf_tdo >> (dr_before + 6), data);
    return ack;
}

#pragma optimize=low
static void jtag_rw_quick(uint32_t bitlen, uint8_t *tms, uint8_t *tdi, uint8_t *tdo)
{
    uint8_t bits, tdi_last, tms_last, tdo_last;

    while (bitlen >= 8) {
        bitlen -= 8;
        bits = 8;
        tms_last = *tms++;
        tdi_last = *tdi++;
        do
        {
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
            tdo_last |= IO_GET_80_or_00(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN);
        } while (bits);
        *tdo++ = tdo_last;
    }

    if (bitlen) {
        bits = 8 - bitlen;
        tms_last = *tms;
        tdi_last = *tdi;
        tdo_last = 0;
        do
        {
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
            tdo_last |= IO_GET_80_or_00(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN);
        } while (bitlen);
        *tdo = tdo_last >> bits;
    }
}

static void jtag_rw_slow(uint32_t bitlen, uint8_t *tms, uint8_t *tdi, uint8_t *tdo)
{
    uint8_t bits, tdi_last, tms_last, tdo_last;

    while (bitlen >= 8) {
        bitlen -= 8;
        bits = 8;
        tms_last = *tms++;
        tdi_last = *tdi++;
        do
        {
            if (tdi_last & 0x1)
                IO_SET(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
            else
                IO_CLEAR(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
            if (tms_last & 0x1)
                IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            else
                IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            IO_CLEAR(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay(jtag_control.delay_tick);
            tms_last >>= 1;
            tdi_last >>= 1;
            tdo_last >>= 1;
            bits--;
            IO_SET(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay(jtag_control.delay_tick);
            tdo_last |= IO_GET_80_or_00(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN);
        } while (bits);
        *tdo++ = tdo_last;
    }

    if (bitlen) {
        bits = 8 - bitlen;
        tms_last = *tms;
        tdi_last = *tdi;
        tdo_last = 0;
        do
        {
            if (tdi_last & 0x1)
                IO_SET(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
            else
                IO_CLEAR(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
            if (tms_last & 0x1)
                IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            else
                IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            IO_CLEAR(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay(jtag_control.delay_tick);
            tms_last >>= 1;
            tdi_last >>= 1;
            tdo_last >>= 1;
            bitlen--;
            IO_SET(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay(jtag_control.delay_tick);
            tdo_last |= IO_GET_80_or_00(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN);
        } while (bitlen);
        *tdo = tdo_last >> bits;
    }
}

#pragma optimize=low
static void jtag_rw_dr_quick(uint32_t bytelen_dma, uint32_t bitlen_tail, uint8_t *tms, uint8_t *tdi, uint8_t *tdo)
{
    uint8_t bits, tdi_last, tms_last, tdo_last;

    // head
    bits = 8;
    tms_last = *tms++;
    tdi_last = *tdi++;
    do
    {
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
        tdo_last |= IO_GET_80_or_00(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN);
    } while (bits);
    *tdo++ = tdo_last;

    // dma
    jtag_set_spi_mode();
    DMA_CHxCTL(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH) = DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
    DMA_CHxMADDR(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH) = (uint32_t)tdo;
    DMA_CHxCNT(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH) = bytelen_dma;
    DMA_CHxCTL(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
    DMA_CHxCTL(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO; 
    DMA_CHxMADDR(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = (uint32_t)tdi;
    DMA_CHxCNT(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = bytelen_dma;
    DMA_CHxCTL(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
    tms += bytelen_dma;
    tdo += bytelen_dma;
    tdi += bytelen_dma;
    while (DMA_CHxCNT(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH));

    jtag_set_io_mode();
    while (bitlen_tail >= 8) {
        bitlen_tail -= 8;
        bits = 8;
        tms_last = *tms++;
        tdi_last = *tdi++;
        do
        {
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
            tdo_last |= IO_GET_80_or_00(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN);
        } while (bits);
        *tdo++ = tdo_last;
    }

    if (bitlen_tail) {
        bits = 8 - bitlen_tail;
        tms_last = *tms;
        tdi_last = *tdi;
        tdo_last = 0;
        do
        {
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
            bitlen_tail--;
            IO_SET(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
            tdo_last |= IO_GET_80_or_00(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN);
        } while (bitlen_tail);
        *tdo = tdo_last >> bits;
    }
}

static void jtag_rw_dr_slow(uint32_t bytelen_dma, uint32_t bitlen_tail, uint8_t *tms, uint8_t *tdi, uint8_t *tdo)
{
    uint8_t bits, tdi_last, tms_last, tdo_last;

    // head
    bits = 8;
    tms_last = *tms++;
    tdi_last = *tdi++;
    do
    {
        if (tdi_last & 0x1)
            IO_SET(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
        else
            IO_CLEAR(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
        if (tms_last & 0x1)
            IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        else
            IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        IO_CLEAR(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
        if (jtag_control.jtag_delay)
            jtag_control.jtag_delay(jtag_control.delay_tick);
        tms_last >>= 1;
        tdi_last >>= 1;
        tdo_last >>= 1;
        bits--;
        IO_SET(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
        if (jtag_control.jtag_delay)
            jtag_control.jtag_delay(jtag_control.delay_tick);
        tdo_last |= IO_GET_80_or_00(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN);
    } while (bits);
    *tdo++ = tdo_last;

    // dma
    jtag_set_spi_mode();
    DMA_CHxCTL(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH) = DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
    DMA_CHxMADDR(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH) = (uint32_t)tdo;
    DMA_CHxCNT(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH) = bytelen_dma;
    DMA_CHxCTL(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
    DMA_CHxCTL(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO; 
    DMA_CHxMADDR(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = (uint32_t)tdi;
    DMA_CHxCNT(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = bytelen_dma;
    DMA_CHxCTL(JTAG_SPI_DMAX, JTAG_SPI_TX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
    tms += bytelen_dma;
    tdo += bytelen_dma;
    tdi += bytelen_dma;
    while (DMA_CHxCNT(JTAG_SPI_DMAX, JTAG_SPI_RX_DMA_CH));

    jtag_set_io_mode();
    while (bitlen_tail >= 8) {
        bitlen_tail -= 8;
        bits = 8;
        tms_last = *tms++;
        tdi_last = *tdi++;
        do
        {
            if (tdi_last & 0x1)
                IO_SET(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
            else
                IO_CLEAR(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
            if (tms_last & 0x1)
                IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            else
                IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            IO_CLEAR(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay(jtag_control.delay_tick);
            tms_last >>= 1;
            tdi_last >>= 1;
            tdo_last >>= 1;
            bits--;
            IO_SET(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay(jtag_control.delay_tick);
            tdo_last |= IO_GET_80_or_00(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN);
        } while (bits);
        *tdo++ = tdo_last;
    }

    if (bitlen_tail) {
        bits = 8 - bitlen_tail;
        tms_last = *tms;
        tdi_last = *tdi;
        tdo_last = 0;
        do
        {
            if (tdi_last & 0x1)
                IO_SET(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
            else
                IO_CLEAR(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
            if (tms_last & 0x1)
                IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            else
                IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            IO_CLEAR(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay(jtag_control.delay_tick);
            tms_last >>= 1;
            tdi_last >>= 1;
            tdo_last >>= 1;
            bitlen_tail--;
            IO_SET(PERIPHERAL_GPIO_TCK_JTAG_IDX, PERIPHERAL_GPIO_TCK_JTAG_PIN);
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay(jtag_control.delay_tick);
            tdo_last |= IO_GET_80_or_00(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN);
        } while (bitlen_tail);
        *tdo = tdo_last >> bits;
    }
}

#if TIMESTAMP_CLOCK
uint32_t vsfhal_jtag_get_timestamp(void)
{
    return jtag_control.dap_timestamp;
}
#endif

#endif
