/***************************************************************************
 *   Copyright (C) 2018 - 2019 by Chen Le <talpachen@gmail.com>            *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef __GD32F3X0_INCLUDE_H__
#define __GD32F3X0_INCLUDE_H__

#ifdef cplusplus
 extern "C" {
#endif 

#include "include/gd32f3x0_adc.h"
#include "include/gd32f3x0_cec.h"
#include "include/gd32f3x0_cmp.h"
#include "include/gd32f3x0_crc.h"
#include "include/gd32f3x0_ctc.h"
#include "include/gd32f3x0_dac.h"
#include "include/gd32f3x0_dbg.h"
#include "include/gd32f3x0_dma.h"
#include "include/gd32f3x0_exti.h"
#include "include/gd32f3x0_fmc.h"
#include "include/gd32f3x0_fwdgt.h"
#include "include/gd32f3x0_gpio.h"
#include "include/gd32f3x0_i2c.h"
#include "include/gd32f3x0_misc.h"
#include "include/gd32f3x0_pmu.h"
#include "include/gd32f3x0_rcu.h"
#include "include/gd32f3x0_rtc.h"
#include "include/gd32f3x0_spi.h"
#include "include/gd32f3x0_syscfg.h"
#include "include/gd32f3x0_timer.h"
#include "include/gd32f3x0_tsi.h"
#include "include/gd32f3x0_usart.h"
#include "include/gd32f3x0_wwdgt.h"
#include "include/gd32f3x0_usb_regs.h"

#define IO_AF_SELECT(port, pin, afsel)	do {REG32((GPIOA + 0x400 * port) + 0x20U + ((pin & 0x8) >> 1)) &= ~(0xful << ((pin & 0x7) * 4));\
 											REG32((GPIOA + 0x400 * port) + 0x20U + ((pin & 0x8) >> 1)) |= (uint32_t)afsel << ((pin & 0x7) * 4);} while(0)

#define IO_CFG_INPUT(port, pin)		(GPIO_CTL(GPIOA + 0x400 * port) &= ~(0x3 << (pin * 2)))
#define IO_CFG_OUTPUT(port, pin)	(GPIO_CTL(GPIOA + 0x400 * port) = (GPIO_CTL(GPIOA + 0x400 * port) & ~(0x3ul << (pin * 2))) | (0x1ul << (pin * 2)))
#define IO_CFG_AF(port, pin)		(GPIO_CTL(GPIOA + 0x400 * port) = (GPIO_CTL(GPIOA + 0x400 * port) & ~(0x3ul << (pin * 2))) | (0x2ul << (pin * 2)))
#define IO_SET(port, pin)			(GPIO_OCTL(GPIOA + 0x400 * port) |= 0x1 << pin)
#define IO_CLEAR(port, pin)			(GPIO_BC(GPIOA + 0x400 * port) = 0x1 << pin)
#define IO_GET(port, pin)			((GPIO_ISTAT(GPIOA + 0x400 * port) >> pin) & 0x1)
												
#ifdef cplusplus
}
#endif

#endif	// __GD32F3X0_INCLUDE_H__
