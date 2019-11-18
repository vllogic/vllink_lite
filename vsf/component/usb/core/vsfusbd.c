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

// events for vsfusbd
#define VSFUSBD_INTEVT_BASE				VSFSM_EVT_USER_LOCAL
enum vsfusbd_evt_t
{
	VSFUSBD_INTEVT_RESET = VSFUSBD_INTEVT_BASE + 0,
	VSFUSBD_INTEVT_SUSPEND = VSFUSBD_INTEVT_BASE + 1,
	VSFUSBD_INTEVT_RESUME = VSFUSBD_INTEVT_BASE + 2,
	VSFUSBD_INTEVT_DETACH = VSFUSBD_INTEVT_BASE + 3,
	VSFUSBD_INTEVT_ATTACH = VSFUSBD_INTEVT_BASE + 4,
	VSFUSBD_INTEVT_SOF = VSFUSBD_INTEVT_BASE + 5,
	VSFUSBD_INTEVT_SETUP = VSFUSBD_INTEVT_BASE + 6,
	VSFUSBD_INTEVT_INOUT = VSFUSBD_INTEVT_BASE + 0x20,
	VSFUSBD_STREAM_INOUT = VSFUSBD_INTEVT_BASE + 0x40,
	VSFUSBD_STREAM_CLOSE_INOUT = VSFUSBD_INTEVT_BASE + 0x60,
	VSFUSBD_INTEVT_ERR = VSFUSBD_INTEVT_BASE + 0x100,
};
#define VSFUSBD_EVT_EP_MASK				0x0F
#define VSFUSBD_EVT_DIR_MASK			0x10
#define VSFUSBD_EVT_DIR_IN				0x10
#define VSFUSBD_EVT_DIR_OUT				0x00
#define VSFUSBD_EVT_EVT_MASK			~0xF
#define VSFUSBD_EVT_ERR_MASK			0xFF
#define VSFUSBD_INTEVT_IN				(VSFUSBD_INTEVT_INOUT | VSFUSBD_EVT_DIR_IN)
#define VSFUSBD_INTEVT_OUT				(VSFUSBD_INTEVT_INOUT | VSFUSBD_EVT_DIR_OUT)
#define VSFUSBD_STREAM_IN				(VSFUSBD_STREAM_INOUT | VSFUSBD_EVT_DIR_IN)
#define VSFUSBD_STREAM_OUT				(VSFUSBD_STREAM_INOUT | VSFUSBD_EVT_DIR_OUT)
#define VSFUSBD_STREAM_CLOSE_IN			(VSFUSBD_STREAM_CLOSE_INOUT | VSFUSBD_EVT_DIR_IN)
#define VSFUSBD_STREAM_CLOSE_OUT		(VSFUSBD_STREAM_CLOSE_INOUT | VSFUSBD_EVT_DIR_OUT)

vsf_err_t vsfusbd_device_get_descriptor(struct vsfusbd_device_t *device,
		struct vsfusbd_desc_filter_t *filter, uint8_t type, uint8_t index,
		uint16_t lanid, struct vsf_buffer_t *buffer)
{
	while ((filter->buffer.buffer != NULL) && (filter->buffer.size != 0))
	{
		if ((filter->type == type) && (filter->index == index) &&
			(filter->lanid == lanid))
		{
			buffer->size = filter->buffer.size;
			buffer->buffer = filter->buffer.buffer;
			return VSFERR_NONE;
		}
		filter++;
	}
	return VSFERR_FAIL;
}

static vsf_err_t vsfusbd_on_IN_do(struct vsfusbd_device_t *device, uint8_t ep);
static vsf_err_t vsfusbd_on_OUT_do(struct vsfusbd_device_t *device, uint8_t ep);

vsf_err_t vsfusbd_set_IN_handler(struct vsfusbd_device_t *device,
		uint8_t ep, vsf_err_t (*handler)(struct vsfusbd_device_t*, uint8_t))
{
	if (ep > VSFUSBD_CFG_EPMAXNO)
		return VSFERR_FAIL;
	device->IN_handler[ep] = handler;
	return VSFERR_NONE;
}

vsf_err_t vsfusbd_set_OUT_handler(struct vsfusbd_device_t *device,
		uint8_t ep, vsf_err_t (*handler)(struct vsfusbd_device_t*, uint8_t))
{
	if (ep > VSFUSBD_CFG_EPMAXNO)
		return VSFERR_FAIL;
	device->OUT_handler[ep] = handler;
	return VSFERR_NONE;
}

static void vsfusbd_stream_on_disconnect_OUT(void *p)
{
	struct vsfusbd_transact_t *transact = (struct vsfusbd_transact_t *)p;
	struct vsfusbd_device_t *device = transact->device;
	uint8_t ep = transact->ep;

	vsfsm_post_evt_pending(&device->sm, VSFUSBD_STREAM_CLOSE_OUT | ep);
}

static void vsfusbd_stream_on_out(void *p)
{
	struct vsfusbd_transact_t *transact = (struct vsfusbd_transact_t *)p;
	struct vsfusbd_device_t *device = transact->device;

	vsfsm_post_evt_pending(&device->sm, VSFUSBD_STREAM_OUT | transact->ep);
}

static void vsfusbd_transact_out(struct vsfusbd_device_t *device,
									struct vsfusbd_transact_t *transact)
{
	uint8_t ep = transact->ep;

	if (transact->idle)
	{
		uint16_t ep_size = vsfhal_usbd_ep_get_OUT_epsize(ep);
		uint16_t pkg_size = min(ep_size, transact->data_size);
		uint32_t free_size = stream_get_free_size(transact->stream);

		transact->idle = free_size < pkg_size;
		if (!transact->idle)
		{
			vsfhal_usbd_ep_enable_OUT(ep);
		}
	}
}

vsf_err_t vsfusbd_ep_recv(struct vsfusbd_device_t *device,
								struct vsfusbd_transact_t *transact)
{
	uint8_t ep = transact->ep;

	if (ep > VSFUSBD_CFG_EPMAXNO)
		return VSFERR_FAIL;
	device->OUT_handler[ep] = vsfusbd_on_OUT_do;
	device->OUT_transact[ep] = transact;
	transact->idle = true;
	transact->device = device;
	if (transact->data_size)
	{
		struct vsf_stream_t *stream = transact->stream;

		stream->callback_tx.param = transact;
		stream->callback_tx.on_connect = NULL;
		stream->callback_tx.on_disconnect = vsfusbd_stream_on_disconnect_OUT;
		stream->callback_tx.on_inout = vsfusbd_stream_on_out;
		stream_connect_tx(stream);
		vsfusbd_transact_out(device, transact);
	}
	else
	{
		vsfhal_usbd_ep_enable_OUT(ep);
	}

	return VSFERR_NONE;
}

void vsfusbd_ep_cancel_recv(struct vsfusbd_device_t *device,
								struct vsfusbd_transact_t *transact)
{
	if (transact->stream != NULL)
	{
		transact->stream->callback_tx.on_disconnect = NULL;
		transact->stream->callback_tx.on_inout = NULL;
	}
	if (transact->ep <= VSFUSBD_CFG_EPMAXNO)
	{
		device->OUT_handler[transact->ep] = NULL;
		device->OUT_transact[transact->ep] = NULL;
		if (transact->cb.on_finish != NULL)
		{
			transact->cb.on_finish(transact->cb.param);
		}
	}
}

static void vsfusbd_stream_on_disconnect_IN(void *p)
{
	struct vsfusbd_transact_t *transact = (struct vsfusbd_transact_t *)p;
	struct vsfusbd_device_t *device = transact->device;
	uint8_t ep = transact->ep;

	vsfsm_post_evt_pending(&device->sm, VSFUSBD_STREAM_CLOSE_IN | ep);
}

static void vsfusbd_stream_on_in(void *p)
{
	struct vsfusbd_transact_t *transact = (struct vsfusbd_transact_t *)p;
	struct vsfusbd_device_t *device = transact->device;

	vsfsm_post_evt_pending(&device->sm, VSFUSBD_STREAM_IN | transact->ep);
}

static vsf_err_t vsfusbd_on_IN_do(struct vsfusbd_device_t *device, uint8_t ep);
vsf_err_t vsfusbd_ep_send(struct vsfusbd_device_t *device,
								struct vsfusbd_transact_t *transact)
{
	uint8_t ep = transact->ep;

	if (ep > VSFUSBD_CFG_EPMAXNO)
		return VSFERR_FAIL;
	device->IN_handler[ep] = vsfusbd_on_IN_do;
	device->IN_transact[ep] = transact;
	transact->idle = true;
	transact->device = device;
	if (transact->data_size)
	{
		struct vsf_stream_t *stream = transact->stream;

		stream->callback_rx.param = transact;
		stream->callback_rx.on_connect = NULL;
		stream->callback_rx.on_disconnect = vsfusbd_stream_on_disconnect_IN;
		stream->callback_rx.on_inout = vsfusbd_stream_on_in;
		stream_connect_rx(stream);

		vsfusbd_on_IN_do(device, ep);
	}
	else
	{
		// send zlp
		vsfhal_usbd_ep_set_IN_count(ep, 0);
	}

	return VSFERR_NONE;
}

void vsfusbd_ep_cancel_send(struct vsfusbd_device_t *device,
								struct vsfusbd_transact_t *transact)
{
	if (transact->stream != NULL)
	{
		transact->stream->callback_rx.on_disconnect = NULL;
		transact->stream->callback_rx.on_inout = NULL;
	}
	if (transact->ep <= VSFUSBD_CFG_EPMAXNO)
	{
		device->IN_handler[transact->ep] = NULL;
		device->IN_transact[transact->ep] = NULL;
		if (transact->cb.on_finish != NULL)
		{
			transact->cb.on_finish(transact->cb.param);
		}
	}
}

// standard request handlers
static int16_t
vsfusbd_get_config(struct vsfusbd_device_t *device, uint8_t value)
{
	uint8_t i;

	for (i = 0; i < device->num_of_configuration; i++)
	{
		if (value == device->config[i].configuration_value)
		{
			return i;
		}
	}
	return -1;
}

#if VSFUSBD_CFG_AUTOSETUP
static vsf_err_t vsfusbd_auto_init(struct vsfusbd_device_t *device)
{
	struct vsfusbd_config_t *config;
	struct vsf_buffer_t desc = {NULL, 0};
	enum vsfhal_usbd_eptype_t ep_type;
	uint16_t pos;
	uint8_t attr, feature;
	uint16_t ep_size;
	uint8_t ep_addr, ep_index, ep_attr;
	int16_t cur_iface;

	config = &device->config[device->configuration];

	// config other eps according to descriptors
	if (vsfusbd_device_get_descriptor(device, device->desc_filter,
				USB_DT_CONFIG, device->configuration, 0, &desc)
#if __VSF_DEBUG__
		|| (NULL == desc.buffer) || (desc.size <= USB_DESC_SIZE_CONFIGURATION)
		|| (desc.buffer[0] != USB_DESC_SIZE_CONFIGURATION)
		|| (desc.buffer[1] != USB_DESC_TYPE_CONFIGURATION)
		|| (config->num_of_ifaces != desc.buffer[USB_DESC_CONFIG_OFF_IFNUM])
#endif
		)
	{
		return VSFERR_FAIL;
	}

	// initialize device feature according to
	// bmAttributes field in configuration descriptor
	attr = desc.buffer[7];
	feature = 0;
	if (attr & USB_CONFIG_ATT_SELFPOWER)
	{
		feature |= 1 << USB_DEVICE_SELF_POWERED;
	}
	if (attr & USB_CONFIG_ATT_WAKEUP)
	{
		feature |= 1 << USB_DEVICE_REMOTE_WAKEUP;
	}

#if __VSF_DEBUG__
	num_iface = desc.buffer[USB_DESC_CONFIG_OFF_IFNUM];
	num_endpoint = 0;
#endif

	cur_iface = -1;
	pos = USB_DT_CONFIG_SIZE;
	while (desc.size > pos)
	{
#if __VSF_DEBUG__
		if ((desc.buffer[pos] < 2) || (desc.size < (pos + desc.buffer[pos])))
		{
			return VSFERR_FAIL;
		}
#endif
		switch (desc.buffer[pos + 1])
		{
		case USB_DT_INTERFACE:
#if __VSF_DEBUG__
			if (num_endpoint)
			{
				return VSFERR_FAIL;
			}
			num_endpoint = desc.buffer[pos + 4];
			num_iface--;
#endif
			cur_iface = desc.buffer[pos + 2];
			break;
		case USB_DT_ENDPOINT:
			ep_addr = desc.buffer[pos + 2];
			ep_attr = desc.buffer[pos + 3];
			ep_size = GET_LE_U16(&desc.buffer[pos + 4]);
			ep_index = ep_addr & 0x0F;
			if (ep_index > VSFUSBD_CFG_EPMAXNO)
			{
				return VSFERR_FAIL;
			}
#if __VSF_DEBUG__
			num_endpoint--;
			if (ep_index > (*vsfhal_usbd_ep_num_of_ep - 1))
			{
				return VSFERR_FAIL;
			}
#endif
			switch (ep_attr & 0x03)
			{
			case 0x00:
				ep_type = USB_EP_TYPE_CONTROL;
				break;
			case 0x01:
				ep_type = USB_EP_TYPE_ISO;
				break;
			case 0x02:
				ep_type = USB_EP_TYPE_BULK;
				break;
			case 0x03:
				ep_type = USB_EP_TYPE_INTERRUPT;
				break;
			default:
				return VSFERR_FAIL;
			}
			if (ep_addr & 0x80)
			{
				// IN ep
				vsfhal_usbd_ep_set_IN_epsize(ep_index, ep_size);
				config->ep_IN_iface_map[ep_index] = cur_iface;
			}
			else
			{
				// OUT ep
				vsfhal_usbd_ep_set_OUT_epsize(ep_index, ep_size);
				config->ep_OUT_iface_map[ep_index] = cur_iface;
			}
			vsfhal_usbd_ep_set_type(ep_index, ep_type);
			break;
		}
		pos += desc.buffer[pos];
	}
#if __VSF_DEBUG__
	if (num_iface || num_endpoint || (desc.size != pos))
	{
		return VSFERR_FAIL;
	}
#endif
	return VSFERR_NONE;
}
#endif	// VSFUSBD_CFG_AUTOSETUP

static vsf_err_t vsfusbd_stdctrl_prepare(struct vsfusbd_device_t *device)
{
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_ctrl_handler_t *ctrl_handler = &device->ctrl_handler;
	struct usb_ctrlrequest_t *request = &ctrl_handler->request;
	struct vsf_buffer_t *buffer = &ctrl_handler->bufstream.mem.buffer;
	uint8_t *reply_buffer = ctrl_handler->reply_buffer;
	uint8_t recip = request->bRequestType & USB_RECIP_MASK;

	if (USB_RECIP_DEVICE == recip)
	{
		switch (request->bRequest)
		{
		case USB_REQ_GET_STATUS:
			if ((request->wValue != 0) || (request->wIndex != 0))
			{
				goto fail;
			}
#ifndef BOARD_TYPE_WINBOX_P1_NEW_BTDPAIR
			reply_buffer[0] = device->feature;
#else
			reply_buffer[0] = 1;
#endif
			reply_buffer[1] = 0;
			buffer->size = 2;
			break;
		case USB_REQ_CLEAR_FEATURE:
			if ((request->wIndex != 0) ||
				(request->wValue != USB_DEVICE_REMOTE_WAKEUP))
			{
				goto fail;
			}
			device->feature &= ~USB_CONFIG_ATT_WAKEUP;
			break;
		case USB_REQ_SET_FEATURE:
			if ((request->wIndex != 0) ||
				(request->wValue != USB_DEVICE_REMOTE_WAKEUP))
			{
				goto fail;
			}
			device->feature |= USB_CONFIG_ATT_WAKEUP;
			break;
		case USB_REQ_SET_ADDRESS:
			if ((request->wValue > 127) || (request->wIndex != 0) ||
				(device->configuration != 0))
			{
				goto fail;
			}
			break;
		case USB_REQ_GET_DESCRIPTOR:
			{
				uint8_t type = (request->wValue >> 8) & 0xFF;
				uint8_t index = request->wValue & 0xFF;
				uint16_t lanid = request->wIndex;

				if (vsfusbd_device_get_descriptor(device, device->desc_filter,
								type, index, lanid, buffer))
				{
					goto fail;
				}
			}
			break;
		case USB_REQ_GET_CONFIGURATION:
			if ((request->wValue != 0) || (request->wIndex != 0))
			{
				goto fail;
			}
			reply_buffer[0] = config->configuration_value;
			buffer->size = 2;
			break;
		case USB_REQ_SET_CONFIGURATION:
			if ((request->wIndex != 0) ||
				(vsfusbd_get_config(device, request->wValue) < 0))
			{
				goto fail;
			}
			device->configured = false;
			break;
		default:
			goto fail;
		}
	}
	else if (USB_RECIP_INTERFACE == recip)
	{
		uint8_t iface_idx = request->wIndex;
		struct vsfusbd_iface_t *iface = &config->iface[iface_idx];
		struct vsfusbd_class_protocol_t *protocol = iface->class_protocol;

		if (iface_idx >= config->num_of_ifaces)
		{
			goto fail;
		}

		switch (request->bRequest)
		{
		case USB_REQ_GET_STATUS:
			if ((request->wValue != 0) ||
				(request->wIndex >= config->num_of_ifaces))
			{
				goto fail;
			}
			reply_buffer[0] = 0;
			reply_buffer[1] = 0;
			buffer->size = 2;
			break;
		case USB_REQ_CLEAR_FEATURE:
			break;
		case USB_REQ_SET_FEATURE:
			break;
		case USB_REQ_GET_DESCRIPTOR:
			{
				uint8_t type = (request->wValue >> 8) & 0xFF;
				uint8_t index = request->wValue & 0xFF;

				if ((NULL == protocol) || (NULL == protocol->get_desc) ||
					protocol->get_desc(device, type, index, 0, buffer))
				{
					goto fail;
				}
			}
			break;
		case USB_REQ_GET_INTERFACE:
			if (request->wValue != 0)
			{
				goto fail;
			}
			reply_buffer[0] = iface->alternate_setting;
			buffer->size = 1;
			break;
		case USB_REQ_SET_INTERFACE:
			iface->alternate_setting = request->wValue;
			if (protocol && protocol->request_prepare)
			{
				protocol->request_prepare(device);
			}
			break;
		default:
			goto fail;
		}
	}
	else if (USB_RECIP_ENDPOINT == recip)
	{
		uint8_t ep_num = request->wIndex & 0x7F;
		uint8_t ep_dir = request->wIndex & 0x80;

		if ((request->bRequestType & USB_DIR_MASK) == USB_DIR_IN)
		{
			return VSFERR_FAIL;
		}

		switch (request->bRequest)
		{
		case USB_REQ_GET_STATUS:
			if ((request->wValue != 0) ||
				(request->wIndex >= vsfhal_usbd_ep_num))
			{
				goto fail;
			}
			if ((ep_dir && vsfhal_usbd_ep_is_IN_stall(ep_num)) ||
				(!ep_dir && vsfhal_usbd_ep_is_OUT_stall(ep_num)))
			{
				reply_buffer[0] = 1;
			}
			else
			{
				reply_buffer[0] = 0;
			}
			reply_buffer[1] = 0;
			buffer->size = 2;
			break;
		case USB_REQ_CLEAR_FEATURE:
			if ((request->wValue != USB_ENDPOINT_HALT) ||
				(ep_num >= vsfhal_usbd_ep_num))
			{
				goto fail;
			}
			if (ep_dir)
			{
				vsfhal_usbd_ep_reset_IN_toggle(ep_num);
				vsfhal_usbd_ep_clear_IN_stall(ep_num);
			}
			else
			{
				vsfhal_usbd_ep_reset_OUT_toggle(ep_num);
				vsfhal_usbd_ep_clear_OUT_stall(ep_num);
				vsfhal_usbd_ep_enable_OUT(ep_num);
			}
			break;
		case USB_REQ_SET_FEATURE:
		default:
			goto fail;
		}
	}
	else
	{
	fail:
		return VSFERR_FAIL;
	}

	ctrl_handler->data_size = buffer->size;
	return VSFERR_NONE;
}

static vsf_err_t vsfusbd_stdctrl_process(struct vsfusbd_device_t *device)
{
	struct usb_ctrlrequest_t *request = &device->ctrl_handler.request;
	uint8_t recip = request->bRequestType & USB_RECIP_MASK;

	if (USB_RECIP_DEVICE == recip)
	{
		switch (request->bRequest)
		{
		case USB_REQ_SET_ADDRESS:
			device->address = (uint8_t)request->wValue;
			return vsfhal_usbd_set_address(device->address);
		case USB_REQ_SET_CONFIGURATION:
			{
				struct vsfusbd_config_t *config;
				int16_t config_idx;
				uint8_t i;
#if __VSF_DEBUG__
				uint8_t num_iface, num_endpoint;
#endif

				config_idx = vsfusbd_get_config(device, request->wValue);
				if (config_idx < 0)
				{
					return VSFERR_FAIL;
				}
				device->configuration = (uint8_t)config_idx;
				config = &device->config[device->configuration];

#if VSFUSBD_CFG_AUTOSETUP
				if (vsfusbd_auto_init(device))
				{
					return VSFERR_FAIL;
				}
#endif

				// call user initialization
				if ((config->init != NULL) && config->init(device))
				{
					return VSFERR_FAIL;
				}

				for (i = 0; i < config->num_of_ifaces; i++)
				{
					config->iface[i].alternate_setting = 0;

					if ((config->iface[i].class_protocol != NULL) &&
							(config->iface[i].class_protocol->init != NULL) &&
							config->iface[i].class_protocol->init(i, device))
					{
						return VSFERR_FAIL;
					}
				}

				device->configured = true;
			}
			break;
		}
	}
	else if (USB_RECIP_INTERFACE == recip)
	{
		uint8_t iface_idx = request->wIndex;
		struct vsfusbd_config_t *config = &device->config[device->configuration];
		struct vsfusbd_iface_t *iface = &config->iface[iface_idx];
		struct vsfusbd_class_protocol_t *protocol = iface->class_protocol;

		if (iface_idx >= config->num_of_ifaces)
		{
			return VSFERR_FAIL;
		}

		switch (request->bRequest)
		{
		case USB_REQ_SET_INTERFACE:
			if (protocol && protocol->request_process)
			{
				protocol->request_process(device);
			}
			break;
		}
	}
	return VSFERR_NONE;
}

static vsf_err_t vsfusbd_ctrl_prepare(struct vsfusbd_device_t *device)
{
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_ctrl_handler_t *ctrl_handler = &device->ctrl_handler;
	struct usb_ctrlrequest_t *request = &ctrl_handler->request;
	uint8_t type = request->bRequestType & USB_TYPE_MASK;
	vsf_err_t err = VSFERR_FAIL;

	// set default stream
	ctrl_handler->stream = (struct vsf_stream_t *)&ctrl_handler->bufstream;
	ctrl_handler->bufstream.stream.op = &bufstream_op;
	ctrl_handler->bufstream.mem.buffer.buffer = ctrl_handler->reply_buffer;
	ctrl_handler->bufstream.mem.buffer.size = 0;
	ctrl_handler->bufstream.mem.read =
						(request->bRequestType & USB_DIR_MASK) == USB_DIR_IN;

	if (USB_TYPE_STANDARD == type)
	{
		err = vsfusbd_stdctrl_prepare(device);
	}
	else if (USB_TYPE_CLASS == type)
	{
		int8_t iface = -1;

		switch (request->bRequestType & USB_RECIP_MASK)
		{
		case USB_RECIP_DEVICE:
			iface = (int8_t)device->device_class_iface;
			break;
		case USB_RECIP_INTERFACE:
			iface = (int8_t)request->wIndex;
			break;
		}
		if ((iface >= 0) && (iface < config->num_of_ifaces) &&
			(config->iface[iface].class_protocol->request_prepare != NULL))
		{
			err = config->iface[iface].class_protocol->request_prepare(device);
		}
	}
	else if (USB_TYPE_VENDOR == type)
	{
		if (config->vendor_prepare != NULL)
			err = config->vendor_prepare(device);
	}
	return err ? err : !ctrl_handler->data_size ? VSFERR_NONE :
										stream_init(ctrl_handler->stream);
}

static void vsfusbd_ctrl_process(struct vsfusbd_device_t *device)
{
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_ctrl_handler_t *ctrl_handler = &device->ctrl_handler;
	struct usb_ctrlrequest_t *request = &ctrl_handler->request;
	uint8_t type = request->bRequestType & USB_TYPE_MASK;

	if (USB_TYPE_STANDARD == type)
	{
		vsfusbd_stdctrl_process(device);
	}
	else if (USB_TYPE_CLASS == type)
	{
		int8_t iface = -1;

		switch (request->bRequestType & USB_RECIP_MASK)
		{
		case USB_RECIP_DEVICE:
			iface = (int8_t)device->device_class_iface;
			break;
		case USB_RECIP_INTERFACE:
			iface = (int8_t)request->wIndex;
			break;
		}
		if ((iface >= 0) && (iface < config->num_of_ifaces) &&
			(config->iface[iface].class_protocol->request_process != NULL))
		{
			config->iface[iface].class_protocol->request_process(device);
		}
	}
}

// on_IN and on_OUT
static vsf_err_t vsfusbd_on_IN_do(struct vsfusbd_device_t *device, uint8_t ep)
{
	struct vsfusbd_transact_t *transact = device->IN_transact[ep];

	if (transact->data_size)
	{
		uint32_t data_size = stream_get_data_size(transact->stream);
		uint16_t ep_size = vsfhal_usbd_ep_get_IN_epsize(ep);
		uint16_t cur_size = min(transact->data_size, ep_size);

		transact->idle = (data_size < cur_size);
		if (transact->idle)
		{
			// If not enough space in stream and stream_tx disconnected, cancel
			if (!transact->stream->tx_ready &&
				(data_size < transact->data_size))
				goto cancel;
			return VSFERR_NONE;
		}

		transact->data_size -= cur_size;
		if (!transact->data_size && (cur_size < ep_size))
		{
			transact->zlp = false;
		}

		{
			struct vsf_buffer_t buffer = {.buffer = device->tmpbuf, .size = cur_size,};
			stream_read(transact->stream, &buffer);
		}

		vsfhal_usbd_ep_write_IN_buffer(ep, device->tmpbuf, cur_size);
		vsfhal_usbd_ep_set_IN_count(ep, cur_size);
	}
	else if (transact->zlp)
	{
		transact->zlp = false;
		vsfhal_usbd_ep_set_IN_count(ep, 0);
	}
	else
	{
cancel:
		vsfusbd_ep_cancel_send(device, transact);
	}

	return VSFERR_NONE;
}

static vsf_err_t vsfusbd_on_OUT_do(struct vsfusbd_device_t *device, uint8_t ep)
{
	struct vsfusbd_transact_t *transact = device->OUT_transact[ep];
	uint16_t ep_size = vsfhal_usbd_ep_get_OUT_epsize(ep);
	uint16_t pkg_size, next_pkg_size;
	uint32_t free_size = 0;
	bool last = true;

	pkg_size = vsfhal_usbd_ep_get_OUT_count(ep);
	vsfhal_usbd_ep_read_OUT_buffer(ep, device->tmpbuf, pkg_size);

	if (transact->data_size < pkg_size)
	{
		return VSFERR_BUG;
	}
	transact->data_size -= pkg_size;

	// If not enough data in buffer, and stream_rx is disconnected, set last
	if (transact->data_size)
	{
		free_size = stream_get_free_size(transact->stream) - pkg_size;
		transact->idle = last = (pkg_size < ep_size) || !transact->data_size ||
			(!transact->stream->rx_ready && (free_size < transact->data_size));
	}
	if (!last)
	{
		next_pkg_size = min(transact->data_size, ep_size);
		transact->idle = (free_size < next_pkg_size);
		if (!transact->idle)
		{
			vsfhal_usbd_ep_enable_OUT(ep);
		}
	}

	if (pkg_size > 0)
	{
		struct vsf_buffer_t buffer = {.buffer = device->tmpbuf, .size = pkg_size,};
		stream_write(transact->stream, &buffer);
	}

	if (last)
	{
		vsfusbd_ep_cancel_recv(device, transact);
	}
	return VSFERR_NONE;
}

static void vsfusbd_setup_end_callback(void *param)
{
	vsfusbd_ctrl_process((struct vsfusbd_device_t *)param);
}

static void vsfusbd_setup_status_callback(void *param)
{
	struct vsfusbd_device_t *device = param;
	struct vsfusbd_ctrl_handler_t *ctrl_handler = &device->ctrl_handler;
	struct usb_ctrlrequest_t *request = &ctrl_handler->request;
	struct vsfusbd_transact_t *transact;
	bool out = (request->bRequestType & USB_DIR_MASK) == USB_DIR_OUT;

	transact = out ? &ctrl_handler->IN_transact : &ctrl_handler->OUT_transact;
	transact->stream = NULL;
	transact->data_size = 0;
	transact->zlp = false;
	transact->cb.param = device;
	transact->cb.on_finish = vsfusbd_setup_end_callback;

	if (out)
	{
		vsfusbd_ep_send(device, transact);
	}
	else
	{
		vsfusbd_ep_recv(device, transact);
	}
}

// interrupts, simply send(pending) interrupt event to sm
static void vsfusbd_on_EVENT(void *p, enum vsfhal_usbd_evt_t evt, uint32_t value)
{
	struct vsfusbd_device_t *device = p;
	struct vsfsm_t *sm = &device->sm;
	vsfsm_evt_t intevt = VSFSM_EVT_INVALID;

	switch (evt)
	{
	case VSFHAL_USBD_ON_ATTACH:	intevt = VSFUSBD_INTEVT_ATTACH;		break;
	case VSFHAL_USBD_ON_DETACH:	intevt = VSFUSBD_INTEVT_DETACH;		break;
	case VSFHAL_USBD_ON_RESET:	intevt = VSFUSBD_INTEVT_RESET;		break;
	case VSFHAL_USBD_ON_SETUP:	intevt = VSFUSBD_INTEVT_SETUP;		break;
	case VSFHAL_USBD_ON_ERROR:	intevt = VSFUSBD_INTEVT_ERR | value;break;
	case VSFHAL_USBD_ON_SUSPEND:intevt = VSFUSBD_INTEVT_SUSPEND;	break;
	case VSFHAL_USBD_ON_RESUME:	intevt = VSFUSBD_INTEVT_RESUME;		break;
	case VSFHAL_USBD_ON_SOF:	intevt = VSFUSBD_INTEVT_SOF;		break;
	case VSFHAL_USBD_ON_IN:		intevt = VSFUSBD_INTEVT_IN | value;	break;
	case VSFHAL_USBD_ON_OUT:	intevt = VSFUSBD_INTEVT_OUT | value;break;
	}
	if (intevt != VSFSM_EVT_INVALID)
		vsfsm_post_evt_pending(sm, intevt);
}

static void vsfusbd_usr_cb(struct vsfusbd_device_t *usbd,
		enum vsfusbd_usr_evt_t evt, void *param)
{
	if (usbd->on_EVENT)
		usbd->on_EVENT(usbd, evt, param);
}

// state machines
static struct vsfsm_state_t *
vsfusbd_evt_handler(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	struct vsfusbd_device_t *device =
								container_of(sm, struct vsfusbd_device_t, sm);
	vsf_err_t err = VSFERR_NONE;

	switch (evt)
	{
	case VSFSM_EVT_ENTER:
	case VSFSM_EVT_EXIT:
		break;
	case VSFSM_EVT_FINI:
		vsfhal_usbd_fini();
		vsfhal_usbd_disconnect();
		vsfusbd_usr_cb(device, VSFUSBD_USREVT_FINI, NULL);
		break;
	case VSFSM_EVT_INIT:
		{
		#if VSFUSBD_CFG_AUTOSETUP
			struct vsf_buffer_t desc = {NULL, 0};
			uint8_t i;
		#endif

			device->configured = false;
			device->configuration = 0;
			device->feature = 0;

		#if VSFUSBD_CFG_AUTOSETUP
			for (i = 0; i < device->num_of_configuration; i++)
			{
				if (vsfusbd_device_get_descriptor(device, device->desc_filter,
									USB_DT_CONFIG, i, 0, &desc)
		#if __VSF_DEBUG__
					|| (NULL == desc.buffer)
					|| (desc.size <= USB_DESC_SIZE_CONFIGURATION)
					|| (desc.buffer[0] != USB_DESC_SIZE_CONFIGURATION)
					|| (desc.buffer[1] != USB_DESC_TYPE_CONFIGURATION)
					|| (config->num_of_ifaces !=
								desc.buffer[USB_DESC_CONFIG_OFF_IFNUM])
		#endif
					)
				{
					err = VSFERR_FAIL;
					goto init_exit;
				}
				device->config[i].configuration_value = desc.buffer[5];
			}
		#endif	// VSFUSBD_CFG_AUTOSETUP

			// initialize callback for low level driver before
			// initializing the hardware
			vsfhal_usbd_callback.param = (void *)device;
			vsfhal_usbd_callback.on_event = vsfusbd_on_EVENT;

			if (vsfhal_usbd_init(VSFHAL_USBD_PRIORITY))
			{
				err = VSFERR_FAIL;
				goto init_exit;
			}
			vsfusbd_usr_cb(device, VSFUSBD_USREVT_INIT, NULL);

		init_exit:
			break;
		}
	case VSFUSBD_INTEVT_RESET:
		{
			struct vsfusbd_config_t *config;
			uint8_t i;
		#if VSFUSBD_CFG_AUTOSETUP
			struct vsf_buffer_t desc = {NULL, 0};
			uint16_t ep_size;
		#endif
			memset(device->IN_transact, 0, sizeof(device->IN_transact));
			memset(device->OUT_transact, 0,sizeof(device->OUT_transact));
			memset(device->IN_handler, 0, sizeof(device->IN_handler));
			memset(device->OUT_handler, 0, sizeof(device->OUT_handler));

			device->configured = false;
			device->configuration = 0;
			device->feature = 0;

			for (i = 0; i < device->num_of_configuration; i++)
			{
				config = &device->config[i];
				memset(config->ep_OUT_iface_map, -1,
											sizeof(config->ep_OUT_iface_map));
				memset(config->ep_IN_iface_map, -1,
											sizeof(config->ep_OUT_iface_map));
			}

			// reset usb hw
			if (vsfhal_usbd_reset())
			{
				err = VSFERR_FAIL;
				goto reset_exit;
			}

		#if VSFUSBD_CFG_AUTOSETUP
			if (vsfusbd_device_get_descriptor(device, device->desc_filter,
												USB_DT_DEVICE, 0, 0, &desc)
		#if __VSF_DEBUG__
				|| (NULL == desc.buffer) || (desc.size != USB_DESC_SIZE_DEVICE)
				|| (desc.buffer[0] != desc.size)
				|| (desc.buffer[1] != USB_DESC_TYPE_DEVICE)
				|| (device->num_of_configuration !=
										desc.buffer[USB_DESC_DEVICE_OFF_CFGNUM])
		#endif
				)
			{
				err = VSFERR_FAIL;
				goto reset_exit;
			}
			ep_size = desc.buffer[7];
			device->ctrl_handler.ep_size = ep_size;

			// config ep0
			if (vsfhal_usbd_prepare_buffer() ||
				vsfhal_usbd_ep_set_IN_epsize(0, ep_size) ||
				vsfhal_usbd_ep_set_OUT_epsize(0, ep_size) ||
				vsfhal_usbd_ep_set_type(0, USB_EP_TYPE_CONTROL))
			{
				err = VSFERR_FAIL;
				goto reset_exit;
			}
		#endif	// VSFUSBD_CFG_AUTOSETUP

			vsfusbd_usr_cb(device, VSFUSBD_USREVT_RESET, NULL);

			if (vsfusbd_set_IN_handler(device, 0, vsfusbd_on_IN_do) ||
				vsfusbd_set_OUT_handler(device, 0, vsfusbd_on_OUT_do) ||
				vsfhal_usbd_set_address(0))
			{
				err = VSFERR_FAIL;
				goto reset_exit;
			}
		reset_exit:
			// what to do if fail to process setup?
			break;
		}
	case VSFUSBD_INTEVT_SETUP:
		{
			struct vsfusbd_ctrl_handler_t *ctrl_handler = &device->ctrl_handler;
			struct usb_ctrlrequest_t *request = &ctrl_handler->request;
			struct vsfusbd_transact_t *transact;

			vsfusbd_usr_cb(device, VSFUSBD_USREVT_SETUP, request);

			if (vsfhal_usbd_get_setup((uint8_t *)request) ||
				vsfusbd_ctrl_prepare(device))
			{
				// fail to get setup request data
				err = VSFERR_FAIL;
				goto setup_exit;
			}

			if (ctrl_handler->data_size > request->wLength)
			{
				ctrl_handler->data_size = request->wLength;
			}

			if ((request->bRequestType & USB_DIR_MASK) == USB_DIR_OUT)
			{
				if (0 == request->wLength)
				{
					vsfusbd_setup_status_callback((void *)device);
				}
				else
				{
					transact = &ctrl_handler->OUT_transact;
					transact->data_size = ctrl_handler->data_size;
					transact->stream = ctrl_handler->stream;
					transact->cb.param = device;
					transact->cb.on_finish = vsfusbd_setup_status_callback;
					err = vsfusbd_ep_recv(device, transact);
				}
			}
			else
			{
				transact = &ctrl_handler->IN_transact;
				transact->data_size = ctrl_handler->data_size;
				transact->stream = ctrl_handler->stream;
				transact->cb.param = device;
				transact->cb.on_finish = vsfusbd_setup_status_callback;
				transact->zlp = ctrl_handler->data_size < request->wLength;
				err = vsfusbd_ep_send(device, transact);
			}

		setup_exit:
			if (err)
			{
				vsfhal_usbd_ep_set_IN_stall(0);
				vsfhal_usbd_ep_set_OUT_stall(0);
			}
			break;
		}
	case VSFUSBD_INTEVT_SUSPEND:
		vsfusbd_usr_cb(device, VSFUSBD_USREVT_SUSPEND, NULL);
		break;
	case VSFUSBD_INTEVT_RESUME:
		vsfusbd_usr_cb(device, VSFUSBD_USREVT_RESUME, NULL);
		break;
	case VSFUSBD_INTEVT_SOF:
		{
			uint32_t frame_no = vsfhal_usbd_get_frame_number();
			vsfusbd_usr_cb(device, VSFUSBD_USREVT_SOF, &frame_no);
			break;
		}
	case VSFUSBD_INTEVT_ATTACH:
		vsfusbd_usr_cb(device, VSFUSBD_USREVT_ATTACH, NULL);
		break;
	case VSFUSBD_INTEVT_DETACH:
		vsfusbd_usr_cb(device, VSFUSBD_USREVT_DETACH, NULL);
		break;
	default:
		// not error and transact not valid
		if ((evt & ~VSFUSBD_EVT_ERR_MASK) == VSFUSBD_INTEVT_ERR)
		{
			uint8_t errcode = evt & VSFUSBD_EVT_ERR_MASK;
			vsfusbd_usr_cb(device, VSFUSBD_USREVT_ERROR, &errcode);
		}
		else
		{
			uint8_t ep = evt & VSFUSBD_EVT_EP_MASK;
			uint8_t dir_in = evt & VSFUSBD_EVT_DIR_IN;
			struct vsfusbd_transact_t *transact;

			if (ep > VSFUSBD_CFG_EPMAXNO)
				break;

			transact = dir_in ?
					device->IN_transact[ep] : device->OUT_transact[ep];
			if (transact != NULL)
			{
				switch (evt & VSFUSBD_EVT_EVT_MASK)
				{
				case VSFUSBD_STREAM_CLOSE_IN:
				case VSFUSBD_STREAM_IN:
					if (transact->idle)
					{
						evt = VSFUSBD_INTEVT_IN;
					}
					break;
				case VSFUSBD_STREAM_CLOSE_OUT:
					if (transact->idle)
						vsfusbd_ep_cancel_recv(device, transact);
					break;
				case VSFUSBD_STREAM_OUT:
					vsfusbd_transact_out(device, transact);
					break;
				}
			}
			switch (evt & VSFUSBD_EVT_EVT_MASK)
			{
			case VSFUSBD_INTEVT_IN:
				vsfusbd_usr_cb(device, VSFUSBD_USREVT_IN, &ep);
				if (device->IN_handler[ep] != NULL)
					device->IN_handler[ep](device, ep);
				break;
			case VSFUSBD_INTEVT_OUT:
				vsfusbd_usr_cb(device, VSFUSBD_USREVT_OUT, &ep);
				if (device->OUT_handler[ep] != NULL)
					device->OUT_handler[ep](device, ep);
				break;
			}
		}
		break;
	}
	return NULL;
}

vsf_err_t vsfusbd_connect(struct vsfusbd_device_t *device)
{
	return vsfhal_usbd_connect();
}

vsf_err_t vsfusbd_disconnect(struct vsfusbd_device_t *device)
{
	return vsfhal_usbd_disconnect();
}

vsf_err_t vsfusbd_wakeup(struct vsfusbd_device_t *device)
{
	return vsfhal_usbd_wakeup();
}

vsf_err_t vsfusbd_device_init(struct vsfusbd_device_t *device)
{
	memset(&device->sm, 0, sizeof(device->sm));
	device->sm.init_state.evt_handler = vsfusbd_evt_handler;
	device->sm.user_data = (void*)device;
	return vsfsm_init(&device->sm);
}

vsf_err_t vsfusbd_device_fini(struct vsfusbd_device_t *device)
{
	return vsfsm_post_evt_pending(&device->sm, VSFSM_EVT_FINI);
}

