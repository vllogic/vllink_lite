/*
Reference Document:
	"Serial Wire Debug and the CoreSightTM Debug and Trace Architecture"
	https://github.com/ARMmbed/DAPLink/blob/master/source/daplink/cmsis-dap/SW_DP.c


Sequence: 
    [W]: SWDIO OUT
    [R]: SWDIO IN
    [C]: SWDIO Ignore

SWD READ:
    Request:[W]*8 --> TRN:[C]*trn --> ACK:[R]*3 --> Data:[R]*32 --> Parity:[R]*1 -> Trn:[C]*trn --> Idle:[C]*idle --> End   // ACK OK;
                                                |-> Data:[C]*32 --> Parity:[C]*1 -> Trn:[C]*trn --> Do Retry                // ACK Wait; Data_force;
                                                |-> Trn:[C]*trn --> Do Retry                                                // ACK Wait; Not Data_force;
                                                |-> Data:[C]*32 --> Parity:[C]*1 -> Trn:[C]*trn --> End                     // ACK Fault; Data_force;
                                                |-> Trn:[C]*trn --> End                                                     // ACK Fault; Not Data_force;
                                                |-> Data:[C]*32 --> Parity:[C]*1 -> End                                     // ACK Other

SWD Write:
    Request:[W]*8 --> TRN:[C]*trn --> ACK:[R]*3 --> TRN:[C]*trn --> Data:[W]*32 --> Parity:[W]*1 -> Idle:[C]*idle --> End   // ACK OK;
                                                |-> Trn:[C]*trn --> Data:[C]*32 --> Parity:[C]*1 -> Do Retry                // ACK Wait; Data_force;
                                                |-> Trn:[C]*trn --> Do Retry                                                // ACK Wait; Not Data_force;
                                                |-> Trn:[C]*trn --> Data:[C]*32 --> Parity:[C]*1 -> End                     // ACK Fault; Data_force;
                                                |-> Trn:[C]*trn --> End                                                     // ACK Fault; Not Data_force;
                                                |-> Data:[C]*32 --> Parity:[C]*1 -> End                                     // ACK Other
*/

#include "swd.h"
#include "./common/io.h"
#include "./common/dma.h"
#include "vsf.h"

#define SWD_SEQOUT_SEQIN_ASYNC_ENABLE       1
#define SWD_SYNC_MODE_FREQ_KHZ              8000

#define SWD_SUCCESS 0x00
#define SWD_FAULT 0x80
#define SWD_RETRY_OUT 0x40
#define SWD_ACK_ERROR 0x20
#define SWD_PARITY_ERROR 0x10

#define SWD_ACK_OK 0x01
#define SWD_ACK_WAIT 0x02
#define SWD_ACK_FAULT 0x04

#define SWD_TRANS_RnW (1 << 2)
#define SWD_TRANS_TIMESTAMP (1 << 7)

#define IO_CFG_INPUT(idx, pin) (((pin) >= 8) ? (GPIO_CTL1(GPIOA + 0x400 * (idx)) = (GPIO_CTL1(GPIOA + 0x400 * (idx)) & ~(0xful << (((pin) - 8) * 4))) | (0x4ul << (((pin) - 8) * 4))) : (GPIO_CTL0(GPIOA + 0x400 * (idx)) = (GPIO_CTL0(GPIOA + 0x400 * (idx)) & ~(0xful << ((pin) * 4))) | (0x4ul << ((pin) * 4))))
#define IO_CFG_OUTPUT(idx, pin) (((pin) >= 8) ? (GPIO_CTL1(GPIOA + 0x400 * (idx)) = (GPIO_CTL1(GPIOA + 0x400 * (idx)) & ~(0xful << (((pin) - 8) * 4))) | (0x3ul << (((pin) - 8) * 4))) : (GPIO_CTL0(GPIOA + 0x400 * (idx)) = (GPIO_CTL0(GPIOA + 0x400 * (idx)) & ~(0xful << ((pin) * 4))) | (0x3ul << ((pin) * 4))))
#define IO_SET(idx, pin) (GPIO_BOP(GPIOA + 0x400 * (idx)) = 0x1ul << (pin))
#define IO_CLEAR(idx, pin) (GPIO_BOP(GPIOA + 0x400 * (idx)) = 0x10000ul << (pin))
#define IO_GET(idx, pin) ((GPIO_ISTAT(GPIOA + 0x400 * (idx)) >> (pin)) & 0x1)


#if PERIPHERAL_GPIO_TCK_SWD_PIN < 8
#   define SWCLK_SWITCH_AFPP()   (GPIO_CTL0(GPIOA + 0x400 * PERIPHERAL_GPIO_TCK_SWD_IDX) |= (0x8ul << (4 * PERIPHERAL_GPIO_TCK_SWD_PIN)))
#else
#   warning "TODO"
#endif
#if PERIPHERAL_GPIO_TCK_SWD_PIN < 8
#   define SWCLK_SWITCH_OUTPP()   (GPIO_CTL0(GPIOA + 0x400 * PERIPHERAL_GPIO_TCK_SWD_IDX) &= ~(0x8ul << (4 * PERIPHERAL_GPIO_TCK_SWD_PIN)))
#else
#   warning "TODO"
#endif

#if (PERIPHERAL_GPIO_TCK_SWD_PIN < 8) && (PERIPHERAL_GPIO_TMS_MO_PIN < 8)
#   define TMS_MO_IN_TCK_OUTPP()   (GPIO_CTL0(GPIOA + 0x400 * PERIPHERAL_GPIO_TCK_SWD_IDX) &= ~((0x8ul << (4 * PERIPHERAL_GPIO_TCK_SWD_PIN)) | (0x3ul << (4 * PERIPHERAL_GPIO_TMS_MO_PIN))))
#else
#   warning "TODO"
#endif

#if (PERIPHERAL_GPIO_TCK_SWD_PIN < 8) && (PERIPHERAL_GPIO_TMS_MO_PIN < 8)
#   define SWDIO_MO_SWCLK_SWITCH_AFPP()   (GPIO_CTL0(GPIOA + 0x400 * PERIPHERAL_GPIO_TCK_SWD_IDX) |= (0x8ul << (4 * PERIPHERAL_GPIO_TCK_SWD_PIN)) | (0xbul << (4 * PERIPHERAL_GPIO_TMS_MO_PIN)))
#else
#   warning "TODO"
#endif

#if (PERIPHERAL_GPIO_TCK_SWD_PIN < 8) && (PERIPHERAL_GPIO_TMS_MO_PIN < 8)
#   define SWDIO_MO_SWITCH_ANALOG_IN_SWCLK_SWITCH_OUTPP()   (GPIO_CTL0(GPIOA + 0x400 * PERIPHERAL_GPIO_TCK_SWD_IDX) &= ~((0x8ul << (4 * PERIPHERAL_GPIO_TCK_SWD_PIN)) | (0xful << (4 * PERIPHERAL_GPIO_TMS_MO_PIN))))
#else
#   warning "TODO"
#endif

#if (PERIPHERAL_GPIO_TMS_MO_PIN < 8)
#   define SWDIO_MO_SWITCH_ANALOG_IN()   (GPIO_CTL0(GPIOA + 0x400 * PERIPHERAL_GPIO_TMS_MO_IDX) &= ~(0xful << (4 * PERIPHERAL_GPIO_TMS_MO_PIN)))
#else
#   warning "TODO"
#endif

#if (PERIPHERAL_GPIO_TMS_MO_PIN < 8)
#   define SWDIO_MO_SWITCH_OUTPP()   (GPIO_CTL0(GPIOA + 0x400 * PERIPHERAL_GPIO_TMS_MO_IDX) |= 0x3ul << (4 * PERIPHERAL_GPIO_TMS_MO_PIN))
#else
#   warning "TODO"
#endif

#if PERIPHERAL_GPIO_TMS_MO_PIN < 8
#   define SWDIO_MO_SWITCH_AFPP()   (GPIO_CTL0(GPIOA + 0x400 * PERIPHERAL_GPIO_TMS_MO_IDX) |= (0xbul << (4 * PERIPHERAL_GPIO_TMS_MO_PIN)))
#else
#   warning "TODO"
#endif

typedef struct swd_transfer_op_t {
    bool is_spi;
    bool is_read;
    uint8_t ack_pos;
    uint8_t io_bitlen_spi_bytelen;
    uint32_t data;
    uint8_t *pdata;
} swd_transfer_op_t;

typedef struct swd_transfer_t {
    volatile bool busy;
    bool read;
    bool need_ack;
    uint8_t ack;
    
    uint16_t retry_cnt;
 
    uint8_t op_idx;
    uint8_t op_num;
    swd_transfer_op_t op[6];    // op[0 : 1]: SEQIN/SEQOUT, op[2 : 5]: Read/Write
} swd_transfer_t;

typedef struct swd_control_t {
    bool sync_mode;
    uint8_t select;

    uint8_t idle;
    uint8_t trn;
    bool data_force;
    uint16_t retry_limit;
    uint16_t delay_tick;

    #if TIMESTAMP_CLOCK
    uint32_t dap_timestamp;
    #endif
    
    int32_t int_priority;
    callback_param_t dma_callback;
	void (*swd_read)(uint8_t *data, uint32_t bits);
	void (*swd_write)(uint8_t *data, uint32_t bits);
    void (*swd_delay)(uint16_t delay_tick);

    swd_transfer_t transfer[2];
} swd_control_t;

//#if SWD_COUNT > 0

static swd_control_t swd_control;

static void swd_read_quick(uint8_t *data, uint32_t bits);
static void swd_read_slow(uint8_t *data, uint32_t bits);
static void swd_write_quick(uint8_t *data, uint32_t bits);
static void swd_write_slow(uint8_t *data, uint32_t bits);
static void swd_transfer_handler_quick(void *param);
static void swd_transfer_handler_slow(void *param);

void vsfhal_swd_init(int32_t int_priority)
{
    PERIPHERAL_GPIO_TMS_INIT();
    PERIPHERAL_GPIO_TCK_INIT();
    PERIPHERAL_GPIO_SRST_INIT();
    vsfhal_swd_io_reconfig();

    switch (SWD_SPI_BASE) {
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

    memset(&swd_control, 0, sizeof(swd_control_t));
    swd_control.int_priority = int_priority;
}

void vsfhal_swd_fini(void)
{
    vsf_dma_config_channel(SWD_SPI_DMA_IDX, SWD_SPI_TX_DMA_CH, NULL, NULL, -1);
    vsf_dma_config_channel(SWD_SPI_DMA_IDX, SWD_SPI_RX_DMA_CH, NULL, NULL, -1);

    IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN);
    IO_CFG_INPUT(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    IO_CFG_INPUT(PERIPHERAL_GPIO_SRST_IDX, PERIPHERAL_GPIO_SRST_PIN);

    PERIPHERAL_GPIO_TMS_FINI();
    PERIPHERAL_GPIO_TCK_FINI();
    PERIPHERAL_GPIO_SRST_FINI();
}

void vsfhal_swd_io_reconfig(void)
{
    PERIPHERAL_SWD_IO_AF_CONFIG();

    /*
        Default:
        SWDIO_MO: Analog Input
        SWDIO_MI: Float Input
        SWCLK   : OUTPP
        SRST    : OUTPP 
    */

    IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    SWDIO_MO_SWITCH_ANALOG_IN();

    //IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN);

    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    
    IO_SET(PERIPHERAL_GPIO_SRST_IDX, PERIPHERAL_GPIO_SRST_PIN);
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_SRST_IDX, PERIPHERAL_GPIO_SRST_PIN);
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
static void delay_swd_2000khz(uint16_t dummy)
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
}

#pragma optimize=none
static void delay_swd_1500khz(uint16_t dummy)
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
}

#pragma optimize=none
static void delay_swd_1000khz(uint16_t dummy)
{
	dummy = 7;
	while (--dummy);
}

#pragma optimize=none
static void swd_delay_ticks(uint16_t delay_tick)
{
    vsf_systimer_cnt_t ticks = vsf_systimer_get() + delay_tick;
    while (ticks >= vsf_systimer_get());
}

static uint32_t inline get_parity_4bit(uint8_t data)
{
    uint8_t temp;
    temp = data >> 2;
    data = data ^ temp;
    temp = data >> 1;
    data = data ^ temp;
    return data & 0x1;
}

static uint32_t inline get_parity_32bit(uint32_t data)
{
    uint32_t temp;
    temp = data >> 16;
    data = data ^ temp;
    temp = data >> 8;
    data = data ^ temp;
    temp = data >> 4;
    data = data ^ temp;
    temp = data >> 2;
    data = data ^ temp;
    temp = data >> 1;
    data = data ^ temp;
    return data & 0x1;
}

void vsfhal_swd_config(uint16_t kHz, uint16_t retry, uint8_t idle, uint8_t trn, bool data_force)
{
    uint32_t temp, apb;
    struct vsfhal_clk_info_t *info = vsfhal_clk_info_get();
    
    #if SWD_SYNC_MODE_FREQ_KHZ
    if (kHz >= SWD_SYNC_MODE_FREQ_KHZ)
        swd_control.sync_mode = true;
    else
        swd_control.sync_mode = false;
    #endif

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

    swd_control.retry_limit = retry;
    swd_control.delay_tick = info->ahb_freq_hz / (kHz * 2000);

    if (idle <= (32 * 6))
        swd_control.idle = idle;
    else
        swd_control.idle = 32 * 6;
    if (trn <= 14)
        swd_control.trn = trn;
    else
        swd_control.trn = 14;
    swd_control.data_force = data_force;

    for (uint8_t i = 0; i < dimof(swd_control.transfer); i++) {
        swd_transfer_t *transfer = &swd_control.transfer[i];
        swd_transfer_op_t *op = transfer->op;

        op[0].is_spi = true;
        op[0].ack_pos = 0;

        op[1].is_spi = false;
        op[1].ack_pos = 0;

        op[2].is_spi = true;
        op[2].is_read = false;
        op[2].ack_pos = 0;
        op[2].io_bitlen_spi_bytelen = 1;
        //op[2].data
        op[2].pdata = (uint8_t *)&op[2].data;
            
        op[3].is_spi = false;
        op[3].is_read = true;
        op[3].ack_pos = trn;
        //op[3].io_bitlen_spi_bytelen
        op[3].pdata = (uint8_t *)&op[3].data;
        
        op[4].is_spi = true;
        //op[4].is_read
        op[4].ack_pos = 0;
        op[4].io_bitlen_spi_bytelen = 4;
        //op[4].pdata
            
        op[5].is_spi = false;
        op[5].ack_pos = 0;
        //op[5].io_bitlen_spi_bytelen
        op[5].pdata = (uint8_t *)&op[5].data;
    }

    if (kHz >= 8000) {
        swd_control.dma_callback = swd_transfer_handler_quick;
        swd_control.swd_read = swd_read_quick;
        swd_control.swd_write = swd_write_quick;
        swd_control.swd_delay = NULL;
    } else if (kHz >= 4000) {
        swd_control.dma_callback = swd_transfer_handler_slow;
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_delay = NULL;
    } else if (kHz >= 2000) {
        swd_control.dma_callback = swd_transfer_handler_slow;
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_delay = delay_swd_2000khz;
    } else if (kHz >= 1500) {
        swd_control.dma_callback = swd_transfer_handler_slow;
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_delay = delay_swd_1500khz;
    } else if (kHz >= 1000) {
        swd_control.dma_callback = swd_transfer_handler_slow;
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_delay = delay_swd_1000khz;
    } else {
        swd_control.dma_callback = swd_transfer_handler_slow;
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_delay = swd_delay_ticks;
    }

    // SPI config
    SPI_CTL0(SWD_SPI_BASE) &= ~(SPI_CTL0_SPIEN | SPI_CTL0_PSC | SPI_CTL0_BDEN | SPI_CTL0_CRCEN | SPI_CTL0_FF16);
    SPI_CTL0(SWD_SPI_BASE) |= temp << 3;
    SPI_CTL0(SWD_SPI_BASE) |= SPI_CTL0_MSTMOD | SPI_CTL0_SWNSSEN | SPI_CTL0_SWNSS | SPI_CTL0_LF | SPI_CTL0_CKPL | SPI_CTL0_CKPH;
    SPI_CTL1(SWD_SPI_BASE) = SPI_CTL1_DMAREN | SPI_CTL1_DMATEN;
    SPI_CTL0(SWD_SPI_BASE) |= SPI_CTL0_SPIEN;

    // DMA TX RX channel config
    DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_RX_DMA_CH) = DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
    DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_FTFIE | DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO; 
    DMA_CHxPADDR(SWD_SPI_DMAX, SWD_SPI_RX_DMA_CH) = SWD_SPI_BASE + 0xc;
    DMA_CHxPADDR(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = SWD_SPI_BASE + 0xc;

    vsf_dma_config_channel(SWD_SPI_DMA_IDX, SWD_SPI_TX_DMA_CH, swd_control.dma_callback, &swd_control, swd_control.int_priority);
    vsf_dma_config_channel(SWD_SPI_DMA_IDX, SWD_SPI_RX_DMA_CH, NULL, NULL, -1);
}

void vsfhal_swd_seqout(uint8_t *data, uint32_t bitlen)
{
    #if SWD_SYNC_MODE_FREQ_KHZ
    if (swd_control.sync_mode) {
        uint_fast32_t bytes = bitlen >> 3;
        if (bytes) {
            SWDIO_MO_SWCLK_SWITCH_AFPP();
            DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO; 
            DMA_CHxMADDR(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = (uint32_t)data;
            DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = bytes;
            DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
            data += bytes;
            bitlen = bitlen & 0x7;
            while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
            SWDIO_MO_SWITCH_ANALOG_IN_SWCLK_SWITCH_OUTPP();
        }
        swd_control.swd_write(data, bitlen);
    }
    else
    #endif
    {
        #if SWD_SEQOUT_SEQIN_ASYNC_ENABLE
        swd_transfer_t *previous_transfer = &swd_control.transfer[swd_control.select];

        if (bitlen >= 8) {
            swd_transfer_t *next_transfer = &swd_control.transfer[swd_control.select ? 0 : 1];
            next_transfer->need_ack = false;
            next_transfer->op_idx = 0;
            next_transfer->op[0].is_read = false;
            next_transfer->op[0].io_bitlen_spi_bytelen = bitlen >> 3;
            next_transfer->op[0].pdata = data;
            if (bitlen & 0x7) {
                next_transfer->op_num = 2;
                next_transfer->op[1].is_read = false;
                next_transfer->op[1].io_bitlen_spi_bytelen = bitlen & 0x7;
                next_transfer->op[1].pdata = data + (bitlen >> 3);
            } else {
                next_transfer->op_num = 1;
            }

            while (previous_transfer->busy);
            next_transfer->busy = true;
            swd_control.select = swd_control.select ? 0 : 1;

            // activer interrupt transfer
            NVIC_SetPendingIRQ(SWD_SPI_IRQ);
        } else {
            while (previous_transfer->busy);
            swd_control.swd_write(data, bitlen);
        }
        #else
            #warning "TODO"
        #endif
    }
}

void vsfhal_swd_seqin(uint8_t *data, uint32_t bitlen)
{
    #if SWD_SYNC_MODE_FREQ_KHZ
    if (swd_control.sync_mode) {
        uint_fast32_t temp, bytes = bitlen >> 3;
        if (bytes) {
            SWCLK_SWITCH_AFPP();
            temp = SPI_DATA(SWD_SPI_BASE);   // clear rx buffer
            DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_RX_DMA_CH) = DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
            DMA_CHxMADDR(SWD_SPI_DMAX, SWD_SPI_RX_DMA_CH) = (uint32_t)data;
            DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_RX_DMA_CH) = bytes;
            DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_RX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
            DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_DIR | DMA_CHXCTL_PRIO; 
            DMA_CHxMADDR(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = (uint32_t)&swd_control.select; // fake tx
            DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = bytes;
            DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_DIR | DMA_CHXCTL_PRIO;
            data += bytes;
            bitlen = bitlen & 0x7;
            while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
            SWDIO_MO_SWITCH_ANALOG_IN_SWCLK_SWITCH_OUTPP();
        }
        swd_control.swd_read(data, bitlen);
    }
    else 
    #endif
    {
        #if SWD_SEQOUT_SEQIN_ASYNC_ENABLE
        swd_transfer_t *previous_transfer = &swd_control.transfer[swd_control.select];

        if (bitlen >= 8) {
            swd_transfer_t *next_transfer = &swd_control.transfer[swd_control.select ? 0 : 1];
            next_transfer->need_ack = false;
            next_transfer->op_idx = 0;
            next_transfer->op[0].is_read = true;
            next_transfer->op[0].io_bitlen_spi_bytelen = bitlen >> 3;
            next_transfer->op[0].pdata = data;
            if (bitlen & 0x7) {
                next_transfer->op_num = 2;
                next_transfer->op[1].is_read = true;
                next_transfer->op[1].io_bitlen_spi_bytelen = bitlen & 0x7;
                next_transfer->op[1].pdata = data + (bitlen >> 3);
            } else {
                next_transfer->op_num = 1;
            }

            while (previous_transfer->busy);
            next_transfer->busy = true;
            swd_control.select = swd_control.select ? 0 : 1;

            // activer interrupt transfer
            NVIC_SetPendingIRQ(SWD_SPI_IRQ);
        } else {
            while (previous_transfer->busy);
            swd_control.swd_read(data, bitlen);
        }
        #else
            #warning "TODO"
        #endif
    }
}

uint32_t vsfhal_swd_read(uint32_t request, uint8_t *r_data)
{
    #if SWD_SYNC_MODE_FREQ_KHZ
    if (swd_control.sync_mode) {
        uint_fast32_t tick, temp, retry = 0;
        uint32_t buffer;

    SYNC_READ_RESTART:
        temp = get_parity_4bit(request) << 5;
        buffer = ((request << 1) & 0x1e) | 0x81 | temp;

        // Request:[W]*8
        SWDIO_MO_SWCLK_SWITCH_AFPP();
        DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO; 
        DMA_CHxMADDR(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = (uint32_t)&buffer;
        DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = 1;
        DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
        temp = 0;
        tick = swd_control.trn;
        if (!r_data)
            r_data = (uint8_t *)&buffer;
        while (DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH));
        while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);

        // TRN:[C]*trn
        SWDIO_MO_SWITCH_ANALOG_IN_SWCLK_SWITCH_OUTPP();
        while (tick--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            //if (swd_control.swd_delay)
            //    swd_control.swd_delay(swd_control.delay_tick);
        }

        // ACK:[R]*3
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        temp = IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        temp |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) << 1;
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        temp |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) << 2;
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);

        if (temp == SWD_ACK_OK) {
            // Data:[R]*32
            SWCLK_SWITCH_AFPP();
            temp = SPI_DATA(SWD_SPI_BASE);   // clear rx buffer
            DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_RX_DMA_CH) = DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
            DMA_CHxMADDR(SWD_SPI_DMAX, SWD_SPI_RX_DMA_CH) = (uint32_t)r_data;
            DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_RX_DMA_CH) = 4;
            DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_RX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
            DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_DIR | DMA_CHXCTL_PRIO; 
            DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = 4;
            DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_DIR | DMA_CHXCTL_PRIO;
            tick = swd_control.trn + swd_control.idle;
            //while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
            while (DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_RX_DMA_CH));
            
            // Parity:[R]*1
            SWDIO_MO_SWITCH_ANALOG_IN_SWCLK_SWITCH_OUTPP();
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            temp = IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);

            // Trn:[C]*trn --> Idle:[C]*idle
            while (tick--) {
                //if (swd_control.swd_delay)
                //    swd_control.swd_delay(swd_control.delay_tick);
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            }

            if (temp == get_parity_32bit(get_unaligned_le32(r_data))) {
                return SWD_ACK_OK | SWD_SUCCESS;
            } else {
                return SWD_ACK_OK | SWD_PARITY_ERROR;
            }
        } else if ((temp == SWD_ACK_WAIT) || (temp == SWD_ACK_FAULT)) {
            if (swd_control.data_force) {
                // Data:[C]*32
                SWCLK_SWITCH_AFPP();
                DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_DIR | DMA_CHXCTL_PRIO; 
                DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = 4;
                DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_DIR | DMA_CHXCTL_PRIO;
                tick = 1 + swd_control.trn;
                while (DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH));
                while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);

                // Parity:[C]*1 -> Trn:[C]*trn
                SWDIO_MO_SWITCH_ANALOG_IN_SWCLK_SWITCH_OUTPP();
                while (tick--) {
                    //if (swd_control.swd_delay)
                    //    swd_control.swd_delay(swd_control.delay_tick);
                    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                    if (swd_control.swd_delay)
                        swd_control.swd_delay(swd_control.delay_tick);
                    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                }
            } else {
                // Trn:[C]*trn
                tick = swd_control.trn;
                while (tick--) {
                    //if (swd_control.swd_delay)
                    //    swd_control.swd_delay(swd_control.delay_tick);
                    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                    if (swd_control.swd_delay)
                        swd_control.swd_delay(swd_control.delay_tick);
                    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                }
            }

            if ((temp == SWD_ACK_WAIT) && (retry++ < swd_control.retry_limit))
                goto SYNC_READ_RESTART;
            else
                return temp;
        } else {
            // Data:[C]*32
            SWCLK_SWITCH_AFPP();
            DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_DIR | DMA_CHXCTL_PRIO; 
            DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = 4;
            DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_DIR | DMA_CHXCTL_PRIO;
            while (DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH));
            while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);

            // Parity:[C]*1
            SWDIO_MO_SWITCH_ANALOG_IN_SWCLK_SWITCH_OUTPP();
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        }
        return temp;
    }
    else
    #endif
    {
        uint32_t ret;
    
        swd_transfer_t *previous_transfer = &swd_control.transfer[swd_control.select];
        swd_transfer_t *next_transfer = &swd_control.transfer[swd_control.select ? 0 : 1];
        
        // prepare next transfer
        {
            swd_transfer_op_t *op = next_transfer->op;
            uint_fast32_t parity = get_parity_4bit(request) << 5;
            uint_fast32_t mid = swd_control.trn + 3;    // TRN:[C]*trn --> ACK:[R]*3
            uint_fast32_t end = 1 + swd_control.trn + swd_control.idle; // Parity:[R]*1 -> Trn:[C]*trn --> Idle:[C]*idle

            op[2].data = ((request << 1) & 0x1e) | 0x81 | parity;
            op[3].io_bitlen_spi_bytelen = mid;
            op[4].is_read = true;
            op[4].pdata = r_data ? r_data : (uint8_t *)&op[4].data;
            op[5].is_read = true;
            op[5].io_bitlen_spi_bytelen = end;

            next_transfer->need_ack = true;
            next_transfer->read = true;
            next_transfer->retry_cnt = 0;
            next_transfer->op_idx = 2;
            next_transfer->op_num = 6;
        }

        while (previous_transfer->busy);
        ret = previous_transfer->ack;
        
        if (ret == SWD_ACK_OK) {
            next_transfer->busy = true;
            swd_control.select = swd_control.select ? 0 : 1;
            
            // activer interrupt transfer
            NVIC_SetPendingIRQ(SWD_SPI_IRQ);
        }
        return ret;
    }

}

uint32_t vsfhal_swd_write(uint32_t request, uint8_t *w_data)
{
    #if SWD_SYNC_MODE_FREQ_KHZ
    if (swd_control.sync_mode) {
        uint_fast32_t tick, temp, retry = 0;
        uint32_t buffer;
        
    SYNC_READ_RESTART:
        temp = get_parity_4bit(request) << 5;
        buffer = ((request << 1) & 0x1e) | 0x81 | temp;

        // Request:[W]*8
        SWDIO_MO_SWCLK_SWITCH_AFPP();
        DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO; 
        DMA_CHxMADDR(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = (uint32_t)&buffer;
        DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = 1;
        DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
        temp = 0;
        tick = swd_control.trn;
        while (DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH));
        while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);

        // TRN:[C]*trn
        SWDIO_MO_SWITCH_ANALOG_IN_SWCLK_SWITCH_OUTPP();
        while (tick--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            //if (swd_control.swd_delay)
            //    swd_control.swd_delay(swd_control.delay_tick);
        }

        // ACK:[R]*3
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        temp = IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        temp |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) << 1;
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        temp |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) << 2;
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);

        if (temp == SWD_ACK_OK) {
            // TRN:[C]*trn
            tick = swd_control.trn;
            SWDIO_MO_SWITCH_ANALOG_IN_SWCLK_SWITCH_OUTPP();
            while (tick--) {
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                //if (swd_control.swd_delay)
                //    swd_control.swd_delay(swd_control.delay_tick);
            }

            // Data:[W]*32
            SWDIO_MO_SWCLK_SWITCH_AFPP();
            DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO; 
            DMA_CHxMADDR(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = (uint32_t)w_data;
            DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = 4;
            DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
            temp = get_parity_32bit(get_unaligned_le32(w_data));
            tick = swd_control.idle;
            while (DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH));
            while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);

            // Parity:[W]*1
            SWDIO_MO_SWITCH_ANALOG_IN_SWCLK_SWITCH_OUTPP();
            SWDIO_MO_SWITCH_OUTPP();
            if (temp)
                IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            else
                IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);

            // Idle:[C]*idle
            while (tick--) {
                //if (swd_control.swd_delay)
                //    swd_control.swd_delay(swd_control.delay_tick);
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            }
            SWDIO_MO_SWITCH_ANALOG_IN();

            return SWD_ACK_OK | SWD_SUCCESS;
        } else if ((temp == SWD_ACK_WAIT) || (temp == SWD_ACK_FAULT)) {
            // TRN:[C]*trn
            tick = swd_control.trn;
            SWDIO_MO_SWITCH_ANALOG_IN_SWCLK_SWITCH_OUTPP();
            while (tick--) {
                //if (swd_control.swd_delay)
                //    swd_control.swd_delay(swd_control.delay_tick);
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            }

            if (swd_control.data_force) {
                // Data:[C]*32
                SWCLK_SWITCH_AFPP();
                DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO; 
                DMA_CHxMADDR(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = (uint32_t)w_data;
                DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = 4;
                DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
                while (DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH));
                while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
                
                // Parity:[C]*1
                SWDIO_MO_SWITCH_ANALOG_IN_SWCLK_SWITCH_OUTPP();
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            }

            if ((temp == SWD_ACK_WAIT) && (retry++ < swd_control.retry_limit))
                goto SYNC_READ_RESTART;
            else
                return temp;
        } else {
            // Data:[C]*32
            SWCLK_SWITCH_AFPP();
            DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_DIR | DMA_CHXCTL_PRIO;
            DMA_CHxMADDR(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = (uint32_t)w_data;
            DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = 4;
            DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_DIR | DMA_CHXCTL_PRIO;
            while (DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH));
            while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);

            // Parity:[C]*1
            SWDIO_MO_SWITCH_ANALOG_IN_SWCLK_SWITCH_OUTPP();
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);

            return temp;
        }
    }
    else
    #endif
    {
        uint32_t ret;
        
        swd_transfer_t *previous_transfer = &swd_control.transfer[swd_control.select];
        swd_transfer_t *next_transfer = &swd_control.transfer[swd_control.select ? 0 : 1];
        
        // prepare next transfer
        {
            swd_transfer_op_t *op = next_transfer->op;
            uint_fast32_t parity = get_parity_4bit(request) << 5;
            uint_fast32_t mid = swd_control.trn * 2 + 3;    // TRN:[C]*trn --> ACK:[R]*3 --> TRN:[C]*trn
            uint_fast32_t end = 1 + swd_control.idle;       // Parity:[W]*1 -> Idle:[C]*idle

            op[2].data = ((request << 1) & 0x1e) | 0x81 | parity;
            op[3].io_bitlen_spi_bytelen = mid;
            op[4].is_read = false;
            op[4].pdata = w_data;
            op[5].is_read = false;
            op[5].data = 0xfffffffe | get_parity_32bit(get_unaligned_le32(w_data));
            op[5].io_bitlen_spi_bytelen = end;

            next_transfer->need_ack = true;
            next_transfer->read = false;
            next_transfer->retry_cnt = 0;
            next_transfer->op_idx = 2;
            next_transfer->op_num = 6;
        }

        while (previous_transfer->busy);
        ret = previous_transfer->ack;
        
        if (ret == SWD_ACK_OK) {
            next_transfer->busy = true;
            swd_control.select = swd_control.select ? 0 : 1;
            
            // activer interrupt transfer
            NVIC_SetPendingIRQ(SWD_SPI_IRQ);
        }
        return ret;
    }
}

uint32_t vsfhal_swd_wait(void)
{
    if (swd_control.sync_mode)
        return SWD_ACK_OK;
    else {
        swd_transfer_t *previous_transfer = &swd_control.transfer[swd_control.select];
        while (previous_transfer->busy);
        return previous_transfer->ack;
    }
}

void vsfhal_swd_clear(void)
{
    swd_control.transfer[0].ack = SWD_ACK_OK;
    swd_control.transfer[1].ack = SWD_ACK_OK;
}

static void swd_read_quick(uint8_t *data, uint32_t bits)
{
    uint32_t byte = 0, pos = 8 - bits;

    while (bits--) {
        byte >>= 1;
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        byte |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) ? 0x80 : 0;
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        __ASM("NOP");
    }
    *data = byte >> pos;
}

static void swd_read_slow(uint8_t *data, uint32_t bits)
{
    uint32_t byte = 0, pos = 8 - bits;

    while (bits--) {
        byte >>= 1;
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        byte |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) ? 0x80 : 0;
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);       
    }
    *data = byte >> pos;
}

static void swd_write_quick(uint8_t *data, uint32_t bits)
{
    uint32_t byte = data[0];

    SWDIO_MO_SWITCH_OUTPP();
    
    while (bits) {
        if (byte & 0x1)
            IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        else
            IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        byte >>= 1;
        bits--;
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    }

    SWDIO_MO_SWITCH_ANALOG_IN();
}

static void swd_write_slow(uint8_t *data, uint32_t bits)
{
    uint32_t byte = data[0];

    SWDIO_MO_SWITCH_OUTPP();
    
    while (bits) {
        if (byte & 0x1)
            IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        else
            IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        byte >>= 1;
        bits--;
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
    }

    SWDIO_MO_SWITCH_ANALOG_IN();
}

#pragma optimize=size
static void swd_transfer_handler_quick(void *param)
{
    VSF_HAL_ASSERT(PERIPHERAL_GPIO_TMS_MO_IDX == PERIPHERAL_GPIO_TCK_SWD_IDX)
    struct swd_transfer_op_t *op;
    swd_control_t *control = param;
    swd_transfer_t *transfer = &control->transfer[control->select];

    // wait spi tx/rx finish
    //while (SPI_STAT(SWD_SPI_BASE) & (SPI_STAT_RBNE | SPI_STAT_TRANS));
    while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);

TRANSFER_RESTART:
    SWDIO_MO_SWITCH_ANALOG_IN_SWCLK_SWITCH_OUTPP();
    while (transfer->op_idx < transfer->op_num) {
        op = &transfer->op[transfer->op_idx];
        transfer->op_idx++;

        if (op->is_spi) {
            if (op->is_read) {
                uint_fast32_t temp;
                SWCLK_SWITCH_AFPP();
                temp = SPI_DATA(SWD_SPI_BASE);   // clear rx buffer
                DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_RX_DMA_CH) = DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
                DMA_CHxMADDR(SWD_SPI_DMAX, SWD_SPI_RX_DMA_CH) = (uint32_t)op->pdata;
                DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_RX_DMA_CH) = op->io_bitlen_spi_bytelen;
                DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_RX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
                DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_FTFIE | DMA_CHXCTL_DIR | DMA_CHXCTL_PRIO; 
                DMA_CHxMADDR(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = (uint32_t)&(op->pdata); // fake tx
                DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = op->io_bitlen_spi_bytelen;
                DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_FTFIE | DMA_CHXCTL_DIR | DMA_CHXCTL_PRIO;
            } else {
                SWDIO_MO_SWCLK_SWITCH_AFPP();
                DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_FTFIE | DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO; 
                DMA_CHxMADDR(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = (uint32_t)op->pdata;
                DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = op->io_bitlen_spi_bytelen;
                DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_FTFIE | DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
            }
            return;
        } else {
            uint_fast32_t bits = op->io_bitlen_spi_bytelen;
            uint_fast32_t pos, byte, ack;

            if (op->is_read) {
                pos = 32 - bits;
                byte = 0;
                do {
                    byte >>= 1;
                    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                    byte |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) ? 0x80000000 : 0;
                    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                    __ASM("NOP");
                    __ASM("NOP");
                    __ASM("NOP");
                    bits--;
                } while (bits);
                byte = byte >> pos;
                if (op->ack_pos) {
                    transfer->ack = (byte >> op->ack_pos) & 0x7;
                    if (transfer->ack == SWD_ACK_OK) {
                        continue;
                    } else if ((transfer->ack == SWD_ACK_WAIT) || (transfer->ack == SWD_ACK_FAULT)) {
                        if (!control->data_force) {
                            if (transfer->read) {
                                bits = control->trn;
                                do {
                                    pos >>= 1;
                                    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                                    pos |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) ? 0x80000000 : 0;
                                    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                                    __ASM("NOP");
                                    __ASM("NOP");
                                    __ASM("NOP");
                                    bits--;
                                } while (bits);
                            }
                            break;
                        } else {
                            // cut idle
                            transfer->op[5].io_bitlen_spi_bytelen -= control->idle;
                            continue;
                        }
                    } else {
                        // WARNING
                        // cut idle
                        transfer->op[5].io_bitlen_spi_bytelen -= control->idle;
                        continue;
                    }
                }
                op->data = byte;
            } else {
                SWDIO_MO_SWITCH_OUTPP();
                byte = op->pdata[0];
                do {
                    if (byte & 0x1)
                        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                    else
                        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                    byte >>= 1;
                    bits--;
                    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                } while (bits);
            }
        }
    }

    if (transfer->need_ack) {
        if (transfer->ack == SWD_ACK_OK) {
            if ((transfer->op[5].data & 0x1) != get_parity_32bit(get_unaligned_le32(transfer->op[4].pdata)))
                transfer->ack = SWD_ACK_OK | SWD_PARITY_ERROR;
            //else
            //    transfer->ack = SWD_ACK_OK | SWD_SUCCESS;
        } else if (transfer->ack == SWD_ACK_WAIT) {
            if (transfer->retry_cnt++ < control->retry_limit) {
                if (control->data_force) {
                    // add idle
                    transfer->op[5].io_bitlen_spi_bytelen += control->idle;
                }
                transfer->op_idx = 2;
                goto TRANSFER_RESTART;
            }
        }
    }
    
    SWDIO_MO_SWITCH_ANALOG_IN_SWCLK_SWITCH_OUTPP();

    transfer->busy = false;
}

#pragma optimize=size
static void swd_transfer_handler_slow(void *param)
{
    VSF_HAL_ASSERT(PERIPHERAL_GPIO_TMS_MO_IDX == PERIPHERAL_GPIO_TCK_SWD_IDX)
    struct swd_transfer_op_t *op;
    swd_control_t *control = param;
    swd_transfer_t *transfer = &control->transfer[control->select];

    // wait spi tx/rx finish
    //while (SPI_STAT(SWD_SPI_BASE) & (SPI_STAT_RBNE | SPI_STAT_TRANS));
    while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);

TRANSFER_RESTART:
    SWDIO_MO_SWITCH_ANALOG_IN_SWCLK_SWITCH_OUTPP();
    while (transfer->op_idx < transfer->op_num) {
        op = &transfer->op[transfer->op_idx];
        transfer->op_idx++;

        if (op->is_spi) {
            if (op->is_read) {
                uint_fast32_t temp;
                SWCLK_SWITCH_AFPP();
                temp = SPI_DATA(SWD_SPI_BASE);   // clear rx buffer
                DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_RX_DMA_CH) = DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
                DMA_CHxMADDR(SWD_SPI_DMAX, SWD_SPI_RX_DMA_CH) = (uint32_t)op->pdata;
                DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_RX_DMA_CH) = op->io_bitlen_spi_bytelen;
                DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_RX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
                DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_FTFIE | DMA_CHXCTL_DIR | DMA_CHXCTL_PRIO; 
                DMA_CHxMADDR(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = (uint32_t)&(op->pdata); // fake tx
                DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = op->io_bitlen_spi_bytelen;
                DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_FTFIE | DMA_CHXCTL_DIR | DMA_CHXCTL_PRIO;
            } else {
                SWDIO_MO_SWCLK_SWITCH_AFPP();
                DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_FTFIE | DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO; 
                DMA_CHxMADDR(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = (uint32_t)op->pdata;
                DMA_CHxCNT(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = op->io_bitlen_spi_bytelen;
                DMA_CHxCTL(SWD_SPI_DMAX, SWD_SPI_TX_DMA_CH) = DMA_CHXCTL_CHEN | DMA_CHXCTL_FTFIE | DMA_CHXCTL_DIR | DMA_CHXCTL_MNAGA | DMA_CHXCTL_PRIO;
            }
            return;
        } else {
            uint_fast32_t bits = op->io_bitlen_spi_bytelen;
            uint_fast32_t pos, byte, ack;

            if (op->is_read) {
                pos = 32 - bits;
                byte = 0;
                do {
                    byte >>= 1;
                    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                    if (control->swd_delay)
                        control->swd_delay(control->delay_tick);
                    byte |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) ? 0x80000000 : 0;
                    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                    if (control->swd_delay)
                        control->swd_delay(control->delay_tick);
                    bits--;
                } while (bits);
                byte = byte >> pos;
                if (op->ack_pos) {
                    transfer->ack = (byte >> op->ack_pos) & 0x7;
                    if (transfer->ack == SWD_ACK_OK) {
                        continue;
                    } else if ((transfer->ack == SWD_ACK_WAIT) || (transfer->ack == SWD_ACK_FAULT)) {
                        if (!control->data_force) {
                            if (transfer->read) {
                                bits = control->trn;
                                do {
                                    pos >>= 1;
                                    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                                    if (control->swd_delay)
                                        control->swd_delay(control->delay_tick);
                                    pos |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) ? 0x80000000 : 0;
                                    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                                    if (control->swd_delay)
                                        control->swd_delay(control->delay_tick);
                                    bits--;
                                } while (bits);
                            }
                            break;
                        } else {
                            // cut idle
                            transfer->op[5].io_bitlen_spi_bytelen -= control->idle;
                            continue;
                        }
                    } else {
                        // WARNING
                        // cut idle
                        transfer->op[5].io_bitlen_spi_bytelen -= control->idle;
                        continue;
                    }
                }
                op->data = byte;
            } else {
                SWDIO_MO_SWITCH_OUTPP();
                byte = op->pdata[0];
                do {
                    if (byte & 0x1)
                        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                    else
                        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                    byte >>= 1;
                    bits--;
                    if (control->swd_delay)
                        control->swd_delay(control->delay_tick);
                    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                    if (control->swd_delay)
                        control->swd_delay(control->delay_tick);
                } while (bits);
            }
        }
    }

    if (transfer->need_ack) {
        if (transfer->ack == SWD_ACK_OK) {
            if ((transfer->op[5].data & 0x1) != get_parity_32bit(get_unaligned_le32(transfer->op[4].pdata)))
                transfer->ack = SWD_ACK_OK | SWD_PARITY_ERROR;
            //else
            //    transfer->ack = SWD_ACK_OK | SWD_SUCCESS;
        } else if (transfer->ack == SWD_ACK_WAIT) {
            if (transfer->retry_cnt++ < control->retry_limit) {
                if (control->data_force) {
                    // add idle
                    transfer->op[5].io_bitlen_spi_bytelen += control->idle;
                }
                transfer->op_idx = 2;
                goto TRANSFER_RESTART;
            }
        }
    }
    
    SWDIO_MO_SWITCH_ANALOG_IN_SWCLK_SWITCH_OUTPP();

    transfer->busy = false;
}

#if TIMESTAMP_CLOCK
uint32_t vsfhal_swd_get_timestamp(void)
{
    return swd_control.dap_timestamp;
}
#endif

//#endif
