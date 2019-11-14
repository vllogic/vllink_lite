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
#ifndef __VSFIP_NETIF_H_INCLUDED__
#define __VSFIP_NETIF_H_INCLUDED__

#include "../vsfip.h"

#ifndef VSFIP_CFG_ARPCACHE_SIZE
#define VSFIP_CFG_ARPCACHE_SIZE				8
#endif
#ifndef VSFIP_CFG_ARP_TIMEOUT_MS
#define VSFIP_CFG_ARP_TIMEOUT_MS			1000
#endif

enum vsfip_netif_proto_t
{
	VSFIP_NETIF_PROTO_IP = 0x0800,
	VSFIP_NETIF_PROTO_ARP = 0x0806,
	VSFIP_NETIF_PROTO_RARP = 0x8035
};

struct vsfip_arp_entry_t
{
	struct vsfip_ipmac_assoc assoc;
	uint32_t time;
};

struct vsfip_buffer_t;
struct vsfip_netif_t;
struct vsfip_netdrv_op_t
{
	vsf_err_t (*init)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
	vsf_err_t (*fini)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);

	vsf_err_t (*header)(struct vsfip_buffer_t *buf,
						enum vsfip_netif_proto_t proto,
						const struct vsfip_macaddr_t *dest_addr);
};

struct vsfip_netdrv_t
{
	const struct vsfip_netdrv_op_t *op;
	void *param;
	uint32_t netif_header_size;
	uint16_t hwtype;
};

struct vsfip_netif_t
{
	struct vsfip_netdrv_t *drv;
	struct vsfip_macaddr_t macaddr;

	struct vsfip_ipaddr_t ipaddr;
	struct vsfip_ipaddr_t netmask;
	struct vsfip_ipaddr_t gateway;
	struct vsfip_ipaddr_t dns[2];

	struct vsfip_macaddr_t mac_broadcast;

	uint16_t mtu;

	// output bufferlist and semaphore
	struct vsfq_t outq;
	struct vsfsm_sem_t output_sem;

	// arp client
	struct
	{
		struct vsfsm_t sm;
		struct vsfsm_pt_t pt;
		struct vsfsm_sem_t sem;
		struct vsfsm_pt_t caller_pt;
		struct vsfsm_t *sm_pending;
		struct vsftimer_t to;
		struct vsfip_buffer_t *buf;
		struct vsfq_t requestq;
		struct vsfip_buffer_t *cur_request;
		struct vsfip_ipaddr_t ip_for_mac;
		uint32_t retry;
	} arpc;

	uint32_t arp_time;
	struct vsfip_arp_entry_t arp_cache[VSFIP_CFG_ARPCACHE_SIZE];

	struct vsfsm_pt_t init_pt;
	bool connected;
	bool quit;

	// for DHCP
	union
	{
		struct vsfip_dhcpc_t *dhcpc;
		struct vsfip_dhcpd_t *dhcpd;
		void *ptr;
	} dhcp;

	struct vsfip_netif_t *next;
};

vsf_err_t vsfip_netif_construct(struct vsfip_netif_t *netif);
vsf_err_t vsfip_netif_init(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
vsf_err_t vsfip_netif_fini(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
vsf_err_t vsfip_netif_ip_output(struct vsfip_buffer_t *buf);

void vsfip_netif_ip4_input(struct vsfip_buffer_t *buf);
void vsfip_netif_ip6_input(struct vsfip_buffer_t *buf);
void vsfip_netif_arp_input(struct vsfip_buffer_t *buf);

#ifndef VSFCFG_EXCLUDE_NETIF
void vsfip_netif_arp_add_assoc(struct vsfip_netif_t *netif,
		uint8_t hwlen, uint8_t *hwaddr, uint8_t protolen, uint8_t *protoaddr);
#endif

#endif		// __VSFIP_NETIF_H_INCLUDED__
