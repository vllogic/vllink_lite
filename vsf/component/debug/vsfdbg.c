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
#include <stdarg.h>
#include <stdio.h>

#ifdef VSFCFG_DEBUG

static uint8_t vsfdbg_buf[VSFCFG_DEBUG_BUFLEN];
static struct vsf_stream_t *vsfdbg_stream = NULL;

uint32_t vsfdbg_prints(char *str)
{
	uint32_t ret = 0;

	if (vsfdbg_stream != NULL)
	{
		struct vsf_buffer_t buffer = {(uint8_t *)str, strlen(str)};

#ifdef VSFCFG_THREAD_SAFTY
		uint8_t origlevel = vsfhal_core_set_intlevel(VSFCFG_MAX_SRT_PRIO);
#endif
		ret = stream_write(vsfdbg_stream, &buffer);
#ifdef VSFCFG_THREAD_SAFTY
		vsfhal_core_set_intlevel(origlevel);
#endif
	}
	return ret;
}

#define VSFDBG_LINEBUF_SIZE			128
uint32_t vsfdbg_printb(uint8_t *buffer, uint32_t len, bool newline)
{
	if ((vsfdbg_stream != NULL) && (len > 0))
	{
		const char map[16] = "0123456789ABCDEF";
		char hex[1 + 3 * VSFDBG_LINEBUF_SIZE], *ptr = hex;

		for (uint32_t i = 0; i < len; i++)
		{
			*ptr++ = map[(buffer[i] >> 4) & 0x0F];
			*ptr++ = map[(buffer[i] >> 0) & 0x0F];
			*ptr++ = ' ';
			if (((ptr - hex) >= 3 * VSFDBG_LINEBUF_SIZE) || (i >= (len - 1)))
			{
				*ptr++ = '\0';
				vsfdbg_prints(hex);
				ptr = hex;
			}
    	}
		if (newline)
	    	vsfdbg_prints(VSFCFG_DEBUG_LINEEND);
	}
	return 0;
}

uint32_t vsfdbg_printf(const char *format, ...)
{
	uint32_t ret = 0;
	if (vsfdbg_stream != NULL)
	{
		va_list ap;
		uint32_t size;

#ifdef VSFCFG_THREAD_SAFTY
		uint8_t origlevel = vsfhal_core_set_intlevel(VSFCFG_MAX_SRT_PRIO);
#endif

		va_start(ap, format);
		size = vsnprintf((char *)vsfdbg_buf, VSFCFG_DEBUG_BUFLEN, format, ap);
		va_end(ap);

		struct vsf_buffer_t buffer = {vsfdbg_buf, size};
		ret = stream_write(vsfdbg_stream, &buffer);

#ifdef VSFCFG_THREAD_SAFTY
		vsfhal_core_set_intlevel(origlevel);
#endif
	}
	return ret;
}

void vsfdbg_init(struct vsf_stream_t *stream)
{
	vsfdbg_stream = stream;
	if (vsfdbg_stream)
		stream_connect_tx(vsfdbg_stream);
}

void vsfdbg_fini(void)
{
	if (vsfdbg_stream)
		stream_disconnect_tx(vsfdbg_stream);
	vsfdbg_stream = NULL;
}

#endif
