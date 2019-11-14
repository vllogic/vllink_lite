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

#ifndef __DEBUG_H_INCLUDED__
#define __DEBUG_H_INCLUDED__

#ifdef VSFCFG_DEBUG

#include "component/stream/stream.h"

#define VSFCFG_DEBUG_INFO_PARSE_LEN 		(VSFCFG_DEBUG_INFO_LEN + 100)

#ifndef VSFCFG_EXCLUDE_DEBUG
void debug_init(struct vsf_stream_t *stream);
void debug_fini(void);
uint32_t debug(const char *format, ...);
#endif

#define vsf_debug(format, ...) debug("%s:%d "format"\r\n", __FILE__,\
										__LINE__, ##__VA_ARGS__)
#define vsf_debug_init(s) debug_init(s)
#define vsf_debug_fini() debug_fini()
#else
#define vsf_debug(format, ...)
#define vsf_debug_init(s)
#define vsf_debug_fini()
#endif // VSFCFG_DEBUG

#endif // __DEBUG_H_INCLUDED__
