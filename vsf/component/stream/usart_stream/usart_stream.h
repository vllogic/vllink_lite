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

#ifndef __USART_STREAM_H_INCLUDED__
#define __USART_STREAM_H_INCLUDED__

struct usart_stream_info_t
{
	uint8_t index;
	int16_t int_priority;
	uint32_t mode;
	uint32_t baudrate;

	struct vsf_stream_t *stream_tx;
	struct vsf_stream_t *stream_rx;
	
	uint8_t buf0[8];
	struct vsf_buffer_t buffer0;
	uint8_t buf1[8];
	struct vsf_buffer_t buffer1;

	struct vsfsm_t sm;
};

vsf_err_t usart_stream_init(struct usart_stream_info_t *usart_stream);
vsf_err_t usart_stream_fini(struct usart_stream_info_t *usart_stream);

#endif	// __USART_STREAM_H_INCLUDED__
