#include "vsf.h"
#include "vsfhal_core.h"

#if VSFHAL_TIMER_EN

#define VSFHAL_TIMER_NUM		8

static TIMER_TypeDef * const timer_list[VSFHAL_TIMER_NUM] =
{
	TIMER1, TIMER2, TIMER3, TIMER6, TIMER14, TIMER15, TIMER16, TIMER17,
};

struct timer_info_t
{
	void (*cb)(void *);
	void *param;
	int32_t trigger_cnt;
} static timer_info[VSFHAL_TIMER_NUM];

vsf_err_t vsfhal_timer_init(uint8_t index, int32_t int_priority)
{
	switch (index)
	{
	case 0:
		RCC->APB2CCR |= RCC_APB2CCR_TIMER1EN;
		RCC->APB2RCR |= RCC_APB2RCR_TIMER1RST;
		RCC->APB2RCR &= ~RCC_APB2RCR_TIMER1RST;
		if (int_priority >= 0)
		{
			NVIC_SetPriority(TIMER1_BRK_UP_TRG_COM_IRQn, int_priority);
			NVIC_EnableIRQ(TIMER1_BRK_UP_TRG_COM_IRQn);
			NVIC_SetPriority(TIMER1_CC_IRQn, int_priority);
			NVIC_EnableIRQ(TIMER1_CC_IRQn);
		}
		break;
	case 1:
		RCC->APB1CCR |= RCC_APB1CCR_TIMER2EN;
		RCC->APB1RCR |= RCC_APB1RCR_TIMER2RST;
		RCC->APB1RCR &= ~RCC_APB1RCR_TIMER2RST;
		if (int_priority >= 0)
		{
			NVIC_SetPriority(TIMER2_IRQn, int_priority);
			NVIC_EnableIRQ(TIMER2_IRQn);
		}
		break;
	case 2:
		RCC->APB1CCR |= RCC_APB1CCR_TIMER3EN;
		RCC->APB1RCR |= RCC_APB1RCR_TIMER3RST;
		RCC->APB1RCR &= ~RCC_APB1RCR_TIMER3RST;
		if (int_priority >= 0)
		{
			NVIC_SetPriority(TIMER3_IRQn, int_priority);
			NVIC_EnableIRQ(TIMER3_IRQn);
		}
		break;
	case 3:
		RCC->APB1CCR |= RCC_APB1CCR_TIMER6EN;
		RCC->APB1RCR |= RCC_APB1RCR_TIMER6RST;
		RCC->APB1RCR &= ~RCC_APB1RCR_TIMER6RST;
		if (int_priority >= 0)
		{
			NVIC_SetPriority(TIMER6_DAC_IRQn, int_priority);
			NVIC_EnableIRQ(TIMER6_DAC_IRQn);
		}
		break;
	case 4:
		RCC->APB1CCR |= RCC_APB1CCR_TIMER14EN;
		RCC->APB1RCR |= RCC_APB1RCR_TIMER14RST;
		RCC->APB1RCR &= ~RCC_APB1RCR_TIMER14RST;
		if (int_priority >= 0)
		{
			NVIC_SetPriority(TIMER14_IRQn, int_priority);
			NVIC_EnableIRQ(TIMER14_IRQn);
		}
		break;
	case 5:
		RCC->APB2CCR |= RCC_APB2CCR_TIMER15EN;
		RCC->APB2RCR |= RCC_APB2RCR_TIMER15RST;
		RCC->APB2RCR &= ~RCC_APB2RCR_TIMER15RST;
		if (int_priority >= 0)
		{
			NVIC_SetPriority(TIMER15_IRQn, int_priority);
			NVIC_EnableIRQ(TIMER15_IRQn);
		}
		break;
	case 6:
		RCC->APB2CCR |= RCC_APB2CCR_TIMER16EN;
		RCC->APB2RCR |= RCC_APB2RCR_TIMER16RST;
		RCC->APB2RCR &= ~RCC_APB2RCR_TIMER16RST;
		if (int_priority >= 0)
		{
			NVIC_SetPriority(TIMER16_IRQn, int_priority);
			NVIC_EnableIRQ(TIMER16_IRQn);
		}
		break;
	case 7:
		RCC->APB2CCR |= RCC_APB2CCR_TIMER17EN;
		RCC->APB2RCR |= RCC_APB2RCR_TIMER17RST;
		RCC->APB2RCR &= ~RCC_APB2RCR_TIMER17RST;
		if (int_priority >= 0)
		{
			NVIC_SetPriority(TIMER17_IRQn, int_priority);
			NVIC_EnableIRQ(TIMER17_IRQn);
		}
		break;
	default:
		return VSFERR_FAIL;
	}

	return VSFERR_NONE;
}

vsf_err_t vsfhal_timer_fini(uint8_t index)
{
	switch (index)
	{
	case 0:
		RCC->APB2CCR &= ~RCC_APB2CCR_TIMER1EN;
		break;
	case 1:
		RCC->APB1CCR &= ~RCC_APB1CCR_TIMER2EN;
		break;
	case 2:
		RCC->APB1CCR &= ~RCC_APB1CCR_TIMER3EN;
		break;
	case 3:
		RCC->APB1CCR &= ~RCC_APB1CCR_TIMER6EN;
		break;
	case 4:
		RCC->APB1CCR &= ~RCC_APB1CCR_TIMER14EN;
		break;
	case 5:
		RCC->APB2CCR &= ~RCC_APB2CCR_TIMER15EN;
		break;
	case 6:
		RCC->APB2CCR &= ~RCC_APB2CCR_TIMER16EN;
		break;
	case 7:
		RCC->APB2CCR &= ~RCC_APB2CCR_TIMER17EN;
		break;
	default:
		return VSFERR_FAIL;
	}
	
	return VSFERR_NONE;
}

vsf_err_t vsfhal_timer_callback_config(uint8_t index, void (*cb)(void *),
		void *param)
{
	uint32_t clk;
	TIMER_TypeDef * const timer = timer_list[index];
	struct vsfhal_info_t *info;
	
	if (vsfhal_core_get_info(&info) != VSFERR_NONE)
		return VSFERR_FAIL;

	timer_info[index].cb = cb;
	timer_info[index].param = param;
	
	if (!index || (index >= 5))
		timer->PSC = info->apb2_freq_hz / 10000000 - 1;
	else
		timer->PSC = info->apb1_freq_hz / 10000000 - 1;	
	
	timer->DIE = TIMER_DIE_UPIE;
	timer->CTLR1 = TIMER_CTLR1_ARSE;
	
	return VSFERR_NONE;
}

vsf_err_t vsfhal_timer_callback_start(uint8_t index, uint32_t interval_us,
		int32_t trigger_cnt)
{
	TIMER_TypeDef * const timer = timer_list[index];
	
	timer_info[index].trigger_cnt = trigger_cnt;
	if (interval_us)
		timer->CARL = interval_us * 10 - 1;
	else
		timer->CARL = 1;
	
	timer->CTLR1 |= TIMER_CTLR1_CNTE;
	
	return VSFERR_NONE;
}

vsf_err_t vsfhal_timer_stop(uint8_t index)
{
	TIMER_TypeDef * const timer = timer_list[index];
	timer->CTLR1 &= ~TIMER_CTLR1_CNTE;
	return VSFERR_NONE;
}

void timer_irq(uint8_t index)
{
	TIMER_TypeDef * const timer = timer_list[index];

	if (timer_info[index].trigger_cnt > 0)
		timer_info[index].trigger_cnt--;
	if (timer_info[index].trigger_cnt == 0)
		timer->CTLR1 &= ~TIMER_CTLR1_CNTE;
	
	if (timer_info[index].cb)
		timer_info[index].cb(timer_info[index].param);
	
	timer->STR = 0;
}

ROOT void TIM6_DAC_IRQHandler(void)
{
	timer_irq(3);
}

#endif	// VSFHAL_TIMER_EN

