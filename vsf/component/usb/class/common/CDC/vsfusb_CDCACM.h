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

#ifndef __VSFUSB_CDCACM_H_INCLUDED__
#define __VSFUSB_CDCACM_H_INCLUDED__

struct usb_CDCACM_line_coding_t
{
	uint32_t bitrate;
	uint8_t stopbittype;
	uint8_t paritytype;
	uint8_t datatype;
};

#define USBCDCACM_CONTROLLINE_RTS			0x02
#define USBCDCACM_CONTROLLINE_DTR			0x01
#define USBCDCACM_CONTROLLINE_MASK			0x03

enum usb_CDCACM_req_t
{
	USB_CDCACMREQ_SET_LINE_CODING			= 0x20,
	USB_CDCACMREQ_GET_LINE_CODING			= 0x21,
	USB_CDCACMREQ_SET_CONTROL_LINE_STATE	= 0x22,
	USB_CDCACMREQ_SEND_BREAK				= 0x23,
};

#endif	// __VSFUSB_CDCACM_H_INCLUDED__
