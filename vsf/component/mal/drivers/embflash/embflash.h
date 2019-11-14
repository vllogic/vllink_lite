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

#ifndef __EMBFLASH_H_INCLUDED__
#define __EMBFLASH_H_INCLUDED__

struct embflash_mal_t
{
	struct vsfmal_t mal;
	uint8_t index;
	uint32_t int_priority;

	// private
	vsf_err_t err;
	uint32_t cursize;
	uint32_t pagesize;
	struct vsfsm_t *notifier;
	struct vsfsm_pt_t pt;
};

#ifndef VSFCFG_EXCLUDE_EMBFLASH
extern struct vsfmal_drv_t embflash_mal_drv;
#endif

#endif	// __EMBFLASH_H_INCLUDED__
