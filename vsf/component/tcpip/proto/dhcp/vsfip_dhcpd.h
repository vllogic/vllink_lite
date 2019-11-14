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
#ifndef __VSFIP_DHCPD_H_INCLUDED__
#define __VSFIP_DHCPD_H_INCLUDED__

#ifndef VSFIP_CFG_DHCPD_ASSOCNUM
#define VSFIP_CFG_DHCPD_ASSOCNUM			2
#endif
#ifndef VSFIP_CFG_DOMAIN
#define VSFIP_CFG_DOMAIN					"vsfip.net"
#endif

#include "vsfip_dhcp_common.h"

struct vsfip_dhcpd_t
{
	struct vsfip_netif_t *netif;

	// private
	struct vsfip_socket_t *so;
	struct vsfip_sockaddr_t sockaddr;
	struct vsfip_ipmac_assoc assoc[VSFIP_CFG_DHCPD_ASSOCNUM];
	uint32_t optlen;
	uint32_t alloc_idx;
};

#ifndef VSFCFG_EXCLUDE_DHCPD
vsf_err_t vsfip_dhcpd_start(struct vsfip_netif_t*, struct vsfip_dhcpd_t*);
#endif

#endif		// __VSFIP_DHCPD_H_INCLUDED__
