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
#ifndef __VSFUSBD_MSCBOT_H_INCLUDED__
#define __VSFUSBD_MSCBOT_H_INCLUDED__

#include "../../common/MSC/vsfusb_MSC.h"

struct vsfusbd_MSCBOT_param_t
{
	uint8_t ep_out;
	uint8_t ep_in;

	struct vsfscsi_device_t *scsi_dev;

	// no need to initialize below by user
	struct USBMSC_CBW_t CBW;
	struct USBMSC_CSW_t CSW;
	struct vsfusbd_transact_t transact;
	struct vsf_bufstream_t bufstream;
	struct vsfusbd_device_t *device;
	uint32_t transact_size;
};

#ifndef VSFCFG_EXCLUDE_USBD_MSC
extern const struct vsfusbd_class_protocol_t vsfusbd_MSCBOT_class;
#endif

#endif	// __VSFUSBD_MSC_H_INCLUDED__
