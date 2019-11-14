/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       GPIO.h                                                    *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    GPIO interface header file                                *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2008-11-07:     created(by SimonQian)                             *
 **************************************************************************/
#include "vsf.h"

#define VSFHAL_GPIO_NUM					6

vsf_err_t vsfhal_gpio_init(uint8_t index)
{
	if (index >= VSFHAL_GPIO_NUM)
		return VSFERR_NOT_SUPPORT;
	
	RCU_AHBEN |= RCU_AHBEN_PAEN << index;
	return VSFERR_NONE;
}

vsf_err_t vsfhal_gpio_fini(uint8_t index)
{
	if (index >= VSFHAL_GPIO_NUM)
		return VSFERR_NOT_SUPPORT;
	
	RCU_AHBRST |= RCU_AHBRST_PARST << index;
	RCU_AHBEN &= ~(RCU_AHBEN_PAEN << index);
	return VSFERR_NONE;
}

vsf_err_t vsfhal_gpio_config(uint8_t index, uint8_t pin_idx, uint32_t mode)
{
	uint32_t gpiox;
	uint8_t bit2_idx;
	
	if (index >= VSFHAL_GPIO_NUM)
		return VSFERR_NOT_SUPPORT;

	gpiox = GPIOA + 0x400 * index;
	bit2_idx = pin_idx * 2;
	
	GPIO_CTL(gpiox) = (GPIO_CTL(gpiox) & ~(0x3 << bit2_idx)) | ((mode & 0x3) << bit2_idx);
	GPIO_OMODE(gpiox) = (GPIO_OMODE(gpiox) & ~(GPIO_OMODE_OM0 << pin_idx)) | (((mode >> 2) & 0x1) << pin_idx);
	GPIO_PUD(gpiox) = (GPIO_PUD(gpiox) & ~(GPIO_PUD_PUD0 << bit2_idx)) | (((mode >> 3) & 0x1) << bit2_idx);
	
	return VSFERR_NONE;
}

vsf_err_t vsfhal_gpio_set(uint8_t index, uint32_t pin_mask)
{
	uint32_t gpiox;

	if (index >= VSFHAL_GPIO_NUM)
		return VSFERR_NOT_SUPPORT;
	
	gpiox = GPIOA + 0x400 * index;
	GPIO_BOP(gpiox) = pin_mask & 0xffff;
	return VSFERR_NONE;
}

vsf_err_t vsfhal_gpio_clear(uint8_t index, uint32_t pin_mask)
{
	uint32_t gpiox;

	if (index >= VSFHAL_GPIO_NUM)
		return VSFERR_NOT_SUPPORT;
	
	gpiox = GPIOA + 0x400 * index;
	GPIO_BOP(gpiox) = pin_mask << 16;
	return VSFERR_NONE;
}

/* registers definitions */
#define GPIO_CTL(gpiox)            REG32((gpiox) + 0x00U)    /*!< GPIO port control register */
#define GPIO_OMODE(gpiox)          REG32((gpiox) + 0x04U)    /*!< GPIO port output mode register */
#define GPIO_OSPD0(gpiox)          REG32((gpiox) + 0x08U)    /*!< GPIO port output speed register 0 */
#define GPIO_PUD(gpiox)            REG32((gpiox) + 0x0CU)    /*!< GPIO port pull-up/pull-down register */
#define GPIO_ISTAT(gpiox)          REG32((gpiox) + 0x10U)    /*!< GPIO port input status register */
#define GPIO_OCTL(gpiox)           REG32((gpiox) + 0x14U)    /*!< GPIO port output control register */
#define GPIO_BOP(gpiox)            REG32((gpiox) + 0x18U)    /*!< GPIO port bit operation register */
#define GPIO_LOCK(gpiox)           REG32((gpiox) + 0x1CU)    /*!< GPIO port configuration lock register */
#define GPIO_AFSEL0(gpiox)         REG32((gpiox) + 0x20U)    /*!< GPIO alternate function selected register 0 */
#define GPIO_AFSEL1(gpiox)         REG32((gpiox) + 0x24U)    /*!< GPIO alternate function selected register 1 */
#define GPIO_BC(gpiox)             REG32((gpiox) + 0x28U)    /*!< GPIO bit clear register */
#define GPIO_TG(gpiox)             REG32((gpiox) + 0x2CU)    /*!< GPIO port bit toggle register */
#define GPIO_OSPD1(gpiox)          REG32((gpiox) + 0x3CU)    /*!< GPIO port output speed register 1 */


vsf_err_t vsfhal_gpio_out(uint8_t index, uint32_t pin_mask, uint32_t value)
{
	uint32_t gpiox;

	if (index >= VSFHAL_GPIO_NUM)
		return VSFERR_NOT_SUPPORT;
	
	gpiox = GPIOA + 0x400 * index;
	GPIO_OCTL(gpiox) = ((pin_mask & ~value) << 16) | (pin_mask & value);
	return VSFERR_NONE;
}

uint32_t vsfhal_gpio_get(uint8_t index, uint32_t pin_mask)
{
	uint32_t gpiox;

	if (index >= VSFHAL_GPIO_NUM)
		return VSFERR_NOT_SUPPORT;
	
	gpiox = GPIOA + 0x400 * index;
	return GPIO_ISTAT(gpiox) & pin_mask;
}

