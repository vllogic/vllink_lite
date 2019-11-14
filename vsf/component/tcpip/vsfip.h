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
#ifndef __VSFIP_H_INCLUDED__
#define __VSFIP_H_INCLUDED__

#ifndef VSFIP_CFG_TTL_INPUT
#define VSFIP_CFG_TTL_INPUT		10
#endif
#ifndef VSFIP_CFG_UDP_PORT
#define VSFIP_CFG_UDP_PORT		40000
#endif
#ifndef VSFIP_CFG_TCP_PORT
#define VSFIP_CFG_TCP_PORT		40000
#endif
#ifndef VSFIP_CFG_HOSTNAME
#define VSFIP_CFG_HOSTNAME		"vsfip"
#endif
#ifndef VSFIP_CFG_MTU
#define VSFIP_CFG_MTU			1500
#endif
#ifndef VSFIP_BUFFER_SIZE
// NETIF_HEAD + 1500(MTU)
#define VSFIP_BUFFER_SIZE		(VSFIP_CFG_MTU + VSFIP_CFG_NETIF_HEADLEN)
#endif
#ifndef VSFIP_CFG_TCP_MSS
// 1500(MTU) - 20(TCP_HEAD) - 20(IP_HEAD)
#define VSFIP_CFG_TCP_MSS		(VSFIP_CFG_MTU - VSFIP_IP_HEADLEN - VSFIP_TCP_HEADLEN)
#endif

#define VSFIP_IPADDR_ANY		0

#define VSFIP_IP_HEADLEN		20
#define VSFIP_UDP_HEADLEN		8
#define VSFIP_TCP_HEADLEN		20

struct vsfip_addr_t
{
	uint32_t size;
	union
	{
		uint32_t s_addr;
		uint64_t s_addr64;
		uint8_t s_addr_buf[16];
	} addr;
};

#define vsfip_ipaddr_t			vsfip_addr_t
#define vsfip_macaddr_t			vsfip_addr_t

struct vsfip_ipmac_assoc
{
	struct vsfip_macaddr_t mac;
	struct vsfip_ipaddr_t ip;
};

#include "netif/vsfip_netif.h"

struct vsfip_buffer_t
{
	// inherent from vsfq_node_t
	struct vsfq_node_t proto_node;
	struct vsfq_node_t netif_node;

	struct vsf_buffer_t buf;
	struct vsf_buffer_t app;

	union
	{
		uint8_t *ipver;
		struct vsfip_ip4head_t *ip4head;
//		struct vsfip_ip6head_t *ip6head;
	} iphead;

	uint16_t ref;
	uint16_t ttl;
	uint16_t retry;

	uint8_t *buffer;
//	uint32_t size;

	struct vsfip_netif_t *netif;
};

#define VSFIP_BUF_GET(s)		vsfip_buffer_get(s)
#define VSFIP_NETIFBUF_GET(s)	VSFIP_BUF_GET((s) + VSFIP_CFG_NETIF_HEADLEN)
#define VSFIP_IPBUF_GET(s)		VSFIP_NETIFBUF_GET((s) + VSFIP_IP_HEADLEN)

#define VSFIP_PROTO_HEADLEN		(VSFIP_CFG_NETIF_HEADLEN + VSFIP_IP_HEADLEN)
#define VSFIP_UDPBUF_GET(s)		vsfip_appbuffer_get(VSFIP_PROTO_HEADLEN + VSFIP_UDP_HEADLEN, (s))
#define VSFIP_TCPBUF_GET(s)		vsfip_appbuffer_get(VSFIP_PROTO_HEADLEN + VSFIP_TCP_HEADLEN, (s))

enum vsfip_sockfamilt_t
{
	AF_NONE		= 0,
	AF_INET		= 2,
	AF_INET6	= 10,
};

enum vsfip_sockproto_t
{
//	IPPROTO_IP	= 0,
	IPPROTO_ICMP= 1,
	IPPROTO_IGMP= 2,
	IPPROTO_TCP	= 6,
	IPPROTO_UDP	= 17,
//	IPPROTO_SCTP= 132,
//	IPPROTO_RAW	= 255,
};

struct vsfip_sockaddr_t
{
	uint16_t sin_port;
	struct vsfip_ipaddr_t sin_addr;
};

struct vsfip_ippcb_t
{
	struct vsfip_ipaddr_t src;
	struct vsfip_ipaddr_t dest;
	struct vsfip_buffer_t *buf;

	struct vsfsm_pt_t output_pt;

	uint32_t len;
	uint32_t xfed_len;
	uint16_t id;
};

// headers
// IPv4
struct vsfip_ip4head_t
{
	uint8_t ver_hl;
	uint8_t tos;
	uint16_t len;
	uint16_t id;
#define VSFIP_IPH_RF			0x8000
#define VSFIP_IPH_DF			0x4000
#define VSFIP_IPH_MF			0x2000
#define VSFIP_IPH_OFFSET_MASK	0x1FFF
	uint16_t offset;
	uint8_t ttl;
	uint8_t proto;
	uint16_t checksum;
	uint32_t ipsrc;
	uint32_t ipdest;
} PACKED;
#define VSFIP_IP4H_V(h)			(h->ver_hl >> 4)
#define VSFIP_IP4H_HLEN(h)		(h->ver_hl & 0x0F)
// PROTO PORT
struct vsfip_protoport_t
{
	uint16_t src;
	uint16_t dst;
} PACKED;
// UDP
struct vsfip_udphead_t
{
	struct vsfip_protoport_t port;
	uint16_t len;
	uint16_t checksum;
} PACKED;
// TCP
struct vsfip_tcphead_t
{
	struct vsfip_protoport_t port;
	uint32_t seq;
	uint32_t ackseq;
	uint8_t headlen;
	uint8_t flags;
	uint16_t window_size;
	uint16_t checksum;
	uint16_t urgent_ptr;
} PACKED;
// ICMP
struct vsfip_icmphead_t
{
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	union
	{
		struct
		{
			uint16_t flags;
			uint16_t seqnum;
		} echo;
	} body;
} PACKED;

struct vsfip_socket_t;
enum vsfip_tcp_stat_t
{
	VSFIP_TCPSTAT_INVALID,
	VSFIP_TCPSTAT_CLOSED,
	VSFIP_TCPSTAT_LISTEN,
	VSFIP_TCPSTAT_SYN_SENT,
	VSFIP_TCPSTAT_SYN_GET,
	VSFIP_TCPSTAT_ESTABLISHED,
	VSFIP_TCPSTAT_FINWAIT1,
	VSFIP_TCPSTAT_FINWAIT2,
	VSFIP_TCPSTAT_CLOSEWAIT,
	VSFIP_TCPSTAT_LASTACK,
};
struct vsfip_tcppcb_t
{
	enum vsfip_tcp_stat_t state;
	uint32_t lseq;
	uint32_t acked_lseq;
	uint32_t rseq;
	uint32_t acked_rseq;
	uint32_t rwnd;

	// tx
	struct vsfsm_t *tx_sm;
	uint32_t tx_timeout_ms;		// only for FIN and SYN
	uint32_t tx_retry;			// only for FIN and SYN
	uint32_t tx_window;

	// rx
	struct vsfsm_t *rx_sm;
	uint32_t rx_timeout_ms;
	uint32_t rx_window;

	uint32_t ack_tick;
	bool rclose;
	bool lclose;
	bool reset;
	bool ack_timeout;

	vsf_err_t err;

	uint8_t flags;
};

struct vsfip_pcb_t
{
	struct vsfip_ippcb_t ippcb;
	void *protopcb;
};

struct vsfip_socket_t
{
	enum vsfip_sockfamilt_t family;
	enum vsfip_sockproto_t protocol;

	struct vsfip_sockaddr_t local_sockaddr;
	struct vsfip_sockaddr_t remote_sockaddr;

	struct vsfip_pcb_t pcb;
	struct vsfsm_sem_t input_sem;
	struct vsfq_t inq;
	struct vsfq_t outq;

	bool can_rx;
	struct
	{
		struct vsfip_socket_t *child;
		uint8_t backlog;
		uint8_t accepted_num;
		bool closing;
	} listener;
	bool accepted;
	struct vsfip_socket_t *father;

	uint32_t tx_timeout_ms;
	uint32_t rx_timeout_ms;
	struct vsftimer_t tx_timer;
	struct vsftimer_t rx_timer;

	struct
	{
		void (*on_input)(struct vsfip_socket_t *so, struct vsfip_buffer_t *buf);
	} proto_callback;
	struct
	{
		void (*on_input)(void *param, struct vsfip_buffer_t *buf);
		void (*on_outputted)(void *param);
		void *param;
	} user_callback;

	struct vsfip_socket_t *next;
};

struct vsfip_t
{
	struct vsfip_netif_t *netif_list;
	struct vsfip_netif_t *netif_default;

	struct vsfip_socket_t *udpconns;
	struct vsfip_socket_t *tcpconns;

	struct vsfsm_t tick_sm;
	struct vsftimer_t tick_timer;

	uint16_t udp_port;
	uint16_t tcp_port;
	uint16_t ip_id;
	uint32_t tsn;

	bool quit;
	struct vsfip_mem_op_t *mem_op;
	void (*input_sniffer)(struct vsfip_buffer_t *buf);
	void (*output_sniffer)(struct vsfip_buffer_t *buf);
};

struct vsfip_mem_op_t
{
	struct vsfip_buffer_t* (*get_buffer)(uint32_t size);
	void (*release_buffer)(struct vsfip_buffer_t*);
	struct vsfip_socket_t* (*get_socket)(void);
	void (*release_socket)(struct vsfip_socket_t*);
	struct vsfip_tcppcb_t* (*get_tcppcb)(void);
	void (*release_tcppcb)(struct vsfip_tcppcb_t*);
};

#ifndef VSFCFG_EXCLUDE_TCPIP
struct vsfip_buffer_t* vsfip_buffer_get(uint32_t size);
struct vsfip_buffer_t* vsfip_appbuffer_get(uint32_t header, uint32_t app);
void vsfip_buffer_reference(struct vsfip_buffer_t *buf);
void vsfip_buffer_release(struct vsfip_buffer_t *buf);

vsf_err_t vsfip_netif_add(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
							struct vsfip_netif_t *netif);
vsf_err_t vsfip_netif_remove(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
							struct vsfip_netif_t *netif);

vsf_err_t vsfip_init(struct vsfip_mem_op_t *mem_op);
vsf_err_t vsfip_fini(void);

// different from stant socket call,
// vsfip_socket will return a pointer to vsfip_socket_t structure
struct vsfip_socket_t* vsfip_socket(enum vsfip_sockfamilt_t family,
									enum vsfip_sockproto_t protocol);
vsf_err_t vsfip_close(struct vsfip_socket_t *socket);
void vsfip_socket_cb(struct vsfip_socket_t *socket,
				void *param, void (*on_input)(void *, struct vsfip_buffer_t *),
				void (*on_outputted)(void *));

// for UPD and TCP
vsf_err_t vsfip_listen(struct vsfip_socket_t *socket, uint8_t backlog);
vsf_err_t vsfip_bind(struct vsfip_socket_t *socket, uint16_t port);

// for TCP
void vsfip_tcp_config_window(struct vsfip_socket_t *socket, uint32_t rx_window,
		uint32_t tx_window);
vsf_err_t vsfip_tcp_connect(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
		struct vsfip_socket_t *socket, struct vsfip_sockaddr_t *sockaddr);
vsf_err_t vsfip_tcp_accept(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
		struct vsfip_socket_t *socket, struct vsfip_socket_t **acceptsocket);
vsf_err_t vsfip_tcp_async_send(struct vsfip_socket_t *socket,
		struct vsfip_buffer_t *buf);
vsf_err_t vsfip_tcp_send(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
		struct vsfip_socket_t *socket, struct vsfip_buffer_t *buf, bool flush);
vsf_err_t vsfip_tcp_async_recv(struct vsfip_socket_t *socket,
		struct vsfip_buffer_t **buf);
vsf_err_t vsfip_tcp_recv(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
		struct vsfip_socket_t *socket, struct vsfip_buffer_t **buf);
vsf_err_t vsfip_tcp_close(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
							struct vsfip_socket_t *socket);

// for UDP
vsf_err_t vsfip_udp_async_send(struct vsfip_socket_t *socket,
		struct vsfip_sockaddr_t *sockaddr, struct vsfip_buffer_t *buf);
vsf_err_t vsfip_udp_send(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
		struct vsfip_socket_t *socket, struct vsfip_sockaddr_t *sockaddr,
		struct vsfip_buffer_t *buf);
vsf_err_t vsfip_udp_async_recv(struct vsfip_socket_t *socket,
		struct vsfip_sockaddr_t *sockaddr, struct vsfip_buffer_t **buf);
vsf_err_t vsfip_udp_recv(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
		struct vsfip_socket_t *socket, struct vsfip_sockaddr_t *sockaddr,
		struct vsfip_buffer_t **buf);

// for proto
vsf_err_t vsfip_ip4_pton(struct vsfip_ipaddr_t *domainip, char *domain);
#endif

#endif		// __VSFIP_H_INCLUDED__
