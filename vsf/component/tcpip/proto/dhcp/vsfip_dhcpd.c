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

#include "vsfip_dhcp_common.h"

static struct vsfip_ipmac_assoc*
vsfip_dhcpd_get_assoc(struct vsfip_netif_t *netif, uint8_t *hwaddr)
{
	struct vsfip_dhcpd_t *dhcpd = netif->dhcp.dhcpd;
	uint8_t macsize = netif->macaddr.size;
	struct vsfip_ipmac_assoc *assoc = NULL;
	uint32_t i;

	for (i = 0; i < dimof(dhcpd->assoc); i++)
	{
		if (dhcpd->assoc[i].mac.size == macsize)
		{
			// allocated assoc
			if (!memcmp(dhcpd->assoc[i].mac.addr.s_addr_buf, hwaddr, macsize))
			{
				return &dhcpd->assoc[i];
			}
		}
		else if (NULL == assoc)
		{
			assoc = &dhcpd->assoc[i];
		}
	}

	if (assoc != NULL)
	{
		uint32_t addr = netif->ipaddr.addr.s_addr & ~netif->netmask.addr.s_addr;
		addr = BE_TO_SYS_U32(addr) + dhcpd->alloc_idx;
		addr = SYS_TO_BE_U32(addr);
		if (addr & netif->netmask.addr.s_addr)
		{
			// no more address available
			return NULL;
		}

		dhcpd->alloc_idx++;
		assoc->ip.size = netif->ipaddr.size;
		assoc->ip.addr.s_addr = addr |
				(netif->netmask.addr.s_addr & netif->ipaddr.addr.s_addr);
		assoc->mac.size = netif->macaddr.size;
		memcpy(assoc->mac.addr.s_addr_buf, hwaddr, netif->macaddr.size);
	}
	return assoc;
}

static void vsfip_dhcpd_input(void *param, struct vsfip_buffer_t *buf)
{
	struct vsfip_dhcpd_t *dhcpd = (struct vsfip_dhcpd_t *)param;
	struct vsfip_netif_t *netif = dhcpd->netif;
	struct vsfip_dhcphead_t *head;
	struct vsfip_ipmac_assoc *assoc;
	uint8_t optlen, op;
	uint8_t *optptr;

	if (netif != buf->netif)
	{
		goto exit;
	}

	head = (struct vsfip_dhcphead_t *)buf->app.buffer;
	if ((head->op != DHCP_TOSERVER) ||
		(head->magic != SYS_TO_BE_U32(DHCP_MAGIC)) ||
		(head->htype != VSFIP_ETH_HWTYPE) || (head->hlen != 6))
	{
		goto exit;
	}

	optlen = vsfip_dhcp_get_opt(buf, DHCPOPT_MSGTYPE, &optptr);
	if (optlen != DHCPOPT_MSGTYPE_LEN)
	{
		goto exit;
	}

	switch (optptr[0])
	{
	case DHCPOP_DISCOVER:
		assoc = vsfip_dhcpd_get_assoc(netif, head->chaddr);
		if (NULL == assoc)
		{
			goto exit;
		}

		op = DHCPOP_OFFER;

	common_reply:
		head->op = DHCP_TOCLIENT;
		head->secs = 0;
		head->flags = 0;
		head->yiaddr = assoc->ip.addr.s_addr;
		head->siaddr = netif->ipaddr.addr.s_addr;
		buf->app.size = sizeof(*head);

		dhcpd->optlen = 0;
		vsfip_dhcp_append_opt(buf, &dhcpd->optlen, DHCPOPT_MSGTYPE,
					DHCPOPT_MSGTYPE_LEN, &op);
		vsfip_dhcp_append_opt(buf, &dhcpd->optlen, DHCPOPT_SERVERID,
					netif->ipaddr.size, netif->ipaddr.addr.s_addr_buf);
		{
			uint32_t lease_time = SYS_TO_BE_U32(0x80000000);
			vsfip_dhcp_append_opt(buf, &dhcpd->optlen, DHCPOPT_LEASE_TIME,
					netif->ipaddr.size, (uint8_t *)&lease_time);
		}
		vsfip_dhcp_append_opt(buf, &dhcpd->optlen, DHCPOPT_SUBNETMASK,
					netif->netmask.size, netif->netmask.addr.s_addr_buf);
		vsfip_dhcp_append_opt(buf, &dhcpd->optlen, DHCPOPT_ROUTER,
					netif->ipaddr.size, netif->ipaddr.addr.s_addr_buf);
		vsfip_dhcp_append_opt(buf, &dhcpd->optlen, DHCPOPT_DNSSERVER,
					netif->ipaddr.size, netif->ipaddr.addr.s_addr_buf);
#ifdef VSFIP_CFG_HOSTNAME
		vsfip_dhcp_append_opt(buf, &dhcpd->optlen, DHCPOPT_HOSTNAME,
					strlen(VSFIP_CFG_HOSTNAME), (uint8_t *)VSFIP_CFG_HOSTNAME);
#endif
#ifdef VSFIP_CFG_DOMAIN
		vsfip_dhcp_append_opt(buf, &dhcpd->optlen, DHCPOPT_DOMAIN,
					strlen(VSFIP_CFG_DOMAIN), (uint8_t *)VSFIP_CFG_DOMAIN);
#endif
		vsfip_dhcp_end_opt(buf, &dhcpd->optlen);

		dhcpd->sockaddr.sin_addr.addr.s_addr = assoc->ip.addr.s_addr;
		vsfip_netif_arp_add_assoc(dhcpd->netif,
									assoc->mac.size, assoc->mac.addr.s_addr_buf,
									assoc->ip.size, assoc->ip.addr.s_addr_buf);
		vsfip_udp_async_send(dhcpd->so, &dhcpd->sockaddr, buf);
		dhcpd->so->remote_sockaddr.sin_addr.addr.s_addr = VSFIP_IPADDR_ANY;
		return;
	case DHCPOP_REQUEST:
		assoc = vsfip_dhcpd_get_assoc(netif, head->chaddr);
		if (NULL == assoc)
		{
			goto exit;
		}
		optlen = vsfip_dhcp_get_opt(buf, DHCPOPT_REQIP, &optptr);
		if ((4 == optlen) && (*(uint32_t *)optptr != assoc->ip.addr.s_addr))
		{
			op = DHCPOP_NAK;
		}
		else
		{
			op = DHCPOP_ACK;
		}
		goto common_reply;
	}

exit:
	vsfip_buffer_release(buf);
}

vsf_err_t vsfip_dhcpd_start(struct vsfip_netif_t *netif,
							struct vsfip_dhcpd_t *dhcpd)
{
	if ((NULL == netif) || (NULL == dhcpd))
	{
		return VSFERR_FAIL;
	}

	dhcpd->alloc_idx = 1;
	netif->dhcp.dhcpd = dhcpd;
	dhcpd->netif = netif;
	memset(&dhcpd->assoc, 0, sizeof(dhcpd->assoc));

	dhcpd->sockaddr.sin_port = DHCP_CLIENT_PORT;
	dhcpd->sockaddr.sin_addr.size = 4;

	dhcpd->so = vsfip_socket(AF_INET, IPPROTO_UDP);
	if (NULL == dhcpd->so)
	{
		goto cleanup;
	}
	vsfip_socket_cb(dhcpd->so, dhcpd, vsfip_dhcpd_input, NULL);
	if (vsfip_bind(dhcpd->so, DHCP_SERVER_PORT))
	{
		goto cleanup;
	}
	return vsfip_listen(dhcpd->so, 0);
cleanup:
	if (dhcpd->so != NULL)
	{
		vsfip_close(dhcpd->so);
		dhcpd->so = NULL;
	}
	return VSFERR_FAIL;
}

