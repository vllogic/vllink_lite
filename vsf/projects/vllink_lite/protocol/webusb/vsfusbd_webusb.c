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
#include "vsfusbd_webusb.h"

static vsf_err_t vsfusbd_webusb_request_prepare(struct vsfusbd_device_t *device)
{
	// TODO
	return VSFERR_NONE;
}

static vsf_err_t vsfusbd_webusb_request_process(struct vsfusbd_device_t *device)
{
	// TODO
	return VSFERR_NONE;
}

const struct vsfusbd_class_protocol_t vsfusbd_webusb_class =
{
	.request_prepare =	vsfusbd_webusb_request_prepare,
	.request_process =	vsfusbd_webusb_request_process,
};

