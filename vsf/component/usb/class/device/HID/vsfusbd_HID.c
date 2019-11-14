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

enum vsfusbd_HID_EVT_t
{
	VSFUSBD_HID_EVT_TIMER4MS = VSFSM_EVT_USER_LOCAL_INSTANT + 0,
	VSFUSBD_HID_EVT_INREPORT = VSFSM_EVT_USER_LOCAL_INSTANT + 1,
};

static struct vsfusbd_HID_report_t* vsfusbd_HID_find_report(
		struct vsfusbd_HID_param_t *param, uint8_t type, uint8_t id)
{
	uint8_t i;

	if (NULL == param)
	{
		return NULL;
	}

	for(i = 0; i < param->num_of_report; i++)
	{
		if (((param->reports[i].type == USB_HID_REPORT_OUTPUT_NO_ID) &&
				(type == USB_HID_REPORT_OUTPUT)) || 
				((param->reports[i].type == type) && (param->reports[i].id == id)))
		{
			return &param->reports[i];
		}
	}

	return NULL;
}

static vsf_err_t vsfusbd_HID_OUT_hanlder(struct vsfusbd_device_t *device,
											uint8_t ep)
{
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	int8_t iface = config->ep_OUT_iface_map[ep];
	struct vsfusbd_HID_param_t *param;
	uint16_t pkg_size, ep_size;
#if defined(VSFUSBD_CFG_HIGHSPEED)
	uint8_t buffer[512];
#else
	uint8_t buffer[64];
#endif
	uint8_t *pbuffer = buffer;
	uint8_t report_id;
	struct vsfusbd_HID_report_t *report;

	if (iface < 0)
	{
		return VSFERR_FAIL;
	}
	param = (struct vsfusbd_HID_param_t *)config->iface[iface].protocol_param;
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

	switch (param->output_state)
	{
	case HID_OUTPUT_STATE_WAIT:
		report_id = buffer[0];
		report = vsfusbd_HID_find_report(param, USB_HID_REPORT_OUTPUT,
											report_id);
		if ((NULL == report) || (pkg_size > report->buffer.size))
		{
			return VSFERR_FAIL;
		}

		memcpy(report->buffer.buffer, pbuffer, pkg_size);
		if (pkg_size < report->buffer.size)
		{
			report->pos = pkg_size;
			param->output_state = HID_OUTPUT_STATE_RECEIVING;
			param->cur_OUT_id = report_id;
		}
		else if (param->on_report_in != NULL)
		{
			param->on_report_in(param, report);
		}
		break;
	case HID_OUTPUT_STATE_RECEIVING:
		report_id = param->cur_OUT_id;
		report = vsfusbd_HID_find_report(param, USB_HID_REPORT_OUTPUT,
											report_id);
		if ((NULL == report) ||
			((pkg_size + report->pos) > report->buffer.size))
		{
			return VSFERR_FAIL;
		}

		memcpy(report->buffer.buffer + report->pos, pbuffer, pkg_size);
		report->pos += pkg_size;
		if (report->pos >= report->buffer.size)
		{
			report->pos = 0;
			if (param->on_report_in != NULL)
			{
				param->on_report_in(param, report);
			}
			param->output_state = HID_OUTPUT_STATE_WAIT;
		}
		break;
	default:
		return VSFERR_NONE;
	}
	return vsfhal_usbd_ep_enable_OUT(param->ep_out);
}

static void vsfusbd_HID_INREPORT_callback(void *param)
{
	struct vsfusbd_HID_param_t *HID_param =
								(struct vsfusbd_HID_param_t *)param;
	struct vsfusbd_HID_report_t *report =
			vsfusbd_HID_find_report(param, USB_HID_REPORT_INPUT, HID_param->cur_IN_id);

	HID_param->busy = false;
	if (HID_param->on_report_out != NULL)
	{
		HID_param->on_report_out(HID_param, report);
	}
	vsfsm_post_evt(&HID_param->iface->sm, VSFUSBD_HID_EVT_INREPORT);
}

vsf_err_t vsfusbd_HID_IN_report_changed(struct vsfusbd_HID_param_t *param,
										struct vsfusbd_HID_report_t *report)
{
	report->changed = true;
	if (!param->busy)
	{
		vsfsm_post_evt(&param->iface->sm, VSFUSBD_HID_EVT_INREPORT);
	}
	return VSFERR_NONE;
}

static struct vsfsm_state_t *
vsfusbd_HID_evt_handler(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	struct vsfusbd_HID_param_t *param =
						(struct vsfusbd_HID_param_t *)sm->user_data;
	struct vsfusbd_device_t *device = param->device;
	uint8_t i;

	switch (evt)
	{
	case VSFSM_EVT_INIT:
		if (param->ep_out != 0)
		{
			vsfusbd_set_OUT_handler(device, param->ep_out,
										vsfusbd_HID_OUT_hanlder);
			vsfhal_usbd_ep_enable_OUT(param->ep_out);
		}

		param->bufstream.mem.read = true;
		param->bufstream.stream.op = &bufstream_op;
		param->bufstream.stream.callback_tx.on_connect = NULL;
		param->bufstream.stream.callback_tx.on_disconnect = NULL;
		param->bufstream.stream.callback_tx.on_inout = NULL;

		param->output_state = HID_OUTPUT_STATE_WAIT;
		param->busy = false;
		for(i = 0; i < param->num_of_report; i++)
		{
			struct vsfusbd_HID_report_t *report = &param->reports[i];

			report->pos = 0;
			report->idle_cnt = 0;
		}

		// enable timer
		param->timer4ms.sm = sm;
		param->timer4ms.evt = VSFUSBD_HID_EVT_TIMER4MS;
		param->timer4ms.interval = 4;
		param->timer4ms.trigger_cnt = -1;
		vsftimer_enqueue(&param->timer4ms);
		break;
	case VSFUSBD_HID_EVT_TIMER4MS:
		{
			struct vsfusbd_HID_report_t *report;
			uint8_t i;

			for (i = 0; i < param->num_of_report; i++)
			{
				report = &param->reports[i];
				if ((report->type == USB_HID_REPORT_INPUT) &&
					(report->idle != 0))
				{
					report->idle_cnt++;
				}
			}
			if (param->busy)
			{
				break;
			}
		}
		// fall through
	case VSFUSBD_HID_EVT_INREPORT:
		{
			struct vsfusbd_transact_t *transact = &param->IN_transact;
			struct vsfusbd_HID_report_t *report;

			if (param->busy)
			{
				break;
			}

			if (param->cur_report >= param->num_of_report)
			{
				param->cur_report = 0;
			}
			for (; param->cur_report < param->num_of_report; param->cur_report++)
			{
				report = &param->reports[param->cur_report];
				if ((report->type == USB_HID_REPORT_INPUT) &&
					(report->changed || ((report->idle != 0) &&
							(report->idle_cnt >= report->idle))))
				{
					param->cur_IN_id = report->id;
					param->bufstream.mem.buffer = report->buffer;
					STREAM_INIT(&param->bufstream);
					STREAM_CONNECT_TX(&param->bufstream);

					transact->ep = param->ep_in;
					transact->data_size = report->buffer.size;
					transact->stream = (struct vsf_stream_t *)&param->bufstream;
					transact->zlp = false;
					transact->cb.on_finish = vsfusbd_HID_INREPORT_callback;
					transact->cb.param = param;
					vsfusbd_ep_send(device, transact);

					report->changed = false;
					report->idle_cnt = 0;
					param->cur_report++;
					param->busy = true;
					break;
				}
			}
		}
		break;
	}

	return NULL;
}

static vsf_err_t vsfusbd_HID_class_init(uint8_t iface, struct vsfusbd_device_t *device)
{
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_iface_t *ifs = &config->iface[iface];
	struct vsfusbd_HID_param_t *param =
						(struct vsfusbd_HID_param_t *)ifs->protocol_param;

	// state machine init
	ifs->sm.init_state.evt_handler = vsfusbd_HID_evt_handler;
	param->iface = ifs;
	param->device = device;
	ifs->sm.user_data = (void*)param;
	return vsfsm_init(&ifs->sm);
}

static vsf_err_t vsfusbd_HID_get_desc(struct vsfusbd_device_t *device, uint8_t type,
			uint8_t index, uint16_t lanid, struct vsf_buffer_t *buffer)
{
	struct usb_ctrlrequest_t *request = &device->ctrl_handler.request;
	uint8_t iface = request->wIndex;
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_HID_param_t *param =
		(struct vsfusbd_HID_param_t *)config->iface[iface].protocol_param;

	if ((NULL == param) || (NULL == param->desc))
	{
		return VSFERR_FAIL;
	}

	return vsfusbd_device_get_descriptor(device, param->desc, type, index,
											lanid, buffer);
}

static vsf_err_t vsfusbd_HID_request_prepare(struct vsfusbd_device_t *device)
{
	struct vsfusbd_ctrl_handler_t *ctrl_handler = &device->ctrl_handler;
	struct vsf_buffer_t *buffer = &ctrl_handler->bufstream.mem.buffer;
	struct usb_ctrlrequest_t *request = &ctrl_handler->request;
	uint8_t iface = request->wIndex;
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_HID_param_t *param =
			(struct vsfusbd_HID_param_t *)config->iface[iface].protocol_param;
	uint8_t type = request->wValue >> 8, id = request->wValue;
	struct vsfusbd_HID_report_t *report =
									vsfusbd_HID_find_report(param, type, id);

	switch (request->bRequest)
	{
	case USB_HIDREQ_GET_REPORT:
		if ((NULL == report) || (type != report->type))
		{
			return VSFERR_FAIL;
		}
		buffer->size = report->buffer.size;
		buffer->buffer = report->buffer.buffer;
		if (param->has_report_id)
		{
			buffer->size--;
			buffer->buffer++;
		}
		break;
	case USB_HIDREQ_GET_IDLE:
		if ((NULL == report) || (request->wLength != 1))
		{
			return VSFERR_FAIL;
		}
		buffer->size = 1;
		buffer->buffer = &report->idle;
		break;
	case USB_HIDREQ_GET_PROTOCOL:
		if ((request->wValue != 0) || (request->wLength != 1))
		{
			return VSFERR_FAIL;
		}
		buffer->size = 1;
		buffer->buffer = &param->protocol;
		break;
	case USB_HIDREQ_SET_REPORT:
		if ((NULL == report) || (type != report->type))
		{
			return VSFERR_FAIL;
		}
		buffer->size = report->buffer.size;
		buffer->buffer = report->buffer.buffer;
		if (param->has_report_id)
		{
			buffer->size--;
			buffer->buffer++;
		}
		break;
	case USB_HIDREQ_SET_IDLE:
		if (request->wLength != 0)
		{
			return VSFERR_FAIL;
		}
		for(uint8_t i = 0; i < param->num_of_report; i++)
		{
			if ((param->reports[i].type == USB_HID_REPORT_INPUT) &&
				((0 == id) || (param->reports[i].id == id)))
			{
				param->reports[i].idle = request->wValue >> 8;
			}
		}
		break;
	case USB_HIDREQ_SET_PROTOCOL:
		if ((request->wLength != 1) ||
			((request->wValue != USB_HID_PROTOCOL_BOOT) &&
			 	(request->wValue != USB_HID_PROTOCOL_REPORT)))
		{
			return VSFERR_FAIL;
		}
		param->protocol = request->wValue;
		break;
	default:
		return VSFERR_FAIL;
	}
	ctrl_handler->data_size = buffer->size;
	return VSFERR_NONE;
}

static vsf_err_t vsfusbd_HID_request_process(struct vsfusbd_device_t *device)
{
	struct vsfusbd_ctrl_handler_t *ctrl_handler = &device->ctrl_handler;
	struct usb_ctrlrequest_t *request = &ctrl_handler->request;
	uint8_t iface = request->wIndex;
	struct vsfusbd_config_t *config = &device->config[device->configuration];
	struct vsfusbd_HID_param_t *param =
			(struct vsfusbd_HID_param_t *)config->iface[iface].protocol_param;
	vsf_err_t (*on_report)(struct vsfusbd_HID_param_t *param,
			struct vsfusbd_HID_report_t *report) =
		(USB_HIDREQ_SET_REPORT == request->bRequest) ? param->on_report_in :
		(USB_HIDREQ_GET_REPORT == request->bRequest) ? param->on_report_out : NULL;

	if (on_report != NULL)
	{
		uint8_t type = request->wValue >> 8, id = request->wValue;
		struct vsfusbd_HID_report_t *report =
									vsfusbd_HID_find_report(param, type, id);

		if ((NULL == report) || (type != report->type))
		{
			return VSFERR_FAIL;
		}

		return on_report(param, report);
	}
	return VSFERR_NONE;
}

const struct vsfusbd_class_protocol_t vsfusbd_HID_class =
{
	.get_desc =			vsfusbd_HID_get_desc,
	.request_prepare =	vsfusbd_HID_request_prepare,
	.request_process =	vsfusbd_HID_request_process,
	.init =				vsfusbd_HID_class_init,
};

