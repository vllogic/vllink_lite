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
#include "vsfusbd_VLLINK.h"


static void vsfusbd_vllink_on_in(void *p)
{
	uint32_t size;
	struct vsfusbd_VLLINK_param_t *param = p;
	struct vsf_stream_t *stream = &param->bufstream_tx.stream;

	size = VSFSTREAM_GET_DATA_SIZE(stream);
	if (size > 0)
	{
		param->IN_transact.data_size = size;
		param->IN_transact.stream = stream;
		param->IN_transact.zlp = true;
		// stream->callback_rx.on_inout will be overwritten
		vsfusbd_ep_send(param->device, &param->IN_transact);
	}
}

static void vsfusbd_vllink_on_IN_finish(void *p)
{
	struct vsfusbd_VLLINK_param_t *param = p;
	struct vsf_stream_t *stream = &param->bufstream_tx.stream;

	stream->callback_rx.param = param;
	stream->callback_rx.on_connect = NULL;
	stream->callback_rx.on_disconnect = NULL;
	stream->callback_rx.on_inout = vsfusbd_vllink_on_in;
	vsfusbd_vllink_on_in(param);

	if (param->dap_connected)
		DAP_send_response_done(param->dap_param);
}

static void vllink_send_response(void *p, uint8_t *buf, uint16_t size)
{
	struct vsf_buffer_t buffer;
	struct vsfusbd_VLLINK_param_t *param = p;
	struct vsf_bufstream_t *bufstream_tx = &param->bufstream_tx;

	if (size && (size <= sizeof(param->txbuff)))
	{
		memcpy(param->txbuff, buf, size);
		if ((size < sizeof(param->txbuff)) && !(size % 64))
			size += 1;
		buffer.buffer = param->txbuff;
		buffer.size = size;
		VSFSTREAM_WRITE(bufstream_tx, &buffer);
		
		//vsfusbd_vllink_on_in(param);
	}
}

static void vsfusbd_vllink_on_rxconn(void *p)
{
	struct vsfusbd_VLLINK_param_t *param = p;

	param->OUT_transact.stream = &param->bufstream_rx.stream;
	param->OUT_transact.data_size = sizeof(param->rxbuff);
	vsfusbd_ep_recv(param->device, &param->OUT_transact);
}

static void vsfusbd_vllink_on_OUT_finish(void *p)
{
	uint32_t size;
	struct vsfusbd_VLLINK_param_t *param = p;
	struct vsf_stream_t *stream = &param->bufstream_rx.stream;

	size = VSFSTREAM_GET_DATA_SIZE(stream);
	if (size)
	{
		struct vsf_buffer_t buffer;

		if (param->dap_connected)
			DAP_recvive_request(param->dap_param, param->rxbuff, size);
		else if (DAP_register(param->dap_param, param, vllink_send_response, DAP_BULK_PACKET_SIZE) == VSFERR_NONE)
		{
			param->dap_connected = true;
			DAP_recvive_request(param->dap_param, param->rxbuff, size);
		}
		
		buffer.buffer = param->rxbuff;
		buffer.size = sizeof(param->rxbuff);
		VSFSTREAM_READ(stream, &buffer);
	}

	if (!stream->rx_ready)
	{
		stream->callback_tx.param = param;
		stream->callback_tx.on_connect = vsfusbd_vllink_on_rxconn;
		stream->callback_tx.on_disconnect = NULL;
		stream->callback_tx.on_inout = NULL;
	}
	else
		vsfusbd_vllink_on_rxconn(param);
}

static vsf_err_t vsfusbd_vllink_class_init(uint8_t iface,
		struct vsfusbd_device_t *device)
{
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_iface_t *ifs = &config->iface[iface];
	struct vsfusbd_VLLINK_param_t *param = ifs->protocol_param;

	if (!param)
		return VSFERR_INVALID_PARAMETER;

	param->device = device;
	param->IN_transact.ep = param->ep_in;
	param->IN_transact.cb.on_finish = vsfusbd_vllink_on_IN_finish;
	param->IN_transact.cb.param = param;
	param->IN_transact.zlp = false;
	param->OUT_transact.ep = param->ep_out;
	param->OUT_transact.cb.on_finish = vsfusbd_vllink_on_OUT_finish;
	param->OUT_transact.cb.param = param;
	param->OUT_transact.zlp = false;

	param->bufstream_tx.stream.op = &vsf_bufstream_op;
	param->bufstream_tx.mem.buffer.buffer = param->txbuff;
	param->bufstream_tx.mem.buffer.size = 0;
	param->bufstream_tx.mem.read = true;
	VSFSTREAM_INIT(&param->bufstream_tx);
	
	param->bufstream_rx.stream.op = &vsf_bufstream_op;
	param->bufstream_rx.mem.buffer.buffer = param->rxbuff;
	param->bufstream_rx.mem.buffer.size = sizeof(param->rxbuff);
	param->bufstream_rx.mem.read = false;
	VSFSTREAM_INIT(&param->bufstream_rx);

	VSFSTREAM_CONNECT_TX(&param->bufstream_tx);
	VSFSTREAM_CONNECT_RX(&param->bufstream_tx);
	VSFSTREAM_CONNECT_RX(&param->bufstream_rx);

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