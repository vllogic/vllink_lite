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

static void vsf_malfs_finish(void *param)
{
	struct vsf_malfs_t *malfs = (struct vsf_malfs_t *)param;
	struct vsf_malstream_t *malstream = &malfs->malstream;
	vsfsm_evt_t evt = malstream->offset < malstream->size ?
							VSF_MALFS_EVT_IOFAIL : VSF_MALFS_EVT_IODONE;

	vsfsm_post_evt_pending(malfs->notifier_sm, evt);
}

vsf_err_t vsf_malfs_init(struct vsf_malfs_t *malfs)
{
	uint32_t bs = malfs->malstream.mal->cap.block_size;

	malfs->malstream.mbufstream = &malfs->mbufstream;
	malfs->malstream.cb.param = malfs;
	malfs->malstream.cb.on_finish = vsf_malfs_finish;
	malfs->mbufstream.stream.op = &mbufstream_op;
	malfs->mbufstream.mem.multibuf.size = malfs->malstream.mal->cap.block_size;
	malfs->mbufstream.mem.multibuf.buffer_list = malfs->mbufstream_buffer;

	vsfsm_crit_init(&malfs->crit, VSF_MALFS_EVT_CRIT);
	malfs->sector_buffer = vsf_bufmgr_malloc(bs);
	return (NULL == malfs->sector_buffer) ?
					VSFERR_NOT_ENOUGH_RESOURCES : VSFERR_NONE;
}

void vsf_malfs_fini(struct vsf_malfs_t *malfs)
{
	if (malfs->sector_buffer != NULL)
	{
		vsf_bufmgr_free(malfs->sector_buffer);
		malfs->sector_buffer = NULL;
	}
	if (malfs->volume_name != NULL)
	{
		vsf_bufmgr_free(malfs->volume_name);
		malfs->volume_name = NULL;
	}
}

vsf_err_t vsf_malfs_read(struct vsf_malfs_t *malfs, uint32_t sector,
							uint8_t *buff, uint32_t num)
{
	uint32_t bs = malfs->malstream.mal->cap.block_size;
	uint32_t bn = malfs->malstream.mal->cap.block_num;

	if ((sector >= bn) || ((sector + num) >= bn))
	{
		return VSFERR_INVALID_PARAMETER;
	}
	if (num > dimof(malfs->mbufstream_buffer))
	{
		return VSFERR_NOT_ENOUGH_RESOURCES;
	}

	for (uint32_t i = 0; i < num; i++, buff += bs)
		malfs->mbufstream_buffer[i] = buff;
	malfs->mbufstream.mem.multibuf.count = num;
	STREAM_INIT(&malfs->mbufstream);
	return vsf_malstream_read(&malfs->malstream, sector * bs, num * bs);
}

vsf_err_t vsf_malfs_write(struct vsf_malfs_t *malfs, uint32_t sector,
							uint8_t *buff, uint32_t num)
{
	uint32_t bs = malfs->malstream.mal->cap.block_size;
	uint32_t bn = malfs->malstream.mal->cap.block_num;

	if ((sector >= bn) || ((sector + num) >= bn))
	{
		return VSFERR_INVALID_PARAMETER;
	}
	if (num > dimof(malfs->mbufstream_buffer))
	{
		return VSFERR_NOT_ENOUGH_RESOURCES;
	}

	for (uint32_t i = 0; i < num; i++, buff += bs)
		malfs->mbufstream_buffer[i] = buff;
	malfs->mbufstream.mem.multibuf.count = num;
	STREAM_INIT(&malfs->mbufstream);
	return vsf_malstream_write(&malfs->malstream, sector * bs, num * bs);
}

