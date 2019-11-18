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

#ifndef __VSFUSBD_H_INCLUDED__
#define __VSFUSBD_H_INCLUDED__

#include "../common/usb_common.h"

#ifndef VSFUSBD_CFG_AUTOSETUP
#define VSFUSBD_CFG_AUTOSETUP			1
#endif
#ifndef VSFUSBD_CFG_EPMAXNO
#define VSFUSBD_CFG_EPMAXNO				15
#endif
#if !defined(VSFUSBD_CFG_HIGHSPEED) && !defined(VSFUSBD_CFG_FULLSPEED) &&\
	!defined(VSFUSBD_CFG_LOWSPEED)
#define VSFUSBD_CFG_FULLSPEED
#endif

#define VSFUSBD_EVT_DATAIO_INEP(ep)		(VSFUSBD_EVT_DATAIO_IN + (ep))
#define VSFUSBD_EVT_DATAIO_OUTEP(ep)	(VSFUSBD_EVT_DATAIO_OUT + (ep))

struct vsfusbd_device_t;

struct vsfusbd_transact_callback_t
{
	void (*on_finish)(void *param);
	void *param;
};
struct vsfusbd_transact_t
{
	uint8_t ep;
	uint32_t data_size;
	struct vsf_stream_t *stream;
	struct vsfusbd_transact_callback_t cb;
	bool zlp;

	// private
	bool idle;
	struct vsfusbd_device_t *device;
};

struct vsfusbd_ctrl_handler_t
{
	struct usb_ctrlrequest_t request;
	struct vsf_bufstream_t bufstream;
	struct vsf_stream_t *stream;
	uint32_t data_size;
	uint16_t ep_size;
	int8_t iface;
	uint8_t reply_buffer[2];
	struct vsfusbd_transact_t IN_transact;
	struct vsfusbd_transact_t OUT_transact;
};

#define VSFUSBD_DESC_DEVICE(lanid, ptr, size)			\
	{USB_DT_DEVICE, 0, (lanid), {(uint8_t*)(ptr), (size)}}
#define VSFUSBD_DESC_CONFIG(lanid, idx, ptr, size)	\
	{USB_DT_CONFIG, (idx), (lanid), {(uint8_t*)(ptr), (size)}}
#define VSFUSBD_BOS_DESC(lanid, idx, ptr, size)	\
	{USB_DT_BOS, (idx), (lanid), {(uint8_t*)(ptr), (size)}}
#define VSFUSBD_DESC_STRING(lanid, idx, ptr, size)	\
	{USB_DT_STRING, (idx), (lanid), {(uint8_t*)(ptr), (size)}}
#define VSFUSBD_DESC_NULL									\
	{0, 0, 0, {NULL, 0}}
struct vsfusbd_desc_filter_t
{
	uint8_t type;
	uint8_t index;
	uint16_t lanid;

	struct vsf_buffer_t buffer;
};

#define VSFUSBD_SETUP_INVALID_TYPE	0xFF
#define VSFUSBD_SETUP_NULL			{VSFUSBD_SETUP_INVALID_TYPE, 0, NULL, NULL}

struct vsfusbd_class_protocol_t
{
	vsf_err_t (*get_desc)(struct vsfusbd_device_t *device, uint8_t type,
				uint8_t index, uint16_t lanid, struct vsf_buffer_t *buffer);
	vsf_err_t (*request_prepare)(struct vsfusbd_device_t *device);
	vsf_err_t (*request_process)(struct vsfusbd_device_t *device);

	vsf_err_t (*init)(uint8_t iface, struct vsfusbd_device_t *device);
	vsf_err_t (*fini)(uint8_t iface, struct vsfusbd_device_t *device);
};

struct vsfusbd_iface_t
{
	// public
	struct vsfusbd_class_protocol_t *class_protocol;
	void *protocol_param;

	// private
	// for interface sm, no evtq is defined, so instant events MUST be used
	struct vsfsm_t sm;
	uint8_t alternate_setting;
};

struct vsfusbd_config_t
{
	// public
	vsf_err_t (*init)(struct vsfusbd_device_t *device);
	vsf_err_t (*fini)(struct vsfusbd_device_t *device);
	vsf_err_t (*vendor_prepare)(struct vsfusbd_device_t *device);

	uint8_t num_of_ifaces;
	struct vsfusbd_iface_t *iface;

	// private
	uint8_t configuration_value;
	int8_t ep_OUT_iface_map[VSFUSBD_CFG_EPMAXNO + 1];
	int8_t ep_IN_iface_map[VSFUSBD_CFG_EPMAXNO + 1];
};

enum vsfusbd_usr_evt_t
{
	VSFUSBD_USREVT_INIT = 0,
	VSFUSBD_USREVT_FINI,
	VSFUSBD_USREVT_ATTACH,
	VSFUSBD_USREVT_DETACH,
	VSFUSBD_USREVT_ERROR,
	VSFUSBD_USREVT_RESET,
	VSFUSBD_USREVT_SETUP,
	VSFUSBD_USREVT_SOF,
	VSFUSBD_USREVT_SUSPEND,
	VSFUSBD_USREVT_RESUME,
	VSFUSBD_USREVT_IN,
	VSFUSBD_USREVT_OUT,
};

struct vsfusbd_device_t
{
	// public
	uint8_t num_of_configuration;
	struct vsfusbd_config_t *config;
	struct vsfusbd_desc_filter_t *desc_filter;
	uint8_t device_class_iface;
	struct vsfhal_usbd_t *drv;
	int32_t int_priority;
	void (*on_EVENT)(struct vsfusbd_device_t *device,
			enum vsfusbd_usr_evt_t evt, void *param);

	// private
	struct vsfsm_t sm;

	uint8_t address;
	bool configured;
	uint8_t configuration;
	uint8_t feature;
	struct vsfusbd_ctrl_handler_t ctrl_handler;

	struct vsfusbd_transact_t *IN_transact[VSFUSBD_CFG_EPMAXNO + 1];
	struct vsfusbd_transact_t *OUT_transact[VSFUSBD_CFG_EPMAXNO + 1];

	vsf_err_t (*IN_handler[VSFUSBD_CFG_EPMAXNO + 1])(struct vsfusbd_device_t*, uint8_t);
	vsf_err_t (*OUT_handler[VSFUSBD_CFG_EPMAXNO + 1])(struct vsfusbd_device_t*, uint8_t);

#if defined(VSFUSBD_CFG_MPS)
	// round up to dword boundary to avoid potential problem
	uint8_t tmpbuf[(VSFUSBD_CFG_MPS + 3) & ~3];
#elif defined(VSFUSBD_CFG_HIGHSPEED)
	uint8_t tmpbuf[512];
#elif defined(VSFUSBD_CFG_FULLSPEED)
	uint8_t tmpbuf[64];
#else
	uint8_t tmpbuf[8];
#endif
};

#ifndef VSFCFG_EXCLUDE_USBD
vsf_err_t vsfusbd_device_get_descriptor(struct vsfusbd_device_t *device,
		struct vsfusbd_desc_filter_t *filter, uint8_t type, uint8_t index,
		uint16_t lanid, struct vsf_buffer_t *buffer);

vsf_err_t vsfusbd_device_init(struct vsfusbd_device_t *device);
vsf_err_t vsfusbd_device_fini(struct vsfusbd_device_t *device);
vsf_err_t vsfusbd_connect(struct vsfusbd_device_t *device);
vsf_err_t vsfusbd_disconnect(struct vsfusbd_device_t *device);
vsf_err_t vsfusbd_wakeup(struct vsfusbd_device_t *device);

vsf_err_t vsfusbd_ep_recv(struct vsfusbd_device_t *device,
								struct vsfusbd_transact_t *transact);
void vsfusbd_ep_cancel_recv(struct vsfusbd_device_t *device,
								struct vsfusbd_transact_t *transact);
vsf_err_t vsfusbd_ep_send(struct vsfusbd_device_t *device,
								struct vsfusbd_transact_t *transact);
void vsfusbd_ep_cancel_send(struct vsfusbd_device_t *device,
								struct vsfusbd_transact_t *transact);

vsf_err_t vsfusbd_set_IN_handler(struct vsfusbd_device_t *device,
		uint8_t ep, vsf_err_t (*handler)(struct vsfusbd_device_t*, uint8_t));
vsf_err_t vsfusbd_set_OUT_handler(struct vsfusbd_device_t *device,
		uint8_t ep, vsf_err_t (*handler)(struct vsfusbd_device_t*, uint8_t));
#endif

#endif	// __VSF_USBD_H_INCLUDED__
