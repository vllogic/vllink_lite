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

#ifndef __VSFUSBD_HID_H_INCLUDED__
#define __VSFUSBD_HID_H_INCLUDED__

enum usb_HID_description_type_t
{
	USB_HID_DT_HID							= 0x21,
	USB_HID_DT_REPORT						= 0x22,
	USB_HID_DT_PHYSICAL						= 0x23,
};

enum usb_HID_req_t
{
	USB_HIDREQ_GET_REPORT					= 0x01,
	USB_HIDREQ_GET_IDLE						= 0x02,
	USB_HIDREQ_GET_PROTOCOL					= 0x03,
	USB_HIDREQ_SET_REPORT					= 0x09,
	USB_HIDREQ_SET_IDLE						= 0x0A,
	USB_HIDREQ_SET_PROTOCOL					= 0x0B,
};

#define USB_HID_PROTOCOL_BOOT				0
#define USB_HID_PROTOCOL_REPORT				1

enum usb_HID_report_type_t
{
	USB_HID_REPORT_INPUT = 1,
	USB_HID_REPORT_OUTPUT = 2,
	USB_HID_REPORT_FEATURE = 3,
	USB_HID_REPORT_OUTPUT_NO_ID = 0x82,
};

struct vsfusbd_HID_param_t;
struct vsfusbd_HID_report_t
{
	enum usb_HID_report_type_t type;
	uint8_t id;
	uint8_t idle;
	struct vsf_buffer_t buffer;
	bool changed;

	// private
	uint32_t pos;
	uint8_t idle_cnt;
};

#define VSFUSBD_DESC_HID_REPORT(ptr, size)			\
	{USB_HID_DT_REPORT, 0, 0, {(uint8_t*)(ptr), (size)}}

enum vsfusbd_HID_output_state_t
{
	HID_OUTPUT_STATE_WAIT,
	HID_OUTPUT_STATE_RECEIVING,
};

struct vsfusbd_HID_param_t
{
	uint8_t ep_out;
	uint8_t ep_in;

	uint8_t has_report_id;
	uint8_t num_of_report;
	struct vsfusbd_HID_report_t *reports;
	struct vsfusbd_desc_filter_t *desc;

	vsf_err_t (*on_report_in)(struct vsfusbd_HID_param_t *param,
			struct vsfusbd_HID_report_t *report);
	vsf_err_t (*on_report_out)(struct vsfusbd_HID_param_t *param,
			struct vsfusbd_HID_report_t *report);

	// private
	uint8_t protocol;
	uint8_t cur_report;
	uint8_t cur_OUT_id;
	uint8_t cur_IN_id;

	enum vsfusbd_HID_output_state_t output_state;

	struct vsfusbd_transact_t IN_transact;
	struct vsf_bufstream_t bufstream;
	// 4ms timer MUST be static,
	//	USB can be reset without notification to class drivers
	struct vsftimer_t timer4ms;
	struct vsfusbd_device_t *device;
	struct vsfusbd_iface_t *iface;
	bool busy;
};

#ifndef VSFCFG_EXCLUDE_USBD_HID
extern const struct vsfusbd_class_protocol_t vsfusbd_HID_class;

vsf_err_t vsfusbd_HID_IN_report_changed(struct vsfusbd_HID_param_t *param,
										struct vsfusbd_HID_report_t *report);
#endif

#endif	// __VSFUSBD_HID_H_INCLUDED__
