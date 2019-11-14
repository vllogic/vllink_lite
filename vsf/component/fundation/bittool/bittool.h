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

#ifndef __BITTOOL_H_INCLUDED__
#define __BITTOOL_H_INCLUDED__

uint8_t BIT_REVERSE_U8(uint8_t);
uint16_t BIT_REVERSE_U16(uint16_t);
uint32_t BIT_REVERSE_U32(uint32_t);
uint64_t BIT_REVERSE_U64(uint64_t);

// GET_UXX_XXXXXXXX and SET_UXX_XXXXXXXX are align independent
uint16_t GET_U16_MSBFIRST(uint8_t *p);
uint32_t GET_U24_MSBFIRST(uint8_t *p);
uint32_t GET_U32_MSBFIRST(uint8_t *p);
uint64_t GET_U64_MSBFIRST(uint8_t *p);
uint16_t GET_U16_LSBFIRST(uint8_t *p);
uint32_t GET_U24_LSBFIRST(uint8_t *p);
uint32_t GET_U32_LSBFIRST(uint8_t *p);
uint64_t GET_U64_LSBFIRST(uint8_t *p);

void SET_U16_MSBFIRST(uint8_t *p, uint16_t v16);
void SET_U24_MSBFIRST(uint8_t *p, uint32_t v32);
void SET_U32_MSBFIRST(uint8_t *p, uint32_t v32);
void SET_U64_MSBFIRST(uint8_t *p, uint64_t v64);
void SET_U16_LSBFIRST(uint8_t *p, uint16_t v16);
void SET_U24_LSBFIRST(uint8_t *p, uint32_t v32);
void SET_U32_LSBFIRST(uint8_t *p, uint32_t v32);
void SET_U64_LSBFIRST(uint8_t *p, uint64_t v64);

uint16_t SWAP_U16(uint16_t);
uint32_t SWAP_U24(uint32_t);
uint32_t SWAP_U32(uint32_t);
uint64_t SWAP_U64(uint64_t);

int msb(uint32_t);
int ffz(uint32_t);

// mask array
void mskarr_set(uint32_t *arr, int bit);
void mskarr_clr(uint32_t *arr, int bit);
int mskarr_ffz(uint32_t *arr, int arrlen);

#endif // __BITTOOL_H_INCLUDED__
