/*============================ INCLUDES ======================================*/

#include "wdt.h"

/*============================ MACROS ========================================*/

#define IWDG_Prescaler_4                    ((uint8_t)0x00)
#define IWDG_Prescaler_8                    ((uint8_t)0x01)
#define IWDG_Prescaler_16                   ((uint8_t)0x02)
#define IWDG_Prescaler_32                   ((uint8_t)0x03)
#define IWDG_Prescaler_64                   ((uint8_t)0x04)
#define IWDG_Prescaler_128                  ((uint8_t)0x05)
#define IWDG_Prescaler_256                  ((uint8_t)0x06)

#define KR_KEY_Reload                       ((uint16_t)0xAAAA)
#define KR_KEY_Enable                       ((uint16_t)0xCCCC)
#define KR_KEY_UNLOCK                       ((uint16_t)0x5555)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

#if WDT_COUNT > 0

static uint16_t wdt_reload[WDT_IDX_NUM];

void vsfhal_wdt_init(enum wdt_idx_t idx, int32_t feed_ms, int32_t rst_delay_ms)
{
    VSF_HAL_ASSERT(idx < WDT_IDX_NUM);

    uint32_t base_clk, reload;
    const vsfhal_clk_info_t *info;

    switch (idx) {
    #if WDT_WINDOW_ENABLE
    case WDT_WINDOW_IDX:
        RCC->AHBCLKCTRL0_SET = RCC_AHBCLKCTRL_WWDG;
        RCC->PRESETCTRL0_CLR = RCC_PRESETCTRL_WWDG;
        RCC->PRESETCTRL0_SET = RCC_PRESETCTRL_WWDG;
        
        #if WDT_WINDOW_DEBUG
        RCC->DBGCTRL |= RCC_DBGCTRL_WWDG;
        #endif
        
        info = vsfhal_clk_info_get();
        base_clk = info->ahb_apb_freq_hz / 16384;
        if (feed_ms < (1000 * 60 / base_clk))
            WWDG->CFR = (WWDG->CFR & ~(WWDG_CFR_WDGTB | WWDG_CFR_W)) | 0x7f;
        else if (feed_ms < (1000 * 60 * 2 / base_clk))
            WWDG->CFR = (WWDG->CFR & ~(WWDG_CFR_WDGTB | WWDG_CFR_W)) | WWDG_CFR_WDGTB0 | 0x7f;
        else if (feed_ms < (1000 * 60 * 4 / base_clk))
            WWDG->CFR = (WWDG->CFR & ~(WWDG_CFR_WDGTB | WWDG_CFR_W)) | WWDG_CFR_WDGTB1 | 0x7f;
        else if (feed_ms < (1000 * 60 * 8 / base_clk))
            WWDG->CFR = (WWDG->CFR & ~(WWDG_CFR_WDGTB | WWDG_CFR_W)) | WWDG_CFR_WDGTB0 | WWDG_CFR_WDGTB1 | 0x7f;
        else
            VSF_HAL_ASSERT(false);
        
        WWDG->CR = WWDG_CR_WDGA | 0x7f;
        break;
    #endif
    #if WDT_INDEP_ENABLE
    case WDT_INDEP_IDX:
        RCC->AHBCLKCTRL0_SET = RCC_AHBCLKCTRL_IWDG;
        RCC->PRESETCTRL0_CLR = RCC_PRESETCTRL_IWDG;
        RCC->PRESETCTRL0_SET = RCC_PRESETCTRL_IWDG;

        #if WDT_INDEP_DEBUG
        RCC->DBGCTRL |= RCC_DBGCTRL_IWDG;
        #endif

        base_clk = 10000;
        IWDG->KR = KR_KEY_UNLOCK;
        if (feed_ms < (1000 * 3600 * 4 / base_clk)) {
            IWDG->PR = IWDG_Prescaler_4;
            base_clk = base_clk / 4;
        } else if (feed_ms < (1000 * 3600 * 8 / base_clk)) {
            IWDG->PR = IWDG_Prescaler_8;
            base_clk = base_clk / 8;
        } else if (feed_ms < (1000 * 3600 * 16 / base_clk)) {
            IWDG->PR = IWDG_Prescaler_16;
            base_clk = base_clk / 16;
        } else if (feed_ms < (1000 * 3600 * 32 / base_clk)) {
            IWDG->PR = IWDG_Prescaler_32;
            base_clk = base_clk / 32;
        } else if (feed_ms < (1000 * 3600 * 64 / base_clk)) {
            IWDG->PR = IWDG_Prescaler_64;
            base_clk = base_clk / 64;
        } else if (feed_ms < (1000 * 3600 * 128 / base_clk)) {
            IWDG->PR = IWDG_Prescaler_128;
            base_clk = base_clk / 128;
        } else if (feed_ms < (1000 * 3600 * 256 / base_clk)) {
            IWDG->PR = IWDG_Prescaler_256;
            base_clk = base_clk / 256;
        } else {
            VSF_HAL_ASSERT(false);
        }
        reload = base_clk * feed_ms / 1000;
        reload += reload / 4;
        if (reload > 0xfff)
            IWDG->RLR = 0xfff;
        else
            IWDG->RLR = reload;
        IWDG->KR = KR_KEY_Enable;
        break;
    #endif
    }
}

void vsfhal_wdt_fini(enum wdt_idx_t idx)
{
    switch (idx) {
    #if WDT_WINDOW_ENABLE
    case WDT_WINDOW_IDX:
        RCC->AHBCLKCTRL0_CLR = RCC_AHBCLKCTRL_WWDG;
        break;
    #endif
    #if WDT_INDEP_ENABLE
    case WDT_INDEP_IDX:
        RCC->AHBCLKCTRL0_CLR = RCC_AHBCLKCTRL_IWDG;
        break;
    #endif
    }
}

void vsfhal_wdt_feed(enum wdt_idx_t idx)
{
    switch (idx) {
    #if WDT_WINDOW_ENABLE
    case WDT_WINDOW_IDX:
        WWDG->CR = 0x7f;
        break;
    #endif
    #if WDT_INDEP_ENABLE
    case WDT_INDEP_IDX:
        IWDG->KR = KR_KEY_Reload;
        break;
    #endif
    }
}

#endif
