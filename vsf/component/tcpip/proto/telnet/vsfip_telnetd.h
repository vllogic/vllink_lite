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
#ifndef __VSFIP_TELNETD_H_INCLUDED__
#define __VSFIP_TELNETD_H_INCLUDED__

struct vsfip_telnetd_session_t
{
	struct vsf_stream_t *stream_tx;
	struct vsf_stream_t *stream_rx;

	// private
	struct vsfsm_sem_t stream_tx_sem;
	struct vsfsm_sem_t stream_rx_sem;

	bool connected;
	bool disconnect;
	struct vsfip_socket_t *so;
	struct vsfip_buffer_t *outbuf;
	struct vsfip_buffer_t *inbuf;

	struct vsfsm_pt_t txpt;
	struct vsfsm_pt_t caller_txpt;
	struct vsfsm_t txsm;
	struct vsfsm_pt_t rxpt;
	struct vsfsm_pt_t caller_rxpt;
	struct vsfsm_t rxsm;
};

struct vsfip_telnetd_t
{
	uint16_t port;
	uint32_t session_num;

	// private
	struct vsfsm_pt_t pt;
	struct vsfsm_pt_t caller_pt;
	struct vsfsm_t sm;

	struct vsfip_socket_t *so;
	struct vsfip_socket_t *cur_session;
	struct vsfip_sockaddr_t sockaddr;

	struct vsfip_telnetd_session_t sessions[0];
};

#ifndef VSFCFG_EXCLUDE_TELNETD
vsf_err_t vsfip_telnetd_start(struct vsfip_telnetd_t *telnetd);
#endif

#endif		// __VSFIP_TELNETD_H_INCLUDED__
