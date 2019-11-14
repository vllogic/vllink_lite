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

#ifndef __FAKEISO_H_INCLUDED__
#define __FAKEISO_H_INCLUDED__

#define FAKEISO_FILEATTR_HIDDEN				(uint8_t)(1 << 0)
#define FAKEISO_FILEATTR_DIRECTORY			(uint8_t)(1 << 1)

#define FAKEISO_BLOCK_SIZE					2048

struct fakeiso_file_t
{
	char *name;		// can not be longer than 221 bytes
	uint8_t attr;
	uint64_t size;
	
	struct fakeiso_file_callback_t
	{
		vsf_err_t (*init)(struct fakeiso_file_t *file);
		vsf_err_t (*fini)(struct fakeiso_file_t *file);
		vsf_err_t (*read)(struct fakeiso_file_t *file, uint64_t addr,
									uint8_t *buff, uint32_t page_size);
		vsf_err_t (*read_isready)(struct fakeiso_file_t *file, uint64_t addr,
									uint8_t *buff, uint32_t page_size);
		
		void (*on_read_file)(struct fakeiso_file_t *file);
	} callback;
	
	// filelist under directory
	struct fakeiso_file_t *filelist;
	
	void *extra;
	
	// private
	uint32_t first_lba;
	uint16_t diridx;
	struct fakeiso_file_t *parent;
	uint16_t volume_sequence_number;
};

struct fakeiso_param_t
{
	uint16_t sequence_number;
	
	char *system_identifier;
	char *volume_identifier;
	char *volume_set_identifier;
	char *publisher_identifier;
	char *data_preparer_identifier;
	char *application_identifier;
	char *copyright_file_identifier;
	char *abstract_file_identifier;
	char *bibliographic_file_identifier;
	
	struct fakeiso_file_t root[2];
	
	struct fakeiso_callback_t
	{
		vsf_err_t (*init)(struct dal_info_t *info);
		vsf_err_t (*fini)(struct dal_info_t *info);
		vsf_err_t (*read_sys_area)(struct dal_info_t *info, uint32_t lba, uint8_t *buff);
	} callback;
	
	void *extra;
	
	uint16_t keepout_num;
	uint32_t keepout_lba[32];
	uint32_t keepout_len[32];
	
	// private
	uint32_t path_table_lba;
	uint32_t path_table_size;
	uint16_t diridx;
	uint32_t block_num;
};

vsf_err_t fakeiso_dir_read(struct fakeiso_file_t *file,
			uint64_t addr, uint8_t *buff, uint32_t page_size);
extern struct mal_driver_t fakeiso_drv;

void fakeiso_print(struct fakeiso_file_t *file);

// file map
#include "app_io.h"
struct fakeiso_filemap_t
{
	char *name;
	
	// private
	FILE *file;
	uint64_t size;
};
vsf_err_t fakeiso_filemap_init(struct fakeiso_file_t *file);
vsf_err_t fakeiso_filemap_fini(struct fakeiso_file_t *file);
vsf_err_t fakeiso_filemap_read(struct fakeiso_file_t *file,
			uint64_t addr, uint8_t *buff, uint32_t page_size);

// helper
char* fakeiso_get_full_path(struct fakeiso_file_t *file, char *path);
struct fakeiso_file_t *fakeiso_get_file_by_path(struct fakeiso_file_t *root,
												char *path);

#endif	// __FAKEISO_H_INCLUDED__
