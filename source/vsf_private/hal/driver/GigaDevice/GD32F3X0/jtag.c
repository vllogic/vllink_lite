/*
Reference Document:
	"IEEE_1149_JTAG_and_Boundary_Scan_Tutorial"
	https://github.com/ARMmbed/DAPLink/blob/master/source/daplink/cmsis-dap/JTAG_DP.c
*/

/*============================ INCLUDES ======================================*/

#include "jtag.h"
#include "./common/io.h"
#include "vsf.h"

/*============================ MACROS ========================================*/

#define DAP_TRANSFER_RnW (1U << 1)
#define DAP_TRANSFER_TIMESTAMP (1U << 7)
#define DAP_TRANSFER_OK (1U << 0)
#define DAP_TRANSFER_WAIT (1U << 1)

#define IO_AF_SELECT(port, pin, afsel)                                                                           \
    do {                                                                                                         \
        REG32((GPIOA_BASE + 0x400 * port) + 0x20U + ((pin & 0x8) >> 1)) &= ~(0xful << ((pin & 0x7) * 4));        \
        REG32((GPIOA_BASE + 0x400 * port) + 0x20U + ((pin & 0x8) >> 1)) |= (uint32_t)afsel << ((pin & 0x7) * 4); \
    } while (0)
#define IO_CFG_INPUT(idx, pin) (GPIO_CTL(GPIOA_BASE + 0x400 * idx) &= ~(0x3 << (pin * 2)))
#define IO_CFG_OUTPUT(idx, pin) (GPIO_CTL(GPIOA_BASE + 0x400 * idx) = (GPIO_CTL(GPIOA_BASE + 0x400 * idx) & ~(0x3ul << (pin * 2))) | (0x1ul << (pin * 2)))
#define IO_CFG_AF(idx, pin) (GPIO_CTL(GPIOA_BASE + 0x400 * idx) = (GPIO_CTL(GPIOA_BASE + 0x400 * idx) & ~(0x3ul << (pin * 2))) | (0x2ul << (pin * 2)))
#define IO_SET(idx, pin) (GPIO_OCTL(GPIOA_BASE + 0x400 * idx) |= 0x1 << pin)
#define IO_CLEAR(idx, pin) (GPIO_BC(GPIOA_BASE + 0x400 * idx) = 0x1 << pin)
#define IO_GET(idx, pin) ((GPIO_ISTAT(GPIOA_BASE + 0x400 * idx) >> pin) & 0x1)
#define IO_CFG_HIGHSPEED(idx, pin) (GPIO_OSPD0(GPIOA_BASE + 0x400 * idx) |= 0x3 << (pin * 2))

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef struct jtag_control_t {
    void (*jtag_delay)(uint16_t delay_tick);
    void (*jtag_rw)(uint16_t bits, uint8_t* tdi, uint8_t* tms, uint8_t* tdo);

    uint16_t delay_tick;

    bool is_spi;
    uint8_t idle;
    uint16_t retry;
} jtag_control_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

#if TIMESTAMP_CLOCK
extern uint32_t dap_timestamp;
#endif

/*============================ IMPLEMENTATION ================================*/

#if JTAG_COUNT > 0

static jtag_control_t jtag_control;

void delay_jtag_125ns(uint16_t dummy)
{
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
    __ASM("NOP");
}

void delay_jtag_250ns(uint16_t dummy)
{
    dummy = 7;
    while (--dummy)
        ;
}

static void jtag_delay_ticks(uint16_t delay_tick)
{
    vsf_systimer_cnt_t ticks = vsf_systimer_get() + delay_tick;
    while (ticks >= vsf_systimer_get()) {
    }
}

static inline void jtag_set_spi_mode(void)
{
    IO_CFG_AF(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
    IO_CFG_AF(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
    IO_CFG_AF(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN);
    jtag_control.is_spi = true;
}

static inline void jtag_set_io_mode(void)
{
    if (IO_GET(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN))
        IO_SET(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
    else
        IO_CLEAR(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
    IO_CFG_INPUT(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN);
    jtag_control.is_spi = false;
}

static void jtag_rw_quick(uint16_t bits, uint8_t* tdi, uint8_t* tms, uint8_t* tdo)
{
    uint8_t bits_io, temp, tdi_last, tms_last, tdo_last;

    while (bits >= 8) {
        if (*tms == 0) {
            if (IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN))
                IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);

            if (!jtag_control.is_spi)
                jtag_set_spi_mode();

            SPI_DATA(JTAG_SPI_BASE) = *tdi;
            tdi += 1;
            tms += 1;
            bits -= 8;
            while (SPI_STAT(JTAG_SPI_BASE) & SPI_STAT_TRANS)
                ;
            *tdo = SPI_DATA(JTAG_SPI_BASE);
            tdo += 1;
        } else if (*tms == 0xff) {
            if (IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN) == 0)
                IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);

            if (!jtag_control.is_spi)
                jtag_set_spi_mode();

            SPI_DATA(JTAG_SPI_BASE) = *tdi;
            tdi += 1;
            tms += 1;
            bits -= 8;
            while (SPI_STAT(JTAG_SPI_BASE) & SPI_STAT_TRANS)
                ;
            *tdo = SPI_DATA(JTAG_SPI_BASE);
            tdo += 1;
        } else {
            if (jtag_control.is_spi)
                jtag_set_io_mode();

            tdi_last = *tdi++;
            tms_last = *tms++;
            tdo_last = 0;
            bits -= 8;
            bits_io = 8;
            do {
                tdo_last >>= 1;
                if (tdi_last & 0x1)
                    IO_SET(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
                else
                    IO_CLEAR(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
                if (tms_last & 0x1)
                    IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                else
                    IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                IO_CLEAR(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
                tms_last >>= 1;
                tdi_last >>= 1;
                bits_io--;
                __ASM("NOP");
                __ASM("NOP");
                __ASM("NOP");
                __ASM("NOP");
                IO_SET(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
                tdo_last |= IO_GET(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN) ? 0x80 : 0;
            } while (bits_io);
            *tdo++ = tdo_last;
        }
    }

    if (bits) {
        if (jtag_control.is_spi)
            jtag_set_io_mode();

        tdi_last = *tdi;
        tms_last = *tms;
        tdo_last = 0;
        temp = 8 - bits;
        do {
            tdo_last >>= 1;
            if (tdi_last & 0x1)
                IO_SET(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
            else
                IO_CLEAR(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
            if (tms_last & 0x1)
                IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            else
                IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            IO_CLEAR(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
            tdi_last >>= 1;
            tms_last >>= 1;
            bits--;
            IO_SET(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
            tdo_last |= IO_GET(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN) ? 0x80 : 0;
        } while (bits);
        *tdo = tdo_last >> temp;
    }
}

static void jtag_rw_slow(uint16_t bits, uint8_t* tdi, uint8_t* tms, uint8_t* tdo)
{
    uint8_t bits_io, temp, tdi_last, tms_last, tdo_last;

    while (bits >= 8) {
        if (*tms == 0) {
            if (IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN))
                IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);

            if (!jtag_control.is_spi)
                jtag_set_spi_mode();

            SPI_DATA(JTAG_SPI_BASE) = *tdi;
            tdi += 1;
            tms += 1;
            bits -= 8;
            while (SPI_STAT(JTAG_SPI_BASE) & SPI_STAT_TRANS)
                ;
            *tdo = SPI_DATA(JTAG_SPI_BASE);
            tdo += 1;
        } else if (*tms == 0xff) {
            if (IO_GET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN) == 0)
                IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);

            if (!jtag_control.is_spi)
                jtag_set_spi_mode();

            SPI_DATA(JTAG_SPI_BASE) = *tdi;
            tdi += 1;
            tms += 1;
            bits -= 8;
            while (SPI_STAT(JTAG_SPI_BASE) & SPI_STAT_TRANS)
                ;
            *tdo = SPI_DATA(JTAG_SPI_BASE);
            tdo += 1;
        } else {
            if (jtag_control.is_spi)
                jtag_set_io_mode();

            tdi_last = *tdi++;
            tms_last = *tms++;
            tdo_last = 0;
            bits -= 8;
            bits_io = 8;
            do {
                tdo_last >>= 1;
                if (tdi_last & 0x1)
                    IO_SET(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
                else
                    IO_CLEAR(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
                if (tms_last & 0x1)
                    IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                else
                    IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
                IO_CLEAR(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
                tdi_last >>= 1;
                tms_last >>= 1;
                bits_io--;
                if (jtag_control.jtag_delay)
                    jtag_control.jtag_delay(jtag_control.delay_tick);
                IO_SET(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
                tdo_last |= IO_GET(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN) ? 0x80 : 0;
                if (jtag_control.jtag_delay)
                    jtag_control.jtag_delay(jtag_control.delay_tick);
            } while (bits_io);
            *tdo++ = tdo_last;
        }
    }

    if (bits) {
        if (jtag_control.is_spi)
            jtag_set_io_mode();

        tdi_last = *tdi;
        tms_last = *tms;
        tdo_last = 0;
        temp = 8 - bits;
        do {
            tdo_last >>= 1;
            if (tdi_last & 0x1)
                IO_SET(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
            else
                IO_CLEAR(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
            if (tms_last & 0x1)
                IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            else
                IO_CLEAR(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
            IO_CLEAR(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay(jtag_control.delay_tick);
            tdi_last >>= 1;
            tms_last >>= 1;
            bits--;
            IO_SET(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
            tdo_last |= IO_GET(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN) ? 0x80 : 0;
            if (jtag_control.jtag_delay)
                jtag_control.jtag_delay(jtag_control.delay_tick);
        } while (bits);
        *tdo = tdo_last >> temp;
    }
}

void vsfhal_jtag_init(int32_t int_priority)
{
    PERIPHERAL_GPIO_TDI_INIT();
    PERIPHERAL_GPIO_TMS_INIT();
    PERIPHERAL_GPIO_TCK_INIT();
    PERIPHERAL_GPIO_TDO_INIT();
    PERIPHERAL_GPIO_SRST_INIT();
    PERIPHERAL_GPIO_TRST_INIT();
    PERIPHERAL_JTAG_IO_AF_CONFIG();

    IO_SET(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
    IO_CFG_HIGHSPEED(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);

    IO_SET(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    IO_CFG_HIGHSPEED(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);

    IO_SET(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
    IO_CFG_HIGHSPEED(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);

    IO_CFG_INPUT(PERIPHERAL_GPIO_TDO_MI_IDX, PERIPHERAL_GPIO_TDO_MI_PIN);

    IO_SET(PERIPHERAL_GPIO_SRST_IDX, PERIPHERAL_GPIO_SRST_PIN);
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_SRST_IDX, PERIPHERAL_GPIO_SRST_PIN);

    IO_SET(PERIPHERAL_GPIO_TRST_MO_IDX, PERIPHERAL_GPIO_TRST_MO_PIN);
    IO_CFG_OUTPUT(PERIPHERAL_GPIO_TRST_MO_IDX, PERIPHERAL_GPIO_TRST_MO_PIN);
    IO_CFG_HIGHSPEED(PERIPHERAL_GPIO_TRST_MO_IDX, PERIPHERAL_GPIO_TRST_MO_PIN);
}

void vsfhal_jtag_fini(void)
{
    IO_CFG_INPUT(PERIPHERAL_GPIO_TDI_IDX, PERIPHERAL_GPIO_TDI_PIN);
    IO_CFG_INPUT(PERIPHERAL_GPIO_TMS_MO_IDX, PERIPHERAL_GPIO_TMS_MO_PIN);
    IO_CFG_INPUT(PERIPHERAL_GPIO_TCK0_IDX, PERIPHERAL_GPIO_TCK0_PIN);
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

void vsfhal_jtag_config(uint16_t kHz, uint16_t retry)
{
    uint32_t temp;
    struct vsfhal_clk_info_t* info = vsfhal_clk_info_get();

    if (kHz < spi_clk_list[dimof(spi_clk_list) - 1])
        kHz = spi_clk_list[dimof(spi_clk_list) - 1];
    for (temp = 0; temp < dimof(spi_clk_list); temp++) {
        if (kHz >= spi_clk_list[temp]) {
            kHz = spi_clk_list[temp];
            break;
        }
    }
    jtag_control.delay_tick = info->ahb_freq_hz / (kHz * 2000);

    if (kHz >= 4000) {
        jtag_control.jtag_rw = jtag_rw_quick;
        jtag_control.jtag_delay = NULL;
    } else if (kHz >= 2000) {
        jtag_control.jtag_rw = jtag_rw_slow;
        jtag_control.jtag_delay = delay_jtag_125ns;
    } else if (kHz >= 1000) {
        jtag_control.jtag_rw = jtag_rw_slow;
        jtag_control.jtag_delay = delay_jtag_250ns;
    } else {
        jtag_control.jtag_rw = jtag_rw_slow;
        jtag_control.jtag_delay = jtag_delay_ticks;
    }

    RCU_APB2EN |= RCU_APB2EN_SPI0EN;
    RCU_APB2RST |= RCU_APB2RST_SPI0RST;
    RCU_APB2RST &= ~RCU_APB2RST_SPI0RST;

    if (temp < dimof(spi_clk_list)) {
        SPI_CTL0(JTAG_SPI_BASE) &= ~SPI_CTL0_PSC;
        SPI_CTL0(JTAG_SPI_BASE) |= temp << 3;
    }
    SPI_CTL0(JTAG_SPI_BASE) &= ~(SPI_CTL0_BDEN | SPI_CTL0_CRCEN | SPI_CTL0_FF16);
    SPI_CTL0(JTAG_SPI_BASE) |= SPI_CTL0_MSTMOD | SPI_CTL0_SWNSSEN | SPI_CTL0_SWNSS | SPI_CTL0_LF | SPI_CTL0_CKPL | SPI_CTL0_CKPH;
    SPI_CTL0(JTAG_SPI_BASE) |= SPI_CTL0_SPIEN;

    jtag_set_io_mode();

    jtag_control.retry = retry;
    jtag_control.idle = idle;
}

void vsfhal_jtag_raw(uint16_t bitlen, uint8_t* tms, uint8_t* tdi, uint8_t* tdo)
{
    jtag_control.jtag_rw(bitlen, tdi, tms, tdo);
}

void vsfhal_jtag_ir(uint32_t ir, uint8_t lr_length, uint16_t ir_before, uint16_t ir_after)
{
    uint16_t bitlen;
    uint64_t buf_tdi, buf_tms, buf_tdo;

    bitlen = 0;
    buf_tdi = 0;
    buf_tms = 0;
    lr_length--;

    // Select-DR-Scan, Select-IR-Scan, Capture-IR, Shift-IR
    buf_tms |= (uint64_t)0x3 << bitlen;
    bitlen += 4;
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
    bitlen += 1;
    // idle
    buf_tdi |= (uint64_t)0x1 << bitlen; // keep tdi high
    bitlen += 1;

    jtag_control.jtag_rw(bitlen, (uint8_t*)&buf_tdi, (uint8_t*)&buf_tms, (uint8_t*)&buf_tdo);
}

/*
Read:	vsfhal_jtag_dr(request, 0, dr_before, dr_after, *read_buf)
Write:	vsfhal_jtag_dr(request, write_value, dr_before, dr_after, NULL)
*/
uint8_t vsfhal_jtag_dr(uint8_t request, uint32_t dr, uint16_t dr_before, uint16_t dr_after,
    uint32_t* data)
{
    uint8_t ack;
    uint16_t retry, bitlen;
    uint64_t buf_tdi, buf_tms, buf_tdo;

    retry = 0;
    bitlen = 0;
    buf_tdi = 0;
    buf_tms = 0;

    // Select-DR-Scan, Capture-DR, Shift-DR
    buf_tms |= (uint64_t)0x1 << bitlen;
    bitlen += 3;
    // Bypass before data
    bitlen += dr_before;
    // RnW, A2, A3
    buf_tdi |= (uint64_t)((request >> 1) & 0x7) << bitlen;
    bitlen += 3;
    // Data Transfer
    if (!(request & DAP_TRANSFER_RnW))
        buf_tdi |= (uint64_t)dr << bitlen;
    bitlen += 31 + dr_after;
    buf_tms |= (uint64_t)0x1 << bitlen;
    bitlen++;
    // Update-DR, Idle
    buf_tms |= (uint64_t)0x1 << bitlen;
    bitlen += 1 + jtag_control.idle;
    buf_tdi |= (uint64_t)0x1 << bitlen; // keep tdi high
    bitlen++;

#if TIMESTAMP_CLOCK
    if (request & DAP_TRANSFER_TIMESTAMP)
        dap_timestamp = vsf_systimer_get_us();
#endif

    do {
        jtag_control.jtag_rw(bitlen, (uint8_t*)&buf_tdi, (uint8_t*)&buf_tms, (uint8_t*)&buf_tdo);
        ack = (buf_tdo >> (dr_before + 3)) & 0x7;
        ack = (ack & 0x4) | ((ack & 0x2) >> 1) | ((ack & 0x1) << 1);
        if (ack != DAP_TRANSFER_WAIT)
            break;
    } while (retry++ < jtag_control.retry);

    *data = buf_tdo >> (dr_before + 6);
    return ack;
}

#endif
