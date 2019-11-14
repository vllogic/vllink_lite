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

#ifndef __VSFUSB_CDC_H_INCLUDED__
#define __VSFUSB_CDC_H_INCLUDED__

enum usb_CDC_req_t
{
	USB_CDCREQ_SEND_ENCAPSULATED_COMMAND	= 0x00,
	USB_CDCREQ_GET_ENCAPSULATED_RESPONSE	= 0x01,
	USB_CDCREQ_SET_COMM_FEATURE				= 0x02,
	USB_CDCREQ_GET_COMM_FEATURE				= 0x03,
	USB_CDCREQ_CLEAR_COMM_FEATURE			= 0x04,
};

struct usb_cdc_union_descriptor_t
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;

	uint8_t bControlInterface;
	uint8_t bSubordinateInterface[1];
} PACKED;

struct usb_cdc_ecm_descriptor_t
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;

	uint8_t iMACAddress;
	uint8_t bmEthernetStatistics[4];
	uint8_t wMaxSegmentSize[2];
	uint8_t wNumberMCFilters[2];
	uint8_t bNumberPowerFilters;
} PACKED;

#endif	// __VSFUSB_CDC_H_INCLUDED__
