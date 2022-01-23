#ifndef __HAL_DRIVER_MIC_MT006_WDT_H__
#define __HAL_DRIVER_MIC_MT006_WDT_H__

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"
#include "__device.h"

/*============================ MACROS ========================================*/

#ifndef WDT_WINDOW_ENABLE
#   define WDT_WINDOW_ENABLE        0
#endif
#ifndef WDT_INDEP_ENABLE
#   define WDT_INDEP_ENABLE          0
#endif

#define WDT_COUNT                   (0 + WDT_WINDOW_ENABLE + WDT_INDEP_ENABLE)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

#if WDT_COUNT
enum wdt_idx_t {
    #if WDT_WINDOW_ENABLE
    WDT_WINDOW_IDX,
    #endif
    #if WDT_INDEP_ENABLE
    WDT_INDEP_IDX,
    #endif
    WDT_IDX_NUM,
    WDT_INVALID_IDX,
};
#endif

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ INCLUDES ======================================*/
/*============================ PROTOTYPES ====================================*/

#if WDT_COUNT
void vsfhal_wdt_init(enum wdt_idx_t idx, int32_t feed_ms, int32_t rst_delay_ms);
void vsfhal_wdt_fini(enum wdt_idx_t idx);
void vsfhal_wdt_feed(enum wdt_idx_t idx);
#endif

#endif
/* EOF */
