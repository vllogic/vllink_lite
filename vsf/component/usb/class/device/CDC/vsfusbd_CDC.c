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

#ifdef VSFUSBD_CDCCFG_TRANSACT
static void vsfusbd_CDCData_on_OUT_finish(void *p);
static void vsfusbd_CDCData_on_rxconn(void *p)
{
	struct vsfusbd_CDC_param_t *param = (struct vsfusbd_CDC_param_t *)p;

	param->OUT_transact.stream = param->stream_rx;
	param->OUT_transact.data_size = 0xFFFFFFFF;
	vsfusbd_ep_recv(param->device, &param->OUT_transact);
}

static void vsfusbd_CDCData_on_OUT_finish(void *p)
{
	struct vsfusbd_CDC_param_t *param = (struct vsfusbd_CDC_param_t *)p;
	struct vsf_stream_t *stream = param->stream_rx;

	if (param->callback.on_rx_finish != NULL)
	{
		param->callback.on_rx_finish(stream->callback_rx.param);
	}

	if (!stream->rx_ready)
	{
		stream->callback_tx.param = param;
		stream->callback_tx.on_connect = vsfusbd_CDCData_on_rxconn;
		stream->callback_tx.on_disconnect = NULL;
		stream->callback_tx.on_inout = NULL;
	}
	else
	{
		vsfusbd_CDCData_on_rxconn(param);
	}
}

static void vsfusbd_CDCData_on_in(void *p)
{
	struct vsfusbd_CDC_param_t *param = (struct vsfusbd_CDC_param_t *)p;
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

static void vsfusbd_CDCData_on_IN_finish(void *p)
{
	struct vsfusbd_CDC_param_t *param = (struct vsfusbd_CDC_param_t *)p;
	struct vsf_stream_t *stream = param->stream_tx;

	if (param->callback.on_tx_finish != NULL)
	{
		param->callback.on_tx_finish(stream->callback_tx.param);
	}

	stream->callback_rx.param = param;
	stream->callback_rx.on_connect = NULL;
	stream->callback_rx.on_disconnect = NULL;
	stream->callback_rx.on_inout = vsfusbd_CDCData_on_in;
	vsfusbd_CDCData_on_in(param);
}
#else
enum vsfusbd_CDC_EVT_t
{
	VSFUSBD_CDC_EVT_STREAMTX_ONIN = VSFSM_EVT_USER_LOCAL + 0,
	VSFUSBD_CDC_EVT_STREAMRX_ONOUT = VSFSM_EVT_USER_LOCAL + 1,
	VSFUSBD_CDC_EVT_STREAMTX_ONCONN = VSFSM_EVT_USER_LOCAL + 2,
	VSFUSBD_CDC_EVT_STREAMRX_ONCONN = VSFSM_EVT_USER_LOCAL + 3,
};

static vsf_err_t vsfusbd_CDCData_OUT_hanlder(struct vsfusbd_device_t *device,
												uint8_t ep)
{
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	int8_t iface = config->ep_OUT_iface_map[ep];
	struct vsfusbd_CDC_param_t *param = NULL;
	uint16_t pkg_size, ep_size;
	uint8_t buffer[64];
	struct vsf_buffer_t rx_buffer;

	if (iface < 0)
	{
		return VSFERR_FAIL;
	}
	param = (struct vsfusbd_CDC_param_t *)config->iface[iface].protocol_param;
	if (NULL == param)
	{
		return VSFERR_FAIL;
	}

	ep_size = vsfhal_usbd_ep_get_OUT_epsize(ep);
	pkg_size = vsfhal_usbd_ep_get_OUT_count(ep);
	if (pkg_size > ep_size)
	{
		return VSFERR_FAIL;
	}
	vsfhal_usbd_ep_read_OUT_buffer(ep, buffer, pkg_size);

	rx_buffer.buffer = buffer;
	rx_buffer.size = pkg_size;
	stream_write(param->stream_rx, &rx_buffer);

	if (stream_get_free_size(param->stream_rx) < ep_size)
	{
		param->out_enable = false;
	}
	else
	{
		vsfhal_usbd_ep_enable_OUT(ep);
	}

	return VSFERR_NONE;
}

static vsf_err_t vsfusbd_CDCData_IN_hanlder(struct vsfusbd_device_t *device,
											uint8_t ep)
{
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	int8_t iface = config->ep_IN_iface_map[ep];
	struct vsfusbd_CDC_param_t *param = NULL;
	uint16_t pkg_size;
	uint8_t buffer[64];
	uint32_t tx_data_length;
	struct vsf_buffer_t tx_buffer;

	if (iface < 0)
	{
		return VSFERR_FAIL;
	}
	param = (struct vsfusbd_CDC_param_t *)config->iface[iface].protocol_param;
	if (NULL == param)
	{
		return VSFERR_FAIL;
	}

	pkg_size = vsfhal_usbd_ep_get_IN_epsize(ep);
	tx_buffer.buffer = buffer;
	tx_buffer.size = pkg_size;
	tx_data_length = stream_read(param->stream_tx, &tx_buffer);
	if (tx_data_length)
	{
		vsfhal_usbd_ep_write_IN_buffer(ep, buffer, tx_data_length);
		vsfhal_usbd_ep_set_IN_count(ep, tx_data_length);
	}
	else
	{
		param->in_enable = false;
	}

	return VSFERR_NONE;
}

static void vsfusbd_CDCData_streamtx_on_in(void *p)
{
	struct vsfusbd_CDC_param_t *param = (struct vsfusbd_CDC_param_t *)p;

	vsfsm_post_evt_pending(&param->iface->sm, VSFUSBD_CDC_EVT_STREAMTX_ONIN);
}

static void vsfusbd_CDCData_streamrx_on_out(void *p)
{
	struct vsfusbd_CDC_param_t *param = (struct vsfusbd_CDC_param_t *)p;

	vsfsm_post_evt_pending(&param->iface->sm, VSFUSBD_CDC_EVT_STREAMRX_ONOUT);
}

static void vsfusbd_CDCData_streamtx_on_txconn(void *p)
{
	struct vsfusbd_CDC_param_t *param = (struct vsfusbd_CDC_param_t *)p;

	vsfsm_post_evt_pending(&param->iface->sm, VSFUSBD_CDC_EVT_STREAMTX_ONCONN);
}

static void vsfusbd_CDCData_streamrx_on_rxconn(void *p)
{
	struct vsfusbd_CDC_param_t *param = (struct vsfusbd_CDC_param_t *)p;

	vsfsm_post_evt_pending(&param->iface->sm, VSFUSBD_CDC_EVT_STREAMRX_ONCONN);
}

static struct vsfsm_state_t *
vsfusbd_CDCData_evt_handler(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	struct vsfusbd_CDC_param_t *param =
						(struct vsfusbd_CDC_param_t *)sm->user_data;
	struct vsfusbd_device_t *device = param->device;

	switch (evt)
	{
	case VSFSM_EVT_INIT:
		param->stream_tx->callback_rx.param = param;
		param->stream_tx->callback_rx.on_inout =
							vsfusbd_CDCData_streamtx_on_in;
		param->stream_tx->callback_rx.on_connect =
							vsfusbd_CDCData_streamtx_on_txconn;
		param->stream_rx->callback_tx.param = param;
		param->stream_rx->callback_tx.on_inout =
							vsfusbd_CDCData_streamrx_on_out;
		param->stream_rx->callback_tx.on_connect =
							vsfusbd_CDCData_streamrx_on_rxconn;

		param->out_enable = false;
		param->in_enable = false;
		break;
	case VSFUSBD_CDC_EVT_STREAMTX_ONCONN:
		vsfusbd_set_IN_handler(device, param->ep_in,
										vsfusbd_CDCData_IN_hanlder);
		break;
	case VSFUSBD_CDC_EVT_STREAMRX_ONCONN:
		vsfusbd_set_OUT_handler(device, param->ep_out,
										vsfusbd_CDCData_OUT_hanlder);
		vsfsm_post_evt(sm, VSFUSBD_CDC_EVT_STREAMRX_ONOUT);
		break;
	case VSFUSBD_CDC_EVT_STREAMTX_ONIN:
		if (!param->in_enable)
		{
			param->in_enable = true;
			vsfusbd_CDCData_IN_hanlder(param->device, param->ep_in);
		}
		break;
	case VSFUSBD_CDC_EVT_STREAMRX_ONOUT:
		if (!param->out_enable &&
			(stream_get_free_size(param->stream_rx) >=
					vsfhal_usbd_ep_get_OUT_epsize(param->ep_out)))
		{
			param->out_enable = true;
			vsfhal_usbd_ep_enable_OUT(param->ep_out);
		}
		break;
	}

	return NULL;
}
#endif		// VSFUSBD_CDCCFG_TRANSACT

static vsf_err_t
vsfusbd_CDCData_class_init(uint8_t iface, struct vsfusbd_device_t *device)
{
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_iface_t *ifs = &config->iface[iface];
	struct vsfusbd_CDC_param_t *param =
						(struct vsfusbd_CDC_param_t *)ifs->protocol_param;

	if ((NULL == param) || (NULL == param->stream_tx) ||
		(NULL == param->stream_rx))
	{
		return VSFERR_INVALID_PARAMETER;
	}

	param->device = device;
#ifdef VSFUSBD_CDCCFG_TRANSACT
	param->IN_transact.ep = param->ep_in;
	param->IN_transact.cb.on_finish = vsfusbd_CDCData_on_IN_finish;
	param->IN_transact.cb.param = param;
	param->IN_transact.zlp = true;
	param->OUT_transact.ep = param->ep_out;
	param->OUT_transact.cb.on_finish = vsfusbd_CDCData_on_OUT_finish;
	param->OUT_transact.cb.param = param;
	param->OUT_transact.zlp = false;

	vsfusbd_CDCData_on_IN_finish(param);
	vsfusbd_CDCData_on_OUT_finish(param);
	return VSFERR_NONE;
#else
	ifs->sm.init_state.evt_handler = vsfusbd_CDCData_evt_handler;
	param->iface = ifs;
	ifs->sm.user_data = (void*)param;
	return vsfsm_init(&ifs->sm);
#endif
}

void vsfusbd_CDCData_connect(struct vsfusbd_CDC_param_t *param)
{
	stream_connect_tx(param->stream_rx);
	stream_connect_rx(param->stream_tx);
}

static vsf_err_t vsfusbd_CDCControl_request_prepare(struct vsfusbd_device_t *device)
{
	struct vsfusbd_ctrl_handler_t *ctrl_handler = &device->ctrl_handler;
	struct vsf_buffer_t *buffer = &ctrl_handler->bufstream.mem.buffer;
	struct usb_ctrlrequest_t *request = &ctrl_handler->request;
	uint8_t iface = request->wIndex;
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_CDC_param_t *param =
			(struct vsfusbd_CDC_param_t *)config->iface[iface].protocol_param;

	switch (request->bRequest)
	{
	case USB_CDCREQ_SEND_ENCAPSULATED_COMMAND:
		if (request->wLength > param->encapsulated_command.size)
		{
			return VSFERR_FAIL;
		}
		buffer->buffer = param->encapsulated_command.buffer;
		buffer->size = request->wLength;
		break;
	case USB_CDCREQ_GET_ENCAPSULATED_RESPONSE:
		if (request->wLength > param->encapsulated_response.size)
		{
			request->wLength = param->encapsulated_response.size;
		}
		buffer->buffer = param->encapsulated_response.buffer;
		buffer->size = request->wLength;
		break;
	default:
		return VSFERR_FAIL;
	}
	ctrl_handler->data_size = buffer->size;
	return VSFERR_NONE;
}

static vsf_err_t vsfusbd_CDCControl_request_process(struct vsfusbd_device_t *device)
{
	struct vsfusbd_ctrl_handler_t *ctrl_handler = &device->ctrl_handler;
	struct vsf_buffer_t *buffer = &ctrl_handler->bufstream.mem.buffer;
	struct usb_ctrlrequest_t *request = &ctrl_handler->request;
	uint8_t iface = request->wIndex;
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_CDC_param_t *param =
			(struct vsfusbd_CDC_param_t *)config->iface[iface].protocol_param;

	if (USB_CDCREQ_SEND_ENCAPSULATED_COMMAND == request->bRequest)
	{
		if ((param->callback.send_encapsulated_command != NULL) &&
			param->callback.send_encapsulated_command(param, buffer))
		{
			return VSFERR_FAIL;
		}
	}
	return VSFERR_NONE;
}

const struct vsfusbd_class_protocol_t vsfusbd_CDCControl_class =
{
	.request_prepare = vsfusbd_CDCControl_request_prepare,
	.request_process = vsfusbd_CDCControl_request_process,
};

const struct vsfusbd_class_protocol_t vsfusbd_CDCData_class =
{
	.init = vsfusbd_CDCData_class_init,
};

