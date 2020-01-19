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

#include "vsf.h"
#include "usart_stream.h"

#define USART_BUF_SIZE	64

static void uart_rx_int(void *p)
{
	uint8_t buf[USART_BUF_SIZE];
	struct vsf_buffer_t buffer;
	struct usart_stream_info_t *param = p;

	if (!param->stream_rx)
		return;
	
	buffer.buffer = buf;
	buffer.size = min(vsfhal_usart_rx_get_data_size(param->index), USART_BUF_SIZE);
	if (buffer.size)
	{
		buffer.size = vsfhal_usart_rx_bytes(param->index, buf, buffer.size);
		if (buffer.size)
			stream_write(param->stream_rx, &buffer);
	}
}

static void uart_on_tx(void *p)
{
	uint8_t buf[USART_BUF_SIZE];
	struct vsf_buffer_t buffer;
	struct usart_stream_info_t *param = p;

	if (!param->stream_tx)
		return;

	buffer.size = min(vsfhal_usart_tx_get_free_size(param->index),
			USART_BUF_SIZE);
	if (buffer.size)
	{
		buffer.buffer = buf;
		buffer.size = stream_read(param->stream_tx, &buffer);
		
		static bool stop = 0;
		if (stop)
			stop = 0;
		
		if (buffer.size == 48)
		{
			stop = 1;
		}
		
		if (buffer.size)
		{
			vsfhal_usart_tx_bytes(param->index, buf, buffer.size);
		}
	}
}

static void uart_on_rx(void *p)
{
	uint8_t buf[USART_BUF_SIZE];
	struct vsf_buffer_t buffer;
	struct usart_stream_info_t *param = p;

	if (!param->stream_rx)
		return;

	buffer.size = vsfhal_usart_rx_get_data_size(param->index);
	if (buffer.size)
	{
		buffer.buffer = buf;
		buffer.size = min(buffer.size, USART_BUF_SIZE);
		buffer.size = vsfhal_usart_rx_bytes(param->index, &buf[0], buffer.size);
		stream_write(param->stream_rx, &buffer);	
	}
}

static void uart_tx_on_connect(void *p)
{
	struct usart_stream_info_t *usart_stream = p;

	vsfhal_usart_config(usart_stream->index, usart_stream->baudrate,
			usart_stream->mode);
}

static void uart_tx_on_disconnect(void *p)
{
	struct usart_stream_info_t *usart_stream = p;

	vsfhal_usart_config(usart_stream->index, 0, usart_stream->mode);
}

vsf_err_t usart_stream_init(struct usart_stream_info_t *usart_stream)
{
	if (usart_stream->index == VSFHAL_DUMMY_PORT)
		return VSFERR_FAIL;
	
	if (!usart_stream->stream_tx && !usart_stream->stream_rx)
		return VSFERR_FAIL;
	
	if (usart_stream->stream_tx)
	{
		stream_init(usart_stream->stream_tx);
		usart_stream->stream_tx->callback_rx.param = usart_stream;
		usart_stream->stream_tx->callback_rx.on_inout = uart_on_tx;
		usart_stream->stream_tx->callback_rx.on_connect = NULL;
		usart_stream->stream_tx->callback_rx.on_disconnect = NULL;

		usart_stream->stream_tx->rx_ready = true;
	}
	if (usart_stream->stream_rx)
	{
		stream_init(usart_stream->stream_rx);
		usart_stream->stream_rx->callback_tx.param = usart_stream;
		usart_stream->stream_rx->callback_tx.on_inout = uart_on_rx;
		usart_stream->stream_rx->callback_tx.on_connect = uart_tx_on_connect;
		usart_stream->stream_rx->callback_tx.on_disconnect = uart_tx_on_disconnect;	
	}

	vsfhal_usart_init(usart_stream->index);
	vsfhal_usart_config_cb(usart_stream->index, usart_stream->int_priority,
			usart_stream, uart_on_tx, uart_rx_int);
	// do not enable usart default
	vsfhal_usart_config(usart_stream->index, 0, usart_stream->mode);

	return VSFERR_NONE;
}

vsf_err_t usart_stream_fini(struct usart_stream_info_t *usart_stream)
{
	if (usart_stream->index == VSFHAL_DUMMY_PORT)
		return VSFERR_FAIL;

	vsfhal_usart_config_cb(usart_stream->index, 0, NULL, NULL, NULL);
	vsfhal_usart_fini(usart_stream->index);
	return VSFERR_NONE;
}

