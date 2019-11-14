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

#define VSFIP_TCP_RETRY			3
#define VSFIP_TCP_ATO			10

// TODO:
// add ICMP support
// add ip_router
// ....

enum vsfip_EVT_t
{
	VSFIP_EVT_IP_OUTPUT			= VSFSM_EVT_USER_LOCAL + 0,
	VSFIP_EVT_IP_OUTPUT_READY	= VSFSM_EVT_USER_LOCAL + 1,

	VSFIP_EVT_TICK				= VSFSM_EVT_USER_LOCAL + 4,

	VSFIP_EVT_TCP_CONNECTOK		= VSFSM_EVT_USER_LOCAL + 5,
	VSFIP_EVT_TCP_CONNECTFAIL	= VSFSM_EVT_USER_LOCAL + 6,
	VSFIP_EVT_TCP_CLOSED		= VSFSM_EVT_USER_LOCAL + 7,
	VSFIP_EVT_TCP_TXOK			= VSFSM_EVT_USER_LOCAL + 8,
	VSFIP_EVT_TCP_RXOK			= VSFSM_EVT_USER_LOCAL + 9,
	VSFIP_EVT_TCP_RXFAIL		= VSFSM_EVT_USER_LOCAL + 10,

	// semaphore and timer use instant event, so that they can be exclusive
	VSFIP_EVT_SOCKET_TIMEOUT	= VSFSM_EVT_USER_LOCAL_INSTANT + 0,
	VSFIP_EVT_SOCKET_RECV		= VSFSM_EVT_USER_LOCAL_INSTANT + 1,
};

struct vsfip_t vsfip;

// socket buffer
static struct vsfip_socket_t* vsfip_socket_get(void)
{
	struct vsfip_socket_t *socket = vsfip.mem_op->get_socket();
	if (socket != NULL)
	{
		socket->family = AF_INET;
	}
	return socket;
}

static void vsfip_socket_release(struct vsfip_socket_t *socket)
{
	if (socket != NULL)
	{
		vsfip.mem_op->release_socket(socket);
	}
}

// tcppcb buffer
static struct vsfip_tcppcb_t* vsfip_tcppcb_get(void)
{
	struct vsfip_tcppcb_t *tcppcb = vsfip.mem_op->get_tcppcb();
	if (tcppcb != NULL)
	{
		tcppcb->state = VSFIP_TCPSTAT_CLOSED;
	}
	return tcppcb;
}

static void vsfip_tcppcb_release(struct vsfip_tcppcb_t *pcb)
{
	if (pcb != NULL)
	{
		vsfip.mem_op->release_tcppcb(pcb);
	}
}

// buffer
struct vsfip_buffer_t* vsfip_buffer_get(uint32_t size)
{
	struct vsfip_buffer_t *buffer = vsfip.mem_op->get_buffer(size);
	if (buffer != NULL)
	{
		buffer->ref = 1;
		buffer->buf.buffer = buffer->app.buffer = buffer->buffer;
		buffer->buf.size = buffer->app.size = size;
		buffer->proto_node.next = buffer->netif_node.next = NULL;
		buffer->netif = NULL;
	}
	return buffer;
}

struct vsfip_buffer_t* vsfip_appbuffer_get(uint32_t header, uint32_t app)
{
	struct vsfip_buffer_t *buf = vsfip_buffer_get(header + app);

	if (buf != NULL)
	{
		buf->app.buffer += header;
		buf->app.size -= header;
	}
	return buf;
}

void vsfip_buffer_reference(struct vsfip_buffer_t *buf)
{
	if (buf != NULL)
	{
		buf->ref++;
	}
}

void vsfip_buffer_release(struct vsfip_buffer_t *buf)
{
	if ((buf != NULL) && buf->ref)
	{
		buf->ref--;
		if (!buf->ref)
		{
			vsfip.mem_op->release_buffer(buf);
		}
	}
}

// bufferlist
static void vsfip_bufferlist_free(struct vsfq_t *list)
{
	struct vsfq_node_t *node;
	struct vsfip_buffer_t *tmpbuf;

	while (1)
	{
		node = vsfq_dequeue(list);
		tmpbuf = container_of(node, struct vsfip_buffer_t, proto_node);
		if (tmpbuf != NULL)
		{
			vsfip_buffer_release(tmpbuf);
		}
		else
		{
			break;
		}
	}
}

static uint32_t vsfip_bufferlist_len(struct vsfq_t *list)
{
	struct vsfq_node_t *node = list->head;
	struct vsfip_buffer_t *tmpbuf =
					container_of(node, struct vsfip_buffer_t, proto_node);
	uint32_t size = 0;

	while (tmpbuf != NULL)
	{
		size += tmpbuf->app.size;
		node = node->next;
		tmpbuf = container_of(node, struct vsfip_buffer_t, proto_node);
	}
	return size;
}

static void vsfip_socketlist_add(struct vsfip_socket_t **head,
									struct vsfip_socket_t *socket)
{
	if (*head != NULL)
	{
		socket->next = *head;
	}
	*head = socket;
}

static bool vsfip_socketlist_remove(struct vsfip_socket_t **head,
									struct vsfip_socket_t *socket)
{
	if (NULL == *head)
	{
		return false;
	}
	if (*head == socket)
	{
		*head = (*head)->next;
		return true;
	}
	else
	{
		struct vsfip_socket_t *tmp;

		tmp = *head;
		while (tmp != NULL)
		{
			if (tmp->next == socket)
			{
				tmp->next = socket->next;
				return true;
			}
			tmp = tmp->next;
		}
	}
	return false;
}

void vsfip_set_default_netif(struct vsfip_netif_t *netif)
{
	vsfip.netif_default = netif;
}

vsf_err_t vsfip_netif_add(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
							struct vsfip_netif_t *netif)
{
	vsf_err_t err;

	vsfsm_pt_begin(pt);

	if (!netif->ipaddr.size)
	{
		netif->ipaddr.size = 4;		// default IPv4
	}

	vsfip_netif_construct(netif);
	netif->init_pt.user_data = netif;
	netif->init_pt.sm = pt->sm;
	netif->init_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfip_netif_init(&netif->init_pt, evt);
	if (err != 0) return err;

	// add to netif list
	if (NULL == vsfip.netif_list)
	{
		// first netif is set to the default netif
		vsfip_set_default_netif(netif);
	}
	netif->next = vsfip.netif_list;
	vsfip.netif_list = netif;

	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

vsf_err_t vsfip_netif_remove(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
							struct vsfip_netif_t *netif)
{
	struct vsfip_netif_t *netif_tmp = vsfip.netif_list;

	vsfsm_pt_begin(pt);

	if (netif_tmp == netif)
	{
		vsfip.netif_list = netif->next;
		goto remove_netif;
	}

	while (netif_tmp != NULL)
	{
		if (netif_tmp->next == netif)
		{
			netif_tmp->next = netif->next;

		remove_netif:
			netif->init_pt.user_data = netif;
			netif->init_pt.sm = pt->sm;
			netif->init_pt.state = 0;
			vsfsm_pt_entry(pt);
			return vsfip_netif_fini(&netif->init_pt, evt);
		}
		netif_tmp = netif_tmp->next;
	}
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

struct vsfip_netif_t* vsfip_ip_route(struct vsfip_ipaddr_t *addr)
{
	// TODO:
	return vsfip.netif_default;
}

static void vsfip_check_buff(struct vsfip_socket_t *socket)
{
	struct vsfq_node_t *node = socket->inq.head;
	struct vsfip_buffer_t *buf =
					container_of(node, struct vsfip_buffer_t, proto_node);

	while (buf != NULL)
	{
		if (!buf->ttl || !--buf->ttl)
		{
			vsfq_remove(&socket->inq, &buf->proto_node);
			vsfip_buffer_release(buf);
		}
		node = node->next;
		buf = container_of(node, struct vsfip_buffer_t, proto_node);
	}
}

static void vsfip_tcp_socket_tick(struct vsfip_socket_t *socket);
struct vsfsm_state_t* vsfip_tick(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	switch (evt)
	{
	case VSFSM_EVT_INIT:
		vsfip.tick_timer.evt = VSFIP_EVT_TICK;
		vsfip.tick_timer.sm = sm;
		vsfip.tick_timer.interval = 1;
		vsfip.tick_timer.trigger_cnt = -1;
		vsftimer_enqueue(&vsfip.tick_timer);
		break;
	case VSFSM_EVT_FINI:
		vsftimer_dequeue(&vsfip.tick_timer);
		break;
	case VSFIP_EVT_TICK:
	{
		struct vsfip_socket_t *socket;
		struct vsfip_socket_t *child;

		// remove ip packet in reass if ttl reach 0

		// remove packet in input buf if ttl reach 0
		socket = vsfip.udpconns;
		while (socket != NULL)
		{
			vsfip_check_buff(socket);
			socket = socket->next;
		}
		socket = vsfip.tcpconns;
		while (socket != NULL)
		{
			child = socket->listener.child;
			while (child != NULL)
			{
				vsfip_check_buff(child);
				vsfip_tcp_socket_tick(child);
				child = child->next;
			}
			vsfip_check_buff(socket);
			vsfip_tcp_socket_tick(socket);
			socket = socket->next;
		}
	}
		break;
	}
	return NULL;
}

vsf_err_t vsfip_init(struct vsfip_mem_op_t *mem_op)
{
	memset(&vsfip, 0, sizeof(vsfip));
	vsfip.udp_port = VSFIP_CFG_UDP_PORT;
	vsfip.tcp_port = VSFIP_CFG_TCP_PORT;
	vsfip.mem_op = mem_op;

	vsfip.tick_sm.init_state.evt_handler = vsfip_tick;
	return vsfsm_init(&vsfip.tick_sm);
}

uint16_t vsfip_get_port(enum vsfip_sockproto_t proto)
{
	uint16_t port = 0;

	switch (proto)
	{
	case IPPROTO_TCP:
		port = vsfip.tcp_port++;
		break;
	case IPPROTO_UDP:
		port = vsfip.udp_port++;
		break;
	}
	return port;
}

vsf_err_t vsfip_fini(void)
{
	return vsfsm_post_evt(&vsfip.tick_sm, VSFSM_EVT_FINI);
}

static uint16_t vsfip_checksum(uint8_t *data, uint16_t len)
{
	uint32_t checksum = 0;

	while (len > 1)
	{
		checksum += GET_BE_U16(data);
		data += 2;
		len -= 2;
	}
	if (1 == len)
	{
		checksum += (uint16_t)(*data) << 8;
	}
	checksum = (checksum & 0xFFFF) + (checksum >> 16);
	checksum = (checksum & 0xFFFF) + (checksum >> 16);
	return (uint16_t)checksum;
}

static uint16_t vsfip_proto_checksum(struct vsfip_socket_t *socket,
										struct vsfip_buffer_t *buf)
{
	uint32_t checksum = vsfip_checksum(buf->buf.buffer, buf->buf.size);
	struct vsfip_ipaddr_t *local_addr = &buf->netif->ipaddr;
	struct vsfip_ipaddr_t *remote_addr = &socket->remote_sockaddr.sin_addr;

	checksum += GET_BE_U16(&local_addr->addr.s_addr_buf[0]);
	checksum += GET_BE_U16(&local_addr->addr.s_addr_buf[2]);
	checksum += GET_BE_U16(&remote_addr->addr.s_addr_buf[0]);
	checksum += GET_BE_U16(&remote_addr->addr.s_addr_buf[2]);
	checksum += socket->protocol;
	checksum += buf->buf.size;
	checksum = (checksum & 0xFFFF) + (checksum >> 16);
	checksum = (checksum & 0xFFFF) + (checksum >> 16);
	return checksum;
}

// ip
static bool vsfip_ip_ismatch(struct vsfip_ipaddr_t *addr1,
						struct vsfip_ipaddr_t *addr2)
{
	return (addr1->size == addr2->size) &&
		!memcmp(addr1->addr.s_addr_buf, addr2->addr.s_addr_buf, addr1->size);
}

static vsf_err_t vsfip_ip_output_do(struct vsfip_buffer_t *buf)
{
	if (vsfip.output_sniffer != NULL)
	{
		vsfip.output_sniffer(buf);
	}
	else
	{
		vsfip_netif_ip_output(buf);
	}
	return VSFERR_NONE;
}

static vsf_err_t vsfip_ip4_output(struct vsfip_socket_t *socket);
static vsf_err_t vsfip_ip6_output(struct vsfip_socket_t *socket);
static vsf_err_t vsfip_ip_output(struct vsfip_socket_t *socket)
{
	return (AF_INET == socket->family) ?
				vsfip_ip4_output(socket) : vsfip_ip6_output(socket);
}

// ipv4
static struct vsfip_buffer_t *vsfip_ip4_reass(struct vsfip_buffer_t *buf)
{
	// TODO: add ip_reass support
	if (buf->iphead.ip4head->offset & (VSFIP_IPH_MF | VSFIP_IPH_OFFSET_MASK))
	{
		vsfip_buffer_release(buf);
		return NULL;
	}

	return buf;
}

static bool vsfip_ip4_isbroadcast(struct vsfip_ipaddr_t *addr,
								struct vsfip_netif_t *netif)
{
	uint32_t addr32 = addr->addr.s_addr;
	uint32_t netmask32 = netif->netmask.addr.s_addr;
	uint32_t netaddr32 = netif->ipaddr.addr.s_addr & netmask32;

	return (addr32 == VSFIP_IPADDR_ANY) || (addr32 == ~VSFIP_IPADDR_ANY) ||
			(addr32 == (netaddr32 | (0xFFFFFFFF & ~netmask32)));
}

#if VSFIP_CFG_IPFORWORD_EN
static void vsfip_ip4_forward(struct vsfip_buffer_t *buf,
					struct vsfip_addr_t *addr, struct vsfip_netif_t *netif_in)
{
	struct vsfip_ip4head_t *iphead = (struct vsfip_ip4head_t *)buf->buf.buffer;
	uint32_t checksum;

	// TODO: skip for link-local address

	buf->netif = vsfip_ip_route(addr);
	if ((NULL == buf->netif) || (buf->netif == netif_in))
	{
		goto release_buf;
	}

	// dec ttl
	if (!--iphead->ttl)
	{
		goto release_buf;
	}

	// add checksum by 0x100(dec 1 in ttl)
	checksum = BE_TO_SYS_U16(iphead->checksum) + 0x100;
	checksum += checksum >> 16;
	iphead->checksum = SYS_TO_BE_U16(checksum);

	vsfip_ip_output_do(buf);
	return;
release_buf:
	vsfip_buffer_release(buf);
	return;
}
#endif

static void vsfip_udp_input(struct vsfip_buffer_t *buf);
static void vsfip_tcp_input(struct vsfip_buffer_t *buf);
static void vsfip_icmp_input(struct vsfip_buffer_t *buf);
void vsfip_ip4_input(struct vsfip_buffer_t *buf)
{
	struct vsfip_ip4head_t *iphead = (struct vsfip_ip4head_t *)buf->buf.buffer;
	struct vsfip_ipaddr_t ipaddr;
	uint16_t iph_hlen = VSFIP_IP4H_HLEN(iphead) * 4;

	// ip header check
	if (vsfip_checksum((uint8_t *)iphead, iph_hlen) != 0xFFFF)
	{
		goto release_buf;
	}

	ipaddr.size = 4;
	ipaddr.addr.s_addr = iphead->ipdest;
	// forward ip if not for us:
	// 		not boradcast and netif->ipaddr is valid and ipaddr not match
	if (!vsfip_ip4_isbroadcast(&ipaddr, buf->netif) &&
		(buf->netif->ipaddr.size > 0) &&
		!vsfip_ip_ismatch(&ipaddr, &buf->netif->ipaddr))
	{
#if VSFIP_CFG_IPFORWORD_EN
		vsfip_ip4_forward(buf, &ipaddr, netif);
#else
		goto release_buf;
#endif
	}
	buf->iphead.ip4head = iphead;

	// endian fix
	iphead->len = BE_TO_SYS_U16(iphead->len);
	iphead->id = BE_TO_SYS_U16(iphead->id);
	iphead->offset = BE_TO_SYS_U16(iphead->offset);
	iphead->checksum = BE_TO_SYS_U16(iphead->checksum);
	if ((VSFIP_IP4H_V(iphead) != 4) || (iph_hlen > iphead->len) ||
		(iphead->len > buf->buf.size))
	{
		goto release_buf;
	}
	buf->buf.size = iphead->len - iph_hlen;
	buf->buf.buffer += iph_hlen;

	// ip reassembly
	if (!(iphead->offset & VSFIP_IPH_DF))
	{
		buf = vsfip_ip4_reass(buf);
		if (NULL == buf)
		{
			return;
		}
	}

/*	if (is multicast)
	{
	}
	else
*/	{
		switch (iphead->proto)
		{
		case IPPROTO_UDP:
			vsfip_udp_input(buf);
			break;
		case IPPROTO_TCP:
			vsfip_tcp_input(buf);
			break;
		case IPPROTO_ICMP:
			vsfip_icmp_input(buf);
			break;
		default:
			vsfip_buffer_release(buf);
			break;
		}
	}
	return;
release_buf:
	vsfip_buffer_release(buf);
}

static vsf_err_t vsfip_add_ip4head(struct vsfip_socket_t *socket)
{
	struct vsfip_ippcb_t *pcb = &socket->pcb.ippcb;
	struct vsfip_buffer_t *buffer;
	struct vsfip_ip4head_t *ip4head;
	uint16_t offset;
	uint16_t checksum;

	if (pcb->buf->app.buffer - pcb->buf->buffer <
			sizeof(struct vsfip_ip4head_t))
	{
		return VSFERR_FAIL;
	}

	buffer = pcb->buf;
	buffer->buf.buffer -= sizeof(struct vsfip_ip4head_t);
	buffer->buf.size += sizeof(struct vsfip_ip4head_t);
	buffer->iphead.ip4head = (struct vsfip_ip4head_t *)buffer->buf.buffer;
	ip4head = pcb->buf->iphead.ip4head;
	ip4head->ver_hl = 0x45;		// IPv4 and 5 dwrod header len
	ip4head->tos = 0;
	ip4head->len = SYS_TO_BE_U16(pcb->buf->buf.size);
	ip4head->id = SYS_TO_BE_U16(pcb->id);
	ip4head->ttl = 32;
	ip4head->proto = (uint8_t)socket->protocol;
	offset = 0;
	offset += pcb->xfed_len;
	ip4head->offset = SYS_TO_BE_U16(offset);
	ip4head->checksum = SYS_TO_BE_U16(0);
	ip4head->ipsrc = buffer->netif->ipaddr.addr.s_addr;
	ip4head->ipdest = socket->remote_sockaddr.sin_addr.addr.s_addr;
	checksum = ~vsfip_checksum((uint8_t *)ip4head, sizeof(*ip4head));
	ip4head->checksum = SYS_TO_BE_U16(checksum);
	return VSFERR_NONE;
}

static vsf_err_t vsfip_ip4_output(struct vsfip_socket_t *socket)
{
	struct vsfip_ippcb_t *pcb = &socket->pcb.ippcb;
	vsf_err_t err = VSFERR_NONE;

	if (pcb->buf->buf.size >
			(pcb->buf->netif->mtu - sizeof(struct vsfip_ip4head_t)))
	{
		err = VSFERR_NOT_ENOUGH_RESOURCES;
		goto cleanup;
	}

	pcb->id = vsfip.ip_id++;
	err = vsfip_add_ip4head(socket);
	if (err) goto cleanup;

	return vsfip_ip_output_do(pcb->buf);
cleanup:
	vsfip_buffer_release(pcb->buf);
	return err;
}

// ipv6
void vsfip_ip6_input(struct vsfip_buffer_t *buf)
{
	vsfip_buffer_release(buf);
}

static vsf_err_t vsfip_ip6_output(struct vsfip_socket_t *socket)
{
	vsfip_buffer_release(socket->pcb.ippcb.buf);
	return VSFERR_NONE;
}

// socket layer
static void vsfip_tcp_socket_input(struct vsfip_socket_t *socket,
									struct vsfip_buffer_t *buf);
static void vsfip_udp_socket_input(struct vsfip_socket_t *socket,
									struct vsfip_buffer_t *buf);
struct vsfip_socket_t* vsfip_socket(enum vsfip_sockfamilt_t family,
									enum vsfip_sockproto_t protocol)
{
	struct vsfip_socket_t *socket = vsfip_socket_get();

	if (socket != NULL)
	{
		memset(socket, 0, sizeof(struct vsfip_socket_t));
		socket->family = family;
		socket->protocol = protocol;
		socket->tx_timer.evt = VSFIP_EVT_SOCKET_TIMEOUT;
		socket->rx_timer.evt = VSFIP_EVT_SOCKET_TIMEOUT;
		vsfq_init(&socket->inq);
		vsfq_init(&socket->outq);
		vsfsm_sem_init(&socket->input_sem, 0, VSFIP_EVT_SOCKET_RECV);

		if (IPPROTO_TCP == protocol)
		{
			socket->can_rx = true;
			socket->proto_callback.on_input = vsfip_tcp_socket_input;

			struct vsfip_tcppcb_t *pcb = vsfip_tcppcb_get();
			if (NULL == pcb)
			{
				vsfip_socket_release(socket);
				return NULL;
			}
			memset(pcb, 0, sizeof(struct vsfip_tcppcb_t));
			pcb->rclose = pcb->lclose = true;
			pcb->state = VSFIP_TCPSTAT_CLOSED;
			pcb->rx_window = VSFIP_CFG_TCP_RX_WINDOW;
			pcb->tx_window = VSFIP_CFG_TCP_TX_WINDOW;
			socket->pcb.protopcb = (struct vsfip_tcppcb_t *)pcb;
		}
		else if (IPPROTO_UDP == protocol)
		{
			socket->proto_callback.on_input = vsfip_udp_socket_input;
		}
	}

	return socket;
}

static struct vsfip_socket_t* vsfip_childsocket(
			struct vsfip_socket_t *father, struct vsfip_sockaddr_t *sockaddr)
{
	struct vsfip_socket_t *new_socket =
								vsfip_socket(father->family, father->protocol);

	if (new_socket != NULL)
	{
		new_socket->can_rx = father->can_rx;
		new_socket->tx_timeout_ms = father->tx_timeout_ms;
		new_socket->rx_timeout_ms = father->rx_timeout_ms;
		memcpy(&new_socket->remote_sockaddr, sockaddr,
					sizeof(struct vsfip_sockaddr_t));
		memcpy(&new_socket->local_sockaddr, &father->local_sockaddr,
					sizeof(struct vsfip_sockaddr_t));
		new_socket->father = father;
		vsfip_socketlist_add(&father->listener.child, new_socket);
	}
	return new_socket;
}

static vsf_err_t vsfip_free_socket(struct vsfip_socket_t *socket)
{
	struct vsfip_socket_t **list;

	if (socket->father != NULL)
	{
		list = &socket->father->listener.child;
	}
	else
	{
		switch (socket->protocol)
		{
		case IPPROTO_UDP:
			list = &vsfip.udpconns;
			break;
		case IPPROTO_TCP:
			list = &vsfip.tcpconns;
			break;
		}
	}
	vsfip_socketlist_remove(list, socket);

	// remove pending incoming buffer
	vsfip_bufferlist_free(&socket->inq);
	vsfip_bufferlist_free(&socket->outq);
	vsftimer_dequeue(&socket->tx_timer);
	vsftimer_dequeue(&socket->rx_timer);

	if ((IPPROTO_TCP == socket->protocol) && (socket->pcb.protopcb != NULL))
	{
		struct vsfip_tcppcb_t *tcppcb =
							(struct vsfip_tcppcb_t *)socket->pcb.protopcb;

		if ((VSFIP_TCPSTAT_LISTEN == tcppcb->state) && (tcppcb->rx_sm != NULL))
		{
			static void vsfip_tcp_postevt(struct vsfsm_t **sm, vsfsm_evt_t evt);
			vsfip_tcp_postevt(&tcppcb->rx_sm, VSFIP_EVT_TCP_CONNECTFAIL);
		}

		vsfip_tcppcb_release(tcppcb);
	}
	vsfip_socket_release(socket);
	return VSFERR_NONE;
}

vsf_err_t vsfip_close(struct vsfip_socket_t *socket)
{
	if ((socket->listener.backlog > 0) && (socket->listener.accepted_num > 0))
	{
		socket->listener.closing = true;
		return VSFERR_NONE;
	}

	if (socket->father != NULL)
	{
		struct vsfip_socket_t *father = socket->father;

		if (vsfip_socketlist_remove(&father->listener.child, socket))
		{
			father->listener.accepted_num--;
		}

		if (father->listener.closing && !father->listener.accepted_num)
		{
			vsfip_free_socket(father);
		}
	}
	return vsfip_free_socket(socket);
}

void vsfip_socket_cb(struct vsfip_socket_t *socket,
				void *param, void (*on_input)(void *, struct vsfip_buffer_t *),
				void (*on_outputted)(void *))
{
	socket->user_callback.param = param;
	socket->user_callback.on_input = on_input;
	socket->user_callback.on_outputted = on_outputted;
}

vsf_err_t vsfip_bind(struct vsfip_socket_t *socket, uint16_t port)
{
	struct vsfip_socket_t **head, *tmp;

	switch (socket->protocol)
	{
	case IPPROTO_UDP:
		head = &vsfip.udpconns;
		break;
	case IPPROTO_TCP:
		head = &vsfip.tcpconns;
		break;
	}

	// check if port already binded
	tmp = *head;
	while (tmp != NULL)
	{
		if (tmp->local_sockaddr.sin_port == port)
		{
			return VSFERR_FAIL;
		}
		tmp = tmp->next;
	}

	socket->next = *head;
	*head = socket;
	socket->local_sockaddr.sin_port = port;
	return VSFERR_NONE;
}

// for UDP port, vsfip_listen will enable rx for the socket
// for TCP port, vsfip_listen is same as listen socket API
vsf_err_t vsfip_listen(struct vsfip_socket_t *socket, uint8_t backlog)
{
	socket->can_rx = true;

	if (socket->protocol == IPPROTO_TCP)
	{
		struct vsfip_tcppcb_t *tcppcb = socket->pcb.protopcb;

		if (0 == backlog)
		{
			return VSFERR_INVALID_PARAMETER;
		}
		if (socket->listener.backlog > 0)
		{
			return VSFERR_FAIL;
		}

		socket->listener.backlog = backlog;
		tcppcb->state = VSFIP_TCPSTAT_LISTEN;
	}

	return VSFERR_NONE;
}

// proto common
// TODO: fix for IPv6
static void vsfip_proto_input(struct vsfip_socket_t *list,
				struct vsfip_buffer_t *buf, struct vsfip_protoport_t *port)
{
	struct vsfip_ip4head_t *iphead = buf->iphead.ip4head;
	struct vsfip_socket_t *socket = list, *child;
	struct vsfip_sockaddr_t *remote_addr, *local_addr;
	uint32_t remote_ipaddr = iphead->ipsrc;

	// find the socket
	while (socket != NULL)
	{
		local_addr = &socket->local_sockaddr;
		// if port match and
		// 		local addr is ADDR_ANY or local addr match ipdest in iphead
		if ((local_addr->sin_port == port->dst) &&
			(!local_addr->sin_addr.addr.s_addr ||
				(local_addr->sin_addr.addr.s_addr == iphead->ipdest)))
		{
			if (socket->listener.backlog == 0)
			{
				// normal socket
				remote_addr = &socket->remote_sockaddr;
				if (((remote_addr->sin_addr.addr.s_addr == VSFIP_IPADDR_ANY) ||
						(remote_addr->sin_addr.addr.s_addr == remote_ipaddr)) &&
					((0 == remote_addr->sin_port) ||
						(remote_addr->sin_port == port->src)))
				{
					break;
				}
			}
			else
			{
				// listener socket
				child = socket->listener.child;
				while (child != NULL)
				{
					remote_addr = &child->remote_sockaddr;
					if ((remote_addr->sin_addr.addr.s_addr == remote_ipaddr) &&
						(remote_addr->sin_port == port->src))
					{
						socket = child;
						break;
					}
					child = child->next;
				}
				break;
			}
		}
		socket = socket->next;
	}

	if ((NULL == socket) || !socket->can_rx)
	{
		goto discard_buffer;
	}

	if (socket->proto_callback.on_input != NULL)
	{
		socket->proto_callback.on_input(socket, buf);
		return;
	}
discard_buffer:
	vsfip_buffer_release(buf);
	return;
}

// UDP
static void vsfip_udp_socket_input(struct vsfip_socket_t *socket,
									struct vsfip_buffer_t *buf)
{
	if (socket->user_callback.on_input != NULL)
	{
		socket->user_callback.on_input(socket->user_callback.param, buf);
	}
	else
	{
		buf->ttl = VSFIP_CFG_TTL_INPUT;
		vsfq_append(&socket->inq, &buf->proto_node);
		vsfsm_sem_post(&socket->input_sem);
	}
}

static void vsfip_udp_input(struct vsfip_buffer_t *buf)
{
	struct vsfip_udphead_t *udphead = (struct vsfip_udphead_t *)buf->buf.buffer;

	// endian fix
	udphead->port.src = BE_TO_SYS_U16(udphead->port.src);
	udphead->port.dst = BE_TO_SYS_U16(udphead->port.dst);
	udphead->len = BE_TO_SYS_U16(udphead->len);
	udphead->checksum = BE_TO_SYS_U16(udphead->checksum);

	buf->app.buffer = buf->buf.buffer + sizeof(struct vsfip_udphead_t);
	buf->app.size = buf->buf.size - sizeof(struct vsfip_udphead_t);
	vsfip_proto_input(vsfip.udpconns, buf, &udphead->port);
}

static vsf_err_t vsfip_add_udphead(struct vsfip_socket_t *socket,
									struct vsfip_buffer_t *buf)
{
	struct vsfip_udphead_t *udphead;

	if (buf->app.buffer - buf->buffer < sizeof(struct vsfip_udphead_t))
	{
		return VSFERR_NOT_ENOUGH_RESOURCES;
	}

	buf->buf.buffer = buf->app.buffer - sizeof(struct vsfip_udphead_t);
	buf->buf.size = buf->app.size + sizeof(struct vsfip_udphead_t);
	udphead = (struct vsfip_udphead_t *)buf->buf.buffer;
	udphead->port.src = SYS_TO_BE_U16(socket->local_sockaddr.sin_port);
	udphead->port.dst = SYS_TO_BE_U16(socket->remote_sockaddr.sin_port);
	udphead->len = SYS_TO_BE_U16(buf->buf.size);
	// no checksum
	udphead->checksum = SYS_TO_BE_U16(0x0000);
	return VSFERR_NONE;
}

vsf_err_t vsfip_udp_async_send(struct vsfip_socket_t *socket,
			struct vsfip_sockaddr_t *sockaddr, struct vsfip_buffer_t *buf)
{
	vsf_err_t err = VSFERR_NONE;

	if ((NULL == socket) || (NULL == sockaddr) || (NULL == buf))
	{
		err = VSFERR_INVALID_PARAMETER;
		goto cleanup;
	}
	socket->remote_sockaddr = *sockaddr;

	buf->netif = vsfip_ip_route(&sockaddr->sin_addr);
	if (NULL == buf->netif)
	{
		err = VSFERR_FAIL;
		goto cleanup;
	}

	// allocate local port if not set
	if ((0 == socket->local_sockaddr.sin_port) &&
		vsfip_bind(socket, vsfip_get_port(IPPROTO_UDP)))
	{
		err = VSFERR_FAIL;
		goto cleanup;
	}

	// add udp header
	err = vsfip_add_udphead(socket, buf);
	if (err) goto cleanup;

	socket->pcb.ippcb.buf = buf;
	err = vsfip_ip_output(socket);
	if (!err && (socket->user_callback.on_outputted != NULL))
	{
		socket->user_callback.on_outputted(socket->user_callback.param);
	}
	return err;
cleanup:
	vsfip_buffer_release(buf);
	return err;
}

vsf_err_t vsfip_udp_send(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
			struct vsfip_socket_t *socket, struct vsfip_sockaddr_t *sockaddr,
			struct vsfip_buffer_t *buf)
{
	return vsfip_udp_async_send(socket, sockaddr, buf);
}

// TODO: fix for IPv6
static struct vsfip_buffer_t *
vsfip_udp_match(struct vsfip_socket_t *socket, struct vsfip_sockaddr_t *addr)
{
	struct vsfq_node_t *node = socket->inq.head;
	struct vsfip_buffer_t *buf =
					container_of(node, struct vsfip_buffer_t, proto_node);
	struct vsfip_ip4head_t *iphead;

	while (buf != NULL)
	{
		iphead = (struct vsfip_ip4head_t *)buf->iphead.ip4head;
		if (VSFIP_IPADDR_ANY == addr->sin_addr.addr.s_addr)
		{
			addr->sin_addr.addr.s_addr = iphead->ipsrc;
		}
		if (addr->sin_addr.addr.s_addr == iphead->ipsrc)
		{
			vsfq_remove(&socket->inq, &buf->proto_node);
			buf->app.buffer = buf->buf.buffer + sizeof(struct vsfip_udphead_t);
			buf->app.size = buf->buf.size - sizeof(struct vsfip_udphead_t);
			break;
		}
		node = node->next;
		buf = container_of(node, struct vsfip_buffer_t, proto_node);
	}

	return buf;
}

vsf_err_t vsfip_udp_async_recv(struct vsfip_socket_t *socket,
		struct vsfip_sockaddr_t *sockaddr, struct vsfip_buffer_t **buf)
{
	if ((NULL == socket) || (NULL == sockaddr) || (NULL == buf) ||
		!socket->local_sockaddr.sin_port)
	{
		return VSFERR_INVALID_PARAMETER;
	}
	socket->remote_sockaddr = *sockaddr;
	*buf = vsfip_udp_match(socket, sockaddr);
	return *buf ? VSFERR_NONE : VSFERR_NOT_READY;
}

vsf_err_t vsfip_udp_recv(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
			struct vsfip_socket_t *socket, struct vsfip_sockaddr_t *sockaddr,
			struct vsfip_buffer_t **buf)
{
	vsf_err_t err;

	vsfsm_pt_begin(pt);

	err = vsfip_udp_async_recv(socket, sockaddr, buf);
	if (err <= 0) return err;

	// set pending with timeout
	// TODO: implement pcb for UDP and put these resources in
	if (socket->rx_timeout_ms)
	{
		socket->rx_timer.interval = socket->rx_timeout_ms;
		socket->rx_timer.sm = pt->sm;
		socket->rx_timer.evt = VSFIP_EVT_SOCKET_TIMEOUT;
		socket->rx_timer.trigger_cnt = 1;
		vsftimer_enqueue(&socket->rx_timer);
	}

	while (1)
	{
		if (vsfsm_sem_pend(&socket->input_sem, pt->sm))
		{
			evt = VSFSM_EVT_NONE;
			vsfsm_pt_entry(pt);
			if ((evt != VSFIP_EVT_SOCKET_RECV) &&
				(evt != VSFIP_EVT_SOCKET_TIMEOUT))
			{
				return VSFERR_NOT_READY;
			}

			if (VSFIP_EVT_SOCKET_TIMEOUT == evt)
			{
				vsftimer_dequeue(&socket->rx_timer);
				vsfsm_sync_cancel(&socket->input_sem, pt->sm);
				return VSFERR_FAIL;
			}
		}

		err = vsfip_udp_async_recv(socket, sockaddr, buf);
		if (!err)
		{
			vsftimer_dequeue(&socket->rx_timer);
			return VSFERR_NONE;
		}
	}

	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

// TCP
#define VSFIP_TCPFLAG_FIN		(uint16_t)(1 << 0)
#define VSFIP_TCPFLAG_SYN		(uint16_t)(1 << 1)
#define VSFIP_TCPFLAG_RST		(uint16_t)(1 << 2)
#define VSFIP_TCPFLAG_PSH		(uint16_t)(1 << 3)
#define VSFIP_TCPFLAG_ACK		(uint16_t)(1 << 4)
#define VSFIP_TCPFLAG_URG		(uint16_t)(1 << 5)

#define VSFIP_TCPOPT_MSS		0x02
#define VSFIP_TCPOPT_MSS_LEN	0x04

static uint32_t vsfip_get_tsn(void)
{
	uint32_t tsn = vsfip.tsn;
	vsfip.tsn += 0x10000;
	return tsn;
}

static void vsfip_tcp_input(struct vsfip_buffer_t *buf)
{
	struct vsfip_tcphead_t *tcphead = (struct vsfip_tcphead_t *)buf->buf.buffer;

	// endian fix
	tcphead->port.src = BE_TO_SYS_U16(tcphead->port.src);
	tcphead->port.dst = BE_TO_SYS_U16(tcphead->port.dst);
	tcphead->seq = BE_TO_SYS_U32(tcphead->seq);
	tcphead->ackseq = BE_TO_SYS_U32(tcphead->ackseq);
	tcphead->headlen = (tcphead->headlen >> 4) << 2;
	tcphead->window_size = BE_TO_SYS_U16(tcphead->window_size);
	tcphead->checksum = BE_TO_SYS_U16(tcphead->checksum);
	tcphead->urgent_ptr = BE_TO_SYS_U16(tcphead->urgent_ptr);

	buf->proto_node.addr = tcphead->seq;
	buf->app.buffer = buf->buf.buffer + tcphead->headlen;
	buf->app.size = buf->buf.size - tcphead->headlen;
	vsfip_proto_input(vsfip.tcpconns, buf,&tcphead->port);
}

static vsf_err_t vsfip_add_tcphead(struct vsfip_socket_t *socket,
									struct vsfip_buffer_t *buf, uint8_t flags)
{
	uint16_t window_size;
	struct vsfip_tcppcb_t *pcb = (struct vsfip_tcppcb_t *)socket->pcb.protopcb;
	struct vsfip_tcphead_t *head;
	uint8_t *option;
	uint8_t headlen = sizeof(struct vsfip_tcphead_t);

	if (flags & VSFIP_TCPFLAG_SYN)
	{
		headlen += 4;
	}
	if (buf->app.buffer - buf->buffer < headlen)
	{
		return VSFERR_NOT_ENOUGH_RESOURCES;
	}

	buf->buf.buffer = buf->app.buffer - headlen;
	buf->buf.size = buf->app.size + headlen;

	head = (struct vsfip_tcphead_t *)buf->buf.buffer;
	option = (uint8_t *)head + sizeof(struct vsfip_tcphead_t);
	memset(head, 0, sizeof(struct vsfip_tcphead_t));
	head->port.src = SYS_TO_BE_U16(socket->local_sockaddr.sin_port);
	head->port.dst = SYS_TO_BE_U16(socket->remote_sockaddr.sin_port);
	head->seq = SYS_TO_BE_U32(pcb->lseq);
	buf->proto_node.addr = pcb->lseq;
	head->ackseq = SYS_TO_BE_U32(pcb->rseq);
	head->headlen = (headlen >> 2) << 4;
	flags |= (pcb->rseq != 0) ? VSFIP_TCPFLAG_ACK : 0;
	head->flags = flags;

	window_size = pcb->rx_window - vsfip_bufferlist_len(&socket->inq);
	head->window_size = SYS_TO_BE_U16(window_size);

	head->checksum = SYS_TO_BE_U16(0);
	head->urgent_ptr = SYS_TO_BE_U16(0);

	if (flags & VSFIP_TCPFLAG_SYN)
	{
		// add MSS
		*option++ = VSFIP_TCPOPT_MSS;
		*option++ = VSFIP_TCPOPT_MSS_LEN;
		SET_BE_U16(option, VSFIP_CFG_TCP_MSS);
	}

	pcb->acked_rseq = pcb->rseq;
	return VSFERR_NONE;
}

static void vsfip_add_tcpchecksum(struct vsfip_socket_t *socket,
									struct vsfip_buffer_t *buf)
{
	uint16_t checksum = ~vsfip_proto_checksum(socket, buf);
	struct vsfip_tcphead_t *head = (struct vsfip_tcphead_t *)buf->buf.buffer;

	head->checksum = SYS_TO_BE_U16(checksum);
}

static vsf_err_t vsfip_tcp_send_do(struct vsfip_socket_t *socket,
									struct vsfip_buffer_t *buf, uint8_t flags)
{
	struct vsfip_tcppcb_t *pcb = (struct vsfip_tcppcb_t *)socket->pcb.protopcb;

	if (vsfip_add_tcphead(socket, buf, flags) < 0)
	{
		vsfip_buffer_release(buf);
		return VSFERR_FAIL;
	}
	vsfip_add_tcpchecksum(socket, buf);
	socket->pcb.ippcb.buf = buf;
	vsfip_ip_output(socket);

	if ((flags & (VSFIP_TCPFLAG_FIN | VSFIP_TCPFLAG_SYN)) ||
		(buf->app.size > 0))
	{
		pcb->flags = flags;
	}
	pcb->lseq += buf->app.size;
	return VSFERR_NONE;
}

static vsf_err_t vsfip_tcp_sendflags(struct vsfip_socket_t *socket,
										uint8_t flags)
{
	struct vsfip_tcppcb_t *pcb = (struct vsfip_tcppcb_t *)socket->pcb.protopcb;
	struct vsfip_buffer_t *buffer;

	// even if buffer not available, also need timeout to resend
	if (flags & (VSFIP_TCPFLAG_SYN | VSFIP_TCPFLAG_FIN))
	{
		pcb->tx_timeout_ms = socket->tx_timeout_ms;
	}

	buffer = VSFIP_TCPBUF_GET(0);
	if (NULL == buffer)
	{
		return VSFERR_NOT_ENOUGH_RESOURCES;
	}
	buffer->netif = vsfip_ip_route(&socket->remote_sockaddr.sin_addr);
	if (NULL == buffer->netif)
	{
		return VSFERR_FAIL;
	}
	return vsfip_tcp_send_do(socket, buffer, flags);
}

static void vsfip_tcp_postevt(struct vsfsm_t **sm, vsfsm_evt_t evt)
{
	struct vsfsm_t *sm_tmp;

	if ((sm != NULL) && (*sm != NULL))
	{
		sm_tmp = *sm;
		*sm = NULL;
		vsfsm_post_evt(sm_tmp, evt);
	}
}

static void vsfip_tcp_socket_tick(struct vsfip_socket_t *socket)
{
	struct vsfip_tcppcb_t *pcb = (struct vsfip_tcppcb_t *)socket->pcb.protopcb;
	struct vsfq_node_t *node;
	struct vsfip_buffer_t *buf;

	if (pcb->tx_timeout_ms > 0)
	{
		pcb->tx_timeout_ms--;
		if (0 == pcb->tx_timeout_ms)
		{
			if (--pcb->tx_retry)
			{
				pcb->lseq = pcb->acked_lseq;
				vsfip_tcp_sendflags(socket, pcb->flags);
				if (pcb->flags & (VSFIP_TCPFLAG_SYN | VSFIP_TCPFLAG_FIN))
				{
					pcb->lseq++;
				}
				goto check_rx_to;
			}

			// timer out
			if (socket->father && (pcb->flags & VSFIP_TCPFLAG_SYN))
			{
				// child socket fail to connect, no need to send event
				vsfip_close(socket);
				goto check_rx_to;
			}

			switch (pcb->state)
			{
			case VSFIP_TCPSTAT_SYN_SENT:
			case VSFIP_TCPSTAT_SYN_GET:
			case VSFIP_TCPSTAT_LASTACK:
				pcb->state = VSFIP_TCPSTAT_CLOSED;
			}
			vsfip_tcp_postevt(&pcb->tx_sm, VSFIP_EVT_SOCKET_TIMEOUT);
			return;
		}
	}

check_rx_to:
	if (pcb->rx_timeout_ms > 0)
	{
		pcb->rx_timeout_ms--;
		if (0 == pcb->rx_timeout_ms)
		{
			vsfip_tcp_postevt(&pcb->rx_sm, VSFIP_EVT_SOCKET_TIMEOUT);
			return;
		}
	}

	node = socket->outq.head;
	buf = container_of(node, struct vsfip_buffer_t, proto_node);
	while (buf != NULL)
	{
		if (!--buf->ttl)
		{
			// if buf->retry is 0
			// 		always retry
			// if passive close(remote sent FIN, VSFIP_TCPSTAT_CLOSEWAIT mode)
			// if timer_out and retry count reach 0
			// 		free bufferlist
			if ((!buf->retry || --buf->retry) &&
				(pcb->state != VSFIP_TCPSTAT_CLOSEWAIT))
			{
				uint32_t lseq = pcb->lseq;
				pcb->lseq = buf->proto_node.addr;
				vsfip_buffer_reference(buf);
				buf->ttl = socket->tx_timeout_ms;
				vsfip_tcp_send_do(socket, buf, VSFIP_TCPFLAG_PSH);
				pcb->lseq = lseq;
			}
			else
			{
				// flush bufferlist
				vsfip_bufferlist_free(&socket->outq);
				pcb->lseq = pcb->acked_lseq;

				// TODO: how to fix if tcp packet time out?
				pcb->tx_retry = 3;
				vsfip_tcp_sendflags(socket, VSFIP_TCPFLAG_FIN);
				pcb->lseq++;
				pcb->state = pcb->rclose ?
					VSFIP_TCPSTAT_LASTACK : VSFIP_TCPSTAT_FINWAIT1;
				pcb->lclose = true;

				vsfip_tcp_postevt(&pcb->tx_sm, VSFIP_EVT_SOCKET_TIMEOUT);
				return;
			}
		}
		node = node->next;
		buf = container_of(node, struct vsfip_buffer_t, proto_node);
	}

	if ((pcb->acked_rseq != pcb->rseq) && !--pcb->ack_tick)
	{
		vsfip_tcp_sendflags(socket, VSFIP_TCPFLAG_ACK);
		pcb->ack_tick = VSFIP_TCP_ATO;
	}
}

static void vsfip_tcp_data_input(struct vsfip_socket_t *socket,
									struct vsfip_buffer_t *buf)
{
	struct vsfip_tcppcb_t *pcb = (struct vsfip_tcppcb_t *)socket->pcb.protopcb;

	pcb->rseq += buf->app.size;
	pcb->ack_tick = VSFIP_TCP_ATO;
	if (socket->user_callback.on_input != NULL)
	{
		socket->user_callback.on_input(socket->user_callback.param, buf);
	}
	else
	{
		buf->ttl = VSFIP_CFG_TTL_INPUT;
		vsfq_append(&socket->inq, &buf->proto_node);
	}
}

static void vsfip_tcp_socket_input(struct vsfip_socket_t *socket,
									struct vsfip_buffer_t *buf)
{
	struct vsfip_tcppcb_t *pcb = (struct vsfip_tcppcb_t *)socket->pcb.protopcb;
	struct vsfip_tcphead_t head = *(struct vsfip_tcphead_t *)buf->buf.buffer;
	bool release_buffer = true;
	enum vsfip_tcp_stat_t pre_state = pcb->state;

	if ((socket->listener.backlog > 0) && (head.flags != VSFIP_TCPFLAG_SYN))
	{
		// for listern socket, can only accept SYN packet
		// maybe some child socket fails and close, but remote still send packet
		// the packet will be directed to the listener socket
		vsfip_buffer_release(buf);
		return;
	}

	pcb->rwnd = head.window_size;
	if (head.flags & VSFIP_TCPFLAG_RST)
	{
		vsfip_buffer_release(buf);
		pcb->tx_timeout_ms = 0;
		pcb->rx_timeout_ms = 0;
		pcb->state = VSFIP_TCPSTAT_CLOSED;
		vsfip_bufferlist_free(&socket->inq);
		vsfip_bufferlist_free(&socket->outq);

		vsfip_tcp_postevt(&pcb->tx_sm, VSFIP_EVT_SOCKET_TIMEOUT);
		vsfip_tcp_postevt(&pcb->rx_sm, VSFIP_EVT_SOCKET_TIMEOUT);
		return;
	}

re_process:
	switch (pcb->state)
	{
	case VSFIP_TCPSTAT_INVALID:
		goto release_buf;
	case VSFIP_TCPSTAT_CLOSED:
		vsfip_tcp_sendflags(socket, VSFIP_TCPFLAG_RST);
		goto release_buf;
	case VSFIP_TCPSTAT_LISTEN:
		if ((VSFIP_TCPFLAG_SYN == head.flags) &&
			(socket->listener.accepted_num < socket->listener.backlog) &&
			!socket->listener.closing)
		{
			struct vsfip_sockaddr_t sockaddr;
			struct vsfip_ip4head_t *iphead = buf->iphead.ip4head;
			struct vsfip_socket_t *new_socket;
			struct vsfip_tcppcb_t *new_pcb;

			sockaddr.sin_port = head.port.src;
			sockaddr.sin_addr.addr.s_addr = iphead->ipsrc;
			sockaddr.sin_addr.size = 4;

			//new a socket to process the data
			new_socket = vsfip_childsocket(socket, &sockaddr);
			if (NULL == new_socket)
			{
				goto release_buf;
			}
			socket->listener.accepted_num++;

			new_pcb = (struct vsfip_tcppcb_t *)new_socket->pcb.protopcb;
			new_pcb->lseq = new_pcb->acked_lseq = vsfip_get_tsn();
			new_pcb->rseq = 0;
			new_pcb->state = VSFIP_TCPSTAT_SYN_GET;
			new_pcb->rseq = head.seq + 1;
			new_pcb->acked_lseq = new_pcb->lseq;
			// tx_sm is notifier to vsfip_tcp_accept
			// in vsfip_tcp_accept, tx_sm of the listener socket will be cleared
			new_pcb->tx_sm = pcb->tx_sm;

			new_pcb->tx_retry = 3;
			vsfip_tcp_sendflags(new_socket,
								VSFIP_TCPFLAG_SYN | VSFIP_TCPFLAG_ACK);
			new_pcb->lseq++;
			new_pcb->rclose = false;
		}
		goto release_buf;
		break;
	case VSFIP_TCPSTAT_SYN_SENT:
		if (VSFIP_TCPFLAG_SYN == head.flags)
		{
			// simultaneous open
			pcb->rseq = head.seq + 1;

			pcb->tx_retry = 3;
			vsfip_tcp_sendflags(socket, VSFIP_TCPFLAG_SYN | VSFIP_TCPFLAG_ACK);
			pcb->lseq++;
			pcb->state = VSFIP_TCPSTAT_SYN_GET;
		}
		else if ((head.flags & (VSFIP_TCPFLAG_SYN | VSFIP_TCPFLAG_ACK)) &&
					(head.ackseq == pcb->lseq))
		{
			pcb->tx_timeout_ms = 0;

			// send ACK
			pcb->rseq = head.seq + 1;
			pcb->acked_lseq = pcb->lseq;

			vsfip_tcp_sendflags(socket, VSFIP_TCPFLAG_ACK);
		connected:
			pcb->rclose = pcb->lclose = false;
			pcb->state = VSFIP_TCPSTAT_ESTABLISHED;
			if (release_buffer)
			{
				vsfip_buffer_release(buf);
			}
			vsfip_tcp_postevt(&pcb->tx_sm, VSFIP_EVT_TCP_CONNECTOK);
			return;
		}
		goto release_buf;
		break;
	case VSFIP_TCPSTAT_SYN_GET:
		if (VSFIP_TCPFLAG_SYN == head.flags)
		{
			// get resend SYN(previous SYN + ACK lost), reply again
			pcb->lseq = pcb->acked_lseq;
			pcb->tx_retry = 3;
			vsfip_tcp_sendflags(socket, VSFIP_TCPFLAG_SYN | VSFIP_TCPFLAG_ACK);
			pcb->lseq++;
		}
		else if ((head.flags & VSFIP_TCPFLAG_ACK) &&
					(head.ackseq == (pcb->acked_lseq + 1)))
		{
			pcb->tx_timeout_ms = 0;
			pcb->acked_lseq = pcb->lseq = head.ackseq;
			if (buf->app.size > 0)
			{
				if (pcb->rseq == head.seq)
				{
					release_buffer = false;
					vsfip_tcp_data_input(socket, buf);
				}
				else
				{
					// send the ACK with the latest valid seq to remote
					vsfip_tcp_sendflags(socket, VSFIP_TCPFLAG_ACK);
				}
			}
			goto connected;
		}
		else if (VSFIP_TCPFLAG_FIN & head.flags)
		{
			pcb->tx_retry = 3;
			vsfip_tcp_sendflags(socket, VSFIP_TCPFLAG_FIN);
			pcb->lseq++;
			pcb->state = VSFIP_TCPSTAT_FINWAIT1;
			pcb->lclose = true;
		}
		goto release_buf;
	case VSFIP_TCPSTAT_ESTABLISHED:
		if (buf->app.size > 0)
		{
			if (pcb->rseq == head.seq)
			{
				release_buffer = false;
				vsfip_tcp_data_input(socket, buf);
				vsfip_tcp_postevt(&pcb->rx_sm, VSFIP_EVT_TCP_RXOK);
				if (pcb->state != pre_state)
				{
					goto re_process;
				}
			}
			else
			{
				vsfip_tcp_sendflags(socket, VSFIP_TCPFLAG_ACK);
			}
		}
	case VSFIP_TCPSTAT_CLOSEWAIT:
		if ((head.flags & VSFIP_TCPFLAG_ACK) &&
			(head.ackseq != pcb->acked_lseq))
		{
			// ACK received
			struct vsfq_node_t *node;
			struct vsfip_buffer_t *buf;

			pcb->tx_timeout_ms = 0;
			pcb->acked_lseq = head.ackseq;

			node = socket->outq.head;
			buf = container_of(node, struct vsfip_buffer_t, proto_node);
			while (buf != NULL)
			{
				if (pcb->acked_lseq >= (buf->proto_node.addr + buf->app.size))
				{
					vsfq_dequeue(&socket->outq);
					if (socket->user_callback.on_outputted != NULL)
					{
						socket->user_callback.on_outputted(
												socket->user_callback.param);
					}
					vsfip_buffer_release(buf);
					node = socket->outq.head;
					buf = container_of(node, struct vsfip_buffer_t, proto_node);
				}
				else
				{
					break;
				}
			}
			vsfip_tcp_postevt(&pcb->tx_sm, VSFIP_EVT_TCP_TXOK);
			if (pcb->state != pre_state)
			{
				goto re_process;
			}
		}

		if (head.flags & VSFIP_TCPFLAG_FIN)
		{
			// passive close
		remote_close:
			// clear FIN, in case next state is CLOSEWAIT, and process again
			head.flags &= ~VSFIP_TCPFLAG_FIN;
			pcb->rclose = true;
			pcb->rseq = head.seq + 1;
			vsfip_tcp_sendflags(socket, VSFIP_TCPFLAG_ACK);

			// remote close, when local socket is closed
			// set the pcb->state = VSFIP_TCPSTAT_LASTACK
			pre_state = pcb->state = pcb->lclose ?
					VSFIP_TCPSTAT_CLOSED : VSFIP_TCPSTAT_CLOSEWAIT;
			vsfip_tcp_postevt(&pcb->rx_sm, VSFIP_EVT_TCP_RXFAIL);
			if (pcb->state != pre_state)
			{
				goto re_process;
			}
			if (VSFIP_TCPSTAT_CLOSED == pcb->state)
			{
				vsfip_tcp_postevt(&pcb->tx_sm, VSFIP_EVT_TCP_CLOSED);
			}
		}

		if (release_buffer)
		{
			goto release_buf;
		}
		break;
	case VSFIP_TCPSTAT_FINWAIT1:
		// local close, but still can receive data from remote
		if ((head.flags & VSFIP_TCPFLAG_ACK) && (head.ackseq == pcb->lseq))
		{
			pcb->lclose = true;
			pcb->tx_timeout_ms = 0;
			pcb->acked_lseq = pcb->lseq;
			if (head.flags & VSFIP_TCPFLAG_FIN)
			{
				goto remote_close;
			}
			pcb->state = VSFIP_TCPSTAT_FINWAIT2;
		}
		goto check_data_in_FIN;
	case VSFIP_TCPSTAT_FINWAIT2:
		if (head.flags & VSFIP_TCPFLAG_FIN)
		{
			goto remote_close;
		}
	check_data_in_FIN:
		if (buf->app.size > 0)
		{
			if (pcb->rseq == head.seq)
			{
				pcb->rseq += buf->app.size;
			}
			vsfip_tcp_sendflags(socket, VSFIP_TCPFLAG_ACK);
		}
		goto release_buf;
	case VSFIP_TCPSTAT_LASTACK:
		// maybe receive FIN again because previous ACK is lost
		if (head.flags & VSFIP_TCPFLAG_FIN)
		{
			vsfip_tcp_sendflags(socket, VSFIP_TCPFLAG_ACK);
		}
		if ((VSFIP_TCPFLAG_ACK == head.flags) && (head.ackseq == pcb->lseq))
		{
			pcb->lclose = true;
			pcb->tx_timeout_ms = 0;
			pcb->acked_lseq = pcb->lseq;
			pcb->state = VSFIP_TCPSTAT_CLOSED;
			vsfip_buffer_release(buf);
			vsfip_tcp_postevt(&pcb->tx_sm, VSFIP_EVT_TCP_CLOSED);
		}
		else
		{
			goto release_buf;
		}
		break;
	default:
	release_buf:
		vsfip_buffer_release(buf);
		break;
	}
}

void vsfip_tcp_config_window(struct vsfip_socket_t *socket, uint32_t rx_window,
		uint32_t tx_window)
{
	struct vsfip_tcppcb_t *pcb = (struct vsfip_tcppcb_t *)socket->pcb.protopcb;
	pcb->rx_window = rx_window;
	pcb->tx_window = tx_window;
}

vsf_err_t vsfip_tcp_connect(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
			struct vsfip_socket_t *socket, struct vsfip_sockaddr_t *sockaddr)
{
	struct vsfip_tcppcb_t *pcb = (struct vsfip_tcppcb_t *)socket->pcb.protopcb;

	vsfsm_pt_begin(pt);

	if ((NULL == socket) || (NULL == sockaddr) || (NULL == pcb) ||
		(pcb->state != VSFIP_TCPSTAT_CLOSED))
	{
		return VSFERR_INVALID_PARAMETER;
	}
	if (pcb->tx_sm != NULL)
	{
		return VSFERR_BUG;
	}
	socket->remote_sockaddr = *sockaddr;

	// allocate local port if not set
	if ((0 == socket->local_sockaddr.sin_port) &&
		vsfip_bind(socket, vsfip_get_port(IPPROTO_TCP)))
	{
		return VSFERR_NONE;
	}

	pcb->lseq = pcb->acked_lseq = vsfip_get_tsn();
	pcb->rseq = 0;

	pcb->tx_retry = 3;
	vsfip_tcp_sendflags(socket, VSFIP_TCPFLAG_SYN);
	pcb->lseq++;
	pcb->state = VSFIP_TCPSTAT_SYN_SENT;

	pcb->tx_sm = pt->sm;
	evt = VSFSM_EVT_NONE;
	vsfsm_pt_entry(pt);
	if ((evt != VSFIP_EVT_TCP_CONNECTOK) &&
		(evt != VSFIP_EVT_TCP_CONNECTFAIL) && (evt != VSFIP_EVT_SOCKET_TIMEOUT))
	{
		return VSFERR_NOT_READY;
	}

	pcb->lseq = pcb->acked_lseq;
	return (VSFIP_EVT_TCP_CONNECTOK == evt) ? VSFERR_NONE : VSFERR_FAIL;

	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static struct vsfip_socket_t*
vsfip_tcp_accept_try(struct vsfip_socket_t *socket)
{
	struct vsfip_tcppcb_t *pcb;

	socket = socket->listener.child;
	while (socket != NULL)
	{
		pcb = (struct vsfip_tcppcb_t *)socket->pcb.protopcb;
		if ((pcb->state == VSFIP_TCPSTAT_ESTABLISHED) && !socket->accepted)
		{
			socket->accepted = true;
			return socket;
		}
		socket = socket->next;
	}
	return NULL;
}

vsf_err_t vsfip_tcp_accept(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
			struct vsfip_socket_t *socket, struct vsfip_socket_t **acceptsocket)
{
	struct vsfip_tcppcb_t *pcb = (struct vsfip_tcppcb_t *)socket->pcb.protopcb;

	vsfsm_pt_begin(pt);

	if ((pcb->state != VSFIP_TCPSTAT_LISTEN) || socket->listener.closing)
	{
		return VSFERR_FAIL;
	}
	if (pcb->tx_sm != NULL)
	{
		return VSFERR_BUG;
	}

	// try find one first
	*acceptsocket = vsfip_tcp_accept_try(socket);
	if (*acceptsocket != NULL)
	{
		return VSFERR_NONE;
	}

	// wait for VSFIP_EVT_TCP_CONNECTOK or VSFIP_EVT_TCP_CONNECTFAIL
	pcb->tx_sm = pt->sm;
	evt = VSFSM_EVT_NONE;
	vsfsm_pt_entry(pt);
	if ((evt != VSFIP_EVT_TCP_CONNECTOK) && (evt != VSFIP_EVT_TCP_CONNECTFAIL))
	{
		return VSFERR_NOT_READY;
	}
	// when post event, will only clear tx_sm of child socket
	// so clear tx_sm of listener socket manually
	pcb->tx_sm = NULL;
	if (VSFIP_EVT_TCP_CONNECTFAIL == evt)
	{
		return VSFERR_FAIL;
	}

	vsfsm_pt_end(pt);

	*acceptsocket = vsfip_tcp_accept_try(socket);
	if (*acceptsocket != NULL)
	{
		return VSFERR_NONE;
	}
	return VSFERR_BUG;
}

vsf_err_t vsfip_tcp_async_send(struct vsfip_socket_t *socket,
			struct vsfip_buffer_t *buf)
{
	struct vsfip_tcppcb_t *pcb = (struct vsfip_tcppcb_t *)socket->pcb.protopcb;
	vsf_err_t err = VSFERR_NONE;
	uint32_t size, window;

	if (pcb->tx_sm != NULL)
	{
		err = VSFERR_BUG;
		goto cleanup;
	}
	if (pcb->lclose)
	{
		err = VSFERR_FAIL;
		goto cleanup;
	}

	size = vsfip_bufferlist_len(&socket->outq);
	window = min(pcb->tx_window, pcb->rwnd);
	if ((size + buf->app.size) > window)
	{
		return VSFERR_NOT_READY;
	}

	buf->netif = vsfip_ip_route(&socket->remote_sockaddr.sin_addr);
	if (NULL == buf->netif)
	{
		return VSFERR_FAIL;
	}
	buf->retry = VSFIP_TCP_RETRY;
	buf->ttl = socket->tx_timeout_ms;
	vsfq_append(&socket->outq, &buf->proto_node);
	vsfip_buffer_reference(buf);
	if (vsfip_tcp_send_do(socket, buf, VSFIP_TCPFLAG_PSH) != 0)
	{
		err = VSFERR_FAIL;
		goto cleanup;
	}
	return VSFERR_NONE;
cleanup:
	vsfip_buffer_release(buf);
	return err;
}

vsf_err_t vsfip_tcp_send(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
			struct vsfip_socket_t *socket, struct vsfip_buffer_t *buf,
			bool flush)
{
	struct vsfip_tcppcb_t *pcb = (struct vsfip_tcppcb_t *)socket->pcb.protopcb;
	vsf_err_t err = VSFERR_NONE;

	vsfsm_pt_begin(pt);
	while (1)
	{
		err = vsfip_tcp_async_send(socket, buf);
		if (err < 0) goto cleanup; else if (!err) break; else
		{
			pcb->tx_sm = pt->sm;
			evt = VSFSM_EVT_NONE;
			vsfsm_pt_entry(pt);
			if ((evt != VSFIP_EVT_TCP_TXOK) &&
				(evt != VSFIP_EVT_SOCKET_TIMEOUT))
			{
				return VSFERR_NOT_READY;
			}

			if (VSFIP_EVT_SOCKET_TIMEOUT == evt)
			{
				err = VSFERR_FAIL;
				goto cleanup;
			}
		}
	}

	if (flush)
	{
		while (socket->outq.head != NULL)
		{
			pcb->tx_sm = pt->sm;
			evt = VSFSM_EVT_NONE;
			vsfsm_pt_entry(pt);
			if ((evt != VSFIP_EVT_TCP_TXOK) &&
				(evt != VSFIP_EVT_SOCKET_TIMEOUT))
			{
				return VSFERR_NOT_READY;
			}

			if (VSFIP_EVT_SOCKET_TIMEOUT == evt)
			{
				err = VSFERR_FAIL;
				goto cleanup;
			}
		}
	}

	vsfsm_pt_end(pt);
	return err;
cleanup:
	vsfip_buffer_release(buf);
	return err;
}

vsf_err_t vsfip_tcp_async_recv(struct vsfip_socket_t *socket,
			struct vsfip_buffer_t **buf)
{
	struct vsfip_tcppcb_t *pcb = (struct vsfip_tcppcb_t *)socket->pcb.protopcb;
	struct vsfq_node_t *node;

	if (pcb->rx_sm != NULL)
	{
		return VSFERR_BUG;
	}
	if (pcb->rclose)
	{
		return VSFERR_FAIL;
	}

	node = vsfq_dequeue(&socket->inq);
	*buf = container_of(node, struct vsfip_buffer_t, proto_node);
	return (NULL == *buf) ? VSFERR_NOT_READY : VSFERR_NONE;
}

vsf_err_t vsfip_tcp_recv(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
			struct vsfip_socket_t *socket, struct vsfip_buffer_t **buf)
{
	struct vsfip_tcppcb_t *pcb = (struct vsfip_tcppcb_t *)socket->pcb.protopcb;

	vsfsm_pt_begin(pt);
	if (vsfip_tcp_async_recv(socket, buf))
	{
		pcb->rx_timeout_ms = socket->rx_timeout_ms;

		// wait VSFIP_EVT_TCP_RXOK, VSFIP_EVT_TCP_RXFAIL,
		// 			VSFIP_EVT_SOCKET_TIMEOUT
		pcb->rx_sm = pt->sm;
		evt = VSFSM_EVT_NONE;
		vsfsm_pt_entry(pt);
		if ((evt != VSFIP_EVT_TCP_RXOK) && (evt != VSFIP_EVT_SOCKET_TIMEOUT) &&
			(evt != VSFIP_EVT_TCP_RXFAIL))
		{
			return VSFERR_NOT_READY;
		}

		return vsfip_tcp_async_recv(socket, buf) ?
						VSFERR_FAIL : VSFERR_NONE;
	}

	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

vsf_err_t vsfip_tcp_close(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
							struct vsfip_socket_t *socket)
{
	struct vsfip_tcppcb_t *pcb = (struct vsfip_tcppcb_t *)socket->pcb.protopcb;

	vsfsm_pt_begin(pt);
	if (VSFIP_TCPSTAT_CLOSED == pcb->state)
	{
		return VSFERR_NONE;
	}
	if (pcb->tx_sm != NULL)
	{
		return VSFERR_BUG;
	}

	// wait outq free
	while (socket->outq.head != NULL)
	{
		pcb->tx_sm = pt->sm;
		evt = VSFSM_EVT_NONE;
		vsfsm_pt_entry(pt);
		if ((evt != VSFIP_EVT_TCP_TXOK) && (evt != VSFIP_EVT_SOCKET_TIMEOUT))
		{
			return VSFERR_NOT_READY;
		}

		if (VSFIP_EVT_TCP_TXOK == evt)
		{
			continue;
		}
		else
		{
			// try to continue to send FIN
			break;
		}
	}

	if (!pcb->lclose)
	{
		pcb->lseq = pcb->acked_lseq;
		pcb->tx_retry = 3;
		vsfip_tcp_sendflags(socket, VSFIP_TCPFLAG_FIN);
		pcb->lseq++;
		pcb->state = pcb->rclose ?
						VSFIP_TCPSTAT_LASTACK : VSFIP_TCPSTAT_FINWAIT1;
	}

	// wait VSFIP_EVT_TCP_CLOSED, VSFIP_EVT_SOCKET_TIMEOUT
	pcb->tx_sm = pt->sm;
	evt = VSFSM_EVT_NONE;
	vsfsm_pt_entry(pt);
	if ((evt != VSFIP_EVT_TCP_CLOSED) && (evt != VSFIP_EVT_SOCKET_TIMEOUT))
	{
		return VSFERR_NOT_READY;
	}
	// packet is sending and waiting
	if (pcb->tx_timeout_ms > 0)
	{
		return VSFERR_NOT_READY;
	}

	pcb->lseq = pcb->acked_lseq;
	return (VSFIP_EVT_TCP_CLOSED == evt) ? VSFERR_NONE : VSFERR_FAIL;

	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

// ICMP
#define VSFIP_ICMP_ECHO_REPLY	0
#define VSFIP_ICMP_ECHO			8
static void vsfip_icmp_input(struct vsfip_buffer_t *buf)
{
	struct vsfip_ip4head_t *iphead = buf->iphead.ip4head;
	uint16_t iph_hlen = VSFIP_IP4H_HLEN(iphead) * 4;
	struct vsfip_icmphead_t *icmphead =
						(struct vsfip_icmphead_t *)buf->buf.buffer;

	if (icmphead->type == VSFIP_ICMP_ECHO)
	{
		uint32_t swapipcache;

		//swap ipaddr and convert to bigger endian
		swapipcache = iphead->ipsrc;
		iphead->ipsrc = iphead->ipdest;
		iphead->ipdest = swapipcache;
		iphead->len = SYS_TO_BE_U16(iphead->len);
		iphead->id = SYS_TO_BE_U16(iphead->id);
		iphead->checksum = SYS_TO_BE_U16(iphead->checksum);

		icmphead->type = VSFIP_ICMP_ECHO_REPLY;
		icmphead->checksum += SYS_TO_BE_U16(VSFIP_ICMP_ECHO << 8);
		if (icmphead->checksum >=
				SYS_TO_BE_U16(0xffff - (VSFIP_ICMP_ECHO << 8)))
		{
			icmphead->checksum++;
		}

		buf->buf.buffer -= iph_hlen;
		buf->buf.size += iph_hlen;
		vsfip_ip_output_do(buf);
		return;
	}
	// TODO:
	vsfip_buffer_release(buf);
}

// helper
// pton
vsf_err_t vsfip_ip4_pton(struct vsfip_ipaddr_t *domainip, char *domain)
{
	uint8_t i;
	char *str = domain;

	//may not head by zero
	i = atoi(str);
	if (i == 0)
		return VSFERR_FAIL;
	//is vaild num
	domainip->addr.s_addr_buf[0] = i;

	for (i = 1; i < 4; i++)
	{
		str = strchr(str, '.');
		if (str == NULL)
			return VSFERR_FAIL;
		str++;
		domainip->addr.s_addr_buf[i] = atoi(str);
	}
	domainip->size = 4;
	return VSFERR_NONE;
}

