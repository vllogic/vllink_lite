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

#ifndef __VSFUSBD_CDCACM_H_INCLUDED__
#define __VSFUSBD_CDCACM_H_INCLUDED__

#include "../../common/CDC/vsfusb_CDCACM.h"

struct vsfusbd_CDCACM_param_t
{
	struct vsfusbd_CDC_param_t CDC;

	struct
	{
		vsf_err_t (*set_line_coding)(struct usb_CDCACM_line_coding_t *line_coding);
		vsf_err_t (*set_control_line)(uint8_t control_line);
		vsf_err_t (*get_control_line)(uint8_t *control_line);
		vsf_err_t (*send_break)(void);
	} callback;

	struct usb_CDCACM_line_coding_t line_coding;

	// no need to initialize below by user
	uint8_t control_line;
	uint8_t line_coding_buffer[7];
};

#ifndef VSFCFG_EXCLUDE_USBD_CDCACM
extern const struct vsfusbd_class_protocol_t vsfusbd_CDCACMControl_class;
extern const struct vsfusbd_class_protocol_t vsfusbd_CDCACMData_class;
#endif

#endif	// __VSFUSBD_CDCACM_H_INCLUDED__
