/*
Reference Document:
	"Serial Wire Debug and the CoreSightTM Debug and Trace Architecture"
	https://github.com/ARMmbed/DAPLink/blob/master/source/daplink/cmsis-dap/SW_DP.c
*/

/*============================ INCLUDES ======================================*/

#include "vsf.h"
#include "swd.h"
#include "./common/io.h"

/*============================ MACROS ========================================*/

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

#define IO_AF_SELECT(port, pin, afsel)	do {REG32((GPIOA_BASE + 0x400 * port) + 0x20U + ((pin & 0x8) >> 1)) &= ~(0xful << ((pin & 0x7) * 4));\
                                            REG32((GPIOA_BASE + 0x400 * port) + 0x20U + ((pin & 0x8) >> 1)) |= (uint32_t)afsel << ((pin & 0x7) * 4);} while(0)
#define IO_CFG_INPUT(idx, pin)		(GPIO_CTL(GPIOA_BASE + 0x400 * idx) &= ~(0x3 << (pin * 2)))
#define IO_CFG_OUTPUT(idx, pin)	    (GPIO_CTL(GPIOA_BASE + 0x400 * idx) = (GPIO_CTL(GPIOA_BASE + 0x400 * idx) & ~(0x3ul << (pin * 2))) | (0x1ul << (pin * 2)))
#define IO_CFG_AF(idx, pin)		    (GPIO_CTL(GPIOA_BASE + 0x400 * idx) = (GPIO_CTL(GPIOA_BASE + 0x400 * idx) & ~(0x3ul << (pin * 2))) | (0x2ul << (pin * 2)))
#define IO_SET(idx, pin)			(GPIO_OCTL(GPIOA_BASE + 0x400 * idx) |= 0x1 << pin)
#define IO_CLEAR(idx, pin)			(GPIO_BC(GPIOA_BASE + 0x400 * idx) = 0x1 << pin)
#define IO_GET(idx, pin)			((GPIO_ISTAT(GPIOA_BASE + 0x400 * idx) >> pin) & 0x1)
#define IO_CFG_HIGHSPEED(idx, pin)	(GPIO_OSPD0(GPIOA_BASE + 0x400 * idx) |= 0x3 << (pin * 2))

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef struct swd_control_t {
    void (*swd_delay)(uint16_t delay_tick);
    void (*swd_read)(uint8_t* data, uint32_t bits);
    void (*swd_write)(uint8_t* data, uint32_t bits);

    uint16_t delay_tick;

    bool data_force;
    uint8_t idle;
    uint8_t trn;
    uint16_t retry;
} swd_control_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

#if TIMESTAMP_CLOCK
extern uint32_t dap_timestamp;
#endif

/*============================ IMPLEMENTATION ================================*/

#if SWD_COUNT > 0

static swd_control_t swd_control;

// TODO: fix optimization
static void swd_delay_125ns(uint16_t dummy)
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

// TODO: fix optimization
static void swd_delay_250ns(uint16_t dummy)
{
    dummy = 9;
    while (--dummy) {
    }
}


static void swd_delay_ticks(uint16_t delay_tick)
{
    vsf_systimer_cnt_t ticks = vsf_systimer_get() + delay_tick;
    while (ticks >= vsf_systimer_get()) {
    }
}

static uint32_t get_parity_4bit(uint8_t data)
{
    uint8_t temp;
    temp = data >> 2;
    data = data ^ temp;
    temp = data >> 1;
    data = data ^ temp;
    return data & 0x1;
}

static uint32_t get_parity_32bit(uint32_t data)
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

static void swd_read_quick(uint8_t* data, uint32_t bits)
{
    uint8_t end, m;

    if (bits >= 8) {
        IO_CFG_AF(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN);
        IO_CFG_AF(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);

        if (bits >= 16) {
            SPI_CTL0(SWD_SPI_BASE) &= ~SPI_CTL0_SPIEN;
            SPI_CTL0(SWD_SPI_BASE) |= SPI_CTL0_FF16;
            SPI_CTL0(SWD_SPI_BASE) |= SPI_CTL0_SPIEN;
            do {
                SPI_DATA(SWD_SPI_BASE) = 0;
                while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS)
                    ;
                *(uint16_t*)data = SPI_DATA(SWD_SPI_BASE);
                data += 2;
                bits -= 16;
            } while (bits >= 16);
            SPI_CTL0(SWD_SPI_BASE) &= ~SPI_CTL0_FF16;
        }
        while (bits >= 8) {
            SPI_DATA(SWD_SPI_BASE) = 0;
            while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS)
                ;
            *data = SPI_DATA(SWD_SPI_BASE);
            data += 1;
            bits -= 8;
        }

        IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN);
        IO_CFG_OUTPUT(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
    }

    if (bits) {
        m = 8 - bits;
        end = 0;
        do {
            end >>= 1;
            IO_CLEAR(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
            end |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) ? 0x80 : 0;
            IO_SET(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
            __ASM("NOP");
            bits--;
        } while (bits);
        *data = end >> m;
    }
}

static void swd_read_slow(uint8_t* data, uint32_t bits)
{
    uint16_t end, m;

    if (bits >= 8) {
        IO_CFG_AF(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN);
        IO_CFG_AF(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);

        if (bits >= 16) {
            SPI_CTL0(SWD_SPI_BASE) &= ~SPI_CTL0_SPIEN;
            SPI_CTL0(SWD_SPI_BASE) |= SPI_CTL0_FF16;
            SPI_CTL0(SWD_SPI_BASE) |= SPI_CTL0_SPIEN;
            do {
                SPI_DATA(SWD_SPI_BASE) = 0;
                while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS)
                    ;
                *(uint16_t*)data = SPI_DATA(SWD_SPI_BASE);
                data += 2;
                bits -= 16;
            } while (bits >= 16);
            SPI_CTL0(SWD_SPI_BASE) &= ~SPI_CTL0_FF16;
        }
        while (bits >= 8) {
            SPI_DATA(SWD_SPI_BASE) = 0;
            while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS)
                ;
            *data = SPI_DATA(SWD_SPI_BASE);
            data += 1;
            bits -= 8;
        }

        IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN);
        IO_CFG_OUTPUT(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
    }

    if (bits) {
        m = 8 - bits;
        end = 0;
        do {
            end >>= 1;
            IO_CLEAR(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            end |= IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN) ? 0x80 : 0;
            IO_SET(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            bits--;
        } while (bits);
        *data = end >> m;
    }
}

static void swd_write_quick(uint8_t* data, uint32_t bits)
{
    uint16_t end;

    if (bits >= 8) {
        IO_CFG_AF(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        IO_CFG_AF(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);

        if (bits >= 16) {
            SPI_CTL0(SWD_SPI_BASE) &= ~SPI_CTL0_SPIEN;
            SPI_CTL0(SWD_SPI_BASE) |= SPI_CTL0_FF16;
            SPI_CTL0(SWD_SPI_BASE) |= SPI_CTL0_SPIEN;
            do {
                SPI_DATA(SWD_SPI_BASE) = *(uint16_t*)data;
                data += 2;
                bits -= 16;
                while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS)
                    ;
                end = SPI_DATA(SWD_SPI_BASE);
            } while (bits >= 16);
            SPI_CTL0(SWD_SPI_BASE) &= ~SPI_CTL0_FF16;
        }
        while (bits >= 8) {
            SPI_DATA(SWD_SPI_BASE) = *data;
            data += 1;
            bits -= 8;
            while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS)
                ;
            end = SPI_DATA(SWD_SPI_BASE);
        }

        if (IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN))
            IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        else
            IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        IO_CFG_OUTPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        IO_CFG_OUTPUT(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
    }

    if (bits) {
        end = *data;
        do {
            if (end & 0x1)
                IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            else
                IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            IO_CLEAR(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
            end >>= 1;
            bits--;
            IO_SET(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
        } while (bits);
    }

    IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
}

static void swd_write_slow(uint8_t* data, uint32_t bits)
{
    uint16_t end;

    if (bits >= 8) {
        IO_CFG_AF(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        IO_CFG_AF(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);

        if (bits >= 16) {
            SPI_CTL0(SWD_SPI_BASE) &= ~SPI_CTL0_SPIEN;
            SPI_CTL0(SWD_SPI_BASE) |= SPI_CTL0_FF16;
            SPI_CTL0(SWD_SPI_BASE) |= SPI_CTL0_SPIEN;
            do {
                SPI_DATA(SWD_SPI_BASE) = *(uint16_t*)data;
                data += 2;
                bits -= 16;
                while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS)
                    ;
                end = SPI_DATA(SWD_SPI_BASE);
            } while (bits >= 16);
            SPI_CTL0(SWD_SPI_BASE) &= ~SPI_CTL0_FF16;
        }
        while (bits >= 8) {
            SPI_DATA(SWD_SPI_BASE) = *data;
            data += 1;
            bits -= 8;
            while (SPI_STAT(SWD_SPI_BASE) & SPI_STAT_TRANS)
                ;
            end = SPI_DATA(SWD_SPI_BASE);
        }

        if (IO_GET(PERIPHERAL_GPIO_TMS_MI_IDX, PERIPHERAL_GPIO_TMS_MI_PIN))
            IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        else
            IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        IO_CFG_OUTPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
        IO_CFG_OUTPUT(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
    }

    if (bits) {
        end = *data;
        do {
            if (end & 0x1)
                IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            else
                IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            IO_CLEAR(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
            end >>= 1;
            bits--;
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
            IO_SET(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
            if (swd_control.swd_delay)
                swd_control.swd_delay(swd_control.delay_tick);
        } while (bits);
    }

    IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
}

void vsfhal_swd_init(int32_t int_priority)
{
    PERIPHERAL_GPIO_TMS_INIT();
    PERIPHERAL_GPIO_TCK_INIT();
    PERIPHERAL_GPIO_SRST_INIT();
    PERIPHERAL_SWD_IO_AF_CONFIG();

    IO_CFG_HIGHSPEED(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    IO_CFG_HIGHSPEED(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);

    IO_SET(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
}

void vsfhal_swd_fini(void)
{
    PERIPHERAL_GPIO_TMS_FINI();
    PERIPHERAL_GPIO_TCK_FINI();
    PERIPHERAL_GPIO_SRST_FINI();
}

/*
    DIV     CLK:64M     CLK:48M
    2       32000       24000
    4       16000       12000
    8       8000        6000
    16      4000        3000
    32      2000        1500
    64      1000        750
    128     500         375
    256     250         187.5
*/
const static uint16_t spi_clk_list[] = {
    // 64M	PSC
    32000, // 2
    16000, // 4
    8000, // 8
    4000, // 16
    2000, // 32
    1000, // 64
    500, // 128
    250, // 256
};
void vsfhal_swd_config(uint16_t kHz, uint16_t retry, uint8_t idle, uint8_t trn, bool data_force)
{
    uint32_t temp;
    struct vsfhal_clk_info_t *info = vsfhal_clk_info_get();

    if (kHz < spi_clk_list[dimof(spi_clk_list) - 1])
        kHz = spi_clk_list[dimof(spi_clk_list) - 1];
    for (temp = 0; temp < dimof(spi_clk_list); temp++) {
        if (kHz >= spi_clk_list[temp]) {
            kHz = spi_clk_list[temp];
            break;
        }
    }
    swd_control.delay_tick = info->ahb_freq_hz / (kHz * 2000);

    if (kHz >= 8000) {
        swd_control.swd_read = swd_read_quick;
        swd_control.swd_write = swd_write_quick;
        swd_control.swd_delay = NULL;
    } else if (kHz >= 4000) {
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_delay = NULL;
    } else if (kHz >= 2000) {
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_delay = swd_delay_125ns;
    } else if (kHz >= 1000) {
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_delay = swd_delay_250ns;
    } else {
        swd_control.swd_read = swd_read_slow;
        swd_control.swd_write = swd_write_slow;
        swd_control.swd_delay = swd_delay_ticks;
    }

    RCU_APB2EN |= RCU_APB2EN_SPI0EN;
    RCU_APB2RST |= RCU_APB2RST_SPI0RST;
    RCU_APB2RST &= ~RCU_APB2RST_SPI0RST;

    if (temp < dimof(spi_clk_list)) {
        SPI_CTL0(SWD_SPI_BASE) &= ~SPI_CTL0_PSC;
        SPI_CTL0(SWD_SPI_BASE) |= temp << 3;
    }
    SPI_CTL0(SWD_SPI_BASE) &= ~(SPI_CTL0_BDEN | SPI_CTL0_CRCEN | SPI_CTL0_FF16);
    SPI_CTL0(SWD_SPI_BASE) |= SPI_CTL0_MSTMOD | SPI_CTL0_SWNSSEN | SPI_CTL0_SWNSS | SPI_CTL0_LF | SPI_CTL0_CKPL | SPI_CTL0_CKPH;
    SPI_CTL0(SWD_SPI_BASE) |= SPI_CTL0_SPIEN;

    if (idle <= (32 * 6))
        swd_control.idle = idle;
    else
        swd_control.idle = 32 * 6;
    if (trn <= 14)
        swd_control.trn = trn;
    else
        swd_control.trn = 14;
    swd_control.data_force = data_force;
    swd_control.retry = retry;
}

void vsfhal_swd_seqout(uint8_t* data, uint32_t bitlen)
{
    swd_control.swd_write(data, bitlen);
}

void vsfhal_swd_seqin(uint8_t* data, uint32_t bitlen)
{
    swd_control.swd_read(data, bitlen);
}

int vsfhal_swd_read(uint32_t request, uint8_t* r_data)
{
    uint16_t retry;
    uint32_t ret[2], ack;

    retry = 0;
    request <<= 1;
    request = (request & 0x1e) | 0x81 | (get_parity_4bit(request >> 1) << 5);
    do {
        swd_control.swd_write(&request, 8);
        swd_control.swd_read((uint8_t*)&ack, swd_control.trn + 3);
        ack = (ack >> swd_control.trn) & 0x7;
        if (ack == SWD_ACK_OK) {
            swd_control.swd_read((uint8_t*)&ret, 32 + 1 + swd_control.trn + swd_control.idle);
#if TIMESTAMP_CLOCK
            if (request & SWD_TRANS_TIMESTAMP)
                dap_timestamp = vsf_systimer_get_us();
#endif
            if ((ret[1] & 0x1) == get_parity_32bit(ret[0])) {
                *r_data = ret[0];
                return SWD_ACK_OK | SWD_SUCCESS;
            } else {
                *r_data = ret[0];
                return SWD_ACK_OK | SWD_PARITY_ERROR;
            }
        } else if ((ack == SWD_ACK_WAIT) || (ack == SWD_ACK_FAULT)) {
            if (swd_control.data_force)
                swd_control.swd_read(NULL, 32 + 1 + swd_control.trn + swd_control.idle);
            else
                swd_control.swd_read(NULL, swd_control.trn);

            if (ack != SWD_ACK_WAIT)
                break;
        } else {
            swd_control.swd_read(NULL, 32 + 1 + swd_control.trn);
            break;
        }
    } while (retry++ < swd_control.retry);

    *r_data = 0;
    return ack;
}

int vsfhal_swd_write(uint8_t request, uint8_t *w_data)
{
    uint16_t retry;
    uint32_t ret[2], ack;

    retry = 0;
    request <<= 1;
    request = (request & 0x1e) | 0x81 | (get_parity_4bit(request >> 1) << 5);
    do {
        swd_control.swd_write(&request, 8);
        swd_control.swd_read((uint8_t*)&ack, swd_control.trn * 2 + 3);
        ack = (ack >> swd_control.trn) & 0x7;
        if (ack == SWD_ACK_OK) {
            ret[0] = w_data;
            ret[1] = 0xfffffffe | get_parity_32bit(w_data);
            swd_control.swd_write((uint8_t*)&ret, 32 + 1 + swd_control.idle);
#if TIMESTAMP_CLOCK
            if (request & SWD_TRANS_TIMESTAMP)
                dap_timestamp = vsf_systimer_get_us();
#endif
            return SWD_ACK_OK | SWD_SUCCESS;
        } else if ((ack == SWD_ACK_WAIT) || (ack == SWD_ACK_FAULT)) {
            if (swd_control.data_force) {
                ret[0] = 0;
                ret[1] = 0;
                swd_control.swd_write((uint8_t*)&ret, 32 + 1);
            }

            if (ack != SWD_ACK_WAIT)
                break;
        } else {
            // Back off data phase
            swd_control.swd_read(NULL, 32 + 1 + swd_control.trn);
            break;
        }
    } while (retry++ < swd_control.retry);

    return ack;
}

#endif
