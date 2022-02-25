/*============================ INCLUDES ======================================*/

#include "./driver.h"

/*============================ MACROS ========================================*/

#ifndef TICKCNT_ENABLE
#   define TICKCNT_ENABLE               1
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
typedef void(*pFunc)(void);
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
extern const pFunc __VECTOR_TABLE[];
/*============================ IMPLEMENTATION ================================*/

static const vsfhal_clk_info_t vsfhal_clk_info = {
	.clken = CHIP_CLKEN,
	.hclksrc = CHIP_HCLKSRC,
	.pllsrc = CHIP_PLLSRC,
	.lse_freq_hz = CHIP_LSE_FREQ_HZ,
	.hse_freq_hz = CHIP_HSE_FREQ_HZ,
	.pll_freq_hz = CHIP_PLL_FREQ_HZ,
	.mainclk_freq_hz = CHIP_MAINCLK_FREQ_HZ,
	.ahb_apb_freq_hz = CHIP_AHB_APB_FREQ_HZ,
};


#if TICKCNT_ENABLE
static uint64_t tickcnt_ms = 0;
static void vsfhal_tickcnt_init(void)
{
    #if CHIP_CLKEN & MT006_CLKEN_PLL
    RCC->OUTCLKSEL = RCC_OUTCLKSEL_SEL_SYSTEM_PLL;
    RCC->OUTCLKDIV = CHIP_PLL_FREQ_HZ / 1000000;
    RCC->OUTCLKUEN = 0;
    RCC->OUTCLKUEN = 1;
    #else
    #   error "Not Support other clock source"
    #endif

    RCC->AHBCLKCTRL0_SET = RCC_AHBCLKCTRL_STIMER;
    RCC->PRESETCTRL0_CLR = RCC_PRESETCTRL_STIMER;
    RCC->PRESETCTRL0_SET = RCC_PRESETCTRL_STIMER;

    ST->CONTROLREG1 = 0;
    ST->LOADCOUNT1 = 1000 - 1;
    ST->CONTROLREG1 = ST_CTRL_MODE;
    ST->CONTROLREG1 |= ST_CTRL_ENABLE;
    NVIC_EnableIRQ(STIMER_IRQn);
    NVIC_SetPriority(STIMER_IRQn, vsf_arch_prio_highest);
}

ROOT void STIMER_IRQHandler(void)
{
    uint32_t status = ST->RAWINTSTATUS;
    
    if (status & STS_EOI_0) {
        status = ST->EOI0;
    }
    
    if (status & STS_EOI_1) {
        status = ST->EOI1;
        tickcnt_ms++;
    }
}

uint64_t vsfhal_tickcnt_get_us_64(void)
{
    uint_fast64_t ms;
    uint_fast32_t us1_inv, us2_inv;

    do {
        us1_inv = ST->CURRENTVALUE1;
        ms = tickcnt_ms;
        us2_inv = ST->CURRENTVALUE1;
    } while (us1_inv < us2_inv);
    
    return ms * 1000 + (1000000 - us2_inv);
}

uint32_t vsfhal_tickcnt_get_us(void)
{
    return vsfhal_tickcnt_get_us_64() & 0xffffffff;
}

uint32_t vsfhal_tickcnt_get_ms(void)
{
    return tickcnt_ms & 0xffffffff;
}

uint64_t vsfhal_tickcnt_get_ms_64(void)
{
    return tickcnt_ms;
}
#else
static void vsfhal_tickcnt_init(void)
{
    // NULL
}
#endif

void vsfhal_core_delay(uint32_t t)
{
	volatile uint32_t delay = t;
	while(delay--)
		__NOP();
}

static void clk_init(const vsfhal_clk_info_t *info)
{
#ifndef PROJ_CFG_CORE_INIT_TINY
    uint32_t tmp32;

    VSF_HAL_ASSERT(info);

    if (info->clken & MT006_CLKEN_HSE) {
        RCC->OSC12_CTRL = RCC_12OSCCTRL_OSC_EI | RCC_12OSCCTRL_OSC_EO | (RCC->OSC12_CTRL & 0xFF);
        while(!(RCC->OSC12_CTRL & RCC_12OSCCTRL_OSC_OK));
        vsfhal_core_delay(3000);
    }

    if (info->clken & MT006_CLKEN_CRS_USBDSOF) {
        RCC->AHBCLKCTRL0_SET = RCC_AHBCLKCTRL_CRS;
        RCC->PRESETCTRL0_CLR = RCC_PRESETCTRL_CRS;
        RCC->PRESETCTRL0_SET = RCC_PRESETCTRL_CRS;
        
        CRS->CR = CRS_CR_AUTOTRIMEN | ((RCC->OSC12_CTRL & 0xFF) << 8);
        CRS->CFGR = 12000 | (0x22 << 16) | CRS_CFGR_SYNCDIV_1 | CRS_CFGR_SYNCSRC_SOF;
        CRS->CR |= CRS_CR_CEN;
    }
    
    if (info->clken & MT006_CLKEN_LSE) {
        // TODO
    }

    if (info->clken & MT006_CLKEN_PLL) {
        uint32_t div;
        if (info->pllsrc == MT006_PLLSRC_HSI12M)
            div = info->pll_freq_hz / 12000000;
        else if (info->pllsrc == MT006_PLLSRC_HSE)
            div = info->pll_freq_hz / info->hse_freq_hz;

        RCC->PDRUNCFG &= ~RCC_PDRUNCFG_SYSPLL;
        RCC->SYSPLLCTRL = RCC_SYSPLLCTRL_FORCELOCK | ((uint32_t)info->pllsrc << 30) | (div - 4);
        while (!(RCC->SYSPLLSTAT & RCC_SYSPLLSTAT_LOCK));
        vsfhal_core_delay(3000);
    } else {
        RCC->PDRUNCFG |= RCC_PDRUNCFG_SYSPLL;
    }

    // Flash Latency
    if (info->ahb_apb_freq_hz <= 24 * 1000 * 1000)
        FLASH->ACR = FLASH_ACR_LATENCY_0;
    else if (info->ahb_apb_freq_hz <= 48 * 1000 * 1000)
        FLASH->ACR = FLASH_ACR_LATENCY_1;
    else if (info->ahb_apb_freq_hz <= 72 * 1000 * 1000)
        FLASH->ACR = FLASH_ACR_LATENCY_2;
    else if (info->ahb_apb_freq_hz <= 96 * 1000 * 1000)
        FLASH->ACR = FLASH_ACR_LATENCY_3;
    else if (info->ahb_apb_freq_hz <= 120 * 1000 * 1000)
        FLASH->ACR = FLASH_ACR_LATENCY_4;
    else
        FLASH->ACR = FLASH_ACR_LATENCY_5;

    if (info->hclksrc == MT006_HCLKSRC_PLL)
        RCC->SYSAHBCLKDIV = info->pll_freq_hz / info->ahb_apb_freq_hz;
    else
        RCC->SYSAHBCLKDIV = 1;

    if (info->ahb_apb_freq_hz == 96 * 1000 * 1000)
        RCC->USBCLKDIV = 2;
    else if (info->ahb_apb_freq_hz == 48 * 1000 * 1000)
        RCC->USBCLKDIV = 1;
    else
        RCC->USBCLKDIV = 0;

    // Select HCLK
    RCC->MAINCLKSEL = info->hclksrc;
    RCC->MAINCLKUEN = 0;
    RCC->MAINCLKUEN = 1;
#else
#   if defined(PROJ_CFG_MT006_HSI12M_PLL_96M)
    if (info->clken & MT006_CLKEN_CRS_USBDSOF) {
        RCC->AHBCLKCTRL0_SET = RCC_AHBCLKCTRL_CRS;
        RCC->PRESETCTRL0_CLR = RCC_PRESETCTRL_CRS;
        RCC->PRESETCTRL0_SET = RCC_PRESETCTRL_CRS;
        
        CRS->CR = CRS_CR_AUTOTRIMEN | ((RCC->OSC12_CTRL & 0xFF) << 8);
        CRS->CFGR = 12000 | (0x22 << 16) | CRS_CFGR_SYNCDIV_1 | CRS_CFGR_SYNCSRC_SOF;
        CRS->CR |= CRS_CR_CEN;
    }
    RCC->PDRUNCFG &= ~RCC_PDRUNCFG_SYSPLL;
    RCC->SYSPLLCTRL = RCC_SYSPLLCTRL_FORCELOCK | (0 << 30) | (8 - 4);
    while (!(RCC->SYSPLLSTAT & RCC_SYSPLLSTAT_LOCK));
    vsfhal_core_delay(3000);

    FLASH->ACR = FLASH_ACR_LATENCY_3;
    
    RCC->SYSAHBCLKDIV = 1;

    RCC->USBCLKDIV = 2;

    RCC->MAINCLKSEL = 1;
    RCC->MAINCLKUEN = 0;
    RCC->MAINCLKUEN = 1;
#   elif defined(PROJ_CFG_MT006_HSI12M_PLL_48M)
    if (info->clken & MT006_CLKEN_CRS_USBDSOF) {
        RCC->AHBCLKCTRL0_SET = RCC_AHBCLKCTRL_CRS;
        RCC->PRESETCTRL0_CLR = RCC_PRESETCTRL_CRS;
        RCC->PRESETCTRL0_SET = RCC_PRESETCTRL_CRS;
        
        CRS->CR = CRS_CR_AUTOTRIMEN | ((RCC->OSC12_CTRL & 0xFF) << 8);
        CRS->CFGR = 12000 | (0x22 << 16) | CRS_CFGR_SYNCDIV_1 | CRS_CFGR_SYNCSRC_SOF;
        CRS->CR |= CRS_CR_CEN;
    }
    RCC->PDRUNCFG &= ~RCC_PDRUNCFG_SYSPLL;
    RCC->SYSPLLCTRL = RCC_SYSPLLCTRL_FORCELOCK | (0 << 30) | (4 - 4);
    while (!(RCC->SYSPLLSTAT & RCC_SYSPLLSTAT_LOCK));
    vsfhal_core_delay(3000);

    FLASH->ACR = FLASH_ACR_LATENCY_3;
    
    RCC->SYSAHBCLKDIV = 1;

    RCC->USBCLKDIV = 1;

    RCC->MAINCLKSEL = 1;
    RCC->MAINCLKUEN = 0;
    RCC->MAINCLKUEN = 1;
#   else
#       error "Not Support!"
#   endif
#endif
    
    vsfhal_tickcnt_init();
}

WEAK(vsf_driver_init_usrapp)
bool vsf_driver_init_usrapp(void) 
{
    return true;
}

bool vsf_driver_init(void)
{
	RCC->BORCTRL = 0x8005;
	RCC->AHBCLKCTRL0_SET = RCC_AHBCLKCTRL_GPIO | RCC_AHBCLKCTRL_IOCON;

    vsf_driver_init_usrapp();

    //NVIC_SetPriorityGrouping(3);  // 4 bits for pre-emption priority
    //SCB->VTOR = (uint32_t)__VECTOR_TABLE; // Not Support On Cortex-M0

    clk_init(&vsfhal_clk_info);
    return true;
}

void vsfhal_core_reset(void *p)
{
    NVIC_SystemReset();
}

const vsfhal_clk_info_t *vsfhal_clk_info_get(void)
{
    return &vsfhal_clk_info;
}

/* EOF */
