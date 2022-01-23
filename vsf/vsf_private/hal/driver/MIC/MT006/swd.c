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
}

void vsfhal_swd_fini(void)
{
    PERIPHERAL_GPIO_TMS_FINI();
    PERIPHERAL_GPIO_TCK_FINI();
    PERIPHERAL_GPIO_SRST_FINI();
}

void vsfhal_swd_io_reconfig(void)
{
    PERIPHERAL_GPIO_TMS_SET_OUTPUT();
    PERIPHERAL_GPIO_TMS_SET();
    
    PERIPHERAL_GPIO_TCK_SET_OUTPUT();
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

void vsfhal_swd_config(uint16_t kHz, uint16_t retry, uint8_t idle, uint8_t trn, bool data_force)
{
    uint32_t temp, apb;
    const vsfhal_clk_info_t *info = vsfhal_clk_info_get();

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
}

void vsfhal_swd_seqout(uint8_t *data, uint32_t bitlen)
{
    swd_control.swd_write_io(data, bitlen);
}

void vsfhal_swd_seqin(uint8_t *data, uint32_t bitlen)
{
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


#define SWD_RW_TCK_QUICK_TOGGLE2()      do {IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);\
                                            NOP();\
                                            NOP();\
                                            NOP();\
                                            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);} while (0)
static uint32_t swd_read_quick(uint32_t request, uint8_t *r_data)
{
    uint_fast32_t buffer, bits, temp, retry = 0;
    uint32_t buf;

    if (!r_data)
        r_data = (uint8_t *)&buf;

SYNC_READ_RESTART:
    temp = get_parity_4bit(request) << 5;
    buffer = ((request << 1) & 0x1e) | 0x81 | temp;

    // Request:[W]*8
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    
    if (buffer & (0x1 << 0))
        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    else
        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    SWD_RW_TCK_QUICK_TOGGLE2();
    if (buffer & (0x1 << 1))
        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    else
        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    SWD_RW_TCK_QUICK_TOGGLE2();
    if (buffer & (0x1 << 2))
        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    else
        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    SWD_RW_TCK_QUICK_TOGGLE2();
    if (buffer & (0x1 << 3))
        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    else
        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    SWD_RW_TCK_QUICK_TOGGLE2();
    if (buffer & (0x1 << 4))
        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    else
        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    SWD_RW_TCK_QUICK_TOGGLE2();
    if (buffer & (0x1 << 5))
        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    else
        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    SWD_RW_TCK_QUICK_TOGGLE2();
    if (buffer & (0x1 << 6))
        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    else
        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    NOP();
    NOP();
    NOP();
    SWD_RW_TCK_QUICK_TOGGLE2();
    if (buffer & (0x1 << 7))
        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    else
        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    NOP();
    NOP();
    NOP();
    SWD_RW_TCK_QUICK_TOGGLE2();

    // TRN:[C]*trn
    IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    bits = swd_control.trn;
    while (bits--) {
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        NOP();
        NOP();
        NOP();
        NOP();
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        NOP();
        NOP();
    }
    NOP();
    
    // ACK:[R]*3
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    NOP();
    NOP();
    temp = IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    NOP();
    NOP();
    NOP();
    NOP();
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    //if (swd_control.swd_delay)
    //    swd_control.swd_delay(swd_control.delay_tick);
    temp |= IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN) << 1;
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    NOP();
    NOP();
    NOP();
    NOP();
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    //if (swd_control.swd_delay)
    //    swd_control.swd_delay(swd_control.delay_tick);
    temp |= IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN) << 2;
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    //if (swd_control.swd_delay)
    //    swd_control.swd_delay(swd_control.delay_tick);

    if (temp == SWD_ACK_OK) {
        // Data:[R]*32
        for (temp = 0; temp < 4; temp++) {
            bits = 8;
            while (bits--) {
                buffer >>= 1;
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                //if (swd_control.swd_delay)
                //    swd_control.swd_delay(swd_control.delay_tick);
                buffer |= IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN) ? 0x80 : 0;
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                //if (swd_control.swd_delay)
                //    swd_control.swd_delay(swd_control.delay_tick);
                NOP();
                NOP();
            }
            r_data[temp] = buffer;
        }

        // Parity:[R]*1
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        //if (swd_control.swd_delay)
        //    swd_control.swd_delay(swd_control.delay_tick);
        temp = IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        //if (swd_control.swd_delay)
        //    swd_control.swd_delay(swd_control.delay_tick);

        bits = swd_control.trn + swd_control.idle;
        
        // Trn:[C]*trn --> Idle:[C]*idle
        while (bits--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            NOP();
            NOP();
            NOP();
            NOP();
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            NOP();
            NOP();
        }
        NOP();

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
            bits = 32;
            while (bits--) {
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                NOP();
                NOP();
                NOP();
                NOP();
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                NOP();
                NOP();
            }
            NOP();
            
            // Parity:[C]*1 -> Trn:[C]*trn
            bits = 1 + swd_control.trn;
            while (bits--) {
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                NOP();
                NOP();
                NOP();
                NOP();
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                NOP();
                NOP();
            }
            NOP();
        } else {
            // Trn:[C]*trn
            bits = swd_control.trn;
            while (bits--) {
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                NOP();
                NOP();
                NOP();
                NOP();
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                NOP();
                NOP();
            }
            NOP();
        }

        if ((temp == SWD_ACK_WAIT) && (retry++ < swd_control.retry_limit))
            goto SYNC_READ_RESTART;
        else
            return temp;
    } else {
        // Data:[C]*32
        bits = 32;
        while (bits--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            NOP();
            NOP();
            NOP();
            NOP();
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            NOP();
            NOP();
        }
        NOP();

        // Parity:[C]*1
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        NOP();
        NOP();
        NOP();
        NOP();
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    }
    return temp;
}

static uint32_t swd_read_slow(uint32_t request, uint8_t *r_data)
{
    uint_fast32_t buffer, bits, temp, retry = 0;
    uint32_t buf;

    if (!r_data)
        r_data = (uint8_t *)&buf;

SYNC_READ_RESTART:
    temp = get_parity_4bit(request) << 5;
    buffer = ((request << 1) & 0x1e) | 0x81 | temp;

    // Request:[W]*8
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    temp = 8;
    while (temp) {
        if (buffer & 0x1)
            IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        else
            IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        buffer >>= 1;
        temp--;
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
    }

    // TRN:[C]*trn
    IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    bits = swd_control.trn;
    while (bits--) {
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
    }
    
    // ACK:[R]*3
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    NOP();
    NOP();
    NOP();
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    temp = IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    NOP();
    NOP();
    NOP();
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    temp |= IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN) << 1;
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    NOP();
    NOP();
    NOP();
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    temp |= IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN) << 2;
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);

    if (temp == SWD_ACK_OK) {
        // Data:[R]*32
        for (temp = 0; temp < 4; temp++) {
            bits = 8;
            while (bits--) {
                buffer >>= 1;
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
                buffer |= IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN) ? 0x80 : 0;
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
            }
            r_data[temp] = buffer;
        }

        // Parity:[R]*1
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        temp = IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
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
        if (swd_control.data_force) {
            // Data:[C]*32
            bits = 32;
            while (bits--) {
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
            }

            // Parity:[C]*1 -> Trn:[C]*trn
            bits = 1 + swd_control.trn;
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
        bits = 32;
        while (bits--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
        }

        // Parity:[C]*1
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
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
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    // Request:[W]*8
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    
    if (buffer & (0x1 << 0))
        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    else
        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    SWD_RW_TCK_QUICK_TOGGLE2();
    if (buffer & (0x1 << 1))
        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    else
        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    SWD_RW_TCK_QUICK_TOGGLE2();
    if (buffer & (0x1 << 2))
        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    else
        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    SWD_RW_TCK_QUICK_TOGGLE2();
    if (buffer & (0x1 << 3))
        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    else
        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    SWD_RW_TCK_QUICK_TOGGLE2();
    if (buffer & (0x1 << 4))
        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    else
        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    SWD_RW_TCK_QUICK_TOGGLE2();
    if (buffer & (0x1 << 5))
        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    else
        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    SWD_RW_TCK_QUICK_TOGGLE2();
    if (buffer & (0x1 << 6))
        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    else
        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    NOP();
    NOP();
    NOP();
    SWD_RW_TCK_QUICK_TOGGLE2();
    if (buffer & (0x1 << 7))
        IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    else
        IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    NOP();
    NOP();
    NOP();
    SWD_RW_TCK_QUICK_TOGGLE2();

    // TRN:[C]*trn
    IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    bits = swd_control.trn;
    while (bits--) {
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        NOP();
        NOP();
        NOP();
        NOP();
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        NOP();
        NOP();
    }
    NOP();

    // ACK:[R]*3
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    NOP();
    NOP();
    temp = IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    NOP();
    NOP();
    NOP();
    NOP();
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    //if (swd_control.swd_delay)
    //    swd_control.swd_delay(swd_control.delay_tick);
    temp |= IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN) << 1;
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    NOP();
    NOP();
    NOP();
    NOP();
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    //if (swd_control.swd_delay)
    //    swd_control.swd_delay(swd_control.delay_tick);
    temp |= IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN) << 2;
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    //if (swd_control.swd_delay)
    //    swd_control.swd_delay(swd_control.delay_tick);

    if (temp == SWD_ACK_OK) {
        // TRN:[C]*trn
        bits = swd_control.trn;
        while (bits--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            NOP();
            NOP();
            NOP();
            NOP();
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            NOP();
            NOP();
        }
        NOP();

        // Data:[W]*32
        IO_CFG_OUTPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        for (temp = 0; temp < 4; temp++) {
            buffer = w_data[temp];
            bits = 8;
            while (bits) {
                if (buffer & 0x1)
                    IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                else
                    IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                buffer >>= 1;
                bits--;
                NOP();
                NOP();
                NOP();
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                //if (swd_control.swd_delay)
                //    swd_control.swd_delay(swd_control.delay_tick);
            }
        }

        // Parity:[W]*1
        temp = get_parity_32bit(get_unaligned_le32(w_data));
        if (temp)
            IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        else
            IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        NOP();
        NOP();
        NOP();
        NOP();
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        //if (swd_control.swd_delay)
        //    swd_control.swd_delay(swd_control.delay_tick);

        // Idle:[C]*idle
        bits = swd_control.idle;
        while (bits--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            NOP();
            NOP();
            NOP();
            NOP();
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            NOP();
            NOP();
        }
        NOP();

        #if TIMESTAMP_CLOCK
        if (request & SWD_TRANS_TIMESTAMP)
            swd_control.dap_timestamp = vsfhal_timestamp_get();
        #endif

        return SWD_ACK_OK | SWD_SUCCESS;
    } else if ((temp == SWD_ACK_WAIT) || (temp == SWD_ACK_FAULT)) {
        // TRN:[C]*trn
        bits = swd_control.trn;
        while (bits--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            NOP();
            NOP();
            NOP();
            NOP();
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            NOP();
            NOP();
        }
        NOP();

        if (swd_control.data_force) {
            // Data:[C]*32
            bits = 32;
            while (bits--) {
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                NOP();
                NOP();
                NOP();
                NOP();
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                NOP();
                NOP();
            }
            NOP();
            
            // Parity:[C]*1
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            NOP();
            NOP();
            NOP();
            NOP();
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
        bits = 32;
        while (bits--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            NOP();
            NOP();
            NOP();
            NOP();
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            NOP();
            NOP();
        }
        NOP();

        // Parity:[C]*1
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        NOP();
        NOP();
        NOP();
        NOP();
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);

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
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    temp = 8;
    while (temp) {
        if (buffer & 0x1)
            IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        else
            IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        buffer >>= 1;
        temp--;
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
    }

    // TRN:[C]*trn
    IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    bits = swd_control.trn;
    while (bits--) {
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
    }

    // ACK:[R]*3
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    NOP();
    NOP();
    NOP();
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    temp = IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    NOP();
    NOP();
    NOP();
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    temp |= IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN) << 1;
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    NOP();
    NOP();
    NOP();
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);
    temp |= IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN) << 2;
    IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    if (swd_control.swd_delay)
        swd_control.swd_delay(swd_control.delay_tick);

    if (temp == SWD_ACK_OK) {
        // TRN:[C]*trn
        bits = swd_control.trn;
        while (bits--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
        }

        // Data:[W]*32
        IO_CFG_OUTPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        for (temp = 0; temp < 4; temp++) {
            buffer = w_data[temp];
            bits = 8;
            while (bits) {
                if (buffer & 0x1)
                    IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                else
                    IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                buffer >>= 1;
                bits--;
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
            }
        }

        // Parity:[W]*1
        temp = get_parity_32bit(get_unaligned_le32(w_data));
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
        bits = swd_control.idle;
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

        return SWD_ACK_OK | SWD_SUCCESS;
    } else if ((temp == SWD_ACK_WAIT) || (temp == SWD_ACK_FAULT)) {
        // TRN:[C]*trn
        bits = swd_control.trn;
        while (bits--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
        }

        if (swd_control.data_force) {
            // Data:[C]*32
            bits = 32;
            while (bits--) {
                IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
                IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
                if (swd_control.swd_delay)
                    swd_control.swd_delay(swd_control.delay_tick);
            }
            
            // Parity:[C]*1
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
        bits = 32;
        while (bits--) {
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
        }

        // Parity:[C]*1
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);

        return temp;
    }
}

static void swd_read_io_quick(uint8_t *data, uint32_t bits)
{
    uint_fast32_t buf, temp;

    while (bits >= 8) {
        temp = 8;
        bits -= temp;
        while (temp--) {
            buf >>= 1;
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            buf |= IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN) ? 0x80 : 0;
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        }
        *data++ = buf;
    }
    
    if (bits) {
        temp = 8 - bits;
        do {
            buf >>= 1;
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            buf |= IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN) ? 0x80 : 0;
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        } while (--bits);
        *data = buf >> temp;
    }
}

static void swd_read_io_slow(uint8_t *data, uint32_t bits)
{
    uint_fast32_t buf, temp;
    
    while (bits >= 8) {
        temp = 8;
        bits -= temp;
        while (temp--) {
            buf >>= 1;
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            buf |= IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN) ? 0x80 : 0;
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
        }
        *data++ = buf;
    }
    
    if (bits) {
        temp = 8 - bits;
        do {
            buf >>= 1;
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            buf |= IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN) ? 0x80 : 0;
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
        } while (--bits);
        *data = buf >> temp;
    }
}

static void swd_write_io_quick(uint8_t *data, uint32_t bits)
{
    uint_fast32_t buf, temp;

    while (bits >= 8) {
        buf = *data++;
        temp = 8;
        bits -= temp;
        while (temp) {
            if (buf & 0x1)
                IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            else
                IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            buf >>= 1;
            temp--;
            NOP();
            NOP();
            NOP();
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        }
    }
    
    buf = *data++;
    while (bits) {
        if (buf & 0x1)
            IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        else
            IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        buf >>= 1;
        bits--;
        NOP();
        NOP();
        NOP();
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
    }
}

static void swd_write_io_slow(uint8_t *data, uint32_t bits)
{
    uint_fast32_t buf, temp;

    while (bits >= 8) {
        buf = *data++;
        temp = 8;
        bits -= temp;
        while (temp) {
            if (buf & 0x1)
                IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            else
                IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            buf >>= 1;
            temp--;
            IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
        }
    }
    
    buf = *data++;
    while (bits) {
        if (buf & 0x1)
            IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        else
            IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        IO_CLEAR(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
        buf >>= 1;
        bits--;
        IO_SET(PERIPHERAL_GPIO_TCK_SWD_IDX, PERIPHERAL_GPIO_TCK_SWD_PIN);
        if (swd_control.swd_delay)
            swd_control.swd_delay(swd_control.delay_tick);
    }
}

#if TIMESTAMP_CLOCK
uint32_t vsfhal_swd_get_timestamp(void)
{
    return swd_control.dap_timestamp;
}
#endif

#endif
