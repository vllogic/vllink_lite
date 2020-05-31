#include "vsf.h"

int main(void)
{
#   if VSF_USE_TRACE == ENABLED
    vsf_start_trace();
#   endif

    //extern void vsf_dfu_start(void);
    //vsf_dfu_start();
    return 0;
}
