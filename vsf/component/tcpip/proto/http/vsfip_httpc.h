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
#ifndef __VSFIP_HTTPC_H_INCLUDED__
#define __VSFIP_HTTPC_H_INCLUDED__

//#define HTTPC_DEBUG

struct vsfip_httpc_op_t
{
	vsf_err_t (*on_connect)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt, void *output);
	vsf_err_t (*on_recv)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt, void *output,
							uint32_t offset, struct vsfip_buffer_t *buf);
};

struct vsfip_httpc_param_t
{
	const struct vsfip_httpc_op_t *op;

	uint32_t resp_length;

	// private
	struct vsfsm_pt_t local_pt;

	struct vsfip_socket_t *so;
	struct vsfip_sockaddr_t hostip;

	char *host;
	char *file;

	struct vsfip_buffer_t *buf;
	uint32_t resp_curptr;
	uint8_t *resp_type;
	uint8_t resp_code;
	uint16_t port;
};

#ifndef VSFCFG_EXCLUDE_HTTPC
extern const struct vsfip_httpc_op_t vsfip_httpc_op_stream;
extern const struct vsfip_httpc_op_t vsfip_httpc_op_buffer;

vsf_err_t vsfip_httpc_get(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
						char *wwwaddr, void *output);
#endif

#endif		// __VSFIP_HTTPD_H_INCLUDED__
