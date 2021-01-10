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
#include "timestamp.h"
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

#define IO_CFG_INPUT(idx, pin)      (GPIO_CTL(GPIOA_BASE + 0x400 * (idx)) &= ~(0x3 << (pin * 2)))
#define IO_CFG_OUTPUT(idx, pin)     (GPIO_CTL(GPIOA_BASE + 0x400 * (idx)) = (GPIO_CTL(GPIOA_BASE + 0x400 * (idx)) & ~(0x3ul << (pin * 2))) | (0x1ul << (pin * 2)))
#define IO_CFG_AF(idx, pin)         (GPIO_CTL(GPIOA_BASE + 0x400 * (idx)) = (GPIO_CTL(GPIOA_BASE + 0x400 * (idx)) & ~(0x3ul << (pin * 2))) | (0x2ul << (pin * 2)))
#define IO_SET(idx, pin)            (GPIO_OCTL(GPIOA_BASE + 0x400 * (idx)) |= 0x1 << pin)
#define IO_CLEAR(idx, pin)          (GPIO_BC(GPIOA_BASE + 0x400 * (idx)) = 0x1 << pin)
#define IO_GET(idx, pin)            ((GPIO_ISTAT(GPIOA_BASE + 0x400 * (idx)) >> pin) & 0x1)

#define SWDIO_MI_TO_AFIN_SWCLK_TO_AFPP()    (IO_CFG_AF(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN),IO_CFG_AF(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN))
#define SWDIO_MI_TO_IN_SWCLK_TO_OUTPP()     (IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN),IO_CFG_OUTPUT(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN))
#define SWDIO_MO_TO_AFPP_SWCLK_TO_AFPP()    (IO_CFG_AF(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN),IO_CFG_AF(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN))
#define SWDIO_MO_TO_IN_SWCLK_TO_OUTPP()     (IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN),IO_CFG_OUTPUT(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN))
#define SWDIO_MO_TO_OUTPP_SWCLK_TO_OUTPP()  (IO_CFG_OUTPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN),IO_CFG_OUTPUT(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN))
#define SWDIO_MO_TO_IN()                    (IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN))
#define SWDIO_MO_TO_OUTPP()                 (IO_CFG_OUTPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN))
#define SWDIO_MO_TO_AFPP()                  (IO_CFG_AF(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN))


typedef struct swd_control_t {
    uint8_t dummy;
    uint8_t idle;
    uint8_t trn;
    bool data_force;
    uint16_t retry_limit;
    uint16_t delay_tick;

    #if TIMESTAMP_CLOCK
    uint32_t dap_timestamp;
    #endif

    uint32_t (*swd_read)(uint32_t request, uint8_t *r_data);
    uint32_t (*swd_write)(uint32_t request, uint8_t *w_data);
    void (*swd_read_io)(uint8_t *data, uint32_t bits);
    void (*swd_write_io)(uint8_t *data, uint32_t bits);
    void (*swd_delay)(uint16_t delay_tick);
} swd_control_t;

#if SWD_COUNT > 0

static swd_control_t swd_control;

static uint32_t swd_read_quick(uint32_t request, uint8_t *r_data);
static uint32_t swd_read_slow(uint32_t request, uint8_t *r_data);
static uint32_t swd_write_quick(uint32_t request, uint8_t *w_data);
static uint32_t swd_write_slow(uint32_t request, uint8_t *w_data);
static void swd_read_io_quick(uint8_t *data, uint32_t bits);
static void swd_read_io_slow(uint8_t *data, uint32_t bits);
static void swd_write_io_quick(uint8_t *data, uint32_t bits);
static void swd_write_io_slow(uint8_t *data, uint32_t bits);

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
    }

    memset(&swd_control, 0, sizeof(swd_control_t));
}

void vsfhal_swd_fini(void)
{
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
        SWDIO_MO: Float Input
        SWDIO_MI: Float Input
        SWCLK   : OUTPP
        SRST    : OUTPP 
    */

    IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    SWDIO_MO_TO_IN();

    IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN);

    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    
    IO_SET(PERIPHERAL_GPIO_SRST_IDX, PERIPHERAL_GPIO_SRST_PIN);
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_SRST_IDX, PERIPHERAL_GPIO_SRST_PIN);
}

#pragma optimize=none
static void delay_swd_4000khz_3000khz(uint16_t dummy)
{
}

#pragma optimize=none
static void delay_swd_2000khz_1500khz(uint16_t dummy)
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
static void delay_swd_1000khz_750khz(uint16_t dummy)
{
	dummy = 7;
	while (--dummy);
    __ASM("NOP");
    __ASM("NOP");
}

#pragma optimize=none
static void delay_swd_500khz_375khz(uint16_t dummy)
{
	dummy = 19;
	while (--dummy);
}

#pragma optimize=none
static void delay_swd_250khz_188khz(uint16_t dummy)
{
	dummy = 40;
	while (--dummy);
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

#ifdef PROJ_CFG_GD32E10X_AHP_APB_UNFIXED
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
#else
const static uint32_t spi_khz_and_apb_clk_table_list[][2] = {
    {64000000, 32000,}, // 64M / 2 = 32M
    {64000000, 16000,}, // 64M / 4 = 16M
    {64000000, 8000,},  // 64M / 8 = 8M
    {64000000, 4000,},  // 64M / 16 = 4M
    {64000000, 2000,},  // 64M / 32 = 2M
    {64000000, 1000,},  // 64M / 64 = 1M
    {64000000, 500,},   // 64M / 128 = 500K
    {64000000, 250,},   // 64M / 256 = 250K
};
#endif

void vsfhal_swd_config(uint16_t kHz, uint16_t retry, uint8_t idle, uint8_t trn, bool data_force)
{
    uint32_t temp, apb;
    struct vsfhal_clk_info_t *info = vsfhal_clk_info_get();

    if (kHz < spi_khz_and_apb_clk_table_list[dimof(spi_khz_and_apb_clk_table_list) - 1][1])
        kHz = spi_khz_and_apb_clk_table_list[dimof(spi_khz_and_apb_clk_table_list) - 1][1];
    for (temp = 0; temp < dimof(spi_khz_and_apb_clk_table_list); temp++) {
        if (kHz >= spi_khz_and_apb_clk_table_list[temp][1]) {
            kHz = spi_khz_and_apb_clk_table_list[temp][1];
            apb = spi_khz_and_apb_clk_table_list[temp][0];
            break;
        }
    }

    #ifdef PROJ_CFG_GD32F3X0_AHP_APB_UNFIXED
    temp = temp / 2;
    vsfhal_clk_reconfig_apb(apb);
    #endif

    info = vsfhal_clk_info_get();

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
    swd_control.retry_limit = retry;

    if (kHz >= 6000) {
        swd_control.swd_read = swd_read_quick;
        swd_control.swd_write = swd_write_quick;
        swd_control.swd_read_io = swd_read_io_quick;
        swd_control.swd_write_io = swd_write_io_quick;
        swd_control.swd_delay = NULL;
    } else if (kHz >= 3000) {
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_read_io = swd_read_io_slow;
        swd_control.swd_write_io = swd_write_io_slow;
        swd_control.swd_delay = delay_swd_4000khz_3000khz;
    } else if (kHz >= 1500) {
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_read_io = swd_read_io_slow;
        swd_control.swd_write_io = swd_write_io_slow;
        swd_control.swd_delay = delay_swd_2000khz_1500khz;
    } else if (kHz >= 750) {
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_read_io = swd_read_io_slow;
        swd_control.swd_write_io = swd_write_io_slow;
        swd_control.swd_delay = delay_swd_1000khz_750khz;
    } else if (kHz >= 375) {
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_read_io = swd_read_io_slow;
        swd_control.swd_write_io = swd_write_io_slow;
        swd_control.swd_delay = delay_swd_500khz_375khz;
    } else {
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_read_io = swd_read_io_slow;
        swd_control.swd_write_io = swd_write_io_slow;
        swd_control.swd_delay = delay_swd_250khz_188khz;
    }

    // SPI config
    SPI_CTL0(SWD_SPI_BASE) &= ~(SPI_CTL0_SPIEN | SPI_CTL0_PSC | SPI_CTL0_BDEN | SPI_CTL0_CRCEN | SPI_CTL0_FF16);
    SPI_CTL0(SWD_SPI_BASE) |= temp << 3;
    SPI_CTL0(SWD_SPI_BASE) |= SPI_CTL0_MSTMOD | SPI_CTL0_SWNSSEN | SPI_CTL0_SWNSS | SPI_CTL0_LF | SPI_CTL0_CKPL | SPI_CTL0_CKPH;
    SPI_CTL0(SWD_SPI_BASE) |= SPI_CTL0_SPIEN;
}

void vsfhal_swd_seqout(uint8_t *data, uint32_t bitlen)
{
    uint8_t dummy;
    uint_fast32_t bytes = bitlen >> 3;
    if (bytes) {
        SWDIO_MO_TO_AFPP_SWCLK_TO_AFPP();
        do {
            SPI_DATA(SWD_SPI_BASE) = *data;
            data++;
            bytes--;
            while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
            dummy = SPI_DATA(SWD_SPI_BASE);
        } while (bytes);
        SWDIO_MO_TO_IN_SWCLK_TO_OUTPP();
    }
    bitlen = bitlen & 0x7;
    if (bitlen)
        swd_control.swd_write_io(data, bitlen);
}

void vsfhal_swd_seqin(uint8_t *data, uint32_t bitlen)
{
    uint_fast32_t bytes = bitlen >> 3;
    if (bytes) {
        SWDIO_MI_TO_AFIN_SWCLK_TO_AFPP();
        do {
            SPI_DATA(SWD_SPI_BASE) = 0xff;
            bytes--;
            while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
            *data = SPI_DATA(SWD_SPI_BASE);
            data++;
        } while (bytes);
        SWDIO_MI_TO_IN_SWCLK_TO_OUTPP();
    }
    bitlen = bitlen & 0x7;
    if (bitlen)
        swd_control.swd_read_io(data, bitlen);
}

uint32_t vsfhal_swd_read(uint32_t request, uint8_t *r_data)
{
    return swd_control.swd_read(request, r_data);
}

uint32_t vsfhal_swd_write(uint32_t request, uint8_t *w_data)
{
    return swd_control.swd_write(request, w_data);
}

// OFF "Instruction scheduling"
static uint32_t swd_read_quick(uint32_t request, uint8_t *r_data)
{
    uint_fast32_t tick, temp, retry = 0;
    uint32_t buffer;

SYNC_READ_RESTART:
    temp = get_parity_4bit(request) << 5;
    buffer = ((request << 1) & 0x1e) | 0x81 | temp;

    // Request:[W]*8
    SWDIO_MO_TO_AFPP_SWCLK_TO_AFPP();
    SPI_DATA(SWD_SPI_BASE) = buffer;
    if (!r_data)
        r_data = (uint8_t *)&buffer;
    tick = swd_control.trn;
    while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
    buffer = SPI_DATA(SWD_SPI_BASE);

    // TRN:[C]*trn
    SWDIO_MO_TO_IN_SWCLK_TO_OUTPP();
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
    //if (swd_control.swd_delay)
    //    swd_control.swd_delay(swd_control.delay_tick);
    temp = IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN);
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    //if (swd_control.swd_delay)
    //    swd_control.swd_delay(swd_control.delay_tick);
    temp |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) << 1;
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    //if (swd_control.swd_delay)
    //    swd_control.swd_delay(swd_control.delay_tick);
    temp |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) << 2;
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    //if (swd_control.swd_delay)
    //    swd_control.swd_delay(swd_control.delay_tick);

    if (temp == SWD_ACK_OK) {
        // Data:[R]*32
        SWDIO_MI_TO_AFIN_SWCLK_TO_AFPP();
        for (temp = 0; temp < 4; temp++) {
            SPI_DATA(SWD_SPI_BASE) = 0xff;
            while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
            r_data[temp] = SPI_DATA(SWD_SPI_BASE);
        }

        // Parity:[R]*1
        SWDIO_MI_TO_IN_SWCLK_TO_OUTPP();
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        //if (swd_control.swd_delay)
        //    swd_control.swd_delay(swd_control.delay_tick);
        temp = IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        //if (swd_control.swd_delay)
        //    swd_control.swd_delay(swd_control.delay_tick);

        tick = swd_control.trn + swd_control.idle;
        
        // Trn:[C]*trn --> Idle:[C]*idle
        while (tick--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            //if (swd_control.swd_delay)
            //    swd_control.swd_delay(swd_control.delay_tick);
        }

        #if TIMESTAMP_CLOCK
        if (request & SWD_TRANS_TIMESTAMP)
            swd_control.dap_timestamp = vsfhal_timestamp_get();
        #endif

        if (temp == get_parity_32bit(get_unaligned_le32(r_data))) {
            return SWD_ACK_OK | SWD_SUCCESS;
        } else {
            return SWD_ACK_OK | SWD_PARITY_ERROR;
        }
    } else if ((temp == SWD_ACK_WAIT) || (temp == SWD_ACK_FAULT)) {
        if (swd_control.data_force) {
            // Data:[C]*32
            SWDIO_MI_TO_AFIN_SWCLK_TO_AFPP();
            temp = 4;
            do {
                SPI_DATA(SWD_SPI_BASE) = 0xff;
                temp--;
                while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
                buffer = SPI_DATA(SWD_SPI_BASE);
            } while (temp);

            tick = 1 + swd_control.trn;

            // Parity:[C]*1 -> Trn:[C]*trn
            SWDIO_MI_TO_IN_SWCLK_TO_OUTPP();
            while (tick--) {
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                //if (swd_control.swd_delay)
                //    swd_control.swd_delay(swd_control.delay_tick);
            }
        } else {
            // Trn:[C]*trn
            tick = swd_control.trn;
            while (tick--) {
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                //if (swd_control.swd_delay)
                //    swd_control.swd_delay(swd_control.delay_tick);
            }
        }

        if ((temp == SWD_ACK_WAIT) && (retry++ < swd_control.retry_limit))
            goto SYNC_READ_RESTART;
        else
            return temp;
    } else {
        // Data:[C]*32
        SWDIO_MI_TO_AFIN_SWCLK_TO_AFPP();
        temp = 4;
        do {
            SPI_DATA(SWD_SPI_BASE) = 0xff;
            temp--;
            while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
            buffer = SPI_DATA(SWD_SPI_BASE);
        } while (temp);

        // Parity:[C]*1
        SWDIO_MI_TO_IN_SWCLK_TO_OUTPP();
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    }
    return temp;
}

static uint32_t swd_read_slow(uint32_t request, uint8_t *r_data)
{
    uint_fast32_t tick, temp, retry = 0;
    uint32_t buffer;

SYNC_READ_RESTART:
    temp = get_parity_4bit(request) << 5;
    buffer = ((request << 1) & 0x1e) | 0x81 | temp;

    // Request:[W]*8
    SWDIO_MO_TO_AFPP_SWCLK_TO_AFPP();
    SPI_DATA(SWD_SPI_BASE) = buffer;
    if (!r_data)
        r_data = (uint8_t *)&buffer;
    tick = swd_control.trn;
    while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
    buffer = SPI_DATA(SWD_SPI_BASE);

    // TRN:[C]*trn
    SWDIO_MO_TO_IN_SWCLK_TO_OUTPP();
    while (tick--) {
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
    }

    // ACK:[R]*3
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    temp = IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN);
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    temp |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) << 1;
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    temp |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) << 2;
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);

    if (temp == SWD_ACK_OK) {
        // Data:[R]*32
        SWDIO_MI_TO_AFIN_SWCLK_TO_AFPP();
        for (temp = 0; temp < 4; temp++) {
            SPI_DATA(SWD_SPI_BASE) = 0xff;
            while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
            r_data[temp] = SPI_DATA(SWD_SPI_BASE);
        }

        // Parity:[R]*1
        SWDIO_MI_TO_IN_SWCLK_TO_OUTPP();
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        temp = IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);

        tick = swd_control.trn + swd_control.idle;
        
        // Trn:[C]*trn --> Idle:[C]*idle
        while (tick--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
        }

        #if TIMESTAMP_CLOCK
        if (request & SWD_TRANS_TIMESTAMP)
            swd_control.dap_timestamp = vsfhal_timestamp_get();
        #endif

        if (temp == get_parity_32bit(get_unaligned_le32(r_data))) {
            return SWD_ACK_OK | SWD_SUCCESS;
        } else {
            return SWD_ACK_OK | SWD_PARITY_ERROR;
        }
    } else if ((temp == SWD_ACK_WAIT) || (temp == SWD_ACK_FAULT)) {
        if (swd_control.data_force) {
            // Data:[C]*32
            SWDIO_MI_TO_AFIN_SWCLK_TO_AFPP();
            temp = 4;
            do {
                SPI_DATA(SWD_SPI_BASE) = 0xff;
                temp--;
                while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
                buffer = SPI_DATA(SWD_SPI_BASE);
            } while (temp);

            tick = 1 + swd_control.trn;

            // Parity:[C]*1 -> Trn:[C]*trn
            SWDIO_MI_TO_IN_SWCLK_TO_OUTPP();
            while (tick--) {
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
            }
        } else {
            // Trn:[C]*trn
            tick = swd_control.trn;
            while (tick--) {
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
            }
        }

        if ((temp == SWD_ACK_WAIT) && (retry++ < swd_control.retry_limit))
            goto SYNC_READ_RESTART;
        else
            return temp;
    } else {
        // Data:[C]*32
        SWDIO_MI_TO_AFIN_SWCLK_TO_AFPP();
        temp = 4;
        do {
            SPI_DATA(SWD_SPI_BASE) = 0xff;
            temp--;
            while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
            buffer = SPI_DATA(SWD_SPI_BASE);
        } while (temp);

        // Parity:[C]*1
        SWDIO_MI_TO_IN_SWCLK_TO_OUTPP();
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    }
    return temp;
}

// OFF "Instruction scheduling"
static uint32_t swd_write_quick(uint32_t request, uint8_t *w_data)
{
    uint_fast32_t tick, temp, retry = 0;
    uint32_t buffer;
    
SYNC_READ_RESTART:
    temp = get_parity_4bit(request) << 5;
    buffer = ((request << 1) & 0x1e) | 0x81 | temp;

    // Request:[W]*8
    SWDIO_MO_TO_AFPP_SWCLK_TO_AFPP();
    SPI_DATA(SWD_SPI_BASE) = buffer;
    tick = swd_control.trn;
    while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
    buffer = SPI_DATA(SWD_SPI_BASE);

    // TRN:[C]*trn
    SWDIO_MO_TO_IN_SWCLK_TO_OUTPP();
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
    //if (swd_control.swd_delay)
    //    swd_control.swd_delay(swd_control.delay_tick);
    temp = IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN);
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    //if (swd_control.swd_delay)
    //    swd_control.swd_delay(swd_control.delay_tick);
    temp |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) << 1;
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    //if (swd_control.swd_delay)
    //    swd_control.swd_delay(swd_control.delay_tick);
    temp |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) << 2;
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    //if (swd_control.swd_delay)
    //    swd_control.swd_delay(swd_control.delay_tick);

    if (temp == SWD_ACK_OK) {
        // TRN:[C]*trn
        tick = swd_control.trn;
        while (tick--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            //if (swd_control.swd_delay)
            //    swd_control.swd_delay(swd_control.delay_tick);
        }

        // Data:[W]*32
        SWDIO_MO_TO_AFPP_SWCLK_TO_AFPP();
        for (temp = 0; temp < 4; temp++) {
            SPI_DATA(SWD_SPI_BASE) = w_data[temp];
            while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
            buffer = SPI_DATA(SWD_SPI_BASE);
        }

        temp = get_parity_32bit(get_unaligned_le32(w_data));
        tick = swd_control.idle;

        // Parity:[W]*1
        SWDIO_MO_TO_OUTPP_SWCLK_TO_OUTPP();
        if (temp)
            IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        else
            IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        //if (swd_control.swd_delay)
        //    swd_control.swd_delay(swd_control.delay_tick);

        // Idle:[C]*idle
        while (tick--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            //if (swd_control.swd_delay)
            //    swd_control.swd_delay(swd_control.delay_tick);
        }
        SWDIO_MO_TO_IN_SWCLK_TO_OUTPP();

        #if TIMESTAMP_CLOCK
        if (request & SWD_TRANS_TIMESTAMP)
            swd_control.dap_timestamp = vsfhal_timestamp_get();
        #endif

        return SWD_ACK_OK | SWD_SUCCESS;
    } else if ((temp == SWD_ACK_WAIT) || (temp == SWD_ACK_FAULT)) {
        // TRN:[C]*trn
        tick = swd_control.trn;
        while (tick--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            //if (swd_control.swd_delay)
            //    swd_control.swd_delay(swd_control.delay_tick);
        }

        if (swd_control.data_force) {
            // Data:[C]*32
            SWDIO_MI_TO_AFIN_SWCLK_TO_AFPP();
            temp = 4;
            do {
                SPI_DATA(SWD_SPI_BASE) = 0xff;
                temp--;
                while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
                buffer = SPI_DATA(SWD_SPI_BASE);
            } while (temp);
            
            // Parity:[C]*1
            SWDIO_MI_TO_IN_SWCLK_TO_OUTPP();
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            //if (swd_control.swd_delay)
            //    swd_control.swd_delay(swd_control.delay_tick);
        }

        if ((temp == SWD_ACK_WAIT) && (retry++ < swd_control.retry_limit))
            goto SYNC_READ_RESTART;
        else
            return temp;
    } else {
        // Data:[C]*32
        SWDIO_MI_TO_AFIN_SWCLK_TO_AFPP();
        temp = 4;
        do {
            SPI_DATA(SWD_SPI_BASE) = 0xff;
            temp--;
            while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
            buffer = SPI_DATA(SWD_SPI_BASE);
        } while (temp);

        // Parity:[C]*1
        SWDIO_MI_TO_IN_SWCLK_TO_OUTPP();
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);

        return temp;
    }
}

static uint32_t swd_write_slow(uint32_t request, uint8_t *w_data)
{
    uint_fast32_t tick, temp, retry = 0;
    uint32_t buffer;
    
SYNC_READ_RESTART:
    temp = get_parity_4bit(request) << 5;
    buffer = ((request << 1) & 0x1e) | 0x81 | temp;

    // Request:[W]*8
    SWDIO_MO_TO_AFPP_SWCLK_TO_AFPP();
    SPI_DATA(SWD_SPI_BASE) = buffer;
    tick = swd_control.trn;
    while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
    buffer = SPI_DATA(SWD_SPI_BASE);

    // TRN:[C]*trn
    SWDIO_MO_TO_IN_SWCLK_TO_OUTPP();
    while (tick--) {
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
    }

    // ACK:[R]*3
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    temp = IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN);
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    temp |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) << 1;
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    temp |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) << 2;
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);

    if (temp == SWD_ACK_OK) {
        // TRN:[C]*trn
        tick = swd_control.trn;
        while (tick--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
        }

        // Data:[W]*32
        SWDIO_MO_TO_AFPP_SWCLK_TO_AFPP();
        for (temp = 0; temp < 4; temp++) {
            SPI_DATA(SWD_SPI_BASE) = w_data[temp];
            while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
            buffer = SPI_DATA(SWD_SPI_BASE);
        }

        temp = get_parity_32bit(get_unaligned_le32(w_data));
        tick = swd_control.idle;

        // Parity:[W]*1
        SWDIO_MO_TO_OUTPP_SWCLK_TO_OUTPP();
        if (temp)
            IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        else
            IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);

        // Idle:[C]*idle
        while (tick--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
        }
        SWDIO_MO_TO_IN_SWCLK_TO_OUTPP();

        #if TIMESTAMP_CLOCK
        if (request & SWD_TRANS_TIMESTAMP)
            swd_control.dap_timestamp = vsfhal_timestamp_get();
        #endif

        return SWD_ACK_OK | SWD_SUCCESS;
    } else if ((temp == SWD_ACK_WAIT) || (temp == SWD_ACK_FAULT)) {
        // TRN:[C]*trn
        tick = swd_control.trn;
        while (tick--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
        }

        if (swd_control.data_force) {
            // Data:[C]*32
            SWDIO_MI_TO_AFIN_SWCLK_TO_AFPP();
            temp = 4;
            do {
                SPI_DATA(SWD_SPI_BASE) = 0xff;
                temp--;
                while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
                buffer = SPI_DATA(SWD_SPI_BASE);
            } while (temp);
            
            // Parity:[C]*1
            SWDIO_MI_TO_IN_SWCLK_TO_OUTPP();
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
        }

        if ((temp == SWD_ACK_WAIT) && (retry++ < swd_control.retry_limit))
            goto SYNC_READ_RESTART;
        else
            return temp;
    } else {
        // Data:[C]*32
        SWDIO_MI_TO_AFIN_SWCLK_TO_AFPP();
        temp = 4;
        do {
            SPI_DATA(SWD_SPI_BASE) = 0xff;
            temp--;
            while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS);
            buffer = SPI_DATA(SWD_SPI_BASE);
        } while (temp);

        // Parity:[C]*1
        SWDIO_MI_TO_IN_SWCLK_TO_OUTPP();
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);

        return temp;
    }
}

// OFF "Instruction scheduling"
static void swd_read_io_quick(uint8_t *data, uint32_t bits)
{
    uint32_t byte = 0, pos = 8 - bits;

    while (bits--) {
        byte >>= 1;
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        byte |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) ? 0x80 : 0;
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    }
    *data = byte >> pos;
}

static void swd_read_io_slow(uint8_t *data, uint32_t bits)
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

// OFF "Instruction scheduling"
static void swd_write_io_quick(uint8_t *data, uint32_t bits)
{
    uint32_t byte = data[0];

    SWDIO_MO_TO_OUTPP();
    
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

    SWDIO_MO_TO_IN();
}

static void swd_write_io_slow(uint8_t *data, uint32_t bits)
{
    uint32_t byte = data[0];

    SWDIO_MO_TO_OUTPP();
    
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

    SWDIO_MO_TO_IN();
}

#if TIMESTAMP_CLOCK
uint32_t vsfhal_swd_get_timestamp(void)
{
    return swd_control.dap_timestamp;
}
#endif

#endif
