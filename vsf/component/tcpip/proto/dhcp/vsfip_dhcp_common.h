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
#ifndef __VSFIP_DHCP_COMMON_H_INCLUDED__
#define __VSFIP_DHCP_COMMON_H_INCLUDED__

#define DHCP_CLIENT_PORT			68
#define DHCP_SERVER_PORT			67
#define DHCP_MAGIC					0x63825363

enum dhcp_opt_t
{
	DHCPOPT_PAD						= 0,
	DHCPOPT_SUBNETMASK				= 1,
	DHCPOPT_ROUTER					= 3,
	DHCPOPT_DNSSERVER				= 6,
	DHCPOPT_HOSTNAME				= 12,
	DHCPOPT_DOMAIN					= 15,
	DHCPOPT_MTU						= 26,
	DHCPOPT_BROADCAST				= 28,
	DHCPOPT_ROUTERDISCOVERY			= 31,
	DHCPOPT_REQIP					= 50,
	DHCPOPT_LEASE_TIME				= 51,
	DHCPOPT_MSGTYPE					= 53,
	DHCPOPT_SERVERID				= 54,
	DHCPOPT_PARAMLIST				= 55,
	DHCPOPT_MAXMSGSIZE				= 57,
	DHCPOPT_RENEW_TIME				= 58,
	DHCPOPT_REBINDING_TIME			= 59,
	DHCPOPT_CLASSID					= 60,
	DHCPOPT_END						= 255,
};
#define DHCPOPT_MSGTYPE_LEN			1
#define DHCPOPT_MAXMSGSIZE_LEN		2
#define DHCPOPT_MINLEN				68

#define DHCP_TOSERVER				1
#define DHCP_TOCLIENT				2

enum dhcp_msg_t
{
	DHCPOP_DISCOVER					= 1,
	DHCPOP_OFFER					= 2,
	DHCPOP_REQUEST					= 3,
	DHCPOP_DECLINE					= 4,
	DHCPOP_ACK						= 5,
	DHCPOP_NAK						= 6,
	DHCPOP_RELEASE					= 7,
	DHCPOP_INFORM					= 8,
};

struct vsfip_dhcphead_t
{
	uint8_t op;
	uint8_t htype;
	uint8_t hlen;
	uint8_t hops;
	uint32_t xid;
	uint16_t secs;
	uint16_t flags;
	uint32_t ciaddr;
	uint32_t yiaddr;
	uint32_t siaddr;
	uint32_t giaddr;
	uint8_t chaddr[16];
	char sname[64];
	char file[128];
	uint32_t magic;
	uint8_t options[DHCPOPT_MINLEN];	// min option size
} PACKED;

void vsfip_dhcp_append_opt(struct vsfip_buffer_t *buf, uint32_t *optlen,
						uint8_t option, uint8_t len, uint8_t *data);
void vsfip_dhcp_end_opt(struct vsfip_buffer_t *buf, uint32_t *optlen);
uint8_t vsfip_dhcp_get_opt(struct vsfip_buffer_t *buf, uint8_t option,
						uint8_t **data);

#endif		// __VSFIP_DHCP_COMMON_H_INCLUDED__
