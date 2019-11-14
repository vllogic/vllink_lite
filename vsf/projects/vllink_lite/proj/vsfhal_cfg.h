/***************************************************************************
 *   Copyright (C) 2009 - 2010 by Simon Qian <SimonQian@SimonQian.com>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef __VSFHAL_CFG_H__
#define __VSFHAL_CFG_H__

#include "proj_cfg.h"

#if defined(BOARD_TYPE_VLLINK_LITE)
#	define SOC_TYPE_GD32F350
#	define DAP_BLOCK_TRANSFER
#	include "vsfhal_cfg_vllink_lite.h"
#else
#	error "UNKNOWN PRODUCT"
#endif

#define PERIPHERAL_JTAG_SPI					SPI0
#define PERIPHERAL_JTAG_PRIORITY			VSFHAL_JTAG_PRIORITY

#define PERIPHERAL_SWD_SPI					SPI0
#define PERIPHERAL_SWD_PRIORITY				VSFHAL_SWD_PRIORITY

#if PERIPHERAL_KEY_VALID_LEVEL
#	define PERIPHERAL_KEY_INIT()			do {vsfhal_gpio_init(PERIPHERAL_KEY_PORT); vsfhal_gpio_config(PERIPHERAL_KEY_PORT, PERIPHERAL_KEY_PIN, VSFHAL_GPIO_INPUT | VSFHAL_GPIO_PULLDOWN);} while (0)
#	define PERIPHERAL_KEY_IsPress()			(vsfhal_gpio_get(PERIPHERAL_KEY_PORT, 1 << PERIPHERAL_KEY_PIN) ? true : false)
#else
#	define PERIPHERAL_KEY_INIT()			do {vsfhal_gpio_init(PERIPHERAL_KEY_PORT); vsfhal_gpio_config(PERIPHERAL_KEY_PORT, PERIPHERAL_KEY_PIN, VSFHAL_GPIO_INPUT | VSFHAL_GPIO_PULLUP);} while (0)
#	define PERIPHERAL_KEY_IsPress()			(vsfhal_gpio_get(PERIPHERAL_KEY_PORT, 1 << PERIPHERAL_KEY_PIN) ? false : true)
#endif

#if PERIPHERAL_LED_VALID_LEVEL
#	define PERIPHERAL_LED_RED_INIT()		do {vsfhal_gpio_init(PERIPHERAL_LED_RED_PORT); vsfhal_gpio_config(PERIPHERAL_LED_RED_PORT, PERIPHERAL_LED_RED_PIN, VSFHAL_GPIO_OUTPP);} while (0)
#	define PERIPHERAL_LED_RED_ON()			vsfhal_gpio_set(PERIPHERAL_LED_RED_PORT, 1 << PERIPHERAL_LED_RED_PIN)
#	define PERIPHERAL_LED_RED_OFF()			vsfhal_gpio_clear(PERIPHERAL_LED_RED_PORT, 1 << PERIPHERAL_LED_RED_PIN)
#	define PERIPHERAL_LED_GREEN_INIT()		do {vsfhal_gpio_init(PERIPHERAL_LED_GREEN_PORT); vsfhal_gpio_config(PERIPHERAL_LED_GREEN_PORT, PERIPHERAL_LED_GREEN_PIN, VSFHAL_GPIO_OUTPP);} while (0)
#	define PERIPHERAL_LED_GREEN_ON()		vsfhal_gpio_set(PERIPHERAL_LED_GREEN_PORT, 1 << PERIPHERAL_LED_GREEN_PIN)
#	define PERIPHERAL_LED_GREEN_OFF()		vsfhal_gpio_clear(PERIPHERAL_LED_GREEN_PORT, 1 << PERIPHERAL_LED_GREEN_PIN)
#else
#	define PERIPHERAL_LED_RED_INIT()		do {vsfhal_gpio_init(PERIPHERAL_LED_RED_PORT); vsfhal_gpio_config(PERIPHERAL_LED_RED_PORT, PERIPHERAL_LED_RED_PIN, VSFHAL_GPIO_OUTOD);} while (0)
#	define PERIPHERAL_LED_RED_ON()			vsfhal_gpio_clear(PERIPHERAL_LED_RED_PORT, 1 << PERIPHERAL_LED_RED_PIN)
#	define PERIPHERAL_LED_RED_OFF()			vsfhal_gpio_set(PERIPHERAL_LED_RED_PORT, 1 << PERIPHERAL_LED_RED_PIN)
#	define PERIPHERAL_LED_GREEN_INIT()		do {vsfhal_gpio_init(PERIPHERAL_LED_GREEN_PORT); vsfhal_gpio_config(PERIPHERAL_LED_GREEN_PORT, PERIPHERAL_LED_GREEN_PIN, VSFHAL_GPIO_OUTOD);} while (0)
#	define PERIPHERAL_LED_GREEN_ON()		vsfhal_gpio_clear(PERIPHERAL_LED_GREEN_PORT, 1 << PERIPHERAL_LED_GREEN_PIN)
#	define PERIPHERAL_LED_GREEN_OFF()		vsfhal_gpio_set(PERIPHERAL_LED_GREEN_PORT, 1 << PERIPHERAL_LED_GREEN_PIN)
#endif

#define PERIPHERAL_GPIO_TDI_INIT()			do {vsfhal_gpio_init(PERIPHERAL_GPIO_TDI_PORT);\
												vsfhal_gpio_config(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN, VSFHAL_GPIO_INPUT);} while (0)
#define PERIPHERAL_GPIO_TDI_FINI()			do {vsfhal_gpio_config(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN, VSFHAL_GPIO_INPUT);} while (0)
#define PERIPHERAL_GPIO_TDI_SET_INPUT()		do {vsfhal_gpio_config(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN, VSFHAL_GPIO_INPUT);} while (0)
#define PERIPHERAL_GPIO_TDI_SET_OUTPUT()	do {vsfhal_gpio_config(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN, VSFHAL_GPIO_OUTPP);} while (0)
#define PERIPHERAL_GPIO_TDI_SET()			do {vsfhal_gpio_set(PERIPHERAL_GPIO_TDI_PORT, 1 << PERIPHERAL_GPIO_TDI_PIN);} while (0)
#define PERIPHERAL_GPIO_TDI_CLEAR()			do {vsfhal_gpio_clear(PERIPHERAL_GPIO_TDI_PORT, 1 << PERIPHERAL_GPIO_TDI_PIN);} while (0)
#define PERIPHERAL_GPIO_TDI_GET()			vsfhal_gpio_get(PERIPHERAL_GPIO_TDI_PORT, 1 << PERIPHERAL_GPIO_TDI_PIN)

#define PERIPHERAL_GPIO_TMS_INIT()			do {vsfhal_gpio_init(PERIPHERAL_GPIO_TMS_MO_PORT);\
												vsfhal_gpio_init(PERIPHERAL_GPIO_TMS_MI_PORT);\
												vsfhal_gpio_config(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN, VSFHAL_GPIO_INPUT);\
												vsfhal_gpio_config(PERIPHERAL_GPIO_TMS_MI_PORT, PERIPHERAL_GPIO_TMS_MI_PIN, VSFHAL_GPIO_INPUT);} while (0)
#define PERIPHERAL_GPIO_TMS_FINI()			do {vsfhal_gpio_config(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN, VSFHAL_GPIO_INPUT);} while (0)
#define PERIPHERAL_GPIO_TMS_SET_INPUT()		do {vsfhal_gpio_config(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN, VSFHAL_GPIO_INPUT);} while (0)
#define PERIPHERAL_GPIO_TMS_SET_OUTPUT()	do {vsfhal_gpio_config(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN, VSFHAL_GPIO_OUTPP);} while (0)
#define PERIPHERAL_GPIO_TMS_SET()			do {vsfhal_gpio_set(PERIPHERAL_GPIO_TMS_MO_PORT, 1 << PERIPHERAL_GPIO_TMS_MO_PIN);} while (0)
#define PERIPHERAL_GPIO_TMS_CLEAR()			do {vsfhal_gpio_clear(PERIPHERAL_GPIO_TMS_MO_PORT, 1 << PERIPHERAL_GPIO_TMS_MO_PIN);} while (0)
#define PERIPHERAL_GPIO_TMS_GET()			vsfhal_gpio_get(PERIPHERAL_GPIO_TMS_MI_PORT, 1 << PERIPHERAL_GPIO_TMS_MI_PIN)

#define PERIPHERAL_GPIO_TCK_INIT()			do {vsfhal_gpio_init(PERIPHERAL_GPIO_TCK0_PORT);\
												vsfhal_gpio_config(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN, VSFHAL_GPIO_INPUT);} while (0)
#define PERIPHERAL_GPIO_TCK_FINI()			do {vsfhal_gpio_config(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN, VSFHAL_GPIO_INPUT);} while (0)
#define PERIPHERAL_GPIO_TCK_SET_INPUT()		do {vsfhal_gpio_config(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN, VSFHAL_GPIO_INPUT);} while (0)
#define PERIPHERAL_GPIO_TCK_SET_OUTPUT()	do {vsfhal_gpio_config(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN, VSFHAL_GPIO_OUTPP);} while (0)
#define PERIPHERAL_GPIO_TCK_SET()			do {vsfhal_gpio_set(PERIPHERAL_GPIO_TCK0_PORT, 1 << PERIPHERAL_GPIO_TCK0_PIN);} while (0)
#define PERIPHERAL_GPIO_TCK_CLEAR()			do {vsfhal_gpio_clear(PERIPHERAL_GPIO_TCK0_PORT, 1 << PERIPHERAL_GPIO_TCK0_PIN);} while (0)
#define PERIPHERAL_GPIO_TCK_GET()			vsfhal_gpio_get(PERIPHERAL_GPIO_TCK0_PORT, 1 << PERIPHERAL_GPIO_TCK0_PIN)

#define PERIPHERAL_GPIO_SRST_INIT()			do {vsfhal_gpio_init(PERIPHERAL_GPIO_SRST_PORT);\
												vsfhal_gpio_config(PERIPHERAL_GPIO_SRST_PORT, PERIPHERAL_GPIO_SRST_PIN, VSFHAL_GPIO_INPUT);} while (0)
#define PERIPHERAL_GPIO_SRST_FINI()			do {vsfhal_gpio_config(PERIPHERAL_GPIO_SRST_PORT, PERIPHERAL_GPIO_SRST_PIN, VSFHAL_GPIO_INPUT);} while (0)
#define PERIPHERAL_GPIO_SRST_SET_INPUT()	do {vsfhal_gpio_config(PERIPHERAL_GPIO_SRST_PORT, PERIPHERAL_GPIO_SRST_PIN, VSFHAL_GPIO_INPUT);} while (0)
#define PERIPHERAL_GPIO_SRST_SET_OUTPUT()	do {vsfhal_gpio_config(PERIPHERAL_GPIO_SRST_PORT, PERIPHERAL_GPIO_SRST_PIN, VSFHAL_GPIO_OUTPP);} while (0)
#define PERIPHERAL_GPIO_SRST_SET()			do {vsfhal_gpio_set(PERIPHERAL_GPIO_SRST_PORT, 1 << PERIPHERAL_GPIO_SRST_PIN);} while (0)
#define PERIPHERAL_GPIO_SRST_CLEAR()		do {vsfhal_gpio_clear(PERIPHERAL_GPIO_SRST_PORT, 1 << PERIPHERAL_GPIO_SRST_PIN);} while (0)
#define PERIPHERAL_GPIO_SRST_GET()			vsfhal_gpio_get(PERIPHERAL_GPIO_SRST_PORT, 1 << PERIPHERAL_GPIO_SRST_PIN)

#define PERIPHERAL_GPIO_TDO_INIT()			do {vsfhal_gpio_init(PERIPHERAL_GPIO_TDO_MI_PORT);\
												vsfhal_gpio_init(PERIPHERAL_GPIO_TDO_RX1_PORT);\
												vsfhal_gpio_config(PERIPHERAL_GPIO_TDO_MI_PORT, PERIPHERAL_GPIO_TDO_MI_PIN, VSFHAL_GPIO_INPUT);\
												vsfhal_gpio_config(PERIPHERAL_GPIO_TDO_RX1_PORT, PERIPHERAL_GPIO_TDO_RX1_PIN, VSFHAL_GPIO_INPUT);} while (0)
#define PERIPHERAL_GPIO_TDO_FINI()			do {vsfhal_gpio_config(PERIPHERAL_GPIO_TDO_MI_PORT, PERIPHERAL_GPIO_TDO_MI_PIN, VSFHAL_GPIO_INPUT);} while (0)
#define PERIPHERAL_GPIO_TDO_SET_INPUT()		do {vsfhal_gpio_config(PERIPHERAL_GPIO_TDO_MI_PORT, PERIPHERAL_GPIO_TDO_MI_PIN, VSFHAL_GPIO_INPUT);} while (0)
#define PERIPHERAL_GPIO_TDO_SET_OUTPUT()	do {vsfhal_gpio_config(PERIPHERAL_GPIO_TDO_MI_PORT, PERIPHERAL_GPIO_TDO_MI_PIN, VSFHAL_GPIO_OUTPP);} while (0)
#define PERIPHERAL_GPIO_TDO_SET()			do {vsfhal_gpio_set(PERIPHERAL_GPIO_TDO_MI_PORT, 1 << PERIPHERAL_GPIO_TDO_MI_PIN);} while (0)
#define PERIPHERAL_GPIO_TDO_CLEAR()			do {vsfhal_gpio_clear(PERIPHERAL_GPIO_TDO_MI_PORT, 1 << PERIPHERAL_GPIO_TDO_MI_PIN);} while (0)
#define PERIPHERAL_GPIO_TDO_GET()			vsfhal_gpio_get(PERIPHERAL_GPIO_TDO_MI_PORT, 1 << PERIPHERAL_GPIO_TDO_MI_PIN)

#define PERIPHERAL_GPIO_TRST_INIT()			do {vsfhal_gpio_init(PERIPHERAL_GPIO_TRST_MO_PORT);\
												vsfhal_gpio_init(PERIPHERAL_GPIO_TRST_TX1_PORT);\
												vsfhal_gpio_config(PERIPHERAL_GPIO_TRST_MO_PORT, PERIPHERAL_GPIO_TRST_MO_PIN, VSFHAL_GPIO_INPUT);\
												vsfhal_gpio_config(PERIPHERAL_GPIO_TRST_TX1_PORT, PERIPHERAL_GPIO_TRST_TX1_PIN, VSFHAL_GPIO_INPUT);} while (0)
#define PERIPHERAL_GPIO_TRST_FINI()			do {vsfhal_gpio_config(PERIPHERAL_GPIO_TRST_MO_PORT, PERIPHERAL_GPIO_TRST_MO_PIN, VSFHAL_GPIO_INPUT);} while (0)
#define PERIPHERAL_GPIO_TRST_SET_INPUT()	do {vsfhal_gpio_config(PERIPHERAL_GPIO_TRST_MO_PORT, PERIPHERAL_GPIO_TRST_MO_PIN, VSFHAL_GPIO_INPUT);} while (0)
#define PERIPHERAL_GPIO_TRST_SET_OUTPUT()	do {vsfhal_gpio_config(PERIPHERAL_GPIO_TRST_MO_PORT, PERIPHERAL_GPIO_TRST_MO_PIN, VSFHAL_GPIO_OUTPP);} while (0)
#define PERIPHERAL_GPIO_TRST_SET()			do {vsfhal_gpio_set(PERIPHERAL_GPIO_TRST_MO_PORT, 1 << PERIPHERAL_GPIO_TRST_MO_PIN);} while (0)
#define PERIPHERAL_GPIO_TRST_CLEAR()		do {vsfhal_gpio_clear(PERIPHERAL_GPIO_TRST_MO_PORT, 1 << PERIPHERAL_GPIO_TRST_MO_PIN);} while (0)
#define PERIPHERAL_GPIO_TRST_GET()			vsfhal_gpio_get(PERIPHERAL_GPIO_TRST_MO_PORT, 1 << PERIPHERAL_GPIO_TRST_MO_PIN)

#define PERIPHERAL_JTAG_IO_AF_CONFIG()		do {IO_AF_SELECT(PERIPHERAL_GPIO_TDI_PORT, PERIPHERAL_GPIO_TDI_PIN, 0);\
												IO_AF_SELECT(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN, 0);\
												IO_AF_SELECT(PERIPHERAL_GPIO_TDO_MI_PORT, PERIPHERAL_GPIO_TDO_MI_PIN, 0);} while (0);

#define PERIPHERAL_SWD_IO_AF_CONFIG()		do {IO_AF_SELECT(PERIPHERAL_GPIO_TMS_MO_PORT, PERIPHERAL_GPIO_TMS_MO_PIN, 0);\
												IO_AF_SELECT(PERIPHERAL_GPIO_TMS_MI_PORT, PERIPHERAL_GPIO_TMS_MI_PIN, 0);\
												IO_AF_SELECT(PERIPHERAL_GPIO_TCK0_PORT, PERIPHERAL_GPIO_TCK0_PIN, 0);} while (0);

#endif // __VSFHAL_CFG_H__

