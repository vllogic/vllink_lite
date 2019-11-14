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

#include "vsfip_dhcp_common.h"

// buf MUST be large enough for all the data
void vsfip_dhcp_append_opt(struct vsfip_buffer_t *buf, uint32_t *optlen,
						uint8_t option, uint8_t len, uint8_t *data)
{
	struct vsfip_dhcphead_t *head = (struct vsfip_dhcphead_t *)buf->app.buffer;
	
	head->options[(*optlen)++] = option;
	head->options[(*optlen)++] = len;
	memcpy(&head->options[*optlen], data, len);
	*optlen += len;
}

void vsfip_dhcp_end_opt(struct vsfip_buffer_t *buf, uint32_t *optlen)
{
	struct vsfip_dhcphead_t *head = (struct vsfip_dhcphead_t *)buf->app.buffer;
	
	head->options[(*optlen)++] = DHCPOPT_END;
	while ((*optlen < DHCPOPT_MINLEN) || (*optlen & 3))
	{
		head->options[(*optlen)++] = 0;
	}
	// tweak options length
	buf->app.size -= sizeof(head->options);
	buf->app.size += *optlen;
}

uint8_t vsfip_dhcp_get_opt(struct vsfip_buffer_t *buf, uint8_t option,
						uint8_t **data)
{
	struct vsfip_dhcphead_t *head = (struct vsfip_dhcphead_t *)buf->app.buffer;
	uint8_t *ptr = head->options;

	while ((ptr[0] != DHCPOPT_END) &&
			((ptr - buf->app.buffer) < buf->app.size))
	{
		if (ptr[0] == option)
		{
			if (data != NULL)
			{
				*data = &ptr[2];
			}
			return ptr[1];
		}
		ptr += 2 + ptr[1];
	}
	return 0;
}
