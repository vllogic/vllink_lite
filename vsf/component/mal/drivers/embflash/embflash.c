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

static uint32_t embflash_maldrv_blocksize(struct vsfmal_t *mal, uint64_t addr,
				uint32_t size, enum vsfmal_op_t op)
{
	return vsfhal_flash_blocksize(((struct embflash_mal_t *)mal)->index,
								(uint32_t)addr, size, (int)op);
}

static void embflash_maldrv_oncb(void *param, vsf_err_t err)
{
	struct embflash_mal_t *embflash = (struct embflash_mal_t *)param;
	embflash->err = err;
	vsfsm_post_evt_pending(embflash->notifier, VSFSM_EVT_USER);
}

static vsf_err_t embflash_maldrv_init(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct embflash_mal_t *embflash = (struct embflash_mal_t *)pt->user_data;
	uint32_t pagesize, pagenum;

	if (vsfhal_flash_init(embflash->index) ||
		vsfhal_flash_capacity(embflash->index, &pagesize, &pagenum) ||
		(0 == pagesize) || (0 == pagenum))
	{
		return VSFERR_FAIL;
	}

	embflash->mal.cap.block_size = pagesize;
	embflash->mal.cap.block_num = pagenum;
	return vsfhal_flash_config_cb(embflash->index, embflash->int_priority,
									embflash, embflash_maldrv_oncb);
}

static vsf_err_t embflash_maldrv_fini(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct embflash_mal_t *embflash = (struct embflash_mal_t *)pt->user_data;
	return vsfhal_flash_fini(embflash->index);
}

static vsf_err_t embflash_maldrv_erase(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
				uint64_t addr, uint32_t size)
{
	struct embflash_mal_t *embflash = (struct embflash_mal_t *)pt->user_data;
	uint32_t pos = (uint32_t)addr + embflash->cursize;

	vsfsm_pt_begin(pt);

	pos = (uint32_t)addr;
	embflash->cursize = 0;
	embflash->notifier = pt->sm;
	while (embflash->cursize < size)
	{
		embflash->pagesize = vsfhal_flash_blocksize(embflash->index, pos,
								size - embflash->cursize, VSFMAL_OP_ERASE);
		if (vsfhal_flash_erase(embflash->index, pos))
			return VSFERR_FAIL;
		vsfsm_pt_wfe(pt, VSFSM_EVT_USER);
		if (embflash->err)
			return embflash->err;
		embflash->cursize += embflash->pagesize;
	}

	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t embflash_maldrv_read(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
				uint64_t addr, uint8_t *buff, uint32_t size)
{
	struct embflash_mal_t *embflash = (struct embflash_mal_t *)pt->user_data;
	uint32_t pos = (uint32_t)addr + embflash->cursize;

	vsfsm_pt_begin(pt);

	if (vsfhal_flash_direct_read)
	{
		uint32_t base = vsfhal_flash_baseaddr(embflash->index);
		memcpy(buff, (uint8_t *)base + addr, size);
		return VSFERR_NONE;
	}

	pos = (uint32_t)addr;
	embflash->cursize = 0;
	embflash->notifier = pt->sm;
	while (embflash->cursize < size)
	{
		embflash->pagesize = vsfhal_flash_blocksize(embflash->index, pos,
								size - embflash->cursize, VSFMAL_OP_READ);
		if (vsfhal_flash_read(embflash->index, pos, buff + embflash->cursize))
			return VSFERR_FAIL;
		vsfsm_pt_wfe(pt, VSFSM_EVT_USER);
		if (embflash->err)
			return embflash->err;
		embflash->cursize += embflash->pagesize;
	}

	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

static vsf_err_t embflash_maldrv_write(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
				uint64_t addr, uint8_t *buff, uint32_t size)
{
	struct embflash_mal_t *embflash = (struct embflash_mal_t *)pt->user_data;
	uint32_t pos = (uint32_t)addr + embflash->cursize;

	vsfsm_pt_begin(pt);

	pos = (uint32_t)addr;
	embflash->cursize = 0;
	embflash->notifier = pt->sm;
	while (embflash->cursize < size)
	{
		embflash->pagesize = vsfhal_flash_blocksize(embflash->index, pos,
								size - embflash->cursize, VSFMAL_OP_WRITE);
		if (vsfhal_flash_write(embflash->index, pos, buff + embflash->cursize))
			return VSFERR_FAIL;
		vsfsm_pt_wfe(pt, VSFSM_EVT_USER);
		if (embflash->err)
			return embflash->err;
		embflash->cursize += embflash->pagesize;
	}

	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

const struct vsfmal_drv_t embflash_mal_drv =
{
	.block_size = embflash_maldrv_blocksize,
	.init = embflash_maldrv_init,
	.fini = embflash_maldrv_fini,
	.erase = embflash_maldrv_erase,
	.read = embflash_maldrv_read,
	.write = embflash_maldrv_write,
};

