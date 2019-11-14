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
#ifndef __VSFIP_ETH_H_INCLUDED__
#define __VSFIP_ETH_H_INCLUDED__

#define VSFIP_ETH_HWTYPE				1
#define VSFIP_ETH_HEADSIZE				14

#ifndef VSFCFG_EXCLUDE_ETH
vsf_err_t vsfip_eth_header(struct vsfip_buffer_t *buf,
	enum vsfip_netif_proto_t proto, const struct vsfip_macaddr_t *dest_addr);
void vsfip_eth_input(struct vsfip_buffer_t *buf);
#endif

#endif		// __VSFIP_ETH_H_INCLUDED__
