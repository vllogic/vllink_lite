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

static void vsfusbd_vllink_on_OUT_finish(void *p);
static void vsfusbd_vllink_on_rxconn(void *p)
{
	struct vsfusbd_VLLINK_param_t *param = p;

	param->OUT_transact.stream = param->stream_rx;
	param->OUT_transact.data_size = 0xFFFFFFFF;
	vsfusbd_ep_recv(param->device, &param->OUT_transact);
}

static void vsfusbd_vllink_on_OUT_finish(void *p)
{
	struct vsfusbd_VLLINK_param_t *param = p;
	struct vsf_stream_t *stream = param->stream_rx;

	if (param->callback.on_rx_finish != NULL)
	{
		param->callback.on_rx_finish(stream->callback_rx.param);
	}

	if (!stream->rx_ready)
	{
		stream->callback_tx.param = param;
		stream->callback_tx.on_connect = vsfusbd_vllink_on_rxconn;
		stream->callback_tx.on_disconnect = NULL;
		stream->callback_tx.on_inout = NULL;
	}
	else
	{
		vsfusbd_vllink_on_rxconn(param);
	}
}

static void vsfusbd_vllink_on_in(void *p)
{
	struct vsfusbd_VLLINK_param_t *param = p;
	struct vsf_stream_t *stream = param->stream_tx;
	uint32_t size;

	size = stream_get_data_size(stream);
	if (size > 0)
	{
		param->IN_transact.data_size = size;
		param->IN_transact.stream = param->stream_tx;
		param->IN_transact.zlp = true;
		// stream->callback_rx.on_inout will be overwritten
		vsfusbd_ep_send(param->device, &param->IN_transact);
	}
}

static void vsfusbd_vllink_on_IN_finish(void *p)
{
	struct vsfusbd_VLLINK_param_t *param = p;
	struct vsf_stream_t *stream = param->stream_tx;

	if (param->callback.on_tx_finish != NULL)
	{
		param->callback.on_tx_finish(stream->callback_tx.param);
	}

	stream->callback_rx.param = param;
	stream->callback_rx.on_connect = NULL;
	stream->callback_rx.on_disconnect = NULL;
	stream->callback_rx.on_inout = vsfusbd_vllink_on_in;
	vsfusbd_vllink_on_in(param);
}

static vsf_err_t vsfusbd_vllink_class_init(uint8_t iface,
		struct vsfusbd_device_t *device)
{
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_iface_t *ifs = &config->iface[iface];
	struct vsfusbd_VLLINK_param_t *param = ifs->protocol_param;

	if ((NULL == param) || (NULL == param->stream_tx) ||
		(NULL == param->stream_rx))
	{
		return VSFERR_INVALID_PARAMETER;
	}

	param->device = device;
	param->IN_transact.ep = param->ep_in;
	param->IN_transact.cb.on_finish = vsfusbd_vllink_on_IN_finish;
	param->IN_transact.cb.param = param;
	param->IN_transact.zlp = false;
	param->OUT_transact.ep = param->ep_out;
	param->OUT_transact.cb.on_finish = vsfusbd_vllink_on_OUT_finish;
	param->OUT_transact.cb.param = param;
	param->OUT_transact.zlp = false;
	
	stream_connect_tx(param->stream_rx);
	stream_connect_rx(param->stream_tx);

	vsfusbd_vllink_on_IN_finish(param);
	vsfusbd_vllink_on_OUT_finish(param);
	return VSFERR_NONE;
}

static vsf_err_t vsfusbd_vllink_class_fini(uint8_t iface,
		struct vsfusbd_device_t *device)
{
	return VSFERR_NONE;
}

const struct vsfusbd_class_protocol_t vsfusbd_VLLINK_class =
{
	NULL, NULL, NULL,
	vsfusbd_vllink_class_init, vsfusbd_vllink_class_fini
};