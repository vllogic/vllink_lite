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

//#include <ctype.h>
//#include <inttypes.h>

#include "compiler.h"

#include "app_cfg.h"
#include "app_type.h"
#include "app_log.h"

#include "dal/mal/mal.h"
#include "dal/mal/mal_driver.h"

#include "fakeiso.h"

#define FAKEISO_FILEATTR_NOT_FINAL			(uint8_t)(1 << 7)

struct fakeiso_file_t *fakeiso_get_file_by_path(struct fakeiso_file_t *root,
												char *path)
{
	uint32_t len;
	struct fakeiso_file_t *ret = NULL;

	if ((NULL == root) || (NULL == root->name) || (NULL == path)) return NULL;
	if ('/' == path[0]) path++;

	len = strlen(root->name);
	if (!memcmp(path, root->name, len)) {
		if ('\0' == path[len]) {
			ret = root;
		} else if (('/' == path[len]) &&
					(root->attr & FAKEISO_FILEATTR_DIRECTORY)) {
			root = root->filelist;
			while ((root != NULL) && (root->name != NULL))
			{
				ret = fakeiso_get_file_by_path(root, path + len);
				if (ret != NULL) break;
				root++;
			}
		}
	}
	return ret;
}

char* fakeiso_get_full_path(struct fakeiso_file_t *file, char *path)
{
	if ((file != NULL) && (path != NULL))
	{
		path[0] = '\0';
		if (file->parent != NULL)
		{
			fakeiso_get_full_path(file->parent, path);
		}
		if (path[strlen(path) - 1] != '/')
		{
			strcat(path, "/");
		}
		strcat(path, file->name);
	}
	return path;
}
#if !LOG_QUIET
static void fakeiso_do_print(struct fakeiso_file_t *file, uint8_t level)
{
	struct fakeiso_filemap_t *filemap;
	uint8_t i;
	char path[256];
	
	while (file->name != NULL)
	{
		for(i = 0; i < level; i++)
		{
			PRINTF(" ");
		}
		PRINTF("%s(%"PRIu64" bytes from sector %d)", fakeiso_get_full_path(file, path), file->size, file->first_lba);
		filemap = (struct fakeiso_filemap_t *)file->extra;
		if (fakeiso_filemap_init == file->callback.init)
		{
			PRINTF("linking to %s", NULL == filemap ? "NULL" : filemap->name);
		}
		PRINTF("\n");
		
		if (file->filelist != NULL)
		{
			fakeiso_do_print(file->filelist, level + 1);
		}
		file++;
	}
}
void fakeiso_print(struct fakeiso_file_t *file)
{
	fakeiso_do_print(file, 0);
}
#endif

// layout:
// 0-15:	system information, all 0s
// 16:		Primary Volume Descriptor(PVD)
// 17:		Volume Descriptor Set Terminator
// 18-XX:	little-endian Path Table
// XX-XX:	bigger-endian Path Table
// XX---:	files

// path table:
// 0(1):	length of directory identifier
// 1(1):	length of extended attribute record
// 2(4):	lba
// 6(2):	parent directory number
// 8(n):	directory identifier

// file record:
// 0(1):	length of record
// 1(1):	length of extended attribute record
// 2(4):	lba in little endian
// 6(4):	lba in bigger endian
// 10(4):	size in little endian
// 14(4):	size in bigger endian
// 18(7):	data/time record
// 25(1):	file flag
// 26(1):	0
// 27(1):	0
// 28(2):	sequence number in little endian
// 30(2):	sequence number in bigger endian
// 32(1):	file identifier length
// 32(n):	file name

static uint16_t fakeiso_calc_path_table_record_size(struct fakeiso_file_t *file)
{
	uint16_t filename_size;
	uint16_t size;
	
	if (!strcmp(file->name, ".") || !strcmp(file->name, ".."))
	{
		filename_size = 1;
	}
	else
	{
		filename_size = (uint16_t)strlen(file->name);
	}
	size = 8 + min(filename_size, (uint16_t)0xFF);
	return (size + 1) & ~1;
}

static uint32_t fakeiso_calc_path_table_size(struct fakeiso_file_t *root)
{
	struct fakeiso_file_t *file;
	uint32_t size = 0;
	
	if ((NULL == root) || !(root->attr & FAKEISO_FILEATTR_DIRECTORY) ||
		(NULL == root->filelist))
	{
		return 0;
	}
	
	size = fakeiso_calc_path_table_record_size(root);
	
	file = root->filelist;
	while (file->name != NULL)
	{
		// search all directory except . and ..
		if ((file->attr & FAKEISO_FILEATTR_DIRECTORY) &&
			strcmp(file->name, ".") && strcmp(file->name, ".."))
		{
			size += (uint32_t)fakeiso_calc_path_table_size(file);
		}
		file++;
	}
	return size;
}

static void fakeiso_fill_path_table_record(struct fakeiso_file_t *file,
			uint8_t *buff, bool little)
{
	if ((file != NULL) && (buff != NULL) && (file->name != NULL) &&
		(file->attr & FAKEISO_FILEATTR_DIRECTORY))
	{
		uint16_t parent = NULL == file->parent ? 1 : file->parent->diridx;
		
		buff[0] = (uint8_t)min(0xFF, (int)strlen(file->name));
		buff[1] = 0;
		if (little)
		{
			SET_LE_U32(&buff[2], file->first_lba);
			SET_LE_U16(&buff[6], parent);
		}
		else
		{
			SET_BE_U32(&buff[2], file->first_lba);
			SET_BE_U16(&buff[6], parent);
		}
	}
}

static vsf_err_t fakeiso_fill_path_table(struct fakeiso_param_t *param,
			struct fakeiso_file_t *file, uint32_t *offset, uint8_t **buff,
			uint16_t *remain_size, bool little, bool root)
{
	uint32_t size = 0;
	
	if ((NULL == offset) || (NULL == file) || (NULL == remain_size) ||
		!(file->attr & FAKEISO_FILEATTR_DIRECTORY) || (NULL == file->filelist))
	{
		return VSFERR_INVALID_PARAMETER;
	}
	if (0 == *remain_size)
	{
		return VSFERR_NONE;
	}
	
	file->diridx = param->diridx++;
	size = fakeiso_calc_path_table_record_size(file);
	if (*offset >= size)
	{
		*offset -= size;
	}
	else
	{
		uint16_t copy_size;
		
		if (*offset < 8)
		{
			uint8_t buff_tmp[8];
			
			copy_size = (uint16_t)min((uint32_t)*remain_size, 8 - *offset);
			if (0 == copy_size)
			{
				return VSFERR_NONE;
			}
			
			if (root)
			{
				fakeiso_fill_path_table_record(file, buff_tmp, little);
			}
			else
			{
				fakeiso_fill_path_table_record(file, buff_tmp, little);
			}
			memcpy(*buff, buff_tmp + *offset, copy_size);
			*remain_size -= copy_size;
			*buff += copy_size;
			*offset = 0;
		}
		
		copy_size = (uint16_t)min((uint32_t)*remain_size, size - 8);
		if (0 == copy_size)
		{
			return VSFERR_NONE;
		}
		
		if (root)
		{
			(*buff)[0] = 0;
		}
		else
		{
			memcpy(*buff, file->name, copy_size);
		}
		
		*remain_size -= copy_size;
		*buff += copy_size;
		*offset = 0;
	}
	
	file = file->filelist;
	while ((file != NULL) && (file->name != NULL))
	{
		// search all directory except . and ..
		if ((file->attr & FAKEISO_FILEATTR_DIRECTORY) &&
			strcmp(file->name, ".") && strcmp(file->name, ".."))
		{
			fakeiso_fill_path_table(param, file, offset, buff, remain_size,
									little, false);
		}
		file++;
	}
	return VSFERR_NONE;
}

static uint8_t fakeiso_calc_file_record_size(struct fakeiso_file_t *file)
{
	uint16_t filename_size;
	uint8_t size;
	
	if (!strcmp(file->name, ".") || !strcmp(file->name, ".."))
	{
		filename_size = 1;
	}
	else if (file->attr & FAKEISO_FILEATTR_DIRECTORY)
	{
		filename_size = (uint16_t)strlen(file->name);
	}
	else
	{
		filename_size = (uint16_t)strlen(file->name) + 2;
	}
	size = 33 + (uint8_t)min(filename_size, (uint16_t)(0xFE - 33));
	return (size + 1) & ~1;
}

static uint64_t fakeiso_calc_dir_size(struct fakeiso_file_t *file)
{
	uint64_t size, cur_page_size, cur_size, fsize, cur_fsize;
	
	if ((NULL == file) || !(file->attr & FAKEISO_FILEATTR_DIRECTORY) ||
		(NULL == file->filelist))
	{
		return 0;
	}
	
	file = file->filelist;
	size = cur_page_size = 0;
	while (file->name != NULL)
	{
		fsize = file->size;
		if (0 == fsize)
		{
			fsize = 1;
		}
		while (fsize)
		{
			cur_fsize = (fsize >= 0x100000000ULL) ?
								0xFFFFF800 : (uint32_t)fsize;
			cur_size = (uint32_t)fakeiso_calc_file_record_size(file);
			if ((cur_page_size + cur_size) > FAKEISO_BLOCK_SIZE)
			{
				size += FAKEISO_BLOCK_SIZE;
				cur_page_size = 0;
			}
			cur_page_size += cur_size;
			fsize -= cur_fsize;
		}
		file++;
	}
	size += cur_page_size;
	return size;
}

static struct fakeiso_file_t* fakeiso_get_file_by_lba(
			struct fakeiso_file_t *file, uint32_t lba)
{
	struct fakeiso_file_t *result = NULL;
	uint32_t lba_start, lba_end, blocks;
	
	blocks = (uint32_t)((file->size + FAKEISO_BLOCK_SIZE - 1) / FAKEISO_BLOCK_SIZE);
	lba_start = file->first_lba;
	lba_end = lba_start + blocks;
	if ((lba >= lba_start) && (lba < lba_end))
	{
		return file;
	}
	
	file = file->filelist;
	while ((file != NULL) && (file->name != NULL))
	{
		result = fakeiso_get_file_by_lba(file, lba);
		if (result != NULL)
		{
			return result;
		}
		file++;
	}
	return NULL;
}

static uint32_t fakeiso_get_free_block(struct fakeiso_param_t *param,
										uint32_t *cur_block, uint32_t num)
{
	uint16_t i;
	
	for (i = 0; i < param->keepout_num; i++)
	{
		if ((*cur_block < (param->keepout_lba[i] + param->keepout_len[i])) &&
			((*cur_block + num) > param->keepout_lba[i]))
		{
			LOG_INFO("skip keepout area(%d blocks at %d)", param->keepout_len[i], param->keepout_lba[i]);
			*cur_block = param->keepout_lba[i] + param->keepout_len[i];
			*cur_block = (*cur_block + 0x1F) & ~0x1F;
		}
	}
	return *cur_block;
}

static vsf_err_t fakeiso_file_init(struct fakeiso_file_t *file)
{
	if (NULL == file)
	{
		return VSFERR_FAIL;
	}
	
	if ((file->callback.init != NULL) &&
		file->callback.init(file))
	{
		return VSFERR_FAIL;
	}
	
	if (file->filelist != NULL)
	{
		file = file->filelist;
		while (file->name != NULL)
		{
			if (fakeiso_file_init(file))
			{
				return VSFERR_FAIL;
			}
			file++;
		}
	}
	return VSFERR_NONE;
}
static vsf_err_t fakeiso_init(struct fakeiso_param_t *param,
				struct fakeiso_file_t *file, uint32_t *cur_block)
{
	uint32_t block_num;
	
	if (NULL == file)
	{
		return VSFERR_FAIL;
	}
	
	file->volume_sequence_number = param->sequence_number;
	*cur_block = (*cur_block + 0x1F) & ~0x1F;
	if (file->attr & FAKEISO_FILEATTR_DIRECTORY)
	{
		if ((!strcmp(file->name, ".") || !strcmp(file->name, "..")) &&
			(file->parent != NULL))
		{
			file->size = fakeiso_calc_dir_size(file->parent);
			file->first_lba = file->parent->first_lba;
		}
		else
		{
			file->size = fakeiso_calc_dir_size(file);
			block_num = (uint32_t)((file->size + FAKEISO_BLOCK_SIZE - 1) / FAKEISO_BLOCK_SIZE);
			if (!file->first_lba)
			{
				file->first_lba = fakeiso_get_free_block(param, cur_block, block_num);
				*cur_block += block_num;
			}
			else if (file->first_lba > *cur_block)
			{
				*cur_block = file->first_lba + block_num;
			}
		}
	}
	else
	{
		block_num = (uint32_t)((file->size + FAKEISO_BLOCK_SIZE - 1) / FAKEISO_BLOCK_SIZE);
		if (!file->first_lba)
		{
			file->first_lba = fakeiso_get_free_block(param, cur_block, block_num);
			*cur_block += block_num;
		}
		else if (file->first_lba > *cur_block)
		{
			*cur_block = file->first_lba + block_num;
		}
	}
	*cur_block = (*cur_block + 0x1F) & ~0x1F;
	param->block_num = *cur_block;
	
	if (file->filelist != NULL)
	{
		struct fakeiso_file_t *parent = file;
		
		file = file->filelist;
		while (file->name != NULL)
		{
			file->parent = parent;
			if (fakeiso_init(param, file, cur_block))
			{
				return VSFERR_FAIL;
			}
			file++;
		}
	}
	return VSFERR_NONE;
}

static void fakeiso_fill_file_record(struct fakeiso_file_t *file,
			uint8_t *buff)
{
	if ((file != NULL) && (buff != NULL) && (file->name != NULL))
	{
		uint32_t size = 0;
		
		// Length of Directory Record
		buff[0] = fakeiso_calc_file_record_size(file);
		// Extended Attribute Record Length
		buff[1] = 0;
		// LBA
		SET_LE_U32(&buff[2], file->first_lba);
		SET_BE_U32(&buff[6], file->first_lba);
		// Data length
		if (file->attr & FAKEISO_FILEATTR_DIRECTORY)
		{
			if (!strcmp(file->name, ".") ||
				(!strcmp(file->name, "..") && (NULL == file->parent)) ||
				(strcmp(file->name, ".") && strcmp(file->name, "..")))
			{
				size = (uint32_t)file->size;
			}
			else if (!strcmp(file->name, "..") && (file->parent != NULL))
			{
				size = (uint32_t)file->parent->size;
			}
			size += FAKEISO_BLOCK_SIZE - 1;
			size /= FAKEISO_BLOCK_SIZE;
			size *= FAKEISO_BLOCK_SIZE;
		}
		else
		{
			size = (uint32_t)file->size;
		}
		SET_LE_U32(&buff[10], size);
		SET_BE_U32(&buff[14], size);
		// Recording data and time
		memset(&buff[18], 0, 7);
		// File flags
		buff[25] = file->attr;
		// not concerned
		buff[26] = buff[27] = 0;
		// Volume sequence number
		SET_LE_U16(&buff[28], file->volume_sequence_number);
		SET_BE_U16(&buff[30], file->volume_sequence_number);
		// Length of file identifier
		if (!strcmp(file->name, ".") || !strcmp(file->name, ".."))
		{
			buff[32] = 1;
		}
		else if (file->attr & FAKEISO_FILEATTR_DIRECTORY)
		{
			buff[32] = (uint8_t)min(0xFE - 33, (int)strlen(file->name));
		}
		else
		{
			buff[32] = (uint8_t)min(0xFE - 33, (int)strlen(file->name) + 2);
		}
	}
}

static vsf_err_t fakeiso_fill_dir(struct fakeiso_file_t *file,
			uint64_t *offset, uint8_t *buff, uint32_t *remain_size)
{
	uint64_t file_size;
	uint32_t entry_size, page_size;
	uint32_t cur_block_size, cur_lba, cur_size;
	bool last_entry;
	
	if ((NULL == offset) || (*offset % FAKEISO_BLOCK_SIZE) ||
		(NULL == remain_size) || (NULL == file) ||
		!(file->attr & FAKEISO_FILEATTR_DIRECTORY) || (NULL == file->filelist))
	{
		return VSFERR_INVALID_PARAMETER;
	}
	if (0 == *remain_size)
	{
		return VSFERR_NONE;
	}
	
	file = file->filelist;
	page_size = 0;
	while ((file != NULL) && (file->name != NULL))
	{
		file_size = file->size;
		cur_lba = 0;
		
		while (file_size)
		{
			last_entry = file_size < 0x100000000ULL;
			cur_size = (file_size >= 0x100000000ULL) ?
								0xFFFFF800 : (uint32_t)file_size;
			cur_block_size = (cur_size + FAKEISO_BLOCK_SIZE - 1) / FAKEISO_BLOCK_SIZE;
			entry_size = fakeiso_calc_file_record_size(file);
			
			if (*offset > 0)
			{
				if ((page_size + entry_size) > FAKEISO_BLOCK_SIZE)
				{
					*offset -= FAKEISO_BLOCK_SIZE;
					page_size = 0;
				}
				page_size += entry_size;
			}
			if (0 == *offset)
			{
				if (*remain_size < entry_size)
				{
					break;
				}
				else
				{
					fakeiso_fill_file_record(file, buff);
					SET_LE_U32(&buff[2], file->first_lba + cur_lba);
					SET_BE_U32(&buff[6], file->first_lba + cur_lba);
					SET_LE_U32(&buff[10], cur_size);
					SET_BE_U32(&buff[14], cur_size);
					if (!last_entry)
					{
						buff[25] |= FAKEISO_FILEATTR_NOT_FINAL;
					}
					buff += 33;
					
					if (!strcmp(file->name, "."))
					{
						buff[0] = 0;
					}
					else if (!strcmp(file->name, ".."))
					{
						buff[0] = 1;
					}
					else if (file->attr & FAKEISO_FILEATTR_DIRECTORY)
					{
						strcpy((char*)buff, file->name);
					}
					else
					{
						strcpy((char*)buff, file->name);
						strcat((char*)buff, ";1");
					}
					
					*remain_size -= entry_size;
					buff += entry_size - 33;
				}
			}
			file_size -= cur_size;
			cur_lba += cur_block_size;
		}
		file++;
	}
	
	return VSFERR_NONE;
}

vsf_err_t fakeiso_dir_read(struct fakeiso_file_t* file, uint64_t addr,
								uint8_t *buff, uint32_t page_size)
{
	return fakeiso_fill_dir(file, &addr, buff, &page_size);
}

static vsf_err_t fakeiso_drv_init_nb(struct dal_info_t *info)
{
	struct fakeiso_param_t *param = (struct fakeiso_param_t *)info->param;
	struct mal_info_t *mal_info = (struct mal_info_t *)info->extra;
	// first 16 blocks (32K bytes) are all 0s
	// followed by 1 Primary Volume Descriptor
	//		and 1 Volume Descriptor Set Terminator
	// followed by Path Table
	uint32_t cur_block = 16 + 2;
	vsf_err_t err = VSFERR_NONE;
	
	if (((param->callback.init != NULL) && param->callback.init(info)) ||
		strcmp(param->root[0].name, "."))
	{
		return VSFERR_FAIL;
	}
	
	mal_info->capacity.block_number = 0;
	mal_info->capacity.block_size = FAKEISO_BLOCK_SIZE;
	param->root[0].attr = FAKEISO_FILEATTR_DIRECTORY;
	param->root[0].parent = NULL;
	param->root[1].name = NULL;
	param->path_table_lba = cur_block;
	param->path_table_size = fakeiso_calc_path_table_size(param->root);
	cur_block += 2 * (param->path_table_size + FAKEISO_BLOCK_SIZE - 1) / FAKEISO_BLOCK_SIZE;
	
	param->block_num = 0;
	err = fakeiso_file_init(param->root);
	if (err)
	{
		return VSFERR_FAIL;
	}
	err = fakeiso_init(param, param->root, &cur_block);
	if (!err)
	{
		// additional 64K empty area
		mal_info->capacity.block_number = param->block_num + 0x20;
#if !LOG_QUIET
		// only print directory structure if the disc size is below 128MB
//		if (mal_info->capacity.block_number < 0x10000)
		{
//			fakeiso_print(param->root);
		}
#endif
		LOG_INFO("disc initialized with %d blocks", (int)mal_info->capacity.block_number);
	}
	return err;
}

static vsf_err_t fakeiso_drv_fini(struct dal_info_t *info)
{
	struct fakeiso_param_t *param = (struct fakeiso_param_t *)info->param;
	if ((param != NULL) && (param->callback.fini != NULL))
	{
		param->callback.fini(info);
	}
	return VSFERR_NONE;
}

static vsf_err_t fakeiso_drv_readblock_nb_start(struct dal_info_t *info, 
								uint64_t address, uint64_t count, uint8_t *buff)
{
	REFERENCE_PARAMETER(info);
	REFERENCE_PARAMETER(address);
	REFERENCE_PARAMETER(count);
	REFERENCE_PARAMETER(buff);
	return VSFERR_NONE;
}

static vsf_err_t fakeiso_drv_readblock_nb(struct dal_info_t *info, 
											uint64_t address, uint8_t *buff)
{
	uint32_t lba = (uint32_t)(address / FAKEISO_BLOCK_SIZE);
	struct fakeiso_param_t *param = (struct fakeiso_param_t *)info->param;
	struct mal_info_t *mal_info = (struct mal_info_t *)info->extra;
	uint32_t path_table_blocks =
		(param->path_table_size + FAKEISO_BLOCK_SIZE - 1) / FAKEISO_BLOCK_SIZE;
	
	memset(buff, 0, FAKEISO_BLOCK_SIZE);
	
	if (lba < 16)
	{
		// first 32K area is system dependent
		if (param->callback.read_sys_area != NULL)
		{
			return param->callback.read_sys_area(info, lba, buff);
		}
	}
	else if (16 == lba)
	{
		uint16_t len;
		// Primary Volume Descriptor
		
		// Type Code: 0x01 for Primary Volume Descriptor
		buff[0x00] = 0x01;
		// Standard Identifier
		strcpy((char *)&buff[1], "CD001");
		// Versaion: 1
		buff[0x06] = 0x01;
		// System Identifier
		memset(&buff[0x08], ' ', 32);
		if (param->system_identifier != NULL)
		{
			len = (uint16_t)min(32, (int)strlen(param->system_identifier));
			memcpy(&buff[0x08], param->system_identifier, len);
		}
		// Volume Identifier
		memset(&buff[0x28], ' ', 32);
		if (param->volume_identifier != NULL)
		{
			len = (uint16_t)min(32, (int)strlen(param->volume_identifier));
			memcpy(&buff[0x28], param->volume_identifier, len);
		}
		// Volume Space Size
		SET_LE_U32(&buff[0x50], (uint32_t)mal_info->capacity.block_number);
		SET_BE_U32(&buff[0x54], (uint32_t)mal_info->capacity.block_number);
		// Volume Set Size
		SET_LE_U16(&buff[0x78], 1);
		SET_BE_U16(&buff[0x7A], 1);
		// Volume Sequence Number
		SET_LE_U16(&buff[0x7C], param->sequence_number);
		SET_BE_U16(&buff[0x7E], param->sequence_number);
		// Logic Block Size
		SET_LE_U16(&buff[0x80], FAKEISO_BLOCK_SIZE);
		SET_BE_U16(&buff[0x82], FAKEISO_BLOCK_SIZE);
		// Path Table Size
		SET_LE_U32(&buff[0x84], param->path_table_size);
		SET_BE_U32(&buff[0x88], param->path_table_size);
		// Location of little-endian Path Table
		SET_LE_U32(&buff[0x8C], param->path_table_lba);
		// Location of little-endian Optional Path Table
		SET_LE_U32(&buff[0x90], 0);
		// Location of bigger-endian Path Table
		SET_BE_U32(&buff[0x94], param->path_table_lba +
				((param->path_table_size + FAKEISO_BLOCK_SIZE - 1) / FAKEISO_BLOCK_SIZE));
		// Location of bigger-endian Optional Path Table
		SET_BE_U32(&buff[0x98], 0);
		// root directory entry
		fakeiso_fill_file_record(param->root, &buff[0x9C]);
		// Volume Set Identifier
		memset(&buff[0xBE], ' ', 128);
		if (param->volume_set_identifier != NULL)
		{
			len = (uint16_t)min(128, (int)strlen(param->volume_set_identifier));
			memcpy(&buff[0xBE], param->volume_set_identifier, len);
		}
		// Publisher Identifier
		memset(&buff[0x13E], ' ', 128);
		if (param->publisher_identifier != NULL)
		{
			len = (uint16_t)min(128, (int)strlen(param->publisher_identifier));
			memcpy(&buff[0x13E], param->publisher_identifier, len);
		}
		// Data Prepare Identifier
		memset(&buff[0x1BE], ' ', 128);
		if (param->data_preparer_identifier != NULL)
		{
			len = (uint16_t)min(128, (int)strlen(param->data_preparer_identifier));
			memcpy(&buff[0x1BE], param->data_preparer_identifier, len);
		}
		// Application Identifier
		memset(&buff[0x23E], ' ', 128);
		if (param->application_identifier != NULL)
		{
			len = (uint16_t)min(128, (int)strlen(param->application_identifier));
			memcpy(&buff[0x23E], param->application_identifier, len);
		}
		// Copyright File Identifier
		memset(&buff[0x2BE], ' ', 38);
		if (param->copyright_file_identifier != NULL)
		{
			len = (uint16_t)min(38, (int)strlen(param->copyright_file_identifier));
			memcpy(&buff[0x2BE], param->copyright_file_identifier, len);
		}
		// Abstract File Identifier
		memset(&buff[0x2E4], ' ', 36);
		if (param->abstract_file_identifier != NULL)
		{
			len = (uint16_t)min(36, (int)strlen(param->abstract_file_identifier));
			memcpy(&buff[0x2E4], param->abstract_file_identifier, len);
		}
		// Bibliographic File Identifier
		memset(&buff[0x308], ' ', 37);
		if (param->bibliographic_file_identifier != NULL)
		{
			len = (uint16_t)min(37, (int)strlen(param->bibliographic_file_identifier));
			memcpy(&buff[0x308], param->bibliographic_file_identifier, len);
		}
		// Volume Creation Data and Time
		memset(&buff[0x32D], '0', 16);
		// Volume Modification Data and Time
		memset(&buff[0x33E], '0', 16);
		// Volume Expiration Data and Time
		memset(&buff[0x34F], '0', 16);
		// Volume Effective Data and Time
		memset(&buff[0x360], '0', 16);
		// File Structure Version: 1
		buff[0x371] = 1;
	}
	else if (17 == lba)
	{
		// Volume Descriptor Set Terminator
		// Type Code: 0xFF for Volume Descriptor Set Terminator
		buff[0] = 0xFF;
		// Standard Identifier
		strcpy((char *)&buff[1], "CD001");
		// Versaion: 1
		buff[6] = 0x01;
	}
	else if ((lba >= param->path_table_lba) &&
			(lba < (param->path_table_lba + 2 * path_table_blocks)))
	{
		bool little;
		uint32_t offset;
		uint16_t remain_size = FAKEISO_BLOCK_SIZE;
		
		// Path Table
		if (lba < (param->path_table_lba + path_table_blocks))
		{
			// Path Table little endian
			offset = (lba - param->path_table_lba) * FAKEISO_BLOCK_SIZE;
			little = true;
		}
		else
		{
			// Path Table bigger endian
			offset = (lba - param->path_table_lba - path_table_blocks) *
						FAKEISO_BLOCK_SIZE;
			little = false;
		}
		
		param->diridx = 1;
		return fakeiso_fill_path_table(param, param->root, &offset, &buff,
										&remain_size, little, true);
	}
	else
	{
		// Files
		struct fakeiso_file_t *file = fakeiso_get_file_by_lba(param->root, lba);
#if !LOG_QUIET
		char full_path[256];
#endif
		static struct fakeiso_file_t *__file = NULL;
		static uint64_t __offset = 0;
		
		if ((file != NULL) && (file->callback.read != NULL))
		{
			uint64_t addr_offset = FAKEISO_BLOCK_SIZE * (lba - file->first_lba);
			vsf_err_t err = file->callback.read(file, addr_offset, buff, FAKEISO_BLOCK_SIZE);
			
			if (!err)
			{
//				LOG_INFO("reading 0x%08X offset 0x%08X at %s("PRIu64" bytes)", lba,
//					(uint32_t)addr_offset, fakeiso_get_full_path(file, full_path), file->size);
				
				if (__file != file)
				{
					if (0 == addr_offset)
					{
						__file = file;
						__offset = FAKEISO_BLOCK_SIZE;
#if !LOG_QUIET
						if (__offset >= __file->size)
						{
							LOG_INFO("%s selected", fakeiso_get_full_path(__file, full_path));
						}
#endif
					}
					else
					{
						__file = NULL;
					}
				}
				else
				{
					if (__offset == addr_offset)
					{
						__offset += FAKEISO_BLOCK_SIZE;
					}
					if (__offset >= __file->size)
					{
#if !LOG_QUIET
						LOG_INFO("%s selected", fakeiso_get_full_path(__file, full_path));
#endif
						if ((__file != NULL) && (file->callback.on_read_file != NULL))
						{
							file->callback.on_read_file(__file);
							__file = NULL;
						}
					}
				}
			}
			return err;
		}
	}
	
	return VSFERR_NONE;
}

static vsf_err_t fakeiso_drv_readblock_nb_isready(struct dal_info_t *info, 
												uint64_t address, uint8_t *buff)
{
	struct fakeiso_param_t *param = (struct fakeiso_param_t *)info->param;
	uint32_t lba = (uint32_t)address / FAKEISO_BLOCK_SIZE;
	struct fakeiso_file_t *file = NULL;
	
	file = fakeiso_get_file_by_lba(param->root, lba);
	if (file != NULL)
	{
		if (file->callback.read_isready != NULL)
		{
			uint32_t addr_offset = FAKEISO_BLOCK_SIZE * (lba - file->first_lba);
			return file->callback.read_isready(file, addr_offset, buff,
												FAKEISO_BLOCK_SIZE);
		}
	}
	return VSFERR_NONE;
}

static vsf_err_t fakeiso_drv_readblock_nb_end(struct dal_info_t *info)
{
	REFERENCE_PARAMETER(info);
	return VSFERR_NONE;
}

#if DAL_INTERFACE_PARSER_EN
static vsf_err_t fakeiso_drv_parse_interface(struct dal_info_t *info, 
												uint8_t *buff)
{
	REFERENCE_PARAMETER(info);
	REFERENCE_PARAMETER(buff);
	return VSFERR_NONE;
}
#endif

struct mal_driver_t fakeiso_drv = 
{
	{
		"fakeiso",
#if DAL_INTERFACE_PARSER_EN
		"",
		fakeiso_drv_parse_interface,
#endif
	},
	
	MAL_SUPPORT_READBLOCK,
	
	fakeiso_drv_init_nb,
	NULL,
	fakeiso_drv_fini,
	NULL,
	NULL,
	
	NULL, NULL, NULL, NULL,
	
	NULL, NULL, NULL, NULL,
	
	NULL, NULL, NULL, NULL, NULL,
	
	fakeiso_drv_readblock_nb_start,
	fakeiso_drv_readblock_nb,
	fakeiso_drv_readblock_nb_isready,
	NULL,
	fakeiso_drv_readblock_nb_end,
	
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

// file map
vsf_err_t fakeiso_filemap_init(struct fakeiso_file_t *file)
{
	struct fakeiso_filemap_t *filemap = (struct fakeiso_filemap_t *)file->extra;
	
	if (filemap != NULL)
	{
		if ((filemap->name != NULL) &&
			((NULL == filemap->file) || !FVALID(filemap->file)))
		{
			filemap->file = FOPEN(filemap->name, O_RDONLY | O_LARGEFILE);
			if (!FVALID(filemap->file))
			{
				LOG_INFO("iso_filemap: fail to open %s", filemap->name);
				return VSFERR_FAIL;
			}
			
			if (FSEEK(filemap->file, 0, SEEK_END) < 0)
			{
				LOG_INFO("iso_filemap: fail to seek %s", filemap->name);
				return VSFERR_FAIL;
			}
			filemap->size = FTELL(filemap->file);
		}
		if (filemap->file != NULL)
		{
			file->size = filemap->size;
		}
	}
	return VSFERR_NONE;
}

vsf_err_t fakeiso_filemap_fini(struct fakeiso_file_t *file)
{
	struct fakeiso_filemap_t *filemap = (struct fakeiso_filemap_t *)file->extra;
	
	if ((filemap != NULL) && (filemap->file != NULL))
	{
		FCLOSE(filemap->file);
		filemap->file = NULL;
	}
	return VSFERR_NONE;
}

vsf_err_t fakeiso_filemap_read(struct fakeiso_file_t *file,
			uint64_t addr, uint8_t *buff, uint32_t page_size)
{
	struct fakeiso_filemap_t *filemap = (struct fakeiso_filemap_t *)file->extra;
	
	if ((filemap != NULL) && (filemap->file != NULL) && (filemap->size > addr))
	{
		uint32_t real_size = min(page_size, (uint32_t)(filemap->size - addr));
		
		FSEEK(filemap->file, addr, SEEK_SET);
		if (FREAD(buff, 1, real_size, filemap->file) != real_size)
		{
			return VSFERR_FAIL;
		}
	}
	return VSFERR_NONE;
}
