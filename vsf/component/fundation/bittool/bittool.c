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

#include "vsf.h"

uint8_t BIT_REVERSE_U8(uint8_t v8)
{
	v8 = ((v8 >> 1) & 0x55) | ((v8 << 1) & 0xAA);
	v8 = ((v8 >> 2) & 0x33) | ((v8 << 2) & 0xCC);
	v8 = ((v8 >> 4) & 0x0F) | ((v8 << 4) & 0xF0);
	return v8;
}

uint16_t BIT_REVERSE_U16(uint16_t v16)
{
	v16 = ((v16 >> 1) & 0x5555) | ((v16 << 1) & 0xAAAA);
	v16 = ((v16 >> 2) & 0x3333) | ((v16 << 2) & 0xCCCC);
	v16 = ((v16 >> 4) & 0x0F0F) | ((v16 << 4) & 0xF0F0);
	return SWAP_U16(v16);
}

uint32_t BIT_REVERSE_U32(uint32_t v32)
{
	v32 = ((v32 >> 1) & 0x55555555ul) | (((v32) << 1) & 0xAAAAAAAAul);
	v32 = ((v32 >> 2) & 0x33333333ul) | (((v32) << 2) & 0xCCCCCCCCul);
	v32 = ((v32 >> 4) & 0x0F0F0F0Ful) | (((v32) << 4) & 0xF0F0F0F0ul);
	return SWAP_U32(v32);
}

uint64_t BIT_REVERSE_U64(uint64_t v64)
{
	v64 = ((v64 >> 1) & 0x5555555555555555ull) | ((v64 << 1) & 0xAAAAAAAAAAAAAAAAull);
	v64 = ((v64 >> 2) & 0x3333333333333333ull) | ((v64 << 2) & 0xCCCCCCCCCCCCCCCCull);
	v64 = ((v64 >> 4) & 0x0F0F0F0F0F0F0F0Full) | ((v64 << 4) & 0xF0F0F0F0F0F0F0F0ull);
	return SWAP_U64(v64);
}

uint16_t SWAP_U16(uint16_t v16)
{
	return (v16 >> 8) | (v16 << 8);
}
uint32_t SWAP_U24(uint32_t v32)
{
	return (v32 >> 16) | (v32 & 0x0000FF00ul) | ((v32 & 0x000000FFul) << 16);
}
uint32_t SWAP_U32(uint32_t v32)
{
	v32 = ((v32 >> 8) & 0x00FF00FFul) | (((v32) << 8) & 0xFF00FF00ul);
	return (v32 >> 16) | (v32 << 16);
}
uint64_t SWAP_U64(uint64_t v64)
{
	v64 = ((v64 >> 8) & 0x00FF00FF00FF00FFull) | ((v64 << 8) & 0xFF00FF00FF00FF00ull);
	v64 = ((v64 >> 16) & 0x0000FFFF0000FFFFull) | ((v64 << 16) & 0xFFFF0000FFFF0000ull);
	return (v64 >> 32) | (v64 << 32);
}

// GET_UXX_XXXXXXXX and SET_UXX_XXXXXXXX are align independent
uint16_t GET_U16_MSBFIRST(uint8_t *p)
{
	return ((uint16_t)p[0] << 8) | ((uint16_t)p[1] << 0);
}

uint32_t GET_U24_MSBFIRST(uint8_t *p)
{
	return ((uint32_t)p[0] << 16) | ((uint32_t)p[1] << 8) |
			((uint32_t)p[2] << 0);
}

uint32_t GET_U32_MSBFIRST(uint8_t *p)
{
	return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
			((uint32_t)p[2] << 8) | ((uint32_t)p[3] << 0);
}

uint64_t GET_U64_MSBFIRST(uint8_t *p)
{
	return ((uint64_t)p[0] << 56) | ((uint64_t)p[1] << 48) |
			((uint64_t)p[2] << 40) | ((uint64_t)p[3] << 32) |
			((uint64_t)p[4] << 24) | ((uint64_t)p[5] << 16) |
			((uint64_t)p[6] << 8) | ((uint64_t)p[7] << 0);
}

uint16_t GET_U16_LSBFIRST(uint8_t *p)
{
	return ((uint16_t)p[0] << 0) | ((uint16_t)p[1] << 8);
}

uint32_t GET_U24_LSBFIRST(uint8_t *p)
{
	return ((uint32_t)p[0] << 0) | ((uint32_t)p[1] << 8) |
			((uint32_t)p[2] << 16);
}

uint32_t GET_U32_LSBFIRST(uint8_t *p)
{
	return ((uint32_t)p[0] << 0) | ((uint32_t)p[1] << 8) |
			((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

uint64_t GET_U64_LSBFIRST(uint8_t *p)
{
	return (p[0] << 0) | ((uint64_t)p[1] << 8) |
			((uint64_t)p[2] << 16) | ((uint64_t)p[3] << 24) |
			((uint64_t)p[4] << 32) | ((uint64_t)p[5] << 40) |
			((uint64_t)p[6] << 48) | ((uint64_t)p[7] << 56);
}

void SET_U16_MSBFIRST(uint8_t *p, uint16_t v16)
{
	p[0] = (v16 >> 8) & 0xFF;
	p[1] = (v16 >> 0) & 0xFF;
}

void SET_U24_MSBFIRST(uint8_t *p, uint32_t v32)
{
	p[0] = (v32 >> 16) & 0xFF;
	p[1] = (v32 >> 8) & 0xFF;
	p[2] = (v32 >> 0) & 0xFF;
}

void SET_U32_MSBFIRST(uint8_t *p, uint32_t v32)
{
	p[0] = (v32 >> 24) & 0xFF;
	p[1] = (v32 >> 16) & 0xFF;
	p[2] = (v32 >> 8) & 0xFF;
	p[3] = (v32 >> 0) & 0xFF;
}

void SET_U64_MSBFIRST(uint8_t *p, uint64_t v64)
{
	p[0] = (v64 >> 56) & 0xFF;
	p[1] = (v64 >> 48) & 0xFF;
	p[2] = (v64 >> 40) & 0xFF;
	p[3] = (v64 >> 32) & 0xFF;
	p[4] = (v64 >> 24) & 0xFF;
	p[5] = (v64 >> 16) & 0xFF;
	p[6] = (v64 >> 8) & 0xFF;
	p[7] = (v64 >> 0) & 0xFF;
}
void SET_U16_LSBFIRST(uint8_t *p, uint16_t v16)
{
	p[0] = (v16 >> 0) & 0xFF;
	p[1] = (v16 >> 8) & 0xFF;
}

void SET_U24_LSBFIRST(uint8_t *p, uint32_t v32)
{
	p[0] = (v32 >> 0) & 0xFF;
	p[1] = (v32 >> 8) & 0xFF;
	p[2] = (v32 >> 16) & 0xFF;
}

void SET_U32_LSBFIRST(uint8_t *p, uint32_t v32)
{
	p[0] = (v32 >> 0) & 0xFF;
	p[1] = (v32 >> 8) & 0xFF;
	p[2] = (v32 >> 16) & 0xFF;
	p[3] = (v32 >> 24) & 0xFF;
}

void SET_U64_LSBFIRST(uint8_t *p, uint64_t v64)
{
	p[0] = (v64 >> 0) & 0xFF;
	p[1] = (v64 >> 8) & 0xFF;
	p[2] = (v64 >> 16) & 0xFF;
	p[3] = (v64 >> 24) & 0xFF;
	p[4] = (v64 >> 32) & 0xFF;
	p[5] = (v64 >> 40) & 0xFF;
	p[6] = (v64 >> 48) & 0xFF;
	p[7] = (v64 >> 56) & 0xFF;
}

int msb(uint32_t a)
{
	int c = -1;
	while (a > 0)
	{
		c++;
		a >>= 1;
	}
	return c;
}

int ffz(uint32_t a)
{
	a = ~a;
	return msb(a & -(int32_t)a);
}

// mask array
void mskarr_set(uint32_t *arr, int bit)
{
	arr[bit >> 5] |= (1 << (bit & 0x1F));
}

void mskarr_clr(uint32_t *arr, int bit)
{
	arr[bit >> 5] &= ~(1 << (bit & 0x1F));
}

int mskarr_ffz(uint32_t *arr, int arrlen)
{
	int i, tmp;

	for (i = 0; i < arrlen; i++)
	{
		tmp = ffz(arr[i]);
		if (tmp >= 0)
		{
			return (i << 5) + tmp;
		}
	}
	return -1;
}
