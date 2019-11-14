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
#ifndef __VSFIP_DHCPC_H_INCLUDED__
#define __VSFIP_DHCPC_H_INCLUDED__

#define VSFIP_DHCPC_XID			0xABCD1234
struct vsfip_dhcpc_local_t
{
	uint32_t xid;
};

struct vsfip_dhcpc_t
{
	struct vsfip_netif_t *netif;
	struct vsfsm_t sm;
	struct vsfip_socket_t *so;
	struct vsfip_sockaddr_t sockaddr;
	struct vsfip_buffer_t *outbuffer;
	struct vsfip_buffer_t *inbuffer;
	struct vsfip_ipaddr_t ipaddr;
	struct vsfip_ipaddr_t gw;
	struct vsfip_ipaddr_t netmask;
	struct vsfip_ipaddr_t dns[2];
	struct vsfsm_sem_t update_sem;
	struct vsftimer_t *to;
	uint32_t xid;
	uint32_t optlen;
	uint32_t retry;
	uint32_t arp_retry;
	uint32_t leasetime;
	uint32_t renew_time;
	uint32_t rebinding_time;
	unsigned ready : 1;
};

#ifndef VSFCFG_EXCLUDE_DHCPC
vsf_err_t vsfip_dhcpc_start(struct vsfip_netif_t*, struct vsfip_dhcpc_t*);
#endif

#endif		// __VSFIP_DHCPC_H_INCLUDED__
