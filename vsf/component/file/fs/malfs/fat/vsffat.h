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

#ifndef __VSFFAT_H_INCLUDED__
#define __VSFFAT_H_INCLUDED__

struct vsffat_t;
struct vsfile_fatfile_t
{
	struct vsfile_t file;

	struct vsffat_t *fat;
	uint32_t first_cluster;
};

enum vsffat_type_t
{
	VSFFAT_NONE = 0,
	VSFFAT_FAT12,
	VSFFAT_FAT16,
	VSFFAT_FAT32,
	VSFFAT_EXFAT,
};

struct vsffat_dentry_parser_t
{
	uint8_t *entry;
	uint16_t entry_num;
	uint8_t lfn;
	char *filename;
};

struct vsffat_t
{
	struct vsf_malfs_t malfs;

	// protected
	enum vsffat_type_t type;

	// information parsed from bpb
	uint8_t sectorsize_bits;
	uint8_t clustersize_bits;
	uint8_t fatnum;
	uint16_t reserved_size;
	uint16_t rootentry;
	uint16_t fsinfo;
	uint32_t fatsize;
	uint32_t root_cluster;

	// information calculated
	uint32_t clusternum;
	uint32_t fatbase;
	uint32_t database;
	uint32_t rootbase;
	uint32_t rootsize;

	// private
	struct vsfile_fatfile_t root;
	struct vsfsm_pt_t caller_pt;

	// for getchild_byname, getchild_byidx, read and write
	uint32_t cur_cluster;
	uint32_t last_sector;
	uint32_t cur_sector;
	union
	{
		// for read and write
		struct
		{
			uint64_t cur_offset;
			uint32_t cur_size;
			uint32_t cur_run_size;
			uint32_t cur_run_sector;
			uint32_t remain_size;
		};
		// for getchild_byname, getchild_byidx
		struct
		{
			uint32_t cur_index;
			uint8_t *cur_name_pos;
		};
	};

	// for vsffat_get_FATentry
	uint32_t readbit;
	uint32_t cur_fatsector;
	// for vsffat_alloc_cluschain
	// for vsffat_parse_dentry_fat
	struct vsffat_dentry_parser_t dparser;
};

#ifndef VSFCFG_EXCLUDE_FAT
extern const struct vsfile_fsop_t vsffat_op;
// helper
bool vsffat_is_LFN(char *name);
bool vsffat_parse_dentry_fat(struct vsffat_dentry_parser_t *parser);
#endif

#endif	// __VSFFAT_H_INCLUDED__
