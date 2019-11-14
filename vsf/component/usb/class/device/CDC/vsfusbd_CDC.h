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

#ifndef __VSFUSBD_CDC_H_INCLUDED__
#define __VSFUSBD_CDC_H_INCLUDED__

#include "../../common/CDC/vsfusb_CDC.h"

#define VSFUSBD_CDCCFG_TRANSACT

struct vsfusbd_CDC_param_t
{
	uint8_t ep_notify;
	uint8_t ep_out;
	uint8_t ep_in;

	// stream_tx is used for data stream send to USB host
	struct vsf_stream_t *stream_tx;
	// stream_rx is used for data stream receive from USB host
	struct vsf_stream_t *stream_rx;

	struct
	{
		vsf_err_t (*send_encapsulated_command)(
				struct vsfusbd_CDC_param_t *param, struct vsf_buffer_t *buffer);
#ifdef VSFUSBD_CDCCFG_TRANSACT
		void (*on_tx_finish)(void *param);
		void (*on_rx_finish)(void *param);
#endif
	} callback;

	// no need to initialize below if encapsulate command/response is not used
	struct vsf_buffer_t encapsulated_command;
	struct vsf_buffer_t encapsulated_response;

	// no need to initialize below by user
	struct vsfusbd_device_t *device;
#ifdef VSFUSBD_CDCCFG_TRANSACT
	struct vsfusbd_transact_t IN_transact;
	struct vsfusbd_transact_t OUT_transact;
#else
	struct vsfusbd_iface_t *iface;
	bool out_enable;
	bool in_enable;
#endif
};

#ifndef VSFCFG_EXCLUDE_USBD_CDC
extern const struct vsfusbd_class_protocol_t vsfusbd_CDCControl_class;
extern const struct vsfusbd_class_protocol_t vsfusbd_CDCData_class;

// helper functions
void vsfusbd_CDCData_connect(struct vsfusbd_CDC_param_t *param);
#endif

#endif	// __VSFUSBD_CDC_H_INCLUDED__
