#include "timestamp.h"
#include "vsf.h"

#if TIMESTAMP_CLOCK

static uint32_t timestamp_x65536 = 0;

void vsfhal_timestamp_init(uint32_t timestamp, int32_t int_priority)
{
    // NULL
}

uint32_t vsfhal_timestamp_get(void)
{
    return vsfhal_tickcnt_get_us();
}

#endif
