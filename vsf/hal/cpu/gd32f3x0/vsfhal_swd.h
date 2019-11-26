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

#define vsfhal_swd_sm_init()
vsf_err_t vsfhal_swd_init(int32_t int_priority);
vsf_err_t vsfhal_swd_fini(void);
vsf_err_t vsfhal_swd_config(uint16_t kHz, uint8_t idle, uint8_t trn,
		bool data_force, uint16_t retry);
void vsfhal_swd_seqout(uint8_t *data, uint16_t bitlen);
void vsfhal_swd_seqin(uint8_t *data, uint16_t bitlen);
int vsfhal_swd_read(uint8_t request, uint32_t *r_data);
int vsfhal_swd_write(uint8_t request, uint32_t w_data);
