/*============================ INCLUDES ======================================*/
#include "hal/vsf_hal_cfg.h"
#include "./device.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
typedef void(*pFunc)(void);
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
extern const pFunc __VECTOR_TABLE[];
/*============================ IMPLEMENTATION ================================*/

static vsfhal_clk_info_t vsfhal_clk_info = {
	.clken = CHIP_CLKEN,
	.hclksrc = CHIP_HCLKSRC,
	.pllsrc = CHIP_PLLSRC,
	.usbsrc = CHIP_USBSRC,
	.lse_freq_hz = CHIP_LSE_FREQ_HZ,
	.hse_freq_hz = CHIP_HSE_FREQ_HZ,
	.pll_freq_hz = CHIP_PLL_FREQ_HZ,
	.ahb_freq_hz = CHIP_AHB_FREQ_HZ,
	.apb1_freq_hz = CHIP_APB1_FREQ_HZ,
	.apb2_freq_hz = CHIP_APB2_FREQ_HZ,
};

static void clk_init(vsfhal_clk_info_t *info)
{
#ifndef PROJ_CFG_CORE_INIT_TINY
    uint32_t tmp32;

    VSF_HAL_ASSERT(info && (info->pllsrc <= GD32F3X0_PLLSRC_HSI48M));
 
    RCU_CTL0 |= RCU_CTL0_IRC8MEN;
    while(!(RCU_CTL0 & RCU_CTL0_IRC8MSTB));
    RCU_CFG0 &= ~RCU_CFG0_SCS;

    if (info->clken & GD32F3X0_CLKEN_HSI48M) {
        RCU_ADDCTL |= RCU_ADDCTL_IRC48MEN;
        while(!(RCU_ADDCTL & RCU_ADDCTL_IRC48MSTB));
    }

    if (info->clken & GD32F3X0_CLKEN_HSE) {
        RCU_ADDCTL |= RCU_CTL0_HXTALEN;
        while(!(RCU_ADDCTL & RCU_CTL0_HXTALSTB));
    }

    RCU_CTL0 &= ~RCU_CTL0_PLLEN;
    if (info->clken & GD32F3X0_CLKEN_PLL) {
		if (info->pllsrc == GD32F3X0_PLLSRC_HSI8M_D2) {
			RCU_CFG0 &= RCU_CFG0_PLLSEL;
		} else if (info->pllsrc == GD32F3X0_PLLSRC_HSE) {
			RCU_CFG1 &= RCU_CFG1_PLLPRESEL | RCU_CFG1_PREDV;
			RCU_CFG1 |= info->hse_freq_hz / 4000000 - 1;
			RCU_CFG0 |= RCU_CFG0_PLLSEL;
		} else if (info->pllsrc == GD32F3X0_PLLSRC_HSI48M) {
			RCU_CFG1 &= RCU_CFG1_PLLPRESEL | RCU_CFG1_PREDV;
			RCU_CFG1 |= RCU_CFG1_PLLPRESEL | (48000000 / 4000000 - 1);
			RCU_CFG0 |= RCU_CFG0_PLLSEL;
		}
        
        tmp32 = info->pll_freq_hz / 4000000;
        if (tmp32 < 2)
            tmp32 = 0;
        else if (tmp32 > 64)
            tmp32 = 63;
        else if (tmp32 <= 16)
            tmp32 -= 2;
        else
            tmp32 -= 1;
        RCU_CFG0 &= ~RCU_CFG0_PLLMF;
        RCU_CFG1 &= ~RCU_CFG1_PLLMF5;
        RCU_CFG0 |= ((tmp32 & 0xf) << 18) | ((tmp32 & 0x10) << 23);
        RCU_CFG1 |= (tmp32 & 0x20) << 26;

        RCU_CTL0 |= RCU_CTL0_PLLEN;
        while(!(RCU_CTL0 & RCU_CTL0_PLLSTB));
    }

	RCU_CFG0 &= ~(RCU_CFG0_AHBPSC | RCU_CFG0_APB1PSC | RCU_CFG0_APB2PSC);
	tmp32 = info->ahb_freq_hz / info->apb1_freq_hz;
    if (tmp32 == 2)
        RCU_CFG0 |= RCU_APB1_CKAHB_DIV2;
    else if (tmp32 == 4)
        RCU_CFG0 |= RCU_APB1_CKAHB_DIV4;
    else if (tmp32 == 8)
        RCU_CFG0 |= RCU_APB1_CKAHB_DIV8;
    else if (tmp32 > 8)
        RCU_CFG0 |= RCU_APB1_CKAHB_DIV16;
	tmp32 = info->ahb_freq_hz / info->apb2_freq_hz;
    if (tmp32 == 2)
        RCU_CFG0 |= RCU_APB2_CKAHB_DIV2;
    else if (tmp32 == 4)
        RCU_CFG0 |= RCU_APB2_CKAHB_DIV4;
    else if (tmp32 == 8)
        RCU_CFG0 |= RCU_APB2_CKAHB_DIV8;
    else if (tmp32 > 8)
        RCU_CFG0 |= RCU_APB2_CKAHB_DIV16;
	if (info->hclksrc == GD32F3X0_HCLKSRC_HSE)
		tmp32 = info->hse_freq_hz;
	else if (info->hclksrc == GD32F3X0_HCLKSRC_PLL)
		tmp32 = info->pll_freq_hz;
	else
		tmp32 = 8000000;
	tmp32 = tmp32 / info->ahb_freq_hz;
	if (tmp32 == 2)
		RCU_CFG0 |= RCU_AHB_CKSYS_DIV2;
	else if (tmp32 == 4)
		RCU_CFG0 |= RCU_AHB_CKSYS_DIV4;
	else if (tmp32 == 8)
		RCU_CFG0 |= RCU_AHB_CKSYS_DIV8;
	else if (tmp32 == 16)
		RCU_CFG0 |= RCU_AHB_CKSYS_DIV16;
	else if (tmp32 == 64)
		RCU_CFG0 |= RCU_AHB_CKSYS_DIV64;
	else if (tmp32 == 128)
		RCU_CFG0 |= RCU_AHB_CKSYS_DIV128;
	else if (tmp32 == 256)
		RCU_CFG0 |= RCU_AHB_CKSYS_DIV256;
	else if (tmp32 > 256)
		RCU_CFG0 |= RCU_AHB_CKSYS_DIV512;

    if (info->hclksrc == GD32F3X0_HCLKSRC_HSE) {
        RCU_CFG0 |= RCU_CKSYSSRC_HXTAL;
        while((RCU_CFG0 & RCU_SCSS_HXTAL) != RCU_SCSS_HXTAL);
    } else if (info->hclksrc == GD32F3X0_HCLKSRC_PLL) {
        RCU_CFG0 |= RCU_CKSYSSRC_PLL;
        while((RCU_CFG0 & RCU_SCSS_PLL) != RCU_SCSS_PLL);
    } else {
        RCU_CFG0 &= ~RCU_CFG0_SCS;
        while((RCU_CFG0 & RCU_SCSS_IRC8M) != RCU_SCSS_IRC8M);
    }
    
    if (!(info->clken & GD32F3X0_CLKEN_HSI))
        RCU_CTL0 &= ~RCU_CTL0_IRC8MEN;
#else
	// select irc8m
	RCU_CTL0 |= RCU_CTL0_IRC8MEN;
	while(!(RCU_CTL0 & RCU_CTL0_IRC8MSTB));
	RCU_CFG0 &= ~RCU_CFG0_SCS;

	RCU_ADDCTL |= RCU_ADDCTL_IRC48MEN;
	while(!(RCU_ADDCTL & RCU_ADDCTL_IRC48MSTB));

	RCU_CTL0 &= ~RCU_CTL0_PLLEN;

#   if defined(PROJ_CFG_GD32F3X0_HSI48M_USB_PLL_108M)
	RCU_CFG1 &= RCU_CFG1_PLLPRESEL | RCU_CFG1_PREDV;
	RCU_CFG1 |= RCU_CFG1_PLLPRESEL | 11;    // 48M / 12 = 4M
	RCU_CFG0 |= RCU_CFG0_PLLSEL;

	RCU_CFG0 &= ~RCU_CFG0_PLLMF;
	RCU_CFG1 &= ~RCU_CFG1_PLLMF5;
	RCU_CFG0 |= (((27 - 1) & 0xf) << 18) | (((27 - 1) & 0x10) << 23);    // 4M * 27 = 108M
	RCU_CFG1 |= ((27 - 1) & 0x20) << 26;

	RCU_CTL0 |= RCU_CTL0_PLLEN;
	while(!(RCU_CTL0 & RCU_CTL0_PLLSTB));
#   elif defined(PROJ_CFG_GD32F3X0_HSI48M_USB_PLL_72M)
	RCU_CFG1 &= RCU_CFG1_PLLPRESEL | RCU_CFG1_PREDV;
	RCU_CFG1 |= RCU_CFG1_PLLPRESEL | 11;    // 48M / 12 = 4M
	RCU_CFG0 |= RCU_CFG0_PLLSEL;

	RCU_CFG0 &= ~RCU_CFG0_PLLMF;
	RCU_CFG1 &= ~RCU_CFG1_PLLMF5;
	RCU_CFG0 |= (((18 - 1) & 0xf) << 18) | (((18 - 1) & 0x10) << 23);    // 4M * 18 = 72M
	RCU_CFG1 |= ((18 - 1) & 0x20) << 26;

	RCU_CTL0 |= RCU_CTL0_PLLEN;
	while(!(RCU_CTL0 & RCU_CTL0_PLLSTB));
#   else
#       error "Not Support!"
#   endif

	// config ahb apb1 apb2
	RCU_CFG0 &= ~(RCU_CFG0_AHBPSC | RCU_CFG0_APB1PSC | RCU_CFG0_APB2PSC);
	RCU_CFG0 |= RCU_AHB_CKSYS_DIV1 | RCU_APB1_CKAHB_DIV2 | RCU_APB2_CKAHB_DIV2;

	RCU_CFG0 |= RCU_CKSYSSRC_PLL;
	while((RCU_CFG0 & RCU_SCSS_PLL) != RCU_SCSS_PLL);

	RCU_CTL0 &= ~RCU_CTL0_IRC8MEN;
#endif

	RCU_AHBEN |= RCU_AHBEN_DMAEN;
}

bool vsf_driver_init(void)
{
	NVIC_SetPriorityGrouping(3);
    SCB->VTOR = (uint32_t)__VECTOR_TABLE;

	clk_init(&vsfhal_clk_info);
    return true;
}

vsfhal_clk_info_t *vsfhal_clk_info_get(void)
{
	return &vsfhal_clk_info;
}

/* EOF */
