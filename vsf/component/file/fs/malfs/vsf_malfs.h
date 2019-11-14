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

#ifndef __VSF_MALFS_H_INCLUDED__
#define __VSF_MALFS_H_INCLUDED__

#define VSF_MALFS_EVT_CRIT				(VSFSM_EVT_USER + 0)
#define VSF_MALFS_EVT_IODONE			(VSFSM_EVT_USER + 1)
#define VSF_MALFS_EVT_IOFAIL			(VSFSM_EVT_USER + 2)

#ifndef VSF_MALFS_CFG_BUFCNT
#	define VSF_MALFS_CFG_BUFCNT			8
#endif
struct vsf_malfs_t
{
	struct vsf_malstream_t malstream;

	// protected
	char *volume_name;
	struct vsfsm_crit_t crit;
	uint8_t *sector_buffer;
	struct vsfsm_t *notifier_sm;

	// private
	struct vsf_mbufstream_t mbufstream;
	uint8_t *mbufstream_buffer[VSF_MALFS_CFG_BUFCNT];
};

#ifndef VSFCFG_EXCLUDE_MALFS
vsf_err_t vsf_malfs_init(struct vsf_malfs_t *malfs);
void vsf_malfs_fini(struct vsf_malfs_t *malfs);

vsf_err_t vsf_malfs_read(struct vsf_malfs_t *malfs, uint32_t sector,
							uint8_t *buff, uint32_t num);
vsf_err_t vsf_malfs_write(struct vsf_malfs_t *malfs, uint32_t sector,
							uint8_t *buff, uint32_t num);
#endif

#endif	// __VSF_MALFS_H_INCLUDED__
