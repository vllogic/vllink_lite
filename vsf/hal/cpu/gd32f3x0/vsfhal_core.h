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

#ifndef __GD32F3X0_CORE_H_INCLUDED__
#define __GD32F3X0_CORE_H_INCLUDED__

#define GD32F3X0_CLK_HSI8M				(1UL << 0)
#define GD32F3X0_CLK_HSI48M				(1UL << 1)
#define GD32F3X0_CLK_HSE				(1UL << 2)
#define GD32F3X0_CLK_PLL				(1UL << 3)

enum vsfhal_clksrc_t
{
	GD32F3X0_CLKSRC_HSI,
	GD32F3X0_CLKSRC_HSE,
	GD32F3X0_CLKSRC_PLL
};

enum vsfhal_pllsrc_t
{
	GD32F3X0_PLLSRC_HSI8M_D2,
	GD32F3X0_PLLSRC_HSI48M,
	GD32F3X0_PLLSRC_HSE,
};

struct vsfhal_info_t
{
	uint8_t priority_group;
	uint32_t vector_table;
	
	uint32_t clk_enable;
	
	enum vsfhal_clksrc_t clksrc;
	enum vsfhal_pllsrc_t pllsrc;
	
	uint32_t hsi8m_freq_hz;
	uint32_t hsi48m_freq_hz;
	uint32_t hse_freq_hz;
	
	uint32_t pll_freq_hz;
	uint32_t ahb_freq_hz;
	uint32_t apb1_freq_hz;
	uint32_t apb2_freq_hz;
	
	uint32_t cpu_freq_hz;
};

vsf_err_t vsfhal_core_get_info(struct vsfhal_info_t **info);

#endif	// __GD32F3X0_CORE_H_INCLUDED__
