/***************************************************************************
 *   Copyright (C) 2018 - 2019 by Chen Le <talpachen@gmail.com>            *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "vsf.h"
#include "vsfusbd_CMSIS_DAP.h"

#if DAP_HID_PACKET_SIZE > 255
const uint8_t vsfusbd_CMSIS_DAP_HIDReportDesc[35] =
#else
const uint8_t vsfusbd_CMSIS_DAP_HIDReportDesc[33] =
#endif
{
	0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined 0xFF00)
	0x09, 0x01,        // Usage (0x01)
	0xA1, 0x01,        // Collection (Application)
	0x15, 0x00,        //   Logical Minimum (0)
	0x26, 0xFF, 0x00,  //   Logical Maximum (255)
	0x75, 0x08,        //   Report Size (8)
#if DAP_HID_PACKET_SIZE > 255
	0x96, DAP_HID_PACKET_SIZE & 0xff, (DAP_HID_PACKET_SIZE >> 8) & 0xff,        //   Report Count (DAP_HID_PACKET_SIZE)
#else
	0x95, DAP_HID_PACKET_SIZE,
#endif
	0x09, 0x01,        //   Usage (0x01)
	0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
#if DAP_HID_PACKET_SIZE > 255
	0x96, DAP_HID_PACKET_SIZE & 0xff, (DAP_HID_PACKET_SIZE >> 8) & 0xff,        //   Report Count (DAP_HID_PACKET_SIZE)
#else
	0x95, DAP_HID_PACKET_SIZE,
#endif
	0x09, 0x01,        //   Usage (0x01)
	0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0x95, 0x01,        //   Report Count (1)
	0x09, 0x01,        //   Usage (0x01)
	0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0xC0,              // End Collection
};

static const struct vsfusbd_desc_filter_t HIDDesc[2] =
{
	VSFUSBD_DESC_HID_REPORT(vsfusbd_CMSIS_DAP_HIDReportDesc, sizeof(vsfusbd_CMSIS_DAP_HIDReportDesc)),
	VSFUSBD_DESC_NULL,
};

static void cmsis_dap_send_response(void *p, uint8_t *buf, uint16_t size)
{
	struct vsfusbd_CMSIS_DAP_param_t *param = p;
	
	if (!buf && !size)	// unregister
		param->dap_connected = false;
	else
	{
		size = min(size, DAP_HID_PACKET_SIZE);
		if (size)
		{
			memcpy(param->in, buf, DAP_HID_PACKET_SIZE);
			vsfusbd_HID_IN_report_changed(&param->HID_param, &param->reports[0]);
		}
	}
}

vsf_err_t vsfusbd_CMSIS_DAP_on_report_in(struct vsfusbd_HID_param_t *p,
		struct vsfusbd_HID_report_t *report)
{
	struct vsfusbd_CMSIS_DAP_param_t *param = (struct vsfusbd_CMSIS_DAP_param_t *)p;

	if ((report->type == USB_HID_REPORT_OUTPUT_NO_ID) && (report->id == 0))
	{
		if (!param->dap_connected)
		{
			if (DAP_register(param->dap_param, param, cmsis_dap_send_response, DAP_HID_PACKET_SIZE) == VSFERR_NONE)
				param->dap_connected = true;
			else
				goto exit;
		}
		DAP_recvive_request(param->dap_param, param->out, DAP_HID_PACKET_SIZE);
	}

exit:
	return VSFERR_NONE;
}

vsf_err_t vsfusbd_CMSIS_DAP_on_report_out(struct vsfusbd_HID_param_t *p,
		struct vsfusbd_HID_report_t *report)
{
	struct vsfusbd_CMSIS_DAP_param_t *param = (struct vsfusbd_CMSIS_DAP_param_t *)p;

	if ((report->type == USB_HID_REPORT_INPUT) && (report->id == 0))
	{
		if (param->dap_connected)
			DAP_send_response_done(param->dap_param);
	}

	return VSFERR_NONE;
}

vsf_err_t vsfusbd_CMSIS_DAP_init(struct vsfusbd_CMSIS_DAP_param_t *param)
{
	struct vsfusbd_HID_report_t *reports = param->reports;
	
	reports[0].type = USB_HID_REPORT_INPUT;
	reports[0].id = 0;
	reports[0].idle = 0;
	reports[0].buffer.buffer = param->in;
	reports[0].buffer.size = DAP_HID_PACKET_SIZE;
	reports[0].changed = false;

	reports[1].type = USB_HID_REPORT_OUTPUT_NO_ID;
	reports[1].id = 0;
	reports[1].idle = 0;
	reports[1].buffer.buffer = param->out;
	reports[1].buffer.size = DAP_HID_PACKET_SIZE;
	reports[1].changed = false;
	
	reports[2].type = USB_HID_REPORT_FEATURE;
	reports[2].id = 0;
	reports[2].idle = 0;
	reports[2].buffer.buffer = param->feature;
	reports[2].buffer.size = sizeof(param->feature);
	reports[2].changed = false;
	
	param->HID_param.has_report_id = 0;
	param->HID_param.num_of_report = 3;
	param->HID_param.reports = reports;
	param->HID_param.desc = (struct vsfusbd_desc_filter_t *)HIDDesc;
	param->HID_param.on_report_in = vsfusbd_CMSIS_DAP_on_report_in;
	param->HID_param.on_report_out = vsfusbd_CMSIS_DAP_on_report_out;
	
	return VSFERR_NONE;
}

