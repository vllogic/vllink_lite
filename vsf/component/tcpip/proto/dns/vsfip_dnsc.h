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
#ifndef __VSFIP_DNSC_H_INCLUDED__
#define __VSFIP_DNSC_H_INCLUDED__

struct vsfip_dnsc_local_t
{
	struct vsfsm_pt_t socket_pt;

	struct vsfip_socket_t *so;
	struct vsfip_sockaddr_t dnsaddr;

	struct vsfip_buffer_t *inbuf;
	struct vsfip_buffer_t *outbuf;

	struct vsfsm_crit_t crit;

	uint16_t id;
	uint8_t try_cnt;
	struct vsfip_ipaddr_t server[2];
	int i;
};

struct vsfip_hostent_t
{
	uint8_t*	h_name;			/* official name of host */
	uint8_t**	h_aliases;		/* alias list */
	uint32_t	h_addrtype;		/* host address type */
	uint32_t	h_length;		/* length of address */
	uint8_t**	h_addr_list;	/* list of addresses */
};

#ifndef VSFCFG_EXCLUDE_DNSC
vsf_err_t vsfip_dnsc_init(void);
vsf_err_t vsfip_dnsc_setserver(uint8_t numdns,
							struct vsfip_ipaddr_t *dnsserver);
vsf_err_t vsfip_gethostbyname(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
							char *domain, struct vsfip_ipaddr_t *domainip);
#endif

#endif		// __VSFIP_DNSC_H_INCLUDED__
