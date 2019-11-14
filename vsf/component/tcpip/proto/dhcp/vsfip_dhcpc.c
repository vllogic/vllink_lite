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

static struct vsfip_dhcpc_local_t vsfip_dhcpc =
{
	.xid = VSFIP_DHCPC_XID,
};

#define VSFIP_DHCPC_RETRY_CNT	10

enum vsfip_dhcpc_EVT_t
{
	VSFIP_DHCP_EVT_READY		= VSFSM_EVT_USER_LOCAL_INSTANT + 0,
	VSFIP_DHCP_EVT_SEND_REQUEST	= VSFSM_EVT_USER_LOCAL_INSTANT + 1,
	VSFIP_DHCP_EVT_TIMEROUT		= VSFSM_EVT_USER_LOCAL_INSTANT + 2,
};

static vsf_err_t vsfip_dhcpc_init_msg(struct vsfip_dhcpc_t *dhcpc, uint8_t op)
{
	struct vsfip_netif_t *netif = dhcpc->netif;
	struct vsfip_buffer_t *buf;
	struct vsfip_dhcphead_t *head;

	dhcpc->outbuffer = VSFIP_UDPBUF_GET(sizeof(struct vsfip_dhcphead_t));
	if (NULL == dhcpc->outbuffer)
	{
		return VSFERR_FAIL;
	}
	buf = dhcpc->outbuffer;

	head = (struct vsfip_dhcphead_t *)buf->app.buffer;
	memset(head, 0, sizeof(struct vsfip_dhcphead_t));
	head->op = DHCP_TOSERVER;
	head->htype = netif->drv->hwtype;
	head->hlen = netif->macaddr.size;
	head->xid = dhcpc->xid;
	// shift right 10-bit for div 1000
	head->secs = 0;
	memcpy(head->chaddr, netif->macaddr.addr.s_addr_buf, netif->macaddr.size);
	head->magic = SYS_TO_BE_U32(DHCP_MAGIC);
	dhcpc->optlen = 0;
	vsfip_dhcp_append_opt(buf, &dhcpc->optlen, DHCPOPT_MSGTYPE,
							DHCPOPT_MSGTYPE_LEN, &op);
	{
		uint16_t tmp16 = SYS_TO_BE_U16(576);
		// RFC 2132 9.10, message size MUST be >= 576
		vsfip_dhcp_append_opt(buf, &dhcpc->optlen, DHCPOPT_MAXMSGSIZE,
							DHCPOPT_MAXMSGSIZE_LEN, (uint8_t *)&tmp16);
	}
	{
		uint8_t requestlist[] = {DHCPOPT_SUBNETMASK,
			DHCPOPT_ROUTER, DHCPOPT_DNSSERVER, DHCPOPT_BROADCAST};
		vsfip_dhcp_append_opt(buf, &dhcpc->optlen, DHCPOPT_PARAMLIST,
								sizeof(requestlist), requestlist);
	}
#ifdef VSFIP_CFG_HOSTNAME
	vsfip_dhcp_append_opt(buf, &dhcpc->optlen, DHCPOPT_HOSTNAME,
					strlen(VSFIP_CFG_HOSTNAME), (uint8_t *)VSFIP_CFG_HOSTNAME);
#endif

	return VSFERR_NONE;
}

static void vsfip_dhcpc_input(void *param, struct vsfip_buffer_t *buf)
{
	struct vsfip_dhcpc_t *dhcpc = (struct vsfip_dhcpc_t *)param;
	struct vsfip_netif_t *netif = dhcpc->netif;
	struct vsfip_dhcphead_t *head;
	uint8_t optlen;
	uint8_t *optptr;

	head = (struct vsfip_dhcphead_t *)buf->app.buffer;
	if ((head->op != DHCP_TOCLIENT) ||
		(head->magic != SYS_TO_BE_U32(DHCP_MAGIC)) ||
		memcmp(head->chaddr, netif->macaddr.addr.s_addr_buf,
				netif->macaddr.size) ||
		(head->xid != dhcpc->xid))
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
	case DHCPOP_OFFER:
		dhcpc->ipaddr.size = 4;
		dhcpc->ipaddr.addr.s_addr = head->yiaddr;
		vsfsm_post_evt(&dhcpc->sm, VSFIP_DHCP_EVT_SEND_REQUEST);
		break;
	case DHCPOP_ACK:
		optlen = vsfip_dhcp_get_opt(buf, DHCPOPT_LEASE_TIME, &optptr);
		dhcpc->leasetime = (4 == optlen) ? GET_BE_U32(optptr) : 0;
		optlen = vsfip_dhcp_get_opt(buf, DHCPOPT_RENEW_TIME, &optptr);
		dhcpc->renew_time = (4 == optlen) ? GET_BE_U32(optptr) : 0;
		optlen = vsfip_dhcp_get_opt(buf, DHCPOPT_REBINDING_TIME, &optptr);
		dhcpc->rebinding_time = (4 == optlen) ? GET_BE_U32(optptr) : 0;
		optlen = vsfip_dhcp_get_opt(buf, DHCPOPT_SUBNETMASK, &optptr);
		dhcpc->netmask.size = optlen;
		dhcpc->netmask.addr.s_addr = (4 == optlen) ? *(uint32_t *)optptr : 0;
		optlen = vsfip_dhcp_get_opt(buf, DHCPOPT_ROUTER, &optptr);
		dhcpc->gw.size = optlen;
		dhcpc->gw.addr.s_addr = (4 == optlen) ? *(uint32_t *)optptr : 0;
		optlen = vsfip_dhcp_get_opt(buf, DHCPOPT_DNSSERVER, &optptr);
		dhcpc->dns[0].size = dhcpc->dns[1].size = 0;
		if (optlen >= 4)
		{
			dhcpc->dns[0].size = 4;
			dhcpc->dns[0].addr.s_addr = *(uint32_t *)optptr;
			if (optlen >= 8)
			{
				dhcpc->dns[1].size = 4;
				dhcpc->dns[1].addr.s_addr = *(uint32_t *)(optptr + 4);
			}
		}

		vsfsm_post_evt(&dhcpc->sm, VSFIP_DHCP_EVT_READY);
		break;
	}

exit:
	vsfip_buffer_release(buf);
}

static struct vsfsm_state_t *
vsfip_dhcpc_evt_handler(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	struct vsfip_dhcpc_t *dhcpc = (struct vsfip_dhcpc_t *)sm->user_data;
	struct vsfip_netif_t *netif = dhcpc->netif;

	switch (evt)
	{
	case VSFSM_EVT_INIT:
		dhcpc->ready = true;
		dhcpc->retry = 0;

	retry:
		dhcpc->xid = vsfip_dhcpc.xid++;
		dhcpc->so = vsfip_socket(AF_INET, IPPROTO_UDP);
		if (NULL == dhcpc->so)
		{
			goto cleanup;
		}
		vsfip_socket_cb(dhcpc->so, dhcpc, vsfip_dhcpc_input, NULL);
		if (vsfip_bind(dhcpc->so, DHCP_CLIENT_PORT))
		{
			goto cleanup;
		}
		vsfip_listen(dhcpc->so, 0);

		// if address already allocated, do resume, send request again
		if (dhcpc->ipaddr.size != 0)
		{
			goto dhcp_request;
		}

		// discover
		memset(&netif->ipaddr, 0, sizeof(netif->ipaddr));
		dhcpc->ipaddr.size = 0;
		if (vsfip_dhcpc_init_msg(dhcpc, (uint8_t)DHCPOP_DISCOVER) < 0)
		{
			goto cleanup;
		}
		vsfip_dhcp_end_opt(dhcpc->outbuffer, &dhcpc->optlen);
		dhcpc->sockaddr.sin_addr.addr.s_addr = 0xFFFFFFFF;
		vsfip_udp_async_send(dhcpc->so, &dhcpc->sockaddr, dhcpc->outbuffer);
		dhcpc->so->remote_sockaddr.sin_addr.addr.s_addr = VSFIP_IPADDR_ANY;

		dhcpc->to = vsftimer_create(sm, 5000, 1, VSFIP_DHCP_EVT_TIMEROUT);
		break;
	case VSFIP_DHCP_EVT_SEND_REQUEST:
	dhcp_request:
		vsftimer_free(dhcpc->to);
		if (vsfip_dhcpc_init_msg(dhcpc, (uint8_t)DHCPOP_REQUEST) < 0)
		{
			goto cleanup;
		}
		vsfip_dhcp_append_opt(dhcpc->outbuffer, &dhcpc->optlen,
								DHCPOPT_REQIP, dhcpc->ipaddr.size,
								dhcpc->ipaddr.addr.s_addr_buf);
		vsfip_dhcp_end_opt(dhcpc->outbuffer, &dhcpc->optlen);
		dhcpc->sockaddr.sin_addr.addr.s_addr = 0xFFFFFFFF;
		vsfip_udp_async_send(dhcpc->so, &dhcpc->sockaddr, dhcpc->outbuffer);
		dhcpc->so->remote_sockaddr.sin_addr.addr.s_addr = VSFIP_IPADDR_ANY;

		dhcpc->to = vsftimer_create(sm, 2000, 1, VSFIP_DHCP_EVT_TIMEROUT);
		break;
	case VSFIP_DHCP_EVT_READY:
		vsftimer_free(dhcpc->to);
		// update netif->ipaddr
		dhcpc->ready = 1;
		netif->ipaddr = dhcpc->ipaddr;
		netif->gateway = dhcpc->gw;
		netif->netmask = dhcpc->netmask;
		netif->dns[0] = dhcpc->dns[0];
		netif->dns[1] = dhcpc->dns[1];
		vsfdbg_printf("dhcpc: %d.%d.%d.%d" VSFCFG_DEBUG_LINEEND,
			netif->ipaddr.addr.s_addr_buf[0], netif->ipaddr.addr.s_addr_buf[1],
			netif->ipaddr.addr.s_addr_buf[2], netif->ipaddr.addr.s_addr_buf[3]);

		// timer out for resume
//		vsftimer_create(sm, 2000, 1, VSFIP_DHCP_EVT_TIMEROUT);
		goto cleanup;
		break;
	case VSFIP_DHCP_EVT_TIMEROUT:
		// maybe need to resume, set the ready to false
		dhcpc->ready = false;
	cleanup:
		if (dhcpc->so != NULL)
		{
			vsfip_close(dhcpc->so);
			dhcpc->so = NULL;
		}
		if (!dhcpc->ready && (++dhcpc->retry < VSFIP_DHCPC_RETRY_CNT))
		{
			goto retry;
		}

		// notify callder
		if (dhcpc->update_sem.evt != VSFSM_EVT_NONE)
		{
			vsfsm_sem_post(&dhcpc->update_sem);
		}
		break;
	}

	return NULL;
}

vsf_err_t vsfip_dhcpc_start(struct vsfip_netif_t *netif,
							struct vsfip_dhcpc_t *dhcpc)
{
	if ((NULL == netif) || (NULL == dhcpc))
	{
		return VSFERR_FAIL;
	}

	netif->dhcp.dhcpc = dhcpc;
	dhcpc->netif = netif;

	dhcpc->sockaddr.sin_port = DHCP_SERVER_PORT;
	dhcpc->sockaddr.sin_addr.size = 4;

	dhcpc->sm.init_state.evt_handler = vsfip_dhcpc_evt_handler;
	dhcpc->sm.user_data = dhcpc;
	return vsfsm_init(&dhcpc->sm);
}

