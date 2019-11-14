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

PACKED_HEAD struct PACKED_MID vsfip_ethhead_t
{
	uint8_t dst_addr[6];
	uint8_t src_addr[6];
	uint16_t type;
}; PACKED_TAIL

#define VSFIP_ETH_TYPE_IP			0x0800
#define VSFIP_ETH_TYPE_IP6			0x86DD
#define VSFIP_ETH_TYPE_ARP			0x0806
#define VSFIP_ETH_TYPE_RARP			0x0835

vsf_err_t vsfip_eth_header(struct vsfip_buffer_t *buf,
	enum vsfip_netif_proto_t proto, const struct vsfip_macaddr_t *dest_addr)
{
	struct vsfip_netif_t *netif = buf->netif;
	struct vsfip_ethhead_t *head;
	
	if (buf->buf.buffer - buf->buffer < sizeof(struct vsfip_ethhead_t))
	{
		return VSFERR_FAIL;
	}
	
	buf->buf.buffer -= sizeof(struct vsfip_ethhead_t);
	buf->buf.size += sizeof(struct vsfip_ethhead_t);
	head = (struct vsfip_ethhead_t *)buf->buf.buffer;
	memcpy(head->dst_addr, dest_addr->addr.s_addr_buf, 6);
	memcpy(head->src_addr, netif->macaddr.addr.s_addr_buf, 6);
	head->type = SYS_TO_BE_U16((uint16_t)proto);
	return VSFERR_NONE;
}

void vsfip_eth_input(struct vsfip_buffer_t *buf)
{
	struct vsfip_ethhead_t *head = (struct vsfip_ethhead_t *)buf->buf.buffer;
	
	buf->buf.buffer += sizeof(struct vsfip_ethhead_t);
	buf->buf.size -= sizeof(struct vsfip_ethhead_t);
	switch (BE_TO_SYS_U16(head->type))
	{
	case VSFIP_ETH_TYPE_IP:
		vsfip_netif_ip4_input(buf);
		break;
	case VSFIP_ETH_TYPE_IP6:
		vsfip_netif_ip6_input(buf);
		break;
	case VSFIP_ETH_TYPE_ARP:
		vsfip_netif_arp_input(buf);
		break;
	default:
	case VSFIP_ETH_TYPE_RARP:
		// not supported
		vsfip_buffer_release(buf);
	}
}
