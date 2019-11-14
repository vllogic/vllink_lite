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

#define VSFIP_DNS_CLIENT_PORT	53
#define VSFIP_DNS_TRY_CNT		3

PACKED_HEAD struct PACKED_MID vsfip_dns_head_t
{
	uint16_t id;
	uint16_t flag;
	uint16_t ques;
	uint16_t answ;
	uint16_t auth;
	uint16_t addrrs;
}; PACKED_TAIL

#define VSFIP_DNS_QTYPE_A		1			// A - IP
#define VSFIP_DNS_QTYPE_NS		2			// NS - NameServer
#define VSFIP_DNS_QTYPE_CNAME	5			// CNAME
#define VSFIP_DNS_QTYPE_SOA		6
#define VSFIP_DNS_QTYPE_WKS		11
#define VSFIP_DNS_QTYPE_PTR		12
#define VSFIP_DNS_QTYPE_HINFO	13
#define VSFIP_DNS_QTYPE_MX		15			// MX - Mail
#define VSFIP_DNS_QTYPE_AAAA	28			// IPV6

PACKED_HEAD struct PACKED_MID vsfip_dns_query_type_t
{
	uint16_t type;
	uint16_t classtype;
}; PACKED_TAIL

PACKED_HEAD struct PACKED_MID vsfip_dns_response_t
{
	uint16_t type;
	uint16_t classtype;
	uint32_t ttl;
	uint16_t len;
}; PACKED_TAIL

static struct vsfip_dnsc_local_t vsfip_dnsc;

#define VSFIP_DNS_PKG_SIZE		512
#define VSFIP_DNS_AFNET			1

static uint16_t vsfip_dns_parse_name(uint8_t *orgin, uint16_t size)
{
	uint8_t n;
	uint8_t *query = orgin;

	do
	{
		n = *query++;

		/** @see RFC 1035 - 4.1.4. Message compression */
		if ((n & 0xc0) == 0xc0)
		{
			/* Compressed name */
			break;
		}
		else
		{
			/* Not compressed name */
			while (n > 0)
			{
				++query;
				--n;
			};
		}
	}
	while (*query != 0);

	return query + 1 - orgin;
}

struct vsfip_buffer_t *vsfip_dns_build_query(char *domain, uint16_t id)
{
	struct vsfip_buffer_t *buf;
	struct vsfip_dns_head_t *head;
	struct vsfip_dns_query_type_t *end;
	uint8_t *name;
	uint16_t count;

	buf = VSFIP_UDPBUF_GET(VSFIP_DNS_PKG_SIZE);
	if (NULL == buf)
	{
		return NULL;
	}

	// fill header
	head = (struct vsfip_dns_head_t *)buf->app.buffer;
	head->id = id;
	head->flag = SYS_TO_BE_U16(0x0100);
	head->ques = SYS_TO_BE_U16(1);
	head->answ = SYS_TO_BE_U16(0);
	head->auth = SYS_TO_BE_U16(0);
	head->addrrs = SYS_TO_BE_U16(0);
	name = (uint8_t *)head + sizeof(struct vsfip_dns_head_t);
	// fill name array
	// empty next size
	count = 0;
	name++;

	while ((*domain != '\0') && (*domain != '/'))
	{
		if (*domain == '.')
		{
			// fill the size
			*(name - count - 1) = count;
			// empty next size
			name++;
			domain++;
			count = 0;
		}
		else
		{
			*name = *domain;
			name++;
			domain++;
			count++;
		}
	}

	*name = 0;
	// fill last size
	// fill the size
	*(name - count - 1) = count;
	name++;
	// finish
	end = (struct vsfip_dns_query_type_t *)name;
	end->type = SYS_TO_BE_U16(VSFIP_DNS_QTYPE_A);
	end->classtype = SYS_TO_BE_U16(VSFIP_DNS_AFNET);
	buf->app.size = name - (uint8_t *)head +
						sizeof(struct vsfip_dns_query_type_t);
	return buf;
}

vsf_err_t vsfip_dns_decode_ans(uint8_t *ans , uint16_t size, uint16_t id,
								struct vsfip_ipaddr_t *domainip)
{
	struct vsfip_dns_head_t *head = (struct vsfip_dns_head_t *)ans;
	struct vsfip_dns_response_t *atype;
	uint16_t qc, ac, i;
	uint8_t	*pdat;
	uint16_t dsize;

	if ((size < sizeof(struct vsfip_dns_head_t)) || (head->id != id))
	{
		return VSFERR_FAIL;
	}

	qc = BE_TO_SYS_U16(head->ques);
	ac = BE_TO_SYS_U16(head->answ);
	pdat = (uint8_t *)ans + sizeof(struct vsfip_dns_head_t);
	dsize = size - sizeof(struct vsfip_dns_head_t);

	for (i = 0; i < qc; i++)
	{
		uint16_t nsize = vsfip_dns_parse_name(pdat, dsize);
		if (dsize < (nsize + sizeof(struct vsfip_dns_query_type_t)))
		{
			return VSFERR_FAIL;
		}

		// skip name and type class
		pdat += nsize + sizeof(struct vsfip_dns_query_type_t);
		dsize -= nsize + sizeof(struct vsfip_dns_query_type_t);
	}

	for (i = 0 ; i < ac ; i++)
	{
		uint16_t nsize = vsfip_dns_parse_name(pdat, dsize);

		pdat += nsize;
		dsize -= nsize;
		if (dsize < sizeof(struct vsfip_dns_response_t))
		{
			return VSFERR_FAIL;
		}

		atype = (struct vsfip_dns_response_t *)pdat;
		if ((atype->classtype != SYS_TO_BE_U16(VSFIP_DNS_AFNET)) ||
			(atype->type != SYS_TO_BE_U16(VSFIP_DNS_QTYPE_A)))
		{
			uint16_t nsize = sizeof(struct vsfip_dns_response_t) +
						SYS_TO_BE_U16(atype->len);

			if (dsize < nsize)
			{
				return VSFERR_FAIL;
			}

			pdat += nsize;
			dsize -= nsize;
			continue;
		}

		// no need ttl
		// get ip size
		domainip->size = BE_TO_SYS_U16(atype->len);
		if (dsize < (sizeof(struct vsfip_dns_response_t) + domainip->size))
		{
			return VSFERR_FAIL;
		}

		pdat += sizeof(struct vsfip_dns_response_t);

		// copy addr
		if (domainip->size == 4)
		{
			domainip->addr.s_addr = *(uint32_t *)pdat;
			return VSFERR_NONE;
		}
	}

	return VSFERR_FAIL;
}

vsf_err_t vsfip_dnsc_setserver(uint8_t numdns, struct vsfip_ipaddr_t *dnsserver)
{
	if (numdns < dimof(vsfip_dnsc.server))
		vsfip_dnsc.server[numdns] = *dnsserver;
	else
		return VSFERR_NOT_AVAILABLE;

	return VSFERR_NONE;
}

vsf_err_t vsfip_dnsc_init(void)
{
	return vsfsm_crit_init(&vsfip_dnsc.crit, VSFSM_EVT_USER_LOCAL);
}

vsf_err_t vsfip_gethostbyname(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
							char *domain, struct vsfip_ipaddr_t *domainip)
{
	vsf_err_t err;

	vsfsm_pt_begin(pt);

	if (vsfsm_crit_enter(&vsfip_dnsc.crit, pt->sm))
	{
		vsfsm_pt_wfe(pt, VSFSM_EVT_USER_LOCAL);
	}

	vsfip_dnsc.so = vsfip_socket(AF_INET, IPPROTO_UDP);
	if (vsfip_dnsc.so == NULL)
	{
		err = VSFERR_NOT_AVAILABLE;
		goto close;
	}

	err = vsfip_listen(vsfip_dnsc.so, 0);
	if (err < 0) goto close;

	vsfip_dnsc.id++;
	vsfip_dnsc.outbuf = vsfip_dns_build_query(domain, vsfip_dnsc.id);
	vsfip_dnsc.socket_pt.sm = pt->sm;
	vsfip_dnsc.so->rx_timeout_ms = 1000;

	vsfip_dnsc.try_cnt = VSFIP_DNS_TRY_CNT;
	do
	{
		for (vsfip_dnsc.i = 0; vsfip_dnsc.i < dimof(vsfip_dnsc.server);
				vsfip_dnsc.i++)
		{
			vsfip_dnsc.dnsaddr.sin_port = VSFIP_DNS_CLIENT_PORT;
			vsfip_dnsc.dnsaddr.sin_addr = vsfip_dnsc.server[vsfip_dnsc.i];
			vsfip_dnsc.socket_pt.state = 0;
			vsfip_udp_async_send(vsfip_dnsc.so, &vsfip_dnsc.dnsaddr,
						vsfip_dnsc.outbuf);

			// receive
			vsfip_dnsc.socket_pt.state = 0;
			vsfsm_pt_entry(pt);
			err = vsfip_udp_recv(&vsfip_dnsc.socket_pt, evt, vsfip_dnsc.so,
						&vsfip_dnsc.dnsaddr, &vsfip_dnsc.inbuf);
			if (err > 0) return err; else if (err < 0) continue;

			// recv success
			err = vsfip_dns_decode_ans(vsfip_dnsc.inbuf->app.buffer,
						vsfip_dnsc.inbuf->app.size, vsfip_dnsc.id, domainip);
			vsfip_buffer_release(vsfip_dnsc.inbuf);
			if (!err) break;
		}
		vsfip_dnsc.try_cnt--;
	} while ((err != 0) && (vsfip_dnsc.try_cnt > 0));

close:
	if (vsfip_dnsc.outbuf != NULL)
	{
		vsfip_buffer_release(vsfip_dnsc.outbuf);
		vsfip_dnsc.outbuf = NULL;
	}
	if (vsfip_dnsc.so != NULL)
	{
		vsfip_close(vsfip_dnsc.so);
		vsfip_dnsc.so = NULL;
	}
	vsfsm_crit_leave(&vsfip_dnsc.crit);

	vsfsm_pt_end(pt);
	return err;
}

