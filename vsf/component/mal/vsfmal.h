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

#ifndef __VSFMAL_H_INCLUDED__
#define __VSFMAL_H_INCLUDED__

struct vsfmal_capacity_t
{
	uint64_t block_size;
	uint64_t block_num;
};

enum vsfmal_op_t
{
	VSFMAL_OP_ERASE,
	VSFMAL_OP_READ,
	VSFMAL_OP_WRITE,
};

struct vsfmal_t;
struct vsfmal_drv_t
{
	uint32_t (*block_size)(struct vsfmal_t *mal, uint64_t addr, uint32_t size,
					enum vsfmal_op_t op);
	vsf_err_t (*init)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
	vsf_err_t (*fini)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
	vsf_err_t (*erase_all)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
	vsf_err_t (*erase)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt, uint64_t addr,
					uint32_t size);
	vsf_err_t (*read)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt, uint64_t addr,
					uint8_t *buff, uint32_t size);
	vsf_err_t (*write)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt, uint64_t addr,
					uint8_t *buff, uint32_t size);
};

struct vsfmal_t
{
	struct vsfmal_capacity_t cap;
	struct vsfmal_drv_t const *drv;
	void *cparam;
	void *param;

	// private
	struct vsfsm_pt_t pt;
	uint32_t op_block_size;
	uint32_t offset;
};

struct vsf_malstream_cb_t
{
	void (*on_finish)(void *param);
	void *param;
};

// mim: mal in mal
struct vsfmim_t
{
	struct vsfmal_t mal;

	// private
	struct vsfmal_t *realmal;
	uint32_t addr;
	uint32_t size;
};

// mal stream, IMPORTANT: stream MUST be multibuf_stream
struct vsf_malstream_t
{
	struct vsfmal_t *mal;
	struct vsf_mbufstream_t *mbufstream;
	struct vsf_malstream_cb_t cb;

	// read only for user
	uint32_t offset;

	// private
	bool mal_ready;
	uint64_t addr;
	uint32_t size;
	struct vsfsm_t sm;
	struct vsfsm_pt_t pt;
};

#ifndef VSFCFG_EXCLUDE_MAL
vsf_err_t vsfmal_init(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
vsf_err_t vsfmal_fini(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
vsf_err_t vsfmal_erase_all(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
vsf_err_t vsfmal_erase(struct vsfsm_pt_t *pt, vsfsm_evt_t evt, uint64_t addr,
					uint32_t size);
vsf_err_t vsfmal_read(struct vsfsm_pt_t *pt, vsfsm_evt_t evt, uint64_t addr,
					uint8_t *buff, uint32_t size);
vsf_err_t vsfmal_write(struct vsfsm_pt_t *pt, vsfsm_evt_t evt, uint64_t addr,
					uint8_t *buff, uint32_t size);

extern const struct vsfmal_drv_t vsfmim_drv;

vsf_err_t vsf_malstream_init(struct vsf_malstream_t *malstream);
vsf_err_t vsf_malstream_read(struct vsf_malstream_t *malstream, uint64_t addr,
							uint32_t size);
vsf_err_t vsf_malstream_write(struct vsf_malstream_t *malstream, uint64_t addr,
							uint32_t size);
#endif

#endif	// __VSFMAL_H_INCLUDED__
