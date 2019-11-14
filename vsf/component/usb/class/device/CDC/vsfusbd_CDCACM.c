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

static vsf_err_t
vsfusbd_CDCACMData_class_init(uint8_t iface, struct vsfusbd_device_t *device)
{
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_CDCACM_param_t *param =
		(struct vsfusbd_CDCACM_param_t *)config->iface[iface].protocol_param;

	param->control_line = 0;
	if ((param->callback.set_line_coding != NULL) &&
		(param->callback.set_line_coding(&param->line_coding)))
	{
		return VSFERR_FAIL;
	}
	return vsfusbd_CDCData_class.init(iface, device);
}

static vsf_err_t vsfusbd_CDCACMControl_request_prepare(struct vsfusbd_device_t *device)
{
	struct vsfusbd_ctrl_handler_t *ctrl_handler = &device->ctrl_handler;
	struct vsf_buffer_t *buffer = &ctrl_handler->bufstream.mem.buffer;
	struct usb_ctrlrequest_t *request = &ctrl_handler->request;
	uint8_t iface = request->wIndex;
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_CDCACM_param_t *param =
		(struct vsfusbd_CDCACM_param_t *)config->iface[iface].protocol_param;
	struct usb_CDCACM_line_coding_t *line_coding = &param->line_coding;

	switch (request->bRequest)
	{
	case USB_CDCACMREQ_SET_LINE_CODING:
		if ((request->wLength != 7) || (request->wValue != 0))
		{
			return VSFERR_FAIL;
		}
		buffer->buffer = param->line_coding_buffer;
		buffer->size = 7;
		break;
	case USB_CDCACMREQ_GET_LINE_CODING:
		if ((request->wLength != 7) || (request->wValue != 0))
		{
			return VSFERR_FAIL;
		}
		SET_LE_U32(&param->line_coding_buffer[0], line_coding->bitrate);
		param->line_coding_buffer[4] = line_coding->stopbittype;
		param->line_coding_buffer[5] = line_coding->paritytype;
		param->line_coding_buffer[6] = line_coding->datatype;
		buffer->buffer = param->line_coding_buffer;
		buffer->size = 7;
		break;
	case USB_CDCACMREQ_SET_CONTROL_LINE_STATE:
		if ((request->wLength != 0) ||
			(request->wValue & ~USBCDCACM_CONTROLLINE_MASK))
		{
			return VSFERR_FAIL;
		}
		param->control_line = (uint8_t)request->wValue;
		if ((param->callback.set_control_line != NULL) &&
			(param->callback.set_control_line(param->control_line)))
		{
			return VSFERR_FAIL;
		}
		break;
	case USB_CDCACMREQ_SEND_BREAK:
		if ((request->wLength != 0) ||
			((param->callback.send_break != NULL) &&
			 	param->callback.send_break()))
		{
			return VSFERR_FAIL;
		}
		break;
	default:
		return vsfusbd_CDCControl_class.request_prepare(device);
	}
	ctrl_handler->data_size = buffer->size;
	return VSFERR_NONE;
}

static vsf_err_t vsfusbd_CDCACMControl_request_process(struct vsfusbd_device_t *device)
{
	struct vsfusbd_ctrl_handler_t *ctrl_handler = &device->ctrl_handler;
	struct vsf_buffer_t *buffer = &ctrl_handler->bufstream.mem.buffer;
	struct usb_ctrlrequest_t *request = &ctrl_handler->request;
	uint8_t iface = request->wIndex;
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_CDCACM_param_t *param =
		(struct vsfusbd_CDCACM_param_t *)config->iface[iface].protocol_param;

	if (USB_CDCACMREQ_SET_LINE_CODING == request->bRequest)
	{
		struct usb_CDCACM_line_coding_t *line_coding = &param->line_coding;

		line_coding->bitrate = GET_LE_U32(&buffer->buffer[0]);
		line_coding->stopbittype = buffer->buffer[4];
		line_coding->paritytype = buffer->buffer[5];
		line_coding->datatype = buffer->buffer[6];

		if ((param->callback.set_line_coding != NULL) &&
			(param->callback.set_line_coding(line_coding)))
		{
			return VSFERR_FAIL;
		}
		vsfusbd_CDCData_connect(&param->CDC);
	}
	else
	{
		return vsfusbd_CDCControl_class.request_process(device);
	}
	return VSFERR_NONE;
}

const struct vsfusbd_class_protocol_t vsfusbd_CDCACMControl_class =
{
	.request_prepare = vsfusbd_CDCACMControl_request_prepare,
	.request_process = vsfusbd_CDCACMControl_request_process,
};

const struct vsfusbd_class_protocol_t vsfusbd_CDCACMData_class =
{
	.init = vsfusbd_CDCACMData_class_init,
};

