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

#define VSFIP_TELNETD_EVT_STREAM_IN				(VSFSM_EVT_USER + 0)
#define VSFIP_TELNETD_EVT_STREAM_OUT			(VSFSM_EVT_USER + 1)

static void vsfip_telnetd_session_stream_on_in(void *param)
{
	vsfsm_sem_post(&((struct vsfip_telnetd_session_t *)param)->stream_tx_sem);
}

static vsf_err_t
vsfip_telnetd_session_tx_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfip_telnetd_session_t *session =
								(struct vsfip_telnetd_session_t *)pt->user_data;
	uint32_t len;
	vsf_err_t err;

	vsfsm_pt_begin(pt);

	session->caller_txpt.sm = pt->sm;
	while (!session->disconnect)
	{
		len = stream_get_data_size(session->stream_tx);
		if (!len && vsfsm_sem_pend(&session->stream_tx_sem, pt->sm))
		{
			vsfsm_pt_wfe(pt, VSFIP_TELNETD_EVT_STREAM_IN);
			if (session->disconnect)
				break;
			continue;
		}

	retry_alloc_buf:
		session->outbuf = VSFIP_TCPBUF_GET(len);
		if (NULL == session->outbuf)
		{
			vsfsm_pt_delay(pt, 5);
			if (session->disconnect)
				break;
			goto retry_alloc_buf;
		}

		session->outbuf->app.size =
					stream_read(session->stream_tx, &session->outbuf->app);
		if (!session->outbuf->app.size)
		{
			vsfip_buffer_release(session->outbuf);
			continue;
		}

		session->caller_txpt.state = 0;
		vsfsm_pt_entry(pt);
		err = vsfip_tcp_send(&session->caller_txpt, evt, session->so,
						session->outbuf, false);
		if (err > 0) return err; else if (err < 0)
		{
			session->disconnect = true;
		}
	}

	// close tcp socket
	session->caller_txpt.state = 0;
	vsfsm_pt_entry(pt);
	err = vsfip_tcp_close(&session->caller_txpt, evt, session->so);
	if (err > 0) return err;

	// close socket no matter if tcp closed OK or not
	vsfip_close(session->so);
	session->connected = false;

	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static void vsfip_telnetd_session_stream_on_out(void *param)
{
	vsfsm_sem_post(&((struct vsfip_telnetd_session_t *)param)->stream_rx_sem);
}

static vsf_err_t
vsfip_telnetd_session_rx_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfip_telnetd_session_t *session =
								(struct vsfip_telnetd_session_t *)pt->user_data;
	uint32_t len;
	vsf_err_t err;

	vsfsm_pt_begin(pt);

	session->caller_rxpt.sm = pt->sm;
	while (!session->disconnect)
	{
		session->caller_rxpt.state = 0;
		vsfsm_pt_entry(pt);
		err = vsfip_tcp_recv(&session->caller_rxpt, evt, session->so,
								&session->inbuf);
		if (err > 0) return err; else if (err < 0)
		{
			session->disconnect = true;
			// fake on_in event, just to wakeup tx_thread to exit
			vsfip_telnetd_session_stream_on_in(session);
			break;
		}

		while (session->inbuf->app.size > 0)
		{
			len = stream_write(session->stream_rx, &session->inbuf->app);
			if (!len && vsfsm_sem_pend(&session->stream_rx_sem, pt->sm))
			{
				vsfsm_pt_wfe(pt, VSFIP_TELNETD_EVT_STREAM_OUT);
				continue;
			}
			session->inbuf->app.buffer += len;
			session->inbuf->app.size -= len;
		}
		vsfip_buffer_release(session->inbuf);
	}

	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t vsfip_telnetd_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfip_telnetd_t *telnetd = (struct vsfip_telnetd_t *)pt->user_data;
	vsf_err_t err = VSFERR_NONE;
	int i;

	vsfsm_pt_begin(pt);

	telnetd->caller_pt.sm = pt->sm;
	telnetd->so = vsfip_socket(AF_INET, IPPROTO_TCP);
	if (NULL == telnetd->so)
	{
		return VSFERR_FAIL;
	}
	telnetd->so->tx_timeout_ms = 100;

	if ((vsfip_bind(telnetd->so, telnetd->port) != 0) ||
		(vsfip_listen(telnetd->so, telnetd->session_num) != 0))
	{
		err = VSFERR_FAIL;
		goto fail_socket_connect;
	}

	while (1)
	{
		telnetd->caller_pt.state = 0;
		vsfsm_pt_entry(pt);
		err = vsfip_tcp_accept(&telnetd->caller_pt, evt, telnetd->so,
								&telnetd->cur_session);
		if (err > 0) return err; else if (err < 0)
		{
			continue;
		}

		// get session
		for (i = 0; i < telnetd->session_num; i++)
		{
			if (!telnetd->sessions[i].connected)
			{
				struct vsfip_telnetd_session_t *session = &telnetd->sessions[i];

				session->connected = true;
				session->disconnect = false;
				session->so = telnetd->cur_session;

				session->stream_rx->callback_tx.param = session;
				session->stream_rx->callback_tx.on_inout =
											vsfip_telnetd_session_stream_on_out;
				session->stream_tx->callback_rx.param = session;
				session->stream_tx->callback_rx.on_inout =
											vsfip_telnetd_session_stream_on_in;
				stream_connect_tx(session->stream_rx);
				stream_connect_rx(session->stream_tx);

				vsfsm_sem_init(&session->stream_tx_sem, 0,
											VSFIP_TELNETD_EVT_STREAM_IN);
				vsfsm_sem_init(&session->stream_rx_sem, 0,
												VSFIP_TELNETD_EVT_STREAM_OUT);

				session->txpt.thread = vsfip_telnetd_session_tx_thread;
				session->txpt.user_data = session;
				session->rxpt.thread = vsfip_telnetd_session_rx_thread;
				session->rxpt.user_data = session;
				vsfsm_pt_init(&session->txsm, &session->txpt);
				vsfsm_pt_init(&session->rxsm, &session->rxpt);
				break;
			}
		}
		if (i == telnetd->session_num)
		{
			telnetd->caller_pt.state = 0;
			vsfsm_pt_entry(pt);
			err = vsfip_tcp_close(&telnetd->caller_pt, evt,
										telnetd->cur_session);
			if (err > 0) return err;
			vsfip_close(telnetd->cur_session);
		}
	}

fail_socket_connect:
	vsfip_close(telnetd->so);
	telnetd->so = NULL;

	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

vsf_err_t vsfip_telnetd_start(struct vsfip_telnetd_t *telnetd)
{
	telnetd->pt.thread = vsfip_telnetd_thread;
	telnetd->pt.user_data = telnetd;
	return vsfsm_pt_init(&telnetd->sm, &telnetd->pt);
}

