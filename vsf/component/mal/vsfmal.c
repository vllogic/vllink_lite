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

vsf_err_t vsfmal_init(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfmal_t *mal = (struct vsfmal_t *)pt->user_data;
	return (NULL == mal->drv->init) ? VSFERR_NONE : mal->drv->init(pt, evt);
}

vsf_err_t vsfmal_fini(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfmal_t *mal = (struct vsfmal_t *)pt->user_data;
	return (NULL == mal->drv->fini) ? VSFERR_NONE : mal->drv->fini(pt, evt);
}

vsf_err_t vsfmal_erase_all(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfmal_t *mal = (struct vsfmal_t *)pt->user_data;
	return (NULL == mal->drv->erase_all) ?
				VSFERR_NOT_SUPPORT : mal->drv->erase_all(pt, evt);
}

vsf_err_t vsfmal_erase(struct vsfsm_pt_t *pt, vsfsm_evt_t evt, uint64_t addr,
					uint32_t size)
{
	struct vsfmal_t *mal = (struct vsfmal_t *)pt->user_data;
	vsf_err_t err;

	vsfsm_pt_begin(pt);
	mal->op_block_size = mal->drv->block_size(mal, addr, size, VSFMAL_OP_ERASE);
	mal->offset = 0;

	while (mal->offset < size)
	{
		mal->pt.user_data = mal;
		mal->pt.state = 0;
		vsfsm_pt_entry(pt);
		err = mal->drv->erase(&mal->pt, evt, addr + mal->offset,
								mal->op_block_size);
		if (err != 0) return err;

		mal->offset += mal->op_block_size;
	}
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

vsf_err_t vsfmal_read(struct vsfsm_pt_t *pt, vsfsm_evt_t evt, uint64_t addr,
					uint8_t *buff, uint32_t size)
{
	struct vsfmal_t *mal = (struct vsfmal_t *)pt->user_data;
	uint32_t offset;
	vsf_err_t err;

	vsfsm_pt_begin(pt);
	mal->op_block_size = mal->drv->block_size(mal, addr, size, VSFMAL_OP_READ);
	mal->offset = 0;

	while (mal->offset < size)
	{
		mal->pt.user_data = mal;
		mal->pt.state = 0;
		vsfsm_pt_entry(pt);
		offset = mal->offset;
		err = mal->drv->read(&mal->pt, evt, addr + offset, buff + offset,
								mal->op_block_size);
		if (err != 0) return err;

		mal->offset += mal->op_block_size;
	}
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

vsf_err_t vsfmal_write(struct vsfsm_pt_t *pt, vsfsm_evt_t evt, uint64_t addr,
					uint8_t *buff, uint32_t size)
{
	struct vsfmal_t *mal = (struct vsfmal_t *)pt->user_data;
	uint32_t offset;
	vsf_err_t err;

	vsfsm_pt_begin(pt);
	mal->op_block_size = mal->drv->block_size(mal, addr, size, VSFMAL_OP_WRITE);
	mal->offset = 0;

	while (mal->offset < size)
	{
		mal->pt.user_data = mal;
		mal->pt.state = 0;
		vsfsm_pt_entry(pt);
		offset = mal->offset;
		err = mal->drv->write(&mal->pt, evt, addr + offset, buff + offset,
								mal->op_block_size);
		if (err != 0) return err;

		mal->offset += mal->op_block_size;
	}
	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

// mim: mal in mal
static uint32_t vsfmim_blocksize(struct vsfmal_t *mal, uint64_t addr,
					uint32_t size, enum vsfmal_op_t op)
{
	struct vsfmim_t *mim = (struct vsfmim_t *)mal;
	struct vsfmal_t *realmal = mim->realmal;
	return realmal->drv->block_size(realmal, mim->addr + addr, size, op);
}

static vsf_err_t vsfmim_init(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfmim_t *mim = (struct vsfmim_t *)pt->user_data;
	mim->mal.cap.block_size = mim->realmal->cap.block_size;
	mim->mal.cap.block_num = mim->size / mim->mal.cap.block_size;
	pt->user_data = mim->realmal;
	return vsfmal_init(pt, evt);
}

static vsf_err_t vsfmim_fini(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	pt->user_data = ((struct vsfmim_t *)pt->user_data)->realmal;
	return vsfmal_fini(pt, evt);
}

static vsf_err_t vsfmim_erase_all(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	// CAN not erase all for a mim
	return VSFERR_NOT_SUPPORT;
}

static vsf_err_t vsfmim_erase(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					uint64_t addr, uint32_t size)
{
	struct vsfmim_t *mim = (struct vsfmim_t *)pt->user_data;
	pt->user_data = mim->realmal;
	return vsfmal_erase(pt, evt, mim->addr + addr, size);
}

static vsf_err_t vsfmim_read(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					uint64_t addr, uint8_t *buff, uint32_t size)
{
	struct vsfmim_t *mim = (struct vsfmim_t *)pt->user_data;
	pt->user_data = mim->realmal;
	return vsfmal_read(pt, evt, mim->addr + addr, buff, size);
}

static vsf_err_t vsfmim_write(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					uint64_t addr, uint8_t *buff, uint32_t size)
{
	struct vsfmim_t *mim = (struct vsfmim_t *)pt->user_data;
	pt->user_data = mim->realmal;
	return vsfmal_write(pt, evt, mim->addr + addr, buff, size);
}

// mal stream
#define VSF_MALSTREAM_ON_INOUT			(VSFSM_EVT_USER + 0)

static void vsf_malstream_on_inout(void *param)
{
	struct vsf_malstream_t *malstream = (struct vsf_malstream_t *)param;
	vsfsm_post_evt_pending(&malstream->sm, VSF_MALSTREAM_ON_INOUT);
}

static struct vsfsm_state_t *
vsf_malstream_init_evt_handler(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	struct vsf_malstream_t *malstream = (struct vsf_malstream_t *)sm->user_data;
	struct vsfmal_t *mal = malstream->mal;
	vsf_err_t err;

	switch (evt)
	{
	case VSFSM_EVT_FINI:
	case VSFSM_EVT_ENTER:
	case VSFSM_EVT_EXIT:
		break;
	case VSFSM_EVT_INIT:
		mal->pt.sm = sm;
		mal->pt.user_data = mal;
		mal->pt.state = 0;
	default:
		err = vsfmal_init(&mal->pt, evt);
		if (!err)
		{
			malstream->mal_ready = true;
		}
	}
	return NULL;
}

vsf_err_t vsf_malstream_init(struct vsf_malstream_t *malstream)
{
	malstream->mal_ready = false;
	malstream->sm.init_state.evt_handler = vsf_malstream_init_evt_handler;
	malstream->sm.user_data = malstream;
	return vsfsm_init(&malstream->sm);
}

static vsf_err_t
vsf_malstream_read_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsf_malstream_t *malstream = (struct vsf_malstream_t *)pt->user_data;
	struct vsfmal_t *mal = malstream->mal;
	struct vsf_stream_t *stream = (struct vsf_stream_t *)malstream->mbufstream;
	uint64_t cur_addr;
	vsf_err_t err;

	vsfsm_pt_begin(pt);

	mal->op_block_size = mal->drv->block_size(mal, malstream->addr, 0,
												VSFMAL_OP_READ);
	if (mal->op_block_size != malstream->mbufstream->mem.multibuf.size)
	{
		return VSFERR_BUG;
	}

	malstream->offset = 0;
	while (malstream->offset < malstream->size)
	{
		while (stream_get_free_size(stream) < mal->op_block_size)
		{
			stream->callback_tx.on_inout = vsf_malstream_on_inout;
			vsfsm_pt_wfe(pt, VSF_MALSTREAM_ON_INOUT);
			stream->callback_tx.on_inout = NULL;
		}

		mal->pt.user_data = mal;
		mal->pt.state = 0;
		vsfsm_pt_entry(pt);
		cur_addr = malstream->addr + malstream->offset;
		err = mal->drv->read(&mal->pt, evt, cur_addr,
				vsf_multibuf_get_empty(&malstream->mbufstream->mem.multibuf),
				mal->op_block_size);
		if (err > 0) return err; else if (err < 0) goto end;
		vsf_multibuf_push(&malstream->mbufstream->mem.multibuf);
		malstream->offset += mal->op_block_size;
		if (malstream->offset >= malstream->size)
		{
			// fix before callback
			stream->callback_tx.on_inout = NULL;
		}
		if (stream->rx_ready && (stream->callback_rx.on_inout != NULL))
		{
			stream->callback_rx.on_inout(stream->callback_rx.param);
		}
	}

end:
	stream_disconnect_tx(stream);
	if (malstream->cb.on_finish != NULL)
	{
		malstream->cb.on_finish(malstream->cb.param);
	}

	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

vsf_err_t vsf_malstream_read(struct vsf_malstream_t *malstream, uint64_t addr,
							uint32_t size)
{
	struct vsf_stream_t *stream = (struct vsf_stream_t *)malstream->mbufstream;

	malstream->addr = addr;
	malstream->size = size;

	stream->callback_tx.param = malstream;
	stream->callback_tx.on_inout = NULL;
	stream->callback_tx.on_connect = NULL;
	stream->callback_tx.on_disconnect = NULL;
	stream_connect_tx(stream);

	malstream->pt.thread = vsf_malstream_read_thread;
	malstream->pt.user_data = malstream;
	return vsfsm_pt_init(&malstream->sm, &malstream->pt);
}

static vsf_err_t
vsf_malstream_write_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsf_malstream_t *malstream = (struct vsf_malstream_t *)pt->user_data;
	struct vsfmal_t *mal = malstream->mal;
	struct vsf_stream_t *stream = (struct vsf_stream_t *)malstream->mbufstream;
	uint64_t cur_addr;
	vsf_err_t err;

	vsfsm_pt_begin(pt);

	mal->op_block_size = mal->drv->block_size(mal, malstream->addr, 0,
												VSFMAL_OP_WRITE);
	if (mal->op_block_size != malstream->mbufstream->mem.multibuf.size)
	{
		return VSFERR_BUG;
	}

	malstream->offset = 0;
	while (malstream->offset < malstream->size)
	{
		while (stream_get_data_size(stream) < mal->op_block_size)
		{
			stream->callback_rx.on_inout = vsf_malstream_on_inout;
			vsfsm_pt_wfe(pt, VSF_MALSTREAM_ON_INOUT);
			stream->callback_rx.on_inout = NULL;
		}

		mal->pt.user_data = mal;
		mal->pt.state = 0;
		vsfsm_pt_entry(pt);
		cur_addr = malstream->addr + malstream->offset;
		err = mal->drv->write(&mal->pt, evt, cur_addr,
				vsf_multibuf_get_payload(&malstream->mbufstream->mem.multibuf),
				mal->op_block_size);
		if (err > 0) return err; else if (err < 0) goto end;
		vsf_multibuf_pop(&malstream->mbufstream->mem.multibuf);
		malstream->offset += mal->op_block_size;
		if (malstream->offset >= malstream->size)
		{
			// fix before callback
			stream->callback_tx.on_inout = NULL;
		}
		if (stream->tx_ready && (stream->callback_tx.on_inout != NULL))
		{
			stream->callback_tx.on_inout(stream->callback_tx.param);
		}
	}

end:
	stream_disconnect_rx(stream);
	if (malstream->cb.on_finish != NULL)
	{
		malstream->cb.on_finish(malstream->cb.param);
	}

	vsfsm_pt_end(pt);
	return VSFERR_NONE;
}

vsf_err_t vsf_malstream_write(struct vsf_malstream_t *malstream, uint64_t addr,
							uint32_t size)
{
	struct vsf_stream_t *stream = (struct vsf_stream_t *)malstream->mbufstream;

	malstream->addr = addr;
	malstream->size = size;

	stream->callback_rx.param = malstream;
	stream->callback_rx.on_inout = NULL;
	stream->callback_rx.on_connect = NULL;
	stream->callback_rx.on_disconnect = NULL;
	stream_connect_rx(stream);

	malstream->pt.thread = vsf_malstream_write_thread;
	malstream->pt.user_data = malstream;
	return vsfsm_pt_init(&malstream->sm, &malstream->pt);
}

const struct vsfmal_drv_t vsfmim_drv =
{
	.block_size = vsfmim_blocksize,
	.init = vsfmim_init,
	.fini = vsfmim_fini,
	.erase_all = vsfmim_erase_all,
	.erase = vsfmim_erase,
	.read = vsfmim_read,
	.write = vsfmim_write,
};

