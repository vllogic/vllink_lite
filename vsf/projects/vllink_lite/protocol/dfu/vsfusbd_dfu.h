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

#ifndef __VSFUSBD_DFU_H_INCLUDED__
#define __VSFUSBD_DFU_H_INCLUDED__

#define DFU_BLOCK_SIZE		1024

struct vsfusbd_dfu_param_t
{
	uint8_t state;
	uint32_t op_addr;
	uint32_t op_size;
	
	uint8_t reply[64];
	uint8_t block[DFU_BLOCK_SIZE];
};

extern const struct vsfusbd_class_protocol_t vsfusbd_dfu_class;

#endif	// __VSFUSBD_DFU_H_INCLUDED__
