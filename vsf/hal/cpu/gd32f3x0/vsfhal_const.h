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
#ifndef __VSFHAL_CONST_H_INCLUDED__
#define __VSFHAL_CONST_H_INCLUDED__

#include "gd32f3x0_include.h"
#include "compiler.h"

// common
#define VSFHAL_DUMMY_PORT					0xFF

// core
#define VSFHAL_SLEEP_WFI					(0x1ul << 0)
#define VSFHAL_SLEEP_PWRDOWN				(0x1ul << 1)

// GPIO
#define VSFHAL_GPIO_INPUT					0
#define VSFHAL_GPIO_OUTPP					0x01
#define VSFHAL_GPIO_OUTOD					0x05
#define VSFHAL_GPIO_PULLUP					0x08
#define VSFHAL_GPIO_PULLDOWN				0x10

// USART
#define VSFHAL_USART_BITLEN_8				(0x0ul << 12)
#define VSFHAL_USART_BITLEN_9				(0x1ul << 12)
#define VSFHAL_USART_PARITY_NONE			(0x0ul << 9)
#define VSFHAL_USART_PARITY_ODD				(0x3ul << 9)
#define VSFHAL_USART_PARITY_EVEN			(0x2ul << 9)
#define VSFHAL_USART_STOPBITS_1				(0x0ul << 28)
#define VSFHAL_USART_STOPBITS_0P5			(0x1ul << 28)
#define VSFHAL_USART_STOPBITS_1P5			(0x3ul << 28)
#define VSFHAL_USART_STOPBITS_2				(0x2ul << 28)
#define vsfhal_usart_t						uint8_t

// SPI
#define VSFHAL_SPI_MASTER					(0x1 << 2)
#define VSFHAL_SPI_SLAVE					0x0
#define VSFHAL_SPI_MODE0					0x00
#define VSFHAL_SPI_MODE1					
#define VSFHAL_SPI_MODE2					
#define VSFHAL_SPI_MODE3					

#define vsfhal_i2c_t						uint8_t

#define VSFHAL_HAS_USBD

#endif	// __VSFHAL_CONST_H_INCLUDED__
