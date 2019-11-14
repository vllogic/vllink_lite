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

#include "../vsfip_priv.h"

enum vsfip_netif_EVT_t
{
	VSFIP_NETIF_EVT_ARPS_REQUEST	= VSFSM_EVT_USER_LOCAL + 0,
	VSFIP_NETIF_EVT_ARPS_REPLIED	= VSFSM_EVT_USER_LOCAL + 1,
	VSFIP_NETIF_EVT_ARPC_REQUEST	= VSFSM_EVT_USER_LOCAL + 2,

	VSFIP_NETIF_EVT_ARPC_TIMEOUT = VSFSM_EVT_USER_LOCAL_INSTANT + 0,
	VSFIP_NETIF_EVT_ARPC_UPDATE	= VSFSM_EVT_USER_LOCAL_INSTANT + 1,
	VSFIP_NETIF_EVT_ARPC_REQOUT = VSFSM_EVT_USER_LOCAL_INSTANT + 2,
};

enum vsfip_netif_ARP_op_t
{
	ARP_REQUEST = 1,
	ARP_REPLY = 2,
	RARP_REQUEST = 3,
	RARP_REPLY = 4,
};

// ARP
PACKED_HEAD struct PACKED_MID vsfip_arphead_t
{
	uint16_t hwtype;
	uint16_t prototype;
	uint8_t hwlen;
	uint8_t protolen;
	uint16_t op;
}; PACKED_TAIL

static vsf_err_t vsfip_netif_arp_client_thread(struct vsfsm_pt_t *pt,
												vsfsm_evt_t evt);
vsf_err_t vsfip_netif_construct(struct vsfip_netif_t *netif)
{
	vsfq_init(&netif->outq);
	netif->arpc.to.evt = VSFIP_NETIF_EVT_ARPC_TIMEOUT;
	netif->arpc.buf = NULL;
	netif->arp_time = 1;

	vsfsm_sem_init(&netif->output_sem, 0, VSFSM_EVT_USER_LOCAL);

	vsfsm_sem_init(&netif->arpc.sem, 0, VSFIP_NETIF_EVT_ARPC_REQUEST);
	netif->arpc.pt.user_data = netif;
	netif->arpc.pt.thread = vsfip_netif_arp_client_thread;
	return vsfsm_pt_init(&netif->arpc.sm, &netif->arpc.pt);
}

vsf_err_t vsfip_netif_init(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	return ((struct vsfip_netif_t *)pt->user_data)->drv->op->init(pt, evt);
}

vsf_err_t vsfip_netif_fini(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	return ((struct vsfip_netif_t *)pt->user_data)->drv->op->fini(pt, evt);
}

static struct vsfip_macaddr_t *
vsfip_netif_get_mac(struct vsfip_netif_t *netif, struct vsfip_ipaddr_t *ip)
{
	uint32_t i;
	if (0xFFFFFFFF == ip->addr.s_addr)
	{
		return &netif->mac_broadcast;
	}
	for (i = 0; i < VSFIP_CFG_ARPCACHE_SIZE; i++)
	{
		if (netif->arp_cache[i].time &&
			(ip->addr.s_addr == netif->arp_cache[i].assoc.ip.addr.s_addr))
		{
			return &netif->arp_cache[i].assoc.mac;
		}
	}
	return NULL;
}

static vsf_err_t vsfip_netif_ip_output_do(struct vsfip_buffer_t *buf,
					enum vsfip_netif_proto_t proto, struct vsfip_macaddr_t *mac)
{
	struct vsfip_netif_t *netif = buf->netif;

	if (netif->drv->op->header(buf, proto, mac))
	{
		vsfip_buffer_release(buf);
		return VSFERR_FAIL;
	}

	// TODO: there will be problem when vsfsm_sem_post fails,
	// 			but it SHOULD not fail except BUG in system
	vsfq_append(&netif->outq, &buf->netif_node);
	return vsfsm_sem_post(&netif->output_sem);
}

static void vsfip_netif_get_ipaddr(struct vsfip_buffer_t *buf,
									struct vsfip_ipaddr_t *ipaddr)
{
	uint8_t ipver = *buf->iphead.ipver >> 4;
	if (4 == ipver)
	{
		// IPv4
		ipaddr->size = 4;
		ipaddr->addr.s_addr = buf->iphead.ip4head->ipdest;
	}
	else //if (6 == ipver)
	{
		// IPv6
		ipaddr->size = 6;
	}
}

static bool vsfip_netif_islocal(struct vsfip_netif_t *netif,
								struct vsfip_ipaddr_t *ipaddr)
{
	int i;
	uint8_t addr_mask;

	if (0xFFFFFFFF == ipaddr->addr.s_addr)
	{
		return true;
	}
	for (i = 0; i < netif->netmask.size; i++)
	{
		addr_mask = netif->netmask.addr.s_addr_buf[i];
		if ((ipaddr->addr.s_addr_buf[i] & addr_mask) !=
			(netif->gateway.addr.s_addr_buf[i] & addr_mask))
		{
			return false;
		}
	}
	return true;
}

vsf_err_t vsfip_netif_ip_output(struct vsfip_buffer_t *buf)
{
	struct vsfip_netif_t *netif = buf->netif;
	struct vsfip_macaddr_t *mac;
	struct vsfip_ipaddr_t dest, *ip_for_mac;

	vsfip_netif_get_ipaddr(buf, &dest);
	ip_for_mac = vsfip_netif_islocal(netif, &dest) || !netif->gateway.size ?
					&dest : &netif->gateway;
	mac = vsfip_netif_get_mac(netif, ip_for_mac);
	if (NULL == mac)
	{
		// TODO: there will be problem when vsfsm_sem_post fails,
		// 			but it SHOULD not fail except BUG in system
		vsfq_append(&netif->arpc.requestq, &buf->netif_node);
		return vsfsm_sem_post(&netif->arpc.sem);
	}
	else if (buf->buf.size)
	{
		return vsfip_netif_ip_output_do(buf, VSFIP_NETIF_PROTO_IP, mac);
	}
	return VSFERR_NONE;
}

void vsfip_netif_ip4_input(struct vsfip_buffer_t *buf)
{
	vsfip_ip4_input(buf);
}

void vsfip_netif_ip6_input(struct vsfip_buffer_t *buf)
{
	vsfip_ip6_input(buf);
}

void vsfip_netif_arp_add_assoc(struct vsfip_netif_t *netif,
		uint8_t hwlen, uint8_t *hwaddr, uint8_t protolen, uint8_t *protoaddr)
{
	struct vsfip_arp_entry_t *entry = &netif->arp_cache[0];
	struct vsfip_ipaddr_t ip;
	uint32_t i;

	ip.size = protolen;
	memcpy(ip.addr.s_addr_buf, protoaddr, protolen);
	if (vsfip_netif_get_mac(netif, &ip) != NULL)
	{
		return;
	}

	for (i = 0; i < VSFIP_CFG_ARPCACHE_SIZE; i++)
	{
		if (0 == netif->arp_cache[i].time)
		{
			entry = &netif->arp_cache[i];
			break;
		}
		if (netif->arp_cache[i].time < entry->time)
		{
			entry = &netif->arp_cache[i];
		}
	}
	entry->assoc.mac.size = hwlen;
	memcpy(entry->assoc.mac.addr.s_addr_buf, hwaddr, hwlen);
	entry->assoc.ip.size = protolen;
	memcpy(entry->assoc.ip.addr.s_addr_buf, protoaddr, protolen);
	entry->time = netif->arp_time++;
}
					

void vsfip_netif_arp_input(struct vsfip_buffer_t *buf)
{
	struct vsfip_netif_t *netif = buf->netif;
	struct vsfip_arphead_t *head = (struct vsfip_arphead_t *)buf->buf.buffer;
	uint8_t *ptr = (uint8_t *)head + sizeof(struct vsfip_arphead_t);
	uint8_t *bufptr;

	// endian fix
	head->hwtype = BE_TO_SYS_U16(head->hwtype);
	head->prototype = BE_TO_SYS_U16(head->prototype);
	head->op = BE_TO_SYS_U16(head->op);

	switch (head->op)
	{
	case ARP_REQUEST:
		bufptr = (uint8_t *)head + sizeof(struct vsfip_arphead_t);

		if ((head->hwlen == netif->macaddr.size) &&
			(head->protolen == netif->ipaddr.size) &&
			(buf->buf.size >= (sizeof(struct vsfip_arphead_t) +
								2 * (head->hwlen + head->protolen))) &&
			!memcmp(bufptr + 2 * head->hwlen + head->protolen,
					netif->ipaddr.addr.s_addr_buf, head->protolen))
		{
			struct vsfip_macaddr_t macaddr;
			struct vsfip_ipaddr_t ipaddr;

			// process the ARP request
			head->hwtype = SYS_TO_BE_U16(netif->drv->hwtype);
			head->prototype = SYS_TO_BE_U16(VSFIP_NETIF_PROTO_IP);
			head->op = SYS_TO_BE_U16(ARP_REPLY);
			macaddr.size = head->hwlen;
			memcpy(macaddr.addr.s_addr_buf, bufptr, head->hwlen);
			memcpy(ipaddr.addr.s_addr_buf, bufptr + head->hwlen,
					head->protolen);
			memcpy(bufptr, netif->macaddr.addr.s_addr_buf, head->hwlen);
			memcpy(bufptr + head->hwlen, netif->ipaddr.addr.s_addr_buf,
					head->protolen);
			memcpy(bufptr + head->hwlen + head->protolen,
					macaddr.addr.s_addr_buf, head->hwlen);
			memcpy(bufptr + 2 * head->hwlen + head->protolen,
					ipaddr.addr.s_addr_buf, head->protolen);

			// send ARP reply
			vsfip_netif_ip_output_do(buf, VSFIP_NETIF_PROTO_ARP, &macaddr);
			return;
		}
		break;
	case ARP_REPLY:
		// for ARP reply, cache and send UPDATE event to netif->arpc.sm_pending
		if ((head->hwlen != netif->macaddr.size) ||
			(head->protolen != netif->ipaddr.size) ||
			(buf->buf.size < (sizeof(struct vsfip_arphead_t) +
								2 * (head->hwlen + head->protolen))))
		{
			break;
		}

		// search a most suitable slot in the ARP cache
		vsfip_netif_arp_add_assoc(netif, head->hwlen, ptr, head->protolen,
									ptr + netif->macaddr.size);

		if (netif->arpc.sm_pending != NULL)
		{
			vsfsm_post_evt(netif->arpc.sm_pending, VSFIP_NETIF_EVT_ARPC_UPDATE);
		}
		break;
	}
	vsfip_buffer_release(buf);
}

static struct vsfip_buffer_t *vsfip_netif_prepare_arp_request(
					struct vsfip_netif_t *netif, struct vsfip_ipaddr_t *ipaddr)
{
	struct vsfip_buffer_t *buf = VSFIP_NETIFBUF_GET(128);

	if (buf != NULL)
	{
		uint32_t headsize = netif->drv->netif_header_size;
		struct vsfip_arphead_t *head;
		uint8_t *ptr;

		buf->netif = netif;

		buf->buf.buffer += headsize;
		head = (struct vsfip_arphead_t *)buf->buf.buffer;
		head->hwtype = SYS_TO_BE_U16(netif->drv->hwtype);
		head->prototype = SYS_TO_BE_U16(VSFIP_NETIF_PROTO_IP);
		head->hwlen = (uint8_t)netif->macaddr.size;
		head->protolen = (uint8_t)netif->ipaddr.size;
		head->op = SYS_TO_BE_U16(ARP_REQUEST);
		ptr = (uint8_t *)head + sizeof(struct vsfip_arphead_t);
		memcpy(ptr, netif->macaddr.addr.s_addr_buf, netif->macaddr.size);
		ptr += netif->macaddr.size;
		memcpy(ptr, netif->ipaddr.addr.s_addr_buf, netif->ipaddr.size);
		ptr += netif->ipaddr.size;
		memset(ptr, 0, netif->macaddr.size);
		ptr += netif->macaddr.size;
		memcpy(ptr, ipaddr->addr.s_addr_buf, ipaddr->size);
		ptr += ipaddr->size;
		buf->buf.size = ptr - (uint8_t *)head;
	}
	return buf;
}

static vsf_err_t vsfip_netif_arp_client_thread(struct vsfsm_pt_t *pt,
												vsfsm_evt_t evt)
{
	struct vsfip_netif_t *netif = (struct vsfip_netif_t *)pt->user_data;
	struct vsfq_node_t *node;
	struct vsfip_macaddr_t *mac;
	struct vsfip_ipaddr_t dest;

	vsfsm_pt_begin(pt);

	while (!netif->quit)
	{
		if (vsfsm_sem_pend(&netif->arpc.sem, pt->sm))
		{
			vsfsm_pt_wfe(pt, VSFIP_NETIF_EVT_ARPC_REQUEST);
		}

		node = vsfq_dequeue(&netif->arpc.requestq);
		netif->arpc.cur_request =
					container_of(node, struct vsfip_buffer_t, netif_node);
		if (netif->arpc.cur_request != NULL)
		{
			vsfip_netif_get_ipaddr(netif->arpc.cur_request, &dest);
			// for local ip, send ARP for dest ip
			// for non-local ip, if gateway is valid, send to gateway
			// for non-local ip, if gateway is not valid, send ARP for dest ip
			// 		and if proxy ARP is enabled on router, router will reply
			netif->arpc.ip_for_mac =
				vsfip_netif_islocal(netif, &dest) || !netif->gateway.size ?
					dest : netif->gateway;
			mac = vsfip_netif_get_mac(netif, &netif->arpc.ip_for_mac);
			if (NULL == mac)
			{
				netif->arpc.retry = 3;
			retry:
				netif->arpc.buf = vsfip_netif_prepare_arp_request(
												netif, &netif->arpc.ip_for_mac);
				if (netif->arpc.buf != NULL)
				{
					if (!vsfip_netif_ip_output_do(netif->arpc.buf,
							VSFIP_NETIF_PROTO_ARP, &netif->mac_broadcast))
					{
						netif->arpc.sm_pending = pt->sm;
						// wait for reply with timeout
						netif->arpc.to.interval = VSFIP_CFG_ARP_TIMEOUT_MS;
						netif->arpc.to.sm = pt->sm;
						netif->arpc.to.trigger_cnt = 1;
						vsftimer_enqueue(&netif->arpc.to);

						evt = VSFSM_EVT_NONE;
						vsfsm_pt_entry(pt);
						if ((evt != VSFIP_NETIF_EVT_ARPC_UPDATE) &&
							(evt != VSFIP_NETIF_EVT_ARPC_TIMEOUT))
						{
							return VSFERR_NOT_READY;
						}
						vsftimer_dequeue(&netif->arpc.to);
						netif->arpc.sm_pending = NULL;
					}

					mac = vsfip_netif_get_mac(netif, &netif->arpc.ip_for_mac);
					if ((NULL == mac) && (netif->arpc.retry > 0))
					{
						netif->arpc.retry--;
						goto retry;
					}
				}
			}

			if (NULL == mac)
			{
				vsfip_buffer_release(netif->arpc.cur_request);
			}
			else
			{
				vsfip_netif_ip_output_do(netif->arpc.cur_request,
											VSFIP_NETIF_PROTO_IP, mac);
			}
		}
	}

	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}
