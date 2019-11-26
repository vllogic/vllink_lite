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

#ifndef __VSFUSBD_WEBUSB_H_INCLUDED__
#define __VSFUSBD_WEBUSB_H_INCLUDED__

#ifdef DAP_BLOCK_TRANSFER
#include "../dap_block/DAP.h"
#else
#include "../dap/DAP.h"
#endif

struct vsfusbd_webusb_param_t
{
	bool dap_connected;
	struct dap_param_t *dap_param;

	uint8_t txbuff[DAP_CTRL_PACKET_SIZE];
	uint8_t rxbuff[DAP_CTRL_PACKET_SIZE];
};

extern const struct vsfusbd_class_protocol_t vsfusbd_webusb_class;

#endif	// __VSFUSBD_WEBUSB_H_INCLUDED__
