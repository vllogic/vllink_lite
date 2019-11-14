#include "vsf.h"
#include "vsfhal_core.h"

static struct vsfhal_info_t vsfhal_info =
{
	0, CORE_VECTOR_TABLE, 
	CORE_CLKEN, CORE_CLKSRC, CORE_PLLSRC,
	HSI8M_FREQ_HZ, HSI48M_FREQ_HZ, HSE_FREQ_HZ,
	CORE_PLL_FREQ_HZ, CORE_AHB_FREQ_HZ, CORE_APB1_FREQ_HZ, CORE_APB2_FREQ_HZ, 
	CORE_CPU_FREQ_HZ, 
};

vsf_err_t vsfhal_core_get_info(struct vsfhal_info_t **info)
{
	*info = &vsfhal_info;
	return VSFERR_NONE;
}

// Pendsv
struct vsfhal_pendsv_t
{
	void (*on_pendsv)(void *);
	void *param;
} static vsfhal_pendsv;

ROOT void PendSV_Handler(void)
{
	if (vsfhal_pendsv.on_pendsv != NULL)
	{
		vsfhal_pendsv.on_pendsv(vsfhal_pendsv.param);
	}
}

vsf_err_t vsfhal_core_pendsv_config(int32_t int_priority, void (*on_pendsv)(void *), void *param)
{
	vsfhal_pendsv.on_pendsv = on_pendsv;
	vsfhal_pendsv.param = param;

	if (vsfhal_pendsv.on_pendsv != NULL)
		NVIC_SetPriority(PendSV_IRQn, (uint32_t)int_priority);
	return VSFERR_NONE;
}

vsf_err_t vsfhal_core_pendsv_trigger(void)
{
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
	return VSFERR_NONE;
}

WEAK void HardFault_Handler(void)
{
	while (1);
}

uint32_t vsfhal_core_get_stack(void)
{
	return __get_MSP();
}

vsf_err_t vsfhal_core_set_stack(uint32_t sp)
{
	__set_MSP(sp);
	return VSFERR_NONE;
}

vsf_err_t vsfhal_core_fini(void *p)
{
	return VSFERR_NONE;
}

void vsfhal_core_reset(void *p)
{
	NVIC_SystemReset();
}

uint8_t vsfhal_core_set_intlevel(uint8_t level)
{
	uint8_t origlevel = __get_BASEPRI();
	__set_BASEPRI(level);
	return origlevel;
}

// sleep will enable interrupt
// for cortex processor, if an interrupt occur between enable the interrupt
// 		and __WFI, wfi will not make the core sleep
void vsfhal_core_sleep(uint32_t mode)
{
	ENABLE_GLOBAL_INTERRUPT();
	__WFI();
}

vsf_err_t vsfhal_core_init(void *p)
{
	uint32_t tmp32;
	
	if (p && (p != &vsfhal_info))
		vsfhal_info = *(struct vsfhal_info_t *)p;

	// select irc8m
	RCU_CTL0 |= RCU_CTL0_IRC8MEN;
	while(!(RCU_CTL0 & RCU_CTL0_IRC8MSTB));
	RCU_CFG0 &= ~RCU_CFG0_SCS;
	
	if (vsfhal_info.clk_enable & GD32F3X0_CLK_HSI48M)
	{
		RCU_ADDCTL |= RCU_ADDCTL_IRC48MEN;
		while(!(RCU_ADDCTL & RCU_ADDCTL_IRC48MSTB));
	}
	
	if (vsfhal_info.clk_enable & GD32F3X0_CLK_HSE)
	{
		RCU_CTL0 |= RCU_CTL0_HXTALEN;
		while(!(RCU_CTL0 & RCU_CTL0_HXTALSTB));
	}
	
	RCU_CTL0 &= ~RCU_CTL0_PLLEN;
	if (vsfhal_info.clk_enable & GD32F3X0_CLK_PLL)
	{
		if (vsfhal_info.pllsrc == GD32F3X0_PLLSRC_HSI8M_D2)
		{
			RCU_CFG0 &= RCU_CFG0_PLLSEL;
		}
		else if (vsfhal_info.pllsrc == GD32F3X0_PLLSRC_HSI48M)
		{
			tmp32 = vsfhal_info.hsi48m_freq_hz / 4000000 - 1;
			RCU_CFG1 &= RCU_CFG1_PLLPRESEL | RCU_CFG1_PREDV;
			RCU_CFG1 |= RCU_CFG1_PLLPRESEL | tmp32;
			RCU_CFG0 |= RCU_CFG0_PLLSEL;
		}
		else if (vsfhal_info.pllsrc == GD32F3X0_PLLSRC_HSE)
		{
			tmp32 = vsfhal_info.hse_freq_hz / 4000000 - 1;
			RCU_CFG1 &= RCU_CFG1_PLLPRESEL | RCU_CFG1_PREDV;
			RCU_CFG1 |= tmp32;
			RCU_CFG0 |= RCU_CFG0_PLLSEL;
		}
		else
			return VSFERR_FAIL;
		
		tmp32 = vsfhal_info.pll_freq_hz / 4000000 - 1;
		RCU_CFG0 &= ~RCU_CFG0_PLLMF;
		RCU_CFG1 &= ~RCU_CFG1_PLLMF5;
		RCU_CFG0 |= ((tmp32 & 0xf) << 18) | ((tmp32 & 0x10) << 23);
		RCU_CFG1 |= ((tmp32 & 0x20) << 26);
		
		RCU_CTL0 |= RCU_CTL0_PLLEN;
		while(!(RCU_CTL0 & RCU_CTL0_PLLSTB));
	}
	
	// config ahb apb1 apb2
	RCU_CFG0 &= ~(RCU_CFG0_AHBPSC | RCU_CFG0_APB1PSC | RCU_CFG0_APB2PSC);
	tmp32 = vsfhal_info.ahb_freq_hz / vsfhal_info.apb1_freq_hz;
	if (tmp32 == 2)
		RCU_CFG0 |= BIT(10);
	else if (tmp32 == 4)
		RCU_CFG0 |= BIT(10) | BIT(8);
	else if (tmp32 == 8)
		RCU_CFG0 |= BITS(9, 10);
	else
		RCU_CFG0 |= BITS(8, 10);
	tmp32 = vsfhal_info.ahb_freq_hz / vsfhal_info.apb2_freq_hz;
	if (tmp32 == 2)
		RCU_CFG0 |= BIT(13);
	else if (tmp32 == 4)
		RCU_CFG0 |= BIT(13) | BIT(11);
	else if (tmp32 == 8)
		RCU_CFG0 |= BITS(12, 13);
	else
		RCU_CFG0 |= BITS(11, 13);
	if (vsfhal_info.clksrc == GD32F3X0_CLKSRC_HSE)
		tmp32 = vsfhal_info.hse_freq_hz;
	else if (vsfhal_info.clksrc == GD32F3X0_CLKSRC_PLL)
		tmp32 = vsfhal_info.pll_freq_hz;
	else
		tmp32 = vsfhal_info.hsi8m_freq_hz;
	tmp32 = vsfhal_info.ahb_freq_hz / tmp32;
	if (tmp32 == 1)
		RCU_CFG0 |= RCU_AHB_CKSYS_DIV1;
	else if (tmp32 == 2)
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
	else
		RCU_CFG0 |= RCU_AHB_CKSYS_DIV512;

	if (vsfhal_info.clksrc == GD32F3X0_CLKSRC_HSE)
	{
		RCU_CFG0 |= RCU_CKSYSSRC_HXTAL;
		while((RCU_CFG0 & RCU_SCSS_HXTAL) != RCU_SCSS_HXTAL);
	}
	else if (vsfhal_info.clksrc == GD32F3X0_CLKSRC_PLL)
	{
		RCU_CFG0 |= RCU_CKSYSSRC_PLL;
		while((RCU_CFG0 & RCU_SCSS_PLL) != RCU_SCSS_PLL);
	}
	else
	{
		RCU_CFG0 &= ~RCU_CFG0_SCS;
		while((RCU_CFG0 & RCU_SCSS_IRC8M) != RCU_SCSS_IRC8M);
	}
	
	if (!(vsfhal_info.clk_enable & GD32F3X0_CLK_HSI8M))
		RCU_CTL0 &= ~RCU_CTL0_IRC8MEN;

	SCB->VTOR = vsfhal_info.vector_table;
	SCB->AIRCR = 0x05FA0000 | vsfhal_info.priority_group;
	return VSFERR_NONE;
}

uint32_t vsfhal_uid_get(uint8_t *buffer, uint32_t size)
{
	if (!buffer || !size)
		return 0;
	size = min(size, 12);	// 96 bit max
	memcpy(buffer, (uint8_t *)0x1FFFF7AC, size);
	return size;
}

static void (*tickclk_callback)(void *param) = NULL;
static void *tickclk_param = NULL;
static volatile uint32_t tickcnt = 0;

static uint32_t tickclk_get_ms_local(void)
{
	return tickcnt;
}

uint32_t vsfhal_tickclk_get_ms(void)
{
	uint32_t count1, count2;

	do {
		count1 = tickclk_get_ms_local();
		count2 = tickclk_get_ms_local();
	} while (count1 != count2);
	return count1;
}

uint32_t vsfhal_tickclk_get_us(void)
{
	uint32_t val, load;
	load = SysTick->LOAD;
	val = (load - SysTick->VAL) * 1000;
	return tickcnt * 1000 + val / load;
}

void vsfhal_tickclk_delay(uint16_t delay_tick)
{
	int32_t tick, load;
	tick = SysTick->VAL;
	load = SysTick->LOAD;
	tick -= delay_tick;
	if (tick <= 0)
		tick += load;
	load = load / 2;
	if (tick < load)
		while ((SysTick->VAL > tick) && (SysTick->VAL < load));
	else
		while (SysTick->VAL > tick);
}

ROOT void SysTick_Handler(void)
{
	tickcnt++;
	if (tickclk_callback != NULL)
	{
		tickclk_callback(tickclk_param);
	}
#if VSFHAL_USART_EN && VSFHAL_USART1_ENABLE
	extern void gd32f3x0_usart1_poll(void);
	gd32f3x0_usart1_poll();
#endif
}

vsf_err_t vsfhal_tickclk_config_cb(void (*callback)(void*), void *param)
{
	tickclk_callback = callback;
	tickclk_param = param;
	return VSFERR_NONE;
}

vsf_err_t vsfhal_tickclk_start(void)
{
	SysTick->VAL = 0;
	SysTick->LOAD = vsfhal_info.cpu_freq_hz / 1000;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	return VSFERR_NONE;
}

vsf_err_t vsfhal_tickclk_stop(void)
{
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	return VSFERR_NONE;
}

void vsfhal_tickclk_poll(void)
{
	if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
		SysTick_Handler();
}

vsf_err_t vsfhal_tickclk_init(int32_t int_priority)
{
	if (int_priority >= 0)
	{
		SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_CLKSOURCE_Msk;
		NVIC_SetPriority(SysTick_IRQn, (uint32_t)int_priority);
	}
	else
	{
		SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk;
	}
	return VSFERR_NONE;
}

vsf_err_t vsfhal_tickclk_fini(void)
{
	return vsfhal_tickclk_stop();
}

