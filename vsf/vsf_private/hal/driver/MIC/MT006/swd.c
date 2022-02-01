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
#include "device.h"
#include "io.h"
#include "vsf.h"


typedef struct
{				
    volatile unsigned long GPIO[3][8];
} GPIO_IOCON_TypeDef_Priv;
#define GPIOIOCON_Priv                      ((GPIO_IOCON_TypeDef_Priv *) IOCON_BASE)

#define IO_CON_CONFIG(idx, pin, config)     (GPIOIOCON_Priv->GPIO[idx][pin] = config)

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

#define IO_CFG_INPUT(idx, pin)      (GPIOBANK0->DIR_CLR = (0x1ul << ((idx) * 8 + (pin))))
#define IO_CFG_OUTPUT(idx, pin)     (GPIOBANK0->DIR_SET = (0x1ul << ((idx) * 8 + (pin))))
#define IO_SET(idx, pin)            (GPIODATA0->DT_SET = (0x1ul << ((idx) * 8 + (pin))))
#define IO_CLEAR(idx, pin)          (GPIODATA0->DT_CLR = (0x1ul << ((idx) * 8 + (pin))))
#define IO_GET(idx, pin)            ((GPIODATA0->DT >> ((idx) * 8 + (pin))) & 0x1ul)


#define PERIPHERAL_SWD_SELECT_SPI()         do {IO_CON_CONFIG(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN, PERIPHERAL_GPIO_TMS_MO_AF);\
                                                IO_CON_CONFIG(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN, PERIPHERAL_GPIO_TMS_MI_AF);\
                                                IO_CON_CONFIG(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN, PERIPHERAL_GPIO_TCK_SWD_AF);} while (0);

#define SWDIO_MO_TO_IN()                    (IO_CON_CONFIG(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN, IO_INPUT_FLOAT))
#define SWDIO_MO_TO_OUTPP()                 (IO_CON_CONFIG(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN, IO_OUTPUT))
#define SWDIO_MO_TO_AFPP()                  (IO_CON_CONFIG(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN, PERIPHERAL_GPIO_TMS_MO_AF))
#define SWDIO_MI_TO_IN()                    (IO_CON_CONFIG(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN, IO_INPUT_FLOAT))
#define SWDIO_MI_TO_AFIN()                  (IO_CON_CONFIG(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN, PERIPHERAL_GPIO_TMS_MI_AF))
#define SWCLK_TO_OUTPP()                    (IO_CON_CONFIG(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN, IO_OUTPUT))
#define SWCLK_TO_AFPP()                     (IO_CON_CONFIG(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN, PERIPHERAL_GPIO_TCK_SWD_AF))

#define SWD_SPI_CR0_DEFAULT         (SPI_CR0_SPO | SPI_CR0_SPH | (0x1 << 8))

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

    memset(&swd_control, 0, sizeof(swd_control_t));
    
    RCC->SPI0CLKDIV = 1;
    RCC->AHBCLKCTRL0_SET = RCC_AHBCLKCTRL_SPI0;
    RCC->PRESETCTRL0_CLR = RCC_PRESETCTRL_SPI0;
    RCC->PRESETCTRL0_SET = RCC_PRESETCTRL_SPI0;
}

void vsfhal_swd_fini(void)
{
    PERIPHERAL_GPIO_TMS_FINI();
    PERIPHERAL_GPIO_TCK_FINI();
    PERIPHERAL_GPIO_SRST_FINI();
}

void vsfhal_swd_io_reconfig(void)
{
    /*
        Default:
        SWDIO_MO: Float Input
        SWDIO_MI: Float Input
        SWCLK   : OUTPP
        SRST    : OUTPP 
    */
    SWDIO_MO_TO_IN();
    SWDIO_MI_TO_IN();
    PERIPHERAL_GPIO_TMS_SET();
    
    SWCLK_TO_OUTPP();
    PERIPHERAL_GPIO_TCK_SET();

    PERIPHERAL_GPIO_SRST_SET_OUTPUT();
    PERIPHERAL_GPIO_SRST_SET();
}

#pragma optimize=none
static void delay_swd_1000khz(uint16_t dummy)
{
}

#pragma optimize=none
static void delay_swd_750khz(uint16_t dummy)
{
    NOP();
    NOP();
    NOP();
    NOP();
}

#pragma optimize=none
static void delay_swd_500khz(uint16_t dummy)
{
    dummy = 3;
	while (--dummy);
}

#pragma optimize=none
static void delay_swd_dynamic(uint16_t dummy)
{
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

/*
    48000000	2	24000000
    48000000	3	16000000
    48000000	4	12000000
    48000000	5	9600000
    48000000	6	8000000
    48000000	8	6000000
    48000000	10	4800000
    48000000	12	4000000
    48000000	15	3200000
    48000000	16	3000000
    48000000	20	2400000
    48000000	24	2000000
    48000000	25	1920000
    48000000	30	1600000
    48000000	32	1500000
    48000000	40	1200000
    48000000	48	1000000
    48000000	50	960000
    48000000	60	800000
    48000000	64	750000
    48000000	75	640000
    48000000	80	600000
    48000000	96	500000
    48000000	100	480000
    48000000	120	400000
    48000000	125	384000
    48000000	128	375000
    48000000	150	320000
    48000000	160	300000
    48000000	192	250000
    48000000	200	240000
    48000000	240	200000
    48000000	250	192000
*/

const static uint16_t spi_khz_and_cpsdvr_table_list[][2] = {
    {24000, 2},
    {16000, 3},
    {12000, 4},
    {9600, 5},
    {8000, 6},
    {6000, 8},
    {4000, 12},
    {3000, 16},
    {2000, 24},
    {1000, 48},
    {750, 64},
    {500, 96},
    {250, 192},
};
void vsfhal_swd_config(uint16_t kHz, uint16_t retry, uint8_t idle, uint8_t trn, bool data_force)
{
    // SCR = 1
    uint32_t temp, cpsdvr;
    const vsfhal_clk_info_t *info = vsfhal_clk_info_get();
    
    if (kHz < spi_khz_and_cpsdvr_table_list[dimof(spi_khz_and_cpsdvr_table_list) - 1][0])
        kHz = spi_khz_and_cpsdvr_table_list[dimof(spi_khz_and_cpsdvr_table_list) - 1][0];
    for (temp = 0; temp < dimof(spi_khz_and_cpsdvr_table_list); temp++) {
        if (kHz >= spi_khz_and_cpsdvr_table_list[temp][0]) {
            kHz = spi_khz_and_cpsdvr_table_list[temp][0];
            cpsdvr = spi_khz_and_cpsdvr_table_list[temp][1];
            break;
        }
    }
    
    swd_control.delay_tick = info->ahb_apb_freq_hz / (kHz * 2000) / 24;

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

    if (kHz >= 4000) {
        swd_control.swd_read = swd_read_quick;
        swd_control.swd_write = swd_write_quick;
        swd_control.swd_read_io = swd_read_io_quick;
        swd_control.swd_write_io = swd_write_io_quick;
        swd_control.swd_delay = NULL;
    } else if (kHz >= 1500) {
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_read_io = swd_read_io_slow;
        swd_control.swd_write_io = swd_write_io_slow;
        swd_control.swd_delay = NULL;
    } else if (kHz >= 1000) {
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_read_io = swd_read_io_slow;
        swd_control.swd_write_io = swd_write_io_slow;
        swd_control.swd_delay = delay_swd_1000khz;
    } else if (kHz >= 750) {
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_read_io = swd_read_io_slow;
        swd_control.swd_write_io = swd_write_io_slow;
        swd_control.swd_delay = delay_swd_750khz;
    } else if (kHz >= 500) {
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_read_io = swd_read_io_slow;
        swd_control.swd_write_io = swd_write_io_slow;
        swd_control.swd_delay = delay_swd_500khz;
    } else {
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_read_io = swd_read_io_slow;
        swd_control.swd_write_io = swd_write_io_slow;
        swd_control.swd_delay = delay_swd_dynamic;
    }
    
    // SPI config
    SPI0->CR0 = SWD_SPI_CR0_DEFAULT | SPI_CR0_DSS_8BIT;
    SPI0->CPSR = cpsdvr;
    SPI0->CR1 = SPI_CR1_SSE;
}

static const uint8_t lsb2msb[] = {
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0, 
    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 
    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4, 
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC, 
    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2, 
    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6, 
    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9, 
    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3, 
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7, 
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

void vsfhal_swd_seqout(uint8_t *data, uint32_t bitlen)
{
    uint8_t dummy;
    uint_fast32_t bytes = bitlen >> 3;
    if (bytes) {
        SWDIO_MO_TO_AFPP();
        SWCLK_TO_AFPP();
        do {
            SPI0->DR = lsb2msb[*data];
            data++;
            bytes--;
            while (SPI0->SR & SPI_SR_BSY);
            dummy = SPI0->DR;
        } while (bytes);
        SWDIO_MO_TO_OUTPP();
        SWCLK_TO_OUTPP();
    }
    bitlen = bitlen & 0x7;
    if (bitlen)
        swd_control.swd_write_io(data, bitlen);
}

void vsfhal_swd_seqin(uint8_t *data, uint32_t bitlen)
{
    uint_fast32_t bytes = bitlen >> 3;
    if (bytes) {
        SWDIO_MI_TO_AFIN();
        SWCLK_TO_AFPP();
        do {
            SPI0->DR = 0xff;
            bytes--;
            while (SPI0->SR & SPI_SR_BSY);
            *data = lsb2msb[SPI0->DR & 0xff];
            data++;
        } while (bytes);
        SWDIO_MI_TO_IN();
        SWCLK_TO_OUTPP();
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
    SWDIO_MO_TO_AFPP();
    SWCLK_TO_AFPP();
    SPI0->DR = lsb2msb[buffer];
    if (!r_data)
        r_data = (uint8_t *)&buffer;
    tick = swd_control.trn - 1;
    while (SPI0->SR & SPI_SR_BSY);
    buffer = SPI0->DR;
    
    // TRN:[C]*(trn - 1)
    if (tick) {
        SWCLK_TO_OUTPP();
        while (tick) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            //if (swd_control.swd_delay)
            //    swd_control.swd_delay(swd_control.delay_tick);
            tick--;
        }
        SWCLK_TO_AFPP();
    }

    // TRN + ACK:[R]*3
    SWDIO_MI_TO_AFIN();
    SWDIO_MO_TO_IN();
    SPI0->CR0 = SWD_SPI_CR0_DEFAULT | SPI_CR0_DSS_4BIT;
    SPI0->DR = 0xff;
    while (SPI0->SR & SPI_SR_BSY);
    temp = lsb2msb[SPI0->DR & 0x7] >> 5;

    if (temp == SWD_ACK_OK) {
        // Data:[R]*32
        SPI0->CR0 = SWD_SPI_CR0_DEFAULT | SPI_CR0_DSS_8BIT;
        SPI0->DR = 0xff;
        SPI0->DR = 0xff;
        SPI0->DR = 0xff;
        SPI0->DR = 0xff;
        while (SPI0->SR & SPI_SR_BSY);
        r_data[0] = lsb2msb[SPI0->DR & 0xff];
        r_data[1] = lsb2msb[SPI0->DR & 0xff];
        r_data[2] = lsb2msb[SPI0->DR & 0xff];
        r_data[3] = lsb2msb[SPI0->DR & 0xff];

        // Parity:[R]*1
        SWDIO_MI_TO_IN();
        SWCLK_TO_OUTPP();
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
        if (swd_control.data_force) {        // Data:[R]*32
            SPI0->CR0 = SWD_SPI_CR0_DEFAULT | SPI_CR0_DSS_8BIT;
            SPI0->DR = 0xff;
            SPI0->DR = 0xff;
            SPI0->DR = 0xff;
            SPI0->DR = 0xff;
            while (SPI0->SR & SPI_SR_BSY);
            buffer = SPI0->DR;
            buffer = SPI0->DR;
            buffer = SPI0->DR;
            buffer = SPI0->DR;

            tick = 1 + swd_control.trn;

            // Parity:[C]*1 -> Trn:[C]*trn
            SWDIO_MI_TO_IN();
            SWCLK_TO_OUTPP();
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
            SWDIO_MI_TO_IN();
            SWCLK_TO_OUTPP();
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
        SPI0->CR0 = SWD_SPI_CR0_DEFAULT | SPI_CR0_DSS_8BIT;
        SPI0->DR = 0xff;
        SPI0->DR = 0xff;
        SPI0->DR = 0xff;
        SPI0->DR = 0xff;
        while (SPI0->SR & SPI_SR_BSY);
        buffer = SPI0->DR;
        buffer = SPI0->DR;
        buffer = SPI0->DR;
        buffer = SPI0->DR;

        // Parity:[C]*1
        SWDIO_MI_TO_IN();
        SWCLK_TO_OUTPP();
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        //if (swd_control.swd_delay)
        //    swd_control.swd_delay(swd_control.delay_tick);
    }
    return temp;
}

static uint32_t swd_read_slow(uint32_t request, uint8_t *r_data)
{
    uint_fast32_t bits, temp, retry = 0;
    uint32_t buffer;

SYNC_READ_RESTART:
    temp = get_parity_4bit(request) << 5;
    buffer = ((request << 1) & 0x1e) | 0x81 | temp;

    // Request:[W]*8
    SWDIO_MO_TO_AFPP();
    SWCLK_TO_AFPP();
    SPI0->DR = lsb2msb[buffer];
    if (!r_data)
        r_data = (uint8_t *)&buffer;
    bits = swd_control.trn - 1;
    while (SPI0->SR & SPI_SR_BSY);
    buffer = SPI0->DR;

    // TRN:[C]*(trn - 1)
    if (bits) {
        SWCLK_TO_OUTPP();
        while (bits) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            bits--;
        }
        SWCLK_TO_AFPP();
    }
    
    // TRN + ACK:[R]*3
    SWDIO_MI_TO_AFIN();
    SWDIO_MO_TO_IN();
    SPI0->CR0 = SWD_SPI_CR0_DEFAULT | SPI_CR0_DSS_4BIT;
    SPI0->DR = 0xff;
    while (SPI0->SR & SPI_SR_BSY);
    temp = lsb2msb[SPI0->DR & 0x7] >> 5;

    if (temp == SWD_ACK_OK) {
        // Data:[R]*32
        SPI0->CR0 = SWD_SPI_CR0_DEFAULT | SPI_CR0_DSS_8BIT;
        SPI0->DR = 0xff;
        SPI0->DR = 0xff;
        SPI0->DR = 0xff;
        SPI0->DR = 0xff;
        while (SPI0->SR & SPI_SR_BSY);
        r_data[0] = lsb2msb[SPI0->DR & 0xff];
        r_data[1] = lsb2msb[SPI0->DR & 0xff];
        r_data[2] = lsb2msb[SPI0->DR & 0xff];
        r_data[3] = lsb2msb[SPI0->DR & 0xff];

        // Parity:[R]*1
        SWDIO_MI_TO_IN();
        SWCLK_TO_OUTPP();
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        temp = IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);

        bits = swd_control.trn + swd_control.idle;
        
        // Trn:[C]*trn --> Idle:[C]*idle
        while (bits--) {
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
        if (swd_control.data_force) {        // Data:[R]*32
            SPI0->CR0 = SWD_SPI_CR0_DEFAULT | SPI_CR0_DSS_8BIT;
            SPI0->DR = 0xff;
            SPI0->DR = 0xff;
            SPI0->DR = 0xff;
            SPI0->DR = 0xff;
            while (SPI0->SR & SPI_SR_BSY);
            buffer = SPI0->DR;
            buffer = SPI0->DR;
            buffer = SPI0->DR;
            buffer = SPI0->DR;

            bits = 1 + swd_control.trn;

            // Parity:[C]*1 -> Trn:[C]*trn
            SWDIO_MI_TO_IN();
            SWCLK_TO_OUTPP();
            while (bits--) {
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
            }
        } else {
            // Trn:[C]*trn
            bits = swd_control.trn;
            SWDIO_MI_TO_IN();
            SWCLK_TO_OUTPP();
            while (bits--) {
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
        SPI0->CR0 = SWD_SPI_CR0_DEFAULT | SPI_CR0_DSS_8BIT;
        SPI0->DR = 0xff;
        SPI0->DR = 0xff;
        SPI0->DR = 0xff;
        SPI0->DR = 0xff;
        while (SPI0->SR & SPI_SR_BSY);
        buffer = SPI0->DR;
        buffer = SPI0->DR;
        buffer = SPI0->DR;
        buffer = SPI0->DR;

        // Parity:[C]*1
        SWDIO_MI_TO_IN();
        SWCLK_TO_OUTPP();
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
    }
    return temp;
}

static uint32_t swd_write_quick(uint32_t request, uint8_t *w_data)
{
    uint_fast32_t bits, temp, retry = 0;
    uint32_t buffer;
    
SYNC_READ_RESTART:
    temp = get_parity_4bit(request) << 5;
    buffer = ((request << 1) & 0x1e) | 0x81 | temp;

    // Request:[W]*8
    SWDIO_MO_TO_AFPP();
    SWCLK_TO_AFPP();
    SPI0->DR = lsb2msb[buffer];
    bits = swd_control.trn - 1;
    while (SPI0->SR & SPI_SR_BSY);
    buffer = SPI0->DR;

    // TRN:[C]*(trn - 1)
    if (bits) {
        SWCLK_TO_OUTPP();
        while (bits--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            //if (swd_control.swd_delay)
            //    swd_control.swd_delay(swd_control.delay_tick);
            bits--;
        }
        SWCLK_TO_AFPP();
    }

    // TRN + ACK:[R]*3
    SWDIO_MI_TO_AFIN();
    SWDIO_MO_TO_IN();
    SPI0->CR0 = SWD_SPI_CR0_DEFAULT | SPI_CR0_DSS_4BIT;
    SPI0->DR = 0xff;
    while (SPI0->SR & SPI_SR_BSY);
    temp = lsb2msb[SPI0->DR & 0x7] >> 5;

    if (temp == SWD_ACK_OK) {
        // TRN:[C]*trn
        bits = swd_control.trn;
        SWCLK_TO_OUTPP();
        while (bits) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            //if (swd_control.swd_delay)
            //    swd_control.swd_delay(swd_control.delay_tick);
            bits--;
        }

        // Data:[W]*32
        SWCLK_TO_AFPP();
        SPI0->CR0 = SWD_SPI_CR0_DEFAULT | SPI_CR0_DSS_8BIT;
        SPI0->DR = lsb2msb[w_data[0]];
        SPI0->DR = lsb2msb[w_data[1]];
        SPI0->DR = lsb2msb[w_data[2]];
        SPI0->DR = lsb2msb[w_data[3]];
        temp = get_parity_32bit(get_unaligned_le32(w_data));
        bits = swd_control.idle;
        while (SPI0->SR & SPI_SR_BSY);
        buffer = SPI0->DR;
        buffer = SPI0->DR;
        buffer = SPI0->DR;
        buffer = SPI0->DR;

        // Parity:[W]*1
        SWDIO_MO_TO_OUTPP();
        SWCLK_TO_OUTPP();
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

        while (bits) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            //if (swd_control.swd_delay)
            //    swd_control.swd_delay(swd_control.delay_tick);
            bits--;
        }

        #if TIMESTAMP_CLOCK
        if (request & SWD_TRANS_TIMESTAMP)
            swd_control.dap_timestamp = vsfhal_timestamp_get();
        #endif

        SWDIO_MO_TO_IN();
        SWDIO_MI_TO_IN();
        return SWD_ACK_OK | SWD_SUCCESS;
    } else if ((temp == SWD_ACK_WAIT) || (temp == SWD_ACK_FAULT)) {
        // TRN:[C]*trn
        bits = swd_control.trn;
        SWCLK_TO_OUTPP();
        while (bits) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            //if (swd_control.swd_delay)
            //    swd_control.swd_delay(swd_control.delay_tick);
            bits--;
        }

        if (swd_control.data_force) {
            // Data:[C]*32
            SWCLK_TO_AFPP();
            SPI0->CR0 = SWD_SPI_CR0_DEFAULT | SPI_CR0_DSS_8BIT;
            SPI0->DR = 0xff;
            SPI0->DR = 0xff;
            SPI0->DR = 0xff;
            SPI0->DR = 0xff;
            bits = swd_control.idle;
            while (SPI0->SR & SPI_SR_BSY);
            buffer = SPI0->DR;
            buffer = SPI0->DR;
            buffer = SPI0->DR;
            buffer = SPI0->DR;
            
            // Parity:[C]*1
            SWCLK_TO_OUTPP();
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            //if (swd_control.swd_delay)
            //    swd_control.swd_delay(swd_control.delay_tick);
        }

        SWDIO_MO_TO_IN();
        SWDIO_MI_TO_IN();
        if ((temp == SWD_ACK_WAIT) && (retry++ < swd_control.retry_limit))
            goto SYNC_READ_RESTART;
        else
            return temp;
    } else {
        // Data:[C]*32
        SWCLK_TO_AFPP();
        SPI0->CR0 = SWD_SPI_CR0_DEFAULT | SPI_CR0_DSS_8BIT;
        SPI0->DR = 0xff;
        SPI0->DR = 0xff;
        SPI0->DR = 0xff;
        SPI0->DR = 0xff;
        bits = swd_control.idle;
        while (SPI0->SR & SPI_SR_BSY);
        buffer = SPI0->DR;
        buffer = SPI0->DR;
        buffer = SPI0->DR;
        buffer = SPI0->DR;

        // Parity:[C]*1
        SWCLK_TO_OUTPP();
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        //if (swd_control.swd_delay)
        //    swd_control.swd_delay(swd_control.delay_tick);
        
        SWDIO_MO_TO_IN();
        SWDIO_MI_TO_IN();
        return temp;
    }
}

static uint32_t swd_write_slow(uint32_t request, uint8_t *w_data)
{
    uint_fast32_t bits, temp, retry = 0;
    uint32_t buffer;
    
SYNC_READ_RESTART:
    temp = get_parity_4bit(request) << 5;
    buffer = ((request << 1) & 0x1e) | 0x81 | temp;

    // Request:[W]*8
    SWDIO_MO_TO_AFPP();
    SWCLK_TO_AFPP();
    SPI0->DR = lsb2msb[buffer];
    bits = swd_control.trn - 1;
    while (SPI0->SR & SPI_SR_BSY);
    buffer = SPI0->DR;

    // TRN:[C]*(trn - 1)
    if (bits) {
        SWCLK_TO_OUTPP();
        while (bits--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            bits--;
        }
        SWCLK_TO_AFPP();
    }

    // TRN + ACK:[R]*3
    SWDIO_MI_TO_AFIN();
    SWDIO_MO_TO_IN();
    SPI0->CR0 = SWD_SPI_CR0_DEFAULT | SPI_CR0_DSS_4BIT;
    SPI0->DR = 0xff;
    while (SPI0->SR & SPI_SR_BSY);
    temp = lsb2msb[SPI0->DR & 0x7] >> 5;

    if (temp == SWD_ACK_OK) {
        // TRN:[C]*trn
        bits = swd_control.trn;
        SWCLK_TO_OUTPP();
        while (bits) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            bits--;
        }

        // Data:[W]*32
        SWCLK_TO_AFPP();
        SPI0->CR0 = SWD_SPI_CR0_DEFAULT | SPI_CR0_DSS_8BIT;
        SPI0->DR = lsb2msb[w_data[0]];
        SPI0->DR = lsb2msb[w_data[1]];
        SPI0->DR = lsb2msb[w_data[2]];
        SPI0->DR = lsb2msb[w_data[3]];
        temp = get_parity_32bit(get_unaligned_le32(w_data));
        bits = swd_control.idle;
        while (SPI0->SR & SPI_SR_BSY);
        buffer = SPI0->DR;
        buffer = SPI0->DR;
        buffer = SPI0->DR;
        buffer = SPI0->DR;

        // Parity:[W]*1
        SWDIO_MO_TO_OUTPP();
        SWCLK_TO_OUTPP();
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

        while (bits) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            bits--;
        }

        #if TIMESTAMP_CLOCK
        if (request & SWD_TRANS_TIMESTAMP)
            swd_control.dap_timestamp = vsfhal_timestamp_get();
        #endif

        SWDIO_MO_TO_IN();
        SWDIO_MI_TO_IN();
        return SWD_ACK_OK | SWD_SUCCESS;
    } else if ((temp == SWD_ACK_WAIT) || (temp == SWD_ACK_FAULT)) {
        // TRN:[C]*trn
        bits = swd_control.trn;
        SWCLK_TO_OUTPP();
        while (bits) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            bits--;
        }

        if (swd_control.data_force) {
            // Data:[C]*32
            SWCLK_TO_AFPP();
            SPI0->CR0 = SWD_SPI_CR0_DEFAULT | SPI_CR0_DSS_8BIT;
            SPI0->DR = 0xff;
            SPI0->DR = 0xff;
            SPI0->DR = 0xff;
            SPI0->DR = 0xff;
            bits = swd_control.idle;
            while (SPI0->SR & SPI_SR_BSY);
            buffer = SPI0->DR;
            buffer = SPI0->DR;
            buffer = SPI0->DR;
            buffer = SPI0->DR;
            
            // Parity:[C]*1
            SWCLK_TO_OUTPP();
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
        }

        SWDIO_MO_TO_IN();
        SWDIO_MI_TO_IN();
        if ((temp == SWD_ACK_WAIT) && (retry++ < swd_control.retry_limit))
            goto SYNC_READ_RESTART;
        else
            return temp;
    } else {
        // Data:[C]*32
        SWCLK_TO_AFPP();
        SPI0->CR0 = SWD_SPI_CR0_DEFAULT | SPI_CR0_DSS_8BIT;
        SPI0->DR = 0xff;
        SPI0->DR = 0xff;
        SPI0->DR = 0xff;
        SPI0->DR = 0xff;
        bits = swd_control.idle;
        while (SPI0->SR & SPI_SR_BSY);
        buffer = SPI0->DR;
        buffer = SPI0->DR;
        buffer = SPI0->DR;
        buffer = SPI0->DR;

        // Parity:[C]*1
        SWCLK_TO_OUTPP();
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        
        SWDIO_MO_TO_IN();
        SWDIO_MI_TO_IN();
        return temp;
    }
}

static void swd_read_io_quick(uint8_t *data, uint32_t bits)
{
    uint_fast32_t byte = 0, pos = 8 - bits;

    while (bits) {
        byte >>= 1;
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        byte |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) ? 0x80 : 0;
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        bits--;
    }
    *data = byte >> pos;
}

static void swd_read_io_slow(uint8_t *data, uint32_t bits)
{
    uint_fast32_t byte = 0, pos = 8 - bits;

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

static void swd_write_io_quick(uint8_t *data, uint32_t bits)
{
    uint_fast32_t byte = data[0];

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
