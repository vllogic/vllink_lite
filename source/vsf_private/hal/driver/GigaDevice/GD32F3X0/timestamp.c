#include "timestamp.h"
#include "vsf.h"

#if TIMESTAMP_CLOCK

static uint32_t timestamp_x65536 = 0;

void vsfhal_timestamp_init(uint32_t timestamp, int32_t int_priority)
{
    struct vsfhal_clk_info_t *info = vsfhal_clk_info_get();

    RCU_APB1EN |= RCU_APB1EN_TIMER5EN;
    TIMER_CTL0(TIMER13) = TIMER_CTL0_ARSE;
    TIMER_DMAINTEN(TIMER13) = TIMER_DMAINTEN_UPIE;
    if (info->apb1_freq_hz == info->ahb_freq_hz)
        TIMER_PSC(TIMER13) = info->apb1_freq_hz / timestamp - 1;
    else
        TIMER_PSC(TIMER13) = info->apb1_freq_hz / 2 / timestamp - 1;
    TIMER_CNT(TIMER13) = 0;
    TIMER_CAR(TIMER13) = 0xffff;
    TIMER_CTL0(TIMER13) |= TIMER_CTL0_CEN;
    NVIC_EnableIRQ(TIMER13_IRQn);
}

static uint32_t get_timestamp(void)
{
    uint32_t ret;
    NVIC_DisableIRQ(TIMER13_IRQn);
    ret = TIMER_CNT(TIMER13);
    ret += timestamp_x65536;
    NVIC_EnableIRQ(TIMER13_IRQn);
    return ret;
}

uint32_t vsfhal_timestamp_get(void)
{
    uint32_t timestamp1, timestamp2;

    do {
        timestamp1 = get_timestamp();
        timestamp2 = get_timestamp();
    } while (timestamp1 > timestamp2);
    return timestamp2;
}

ROOT void TIMER13_IRQHandler(void)
{
    TIMER_INTF(TIMER13) = 0;
    timestamp_x65536 += 0x1ul << 16;
}

#endif
