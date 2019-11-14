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

#define FAKEFAT32_FILEATTR_LONGNAME			\
				(VSFILE_ATTR_READONLY | VSFILE_ATTR_HIDDEN |\
				VSFILE_ATTR_SYSTEM | VSFILE_ATTR_VOLUMID)

#define FAKEFAT32_NAMEATTR_NAMELOWERCASE	0x08
#define FAKEFAT32_NAMEATTR_EXTLOWERCASE		0x10

#define FAT32_FAT_FILEEND					0x0FFFFFFF
#define FAT32_FAT_START						0x0FFFFFF8
#define FAT32_FAT_INVALID					0xFFFFFFFF

#define FAT32_RES_SECTORS					GET_LE_U16(&fakefat32_mbr[0x0E])
#define FAT32_FAT_NUM						fakefat32_mbr[0x10]
#define FAT32_HIDDEN_SECTORS				GET_LE_U32(&fakefat32_mbr[0x1C])
#define FAT32_ROOT_CLUSTER					GET_LE_U32(&fakefat32_mbr[0x2C])
#define FAT32_FSINFO_SECTOR					fakefat32_mbr[0x30]
#define FAT32_BACKUP_SECTOR					GET_LE_U16(&fakefat32_mbr[0x32])

static uint8_t fakefat32_mbr[512] =
{
//00   01   02   03   04   05   06   07   08   09   0A   0B   0C   0D   0E   0F
0xEB,0x58,0x90,0x4D,0x53,0x44,0x4F,0x53,0x35,0x2E,0x30,0x00,0x00,0x00,0x10,0x00,
0x02,0x00,0x00,0x00,0x00,0xF8,0x00,0x00,0x3F,0x00,0xFF,0x00,0x40,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,
0x01,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x29,0x00,0x00,0x00,0x00,0x4E,0x4F,0x20,0x4E,0x41,0x4D,0x45,0x20,0x20,
0x20,0x20,0x46,0x41,0x54,0x33,0x32,0x20,0x20,0x20,0x33,0xC9,0x8E,0xD1,0xBC,0xF4,
0x7B,0x8E,0xC1,0x8E,0xD9,0xBD,0x00,0x7C,0x88,0x4E,0x02,0x8A,0x56,0x40,0xB4,0x08,
0xCD,0x13,0x73,0x05,0xB9,0xFF,0xFF,0x8A,0xF1,0x66,0x0F,0xB6,0xC6,0x40,0x66,0x0F,
0xB6,0xD1,0x80,0xE2,0x3F,0xF7,0xE2,0x86,0xCD,0xC0,0xED,0x06,0x41,0x66,0x0F,0xB7,
0xC9,0x66,0xF7,0xE1,0x66,0x89,0x46,0xF8,0x83,0x7E,0x16,0x00,0x75,0x38,0x83,0x7E,
0x2A,0x00,0x77,0x32,0x66,0x8B,0x46,0x1C,0x66,0x83,0xC0,0x0C,0xBB,0x00,0x80,0xB9,
0x01,0x00,0xE8,0x2B,0x00,0xE9,0x48,0x03,0xA0,0xFA,0x7D,0xB4,0x7D,0x8B,0xF0,0xAC,
0x84,0xC0,0x74,0x17,0x3C,0xFF,0x74,0x09,0xB4,0x0E,0xBB,0x07,0x00,0xCD,0x10,0xEB,
0xEE,0xA0,0xFB,0x7D,0xEB,0xE5,0xA0,0xF9,0x7D,0xEB,0xE0,0x98,0xCD,0x16,0xCD,0x19,
0x66,0x60,0x66,0x3B,0x46,0xF8,0x0F,0x82,0x4A,0x00,0x66,0x6A,0x00,0x66,0x50,0x06,
0x53,0x66,0x68,0x10,0x00,0x01,0x00,0x80,0x7E,0x02,0x00,0x0F,0x85,0x20,0x00,0xB4,
0x41,0xBB,0xAA,0x55,0x8A,0x56,0x40,0xCD,0x13,0x0F,0x82,0x1C,0x00,0x81,0xFB,0x55,
0xAA,0x0F,0x85,0x14,0x00,0xF6,0xC1,0x01,0x0F,0x84,0x0D,0x00,0xFE,0x46,0x02,0xB4,
0x42,0x8A,0x56,0x40,0x8B,0xF4,0xCD,0x13,0xB0,0xF9,0x66,0x58,0x66,0x58,0x66,0x58,
0x66,0x58,0xEB,0x2A,0x66,0x33,0xD2,0x66,0x0F,0xB7,0x4E,0x18,0x66,0xF7,0xF1,0xFE,
0xC2,0x8A,0xCA,0x66,0x8B,0xD0,0x66,0xC1,0xEA,0x10,0xF7,0x76,0x1A,0x86,0xD6,0x8A,
0x56,0x40,0x8A,0xE8,0xC0,0xE4,0x06,0x0A,0xCC,0xB8,0x01,0x02,0xCD,0x13,0x66,0x61,
0x0F,0x82,0x54,0xFF,0x81,0xC3,0x00,0x02,0x66,0x40,0x49,0x0F,0x85,0x71,0xFF,0xC3,
0x4E,0x54,0x4C,0x44,0x52,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0D,0x0A,0x52,0x65,
0x6D,0x6F,0x76,0x65,0x20,0x64,0x69,0x73,0x6B,0x73,0x20,0x6F,0x72,0x20,0x6F,0x74,
0x68,0x65,0x72,0x20,0x6D,0x65,0x64,0x69,0x61,0x2E,0xFF,0x0D,0x0A,0x44,0x69,0x73,
0x6B,0x20,0x65,0x72,0x72,0x6F,0x72,0xFF,0x0D,0x0A,0x50,0x72,0x65,0x73,0x73,0x20,
0x61,0x6E,0x79,0x20,0x6B,0x65,0x79,0x20,0x74,0x6F,0x20,0x72,0x65,0x73,0x74,0x61,
0x72,0x74,0x0D,0x0A,0x00,0x00,0x00,0x00,0x00,0xAC,0xCB,0xD8,0x00,0x00,0x55,0xAA
};

static vsf_err_t fakefat32_dir_read(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
			struct fakefat32_file_t *file, uint64_t addr, uint8_t *buff,
			uint32_t page_size);
static vsf_err_t fakefat32_dir_write(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
			struct fakefat32_file_t *file, uint64_t addr, uint8_t *buff,
			uint32_t page_size);

// helper functions
static char find_first_alphabet(const char *str)
{
	if (NULL == str)
	{
		return 0;
	}

	while ((*str != '\0') && !isalpha(*str))
	{
		str++;
	}
	return *str;
}

static char* strncpy_fill(char *dst, const char *src, char fill, uint32_t n)
{
	if (n != 0)
	{
		char *d = dst;
		const char *s = src;

		if (NULL == s)
		{
			memset(dst, fill, n);
			return (dst);
		}

		do
		{
			if ((*d++ = *s++) == 0)
			{
				d--;
				n++;
				while (--n != 0)
				{
					*d++ = fill;
				}
				break;
			}
		} while (--n != 0);
	}
	return (dst);
}

static char* memncpy_toupper(char *dst, const char *src, uint32_t n)
{
	if (n != 0)
	{
		char *d = dst;
		const char *s = src;

		if (NULL == s)
		{
			return (dst);
		}

		do
		{
			if ((*d++ = (char)toupper(*s++)) == 0)
			{
				break;
			}
		} while (--n != 0);
	}
	return (dst);
}

static uint32_t fakefat32_calc_fat_sectors(struct fakefat32_param_t *param)
{
	// simple but safe
	return ((param->sector_number / param->sectors_per_cluster) + 1 +\
					FAT32_ROOT_CLUSTER) * 4 / param->sector_size + 1;
}

static struct fakefat32_file_t* fakefat32_get_file_by_cluster(
			struct fakefat32_param_t *param, struct fakefat32_file_t *cur_file,
			uint32_t cluster)
{
	struct vsfile_t *rawfile = &cur_file->memfile.file;
	struct fakefat32_file_t *result = NULL;
	uint32_t cluster_size = param->sector_size * param->sectors_per_cluster;
	uint32_t cluster_start, cluster_end, clusters;

	clusters = ((uint64_t)rawfile->size + cluster_size - 1) / cluster_size;
	cluster_start = cur_file->first_cluster;
	cluster_end = cluster_start + clusters;
	if ((cluster >= cluster_start) && (cluster < cluster_end))
	{
		return cur_file;
	}

	if ((cur_file->memfile.d.child != NULL) &&
		(rawfile->attr & VSFILE_ATTR_DIRECTORY))
	{
		result = fakefat32_get_file_by_cluster(param,
				(struct fakefat32_file_t *)cur_file->memfile.d.child, cluster);
		if (result != NULL)
		{
			return result;
		}
	}
	cur_file++;
	rawfile = &cur_file->memfile.file;
	if (rawfile->name != NULL)
	{
		result = fakefat32_get_file_by_cluster(param, cur_file, cluster);
		if (result != NULL)
		{
			return result;
		}
	}
	return NULL;
}

static bool fakefat32_file_is_longname(struct fakefat32_file_t *file)
{
	return vsffat_is_LFN(file->memfile.file.name);
}

static uint32_t fakefat32_calc_longname_len(struct fakefat32_file_t *file)
{
	struct vsfile_t *rawfile = &file->memfile.file;
	return rawfile->name ? strlen(rawfile->name) : 0;
}

static uint32_t fakefat32_calc_dir_clusters(struct fakefat32_param_t *param,
							struct fakefat32_file_t *file)
{
	struct vsfile_t *rawfile;
	uint32_t cluster_size = param->sector_size * param->sectors_per_cluster;
	uint32_t size = 0;

	if (NULL == file)
	{
		return 0;
	}
	rawfile = &file->memfile.file;
	if (!(rawfile->attr & VSFILE_ATTR_DIRECTORY) ||
		(NULL == file->memfile.d.child))
	{
		return 0;
	}

	file = (struct fakefat32_file_t *)file->memfile.d.child;
	rawfile = &file->memfile.file;
	while (rawfile->name != NULL)
	{
		if ((rawfile->attr != VSFILE_ATTR_VOLUMID) &&
			fakefat32_file_is_longname(file))
		{
			// one long name can contain 13 unicode max
			size += 0x20 * ((fakefat32_calc_longname_len(file) + 12) / 13);
		}
		size += 0x20;
		file++;
		rawfile = &file->memfile.file;
	}
	return (size + cluster_size - 1) / cluster_size;
}

static vsf_err_t fakefat32_init_recursion(struct fakefat32_param_t *param,
						struct fakefat32_file_t *file, uint32_t *cur_cluster)
{
	struct vsfile_t *rawfile;
	uint32_t cluster_size = param->sector_size * param->sectors_per_cluster;
	uint32_t clusters;

	if (NULL == file)
	{
		return VSFERR_FAIL;
	}
	file->memfile.file.op = (struct vsfile_fsop_t *)&fakefat32_fs_op;
	file->memfile.d.child_size = sizeof(struct fakefat32_file_t);
	rawfile = &file->memfile.file;

	if (rawfile->attr & VSFILE_ATTR_DIRECTORY)
	{
		if (!strcmp(rawfile->name, "."))
		{
			clusters = 0;
			file->first_cluster =
				((struct fakefat32_file_t *)rawfile->parent)->first_cluster;
		}
		else if (!strcmp(rawfile->name, ".."))
		{
			clusters = 0;
		}
		else
		{
			clusters = fakefat32_calc_dir_clusters(param, file);
			rawfile->size = clusters * cluster_size;
		}
		file->cb.read = fakefat32_dir_read;
		file->cb.write = fakefat32_dir_write;
	}
	else if (rawfile->attr == VSFILE_ATTR_VOLUMID)
	{
		clusters = 0;
	}
	else
	{
		clusters = ((uint64_t)rawfile->size + cluster_size - 1) / cluster_size;
	}

	if (clusters)
	{
		file->first_cluster = *cur_cluster;
		file->record.FstClusHI = (file->first_cluster >> 16) & 0xFFFF;
		file->record.FstClusLO = (file->first_cluster >>  0) & 0xFFFF;
		*cur_cluster += clusters;
	}

	if ((file->memfile.d.child != NULL) &&
		(rawfile->attr & VSFILE_ATTR_DIRECTORY))
	{
		struct fakefat32_file_t *parent = file;

		file = (struct fakefat32_file_t *)file->memfile.d.child;
		rawfile = &file->memfile.file;
		while (rawfile->name != NULL)
		{
			rawfile->parent = (struct vsfile_t *)parent;
			if (fakefat32_init_recursion(param, file++, cur_cluster))
			{
				return VSFERR_FAIL;
			}
			rawfile = &file->memfile.file;
		}
	}
	return VSFERR_NONE;
}

static void fakefat32_get_shortname(struct fakefat32_file_t* file,
											char shortname[11])
{
	struct vsfile_t *rawfile = &file->memfile.file;
	char *ext = vsfile_getfileext(rawfile->name);
	uint32_t extlen = ext ? strlen(ext) : 0;

	memset(shortname, ' ', 11);
	if (!strcmp(rawfile->name, "."))
	{
		shortname[0] = '.';
		return;
	}
	else if (!strcmp(rawfile->name, ".."))
	{
		shortname[0] = shortname[1] = '.';
		return;
	}

	if (!fakefat32_file_is_longname(file))
	{
		uint32_t file_name_len = strlen(rawfile->name);
		if (ext)
		{
			file_name_len -= extlen + 1;
		}
		memncpy_toupper(shortname, rawfile->name, file_name_len);
	}
	else
	{
		uint32_t n = strlen(rawfile->name);
		n = min(n, 6);

		memncpy_toupper(shortname, rawfile->name, n);
		shortname[n] = '~';
		// TODO: fix file index here, now use 1
		// BUG here if multiple same short names under one directory
		shortname[n + 1] = '1';
	}
	if (ext)
	{
		extlen = min(extlen, 3);
		memncpy_toupper((char *)&shortname[8], ext, extlen);
	}
}

static vsf_err_t fakefat32_dir_read(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
			struct fakefat32_file_t *file, uint64_t addr, uint8_t *buff,
			uint32_t page_size)
{
	struct vsfile_t *rawfile = &file->memfile.file;
	struct fakefat32_file_t* file_dir = file;
	uint8_t longname_index_offset = 0;

	memset(buff, 0, page_size);
	if (!(rawfile->attr & VSFILE_ATTR_DIRECTORY))
	{
		return VSFERR_FAIL;
	}

	file = (struct fakefat32_file_t *)file->memfile.d.child;
	if (NULL == file)
	{
		return VSFERR_NONE;
	}

	while (addr)
	{
		uint32_t current_entry_size;

		// no file record here, return
		rawfile = &file->memfile.file;
		if (!rawfile->name)
		{
			return VSFERR_NONE;
		}

		if ((rawfile->attr != VSFILE_ATTR_VOLUMID) &&
			fakefat32_file_is_longname(file))
		{
			uint32_t longname_len = fakefat32_calc_longname_len(file);
			uint8_t longname_entry_num = (uint8_t)((longname_len + 12) / 13);
			current_entry_size = (1 + longname_entry_num) * 0x20;
		} else
		{
			current_entry_size = 0x20;
		}
		if (addr < current_entry_size)
		{
			longname_index_offset = (uint8_t)(addr / 0x20);
			break;
		}
		addr -= current_entry_size;
		file++;
	}

	while (page_size && (file != NULL))
	{
		char short_filename[11];
		bool longname = false;

		rawfile = &file->memfile.file;
		if (!rawfile->name)
		{
			break;
		}
		if (VSFILE_ATTR_VOLUMID == rawfile->attr)
		{
			// ONLY file->name is valid for volume_id
			// volume_id is 11 characters max
			strncpy_fill((char *)buff, rawfile->name, ' ', 11);
			buff[11] = rawfile->attr;
			memset(&buff[12], 0, 20);
			goto fakefat32_dir_read_next;
		}

		// generate short 8.3 filename
		fakefat32_get_shortname(file, short_filename);

		if (fakefat32_file_is_longname(file))
		{
			// process entries for long file name
			uint32_t longname_len = fakefat32_calc_longname_len(file);
			uint8_t longname_entry_num = (uint8_t)((longname_len + 12) / 13);
			uint8_t longname_index;
			uint32_t i, j;

			longname = true;

			longname_index = longname_entry_num;
			if (longname_index >= 0x40)
			{
				longname_index = 0x3F;
			}
			if (!longname_index_offset)
			{
				longname_index |= 0x40;
			}
			else
			{
				longname_index -= longname_index_offset;
				longname_index_offset = 0;
			}

			while (page_size && longname_index)
			{
				uint8_t checksum = 0;

				memset(buff, 0xFF, 0x20);
				buff[0x0B] = FAKEFAT32_FILEATTR_LONGNAME;
				buff[0x0C] = buff[0x1A] = buff[0x1B] = 0;

				for (i = 0; i < 11; i++)
				{
					checksum = ((checksum << 7) | (checksum >> 1)) +
									short_filename[i];
				}
				buff[0x0D] = checksum;

				buff[0] = longname_index;
				j = 1;
				i = 13 * ((longname_index & ~0x40) - 1);
				while ((i < longname_len) && (j < 32))
				{
					buff[j] = rawfile->name[i];
					buff[j + 1] = 0;
					j += 2;
					if (0x0B == j)
					{
						j = 0x0E;
					} else if (0x1A == j)
					{
						j = 0x1C;
					}
					i++;
				}

				if (j < 32)
				{
					buff[j] = buff[j + 1] = 0;
				}

				longname_index &= ~0x40;
				longname_index--;
				buff += 0x20;
				page_size -= 0x20;
			}

			if (!page_size)
			{
				break;
			}
		}

		// 8.3 file name
		memcpy(&buff[0], short_filename, 11);

		// File Attribute
		buff[11] = rawfile->attr;

		// File Nt Attribute
		buff[12] = 0;
		if (!longname)
		{
			char *ext = vsfile_getfileext(rawfile->name);
			if (islower(find_first_alphabet(rawfile->name)))
			{
				buff[12] |= FAKEFAT32_NAMEATTR_NAMELOWERCASE;
			}
			if (islower(find_first_alphabet(ext)))
			{
				buff[12] |= FAKEFAT32_NAMEATTR_EXTLOWERCASE;
			}
		}

		// File Size
		if (rawfile->attr & VSFILE_ATTR_DIRECTORY)
		{
			// File Size for directory SHOULD be 0
			SET_LE_U32(&buff[28], 0);
		}
		else
		{
			SET_LE_U32(&buff[28], rawfile->size);
		}

		// fix for current directory
		if (!strcmp(rawfile->name, "."))
		{
			file->record.FstClusHI = (file_dir->first_cluster >> 16) & 0xFFFF;
			file->record.FstClusLO = (file_dir->first_cluster >>  0) & 0xFFFF;
		}
		// fix for parent directory
		else if (!strcmp(rawfile->name, "..") &&
				(file_dir->memfile.file.parent != NULL) &&
				// if parent->parent is NULL, parent is root dir
				(file_dir->memfile.file.parent->parent != NULL))
		{
			file->record.FstClusHI = (((struct fakefat32_file_t *)\
				(file_dir->memfile.file.parent))->first_cluster >> 16) & 0xFFFF;
			file->record.FstClusLO = (((struct fakefat32_file_t *)\
				(file_dir->memfile.file.parent))->first_cluster >>  0) & 0xFFFF;
		}

		// record
		memcpy(&buff[13], &file->record, sizeof(file->record));

	fakefat32_dir_read_next:
		buff += 0x20;
		page_size -= 0x20;
		file++;
	}
	return VSFERR_NONE;
}

static vsf_err_t fakefat32_dir_write(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
			struct fakefat32_file_t *file, uint64_t addr, uint8_t *buff,
			uint32_t page_size)
{
	struct fakefat32_file_t *file_temp, *file_match;
	uint8_t *entry;
	uint32_t want_size;
	uint16_t want_first_cluster;
	struct vsffat_dentry_parser_t dparser;

	REFERENCE_PARAMETER(addr);

	file = (struct fakefat32_file_t *)file->memfile.d.child;
	dparser.entry = buff;
	dparser.entry_num = page_size >> 5;
	dparser.lfn = 0;
	dparser.filename = (char *)buff;
	while (dparser.entry_num)
	{
		if (vsffat_parse_dentry_fat(&dparser))
		{
			entry = dparser.entry;
			file_temp = file;
			file_match = NULL;
			while (file_temp->memfile.file.name != NULL)
			{
				if (!strcmp(file_temp->memfile.file.name, dparser.filename))
				{
					file_match = file_temp;
					break;
				}
				file_temp++;
			}
			// seems host add some file, just ignore it
			if (NULL == file_match)
			{
				goto fakefat32_dir_write_next;
			}

			want_size = GET_LE_U32(&entry[28]);
			want_first_cluster =
						GET_LE_U16(&entry[26]) + (GET_LE_U16(&entry[20]) << 16);

			// host can change the size and first_cluster
			// ONLY one limitation:
			// 		host MUST guarantee that the space is continuous
			if (!(file_temp->memfile.file.attr & VSFILE_ATTR_DIRECTORY) &&
				(file_temp->memfile.file.size != want_size))
			{
/*				if ((file_match->callback.change_size != NULL) &&
					file_match->callback.change_size(file_match, want_size))
				{
					return VSFERR_FAIL;
				}
*/				file_temp->memfile.file.size = want_size;
			}
			file_match->first_cluster = want_first_cluster;
			memcpy(&file_match->record, &entry[13], sizeof(file_match->record));

fakefat32_dir_write_next:
			dparser.entry += 32;
			dparser.filename = (char *)buff;
		}
		else
		{
			break;
		}
	}
	return VSFERR_NONE;
}

static vsf_err_t fakefat32_init(struct fakefat32_param_t *param)
{
	if (!param->root->memfile.file.op)
	{
		uint32_t cur_cluster = FAT32_ROOT_CLUSTER;

		param->root[0].memfile.file.attr = VSFILE_ATTR_DIRECTORY;
		param->root[0].memfile.file.parent = NULL;
		param->root[1].memfile.file.name = NULL;

		// set directory size and first_cluster of every file
		return fakefat32_init_recursion(param, param->root, &cur_cluster);
	}
	return VSFERR_NONE;
}

// fs
static vsf_err_t fakefat32_fs_mount(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
								struct vsfile_t *dir)
{
	return fakefat32_init((struct fakefat32_param_t *)pt->user_data);
}

static vsf_err_t fakefat32_getchild(struct vsfsm_pt_t *pt,
					vsfsm_evt_t evt, struct vsfile_t *dir, char *name,
					uint32_t idx, struct vsfile_t **file)
{
	if (NULL == dir)
	{
		struct fakefat32_param_t *param =
									(struct fakefat32_param_t *)pt->user_data;

		pt->user_data = &param->root[0].memfile;
	}
	return vsfile_memfs_op.d_op.getchild(pt, evt, dir, name, idx, file);
}

static uint32_t fakefat32_mal_blocksize(struct vsfmal_t *mal, uint64_t addr,
										uint32_t size, enum vsfmal_op_t op)
{
	return mal->cap.block_size;
}

static vsf_err_t fakefat32_mal_init(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfmal_t *mal = (struct vsfmal_t*)pt->user_data;
	struct fakefat32_param_t *param = (struct fakefat32_param_t *)mal->param;

	mal->cap.block_size = param->sector_size;
	mal->cap.block_num = param->sector_number + FAT32_HIDDEN_SECTORS;

	return fakefat32_init(param);
}

static vsf_err_t fakefat32_mal_fini(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	return VSFERR_NONE;
}

static vsf_err_t fakefat32_mal_read(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
									uint64_t addr, uint8_t *buff, uint32_t size)
{
	struct vsfmal_t *mal = (struct vsfmal_t *)pt->user_data;
	uint32_t page_size = (uint32_t)mal->cap.block_size;
	uint32_t block_addr = (uint32_t)addr / page_size;
	struct fakefat32_param_t *param = (struct fakefat32_param_t *)mal->param;
	uint32_t fat_sectors = fakefat32_calc_fat_sectors(param);
	uint32_t cluster_size = param->sectors_per_cluster * param->sector_size;
	uint32_t root_cluster = FAT32_ROOT_CLUSTER;

	if (block_addr < (FAT32_HIDDEN_SECTORS + FAT32_RES_SECTORS +
						FAT32_FAT_NUM * fat_sectors))
	{
		memset(buff, 0, page_size);
	}

	// first sector and first backup copy of boot sector
	if ((FAT32_HIDDEN_SECTORS > 0) && (0 == block_addr))
	{
		// DPT
		// Only PTE1 is valid

		// Disk ID
		SET_LE_U32(&buff[0x1B8], param->disk_id);

		// PTE1
		// Status: Active
		buff[0x1BE] = 0x80;
		// CHS address of first sector
		buff[0x1BF] = 0x01;
		buff[0x1C0] = 0x01;
		buff[0x1C1] = 0x00;
		// Partition type: FAT32
		buff[0x1C2] = 0x0B;
		// CHS address of last sector
		buff[0x1C3] = 0x01;
		buff[0x1C4] = 0xFF;
		buff[0x1C5] = 0xFF;
		// LBA of first sector in the partition
		SET_LE_U32(&buff[0x1C6], FAT32_HIDDEN_SECTORS);
		// Number of sectors in partition
		SET_LE_U32(&buff[0x1CA], param->sector_number);

		// Boot signature
		SET_LE_U16(&buff[510], 0xAA55);
	}
	else if ((FAT32_HIDDEN_SECTORS > 0) && (block_addr < FAT32_HIDDEN_SECTORS))
	{
		// other data in hidden sectors, all 0
	}
	else if ((FAT32_HIDDEN_SECTORS == block_addr) ||
		((FAT32_HIDDEN_SECTORS + FAT32_BACKUP_SECTOR) == block_addr))
	{
		// MBR
		memcpy(buff, fakefat32_mbr, sizeof(fakefat32_mbr));

		// Sector size in bytes
		SET_LE_U16(&buff[0x0B], param->sector_size);
		// Number of sectors per cluster
		buff[0x0D] = param->sectors_per_cluster;
		// Total number of sectors
		SET_LE_U32(&buff[0x20], param->sector_number);
		// Number of sectors of one FAT
		SET_LE_U32(&buff[0x24], fat_sectors);
		// Volume ID
		SET_LE_U32(&buff[0x43], param->volume_id);
	}
	// FSInfo sector or backup copy of FSInfo sector
	else if (((FAT32_HIDDEN_SECTORS + FAT32_FSINFO_SECTOR) == block_addr) ||
		((FAT32_HIDDEN_SECTORS + FAT32_BACKUP_SECTOR +
						FAT32_FSINFO_SECTOR) == block_addr))
	{
		// FSInfo
		// refer to FAT32 FSInfo sector Structure in FAT32 white paper

		// The lead signature is used to validate that this is in fact an
		// FSInfo sector.
		SET_LE_U32(&buff[0], 0x41615252);
		// Another signature that is more localized in the sector to the
		// location of the fields that are used.
		SET_LE_U32(&buff[484], 0x61417272);
		// Contains the last known free cluster count on the volume.
		// If the value is 0xFFFFFFFF, then the free count is unknown and
		// must be computed.
		// Any other value can be used, but is not necessarily correct.
		// Is should be range checked at least to make sure it is <= volume
		// cluser count.
		SET_LE_U32(&buff[488], 0xFFFFFFFF);
		// This trail signature is used to validate that this is in fact
		// an FSInfo sector.
		// Note that the high 2 bytes of this value match the signature
		// bytes used at the same offsets in sector 0.
		SET_LE_U16(&buff[510], 0xAA55);
	}
	else if (((FAT32_HIDDEN_SECTORS + FAT32_FSINFO_SECTOR + 1) == block_addr) ||
		((FAT32_HIDDEN_SECTORS + FAT32_BACKUP_SECTOR +
						FAT32_FSINFO_SECTOR + 1) == block_addr))
	{
		// empty sector, with only Boot sector signature
		SET_BE_U16(&buff[510], 0xAA55);
	}
	else if (block_addr < (FAT32_HIDDEN_SECTORS + FAT32_RES_SECTORS))
	{
		// other reserved sectors, all data is 0
	}
	// FAT starts after reserved sectors
	else if (block_addr < (FAT32_FAT_NUM * fat_sectors +
						FAT32_RES_SECTORS + FAT32_HIDDEN_SECTORS))
	{
		// FAT
		uint32_t fat_sector = (block_addr - FAT32_HIDDEN_SECTORS -
								FAT32_RES_SECTORS) % fat_sectors;
		uint32_t max_cluster = page_size / 4;
		uint32_t remain_size = page_size;
		uint32_t cluster_index = fat_sector * max_cluster;
		uint32_t *buff32 = (uint32_t *)buff;

		while (remain_size && (cluster_index < root_cluster))
		{
			*buff32++ =
				(0 == cluster_index) ? FAT32_FAT_START : FAT32_FAT_INVALID;
			remain_size -= 4;
			cluster_index++;
		}

		while (remain_size)
		{
			struct fakefat32_file_t *file = NULL;

			file = fakefat32_get_file_by_cluster(param, param->root,
													cluster_index);

			if (NULL == file)
			{
				// file not found
				*buff32++ = 0;
				remain_size -= 4;
				cluster_index++;
			}
			else
			{
				// file found
				struct vsfile_t *rawfile = &file->memfile.file;
				uint32_t cluster_offset = cluster_index - file->first_cluster;
				uint32_t file_clusters = ((uint64_t)rawfile->size +
											cluster_size - 1) / cluster_size;

				while (remain_size && (cluster_offset < file_clusters))
				{
					if (cluster_offset == (file_clusters - 1))
					{
						// last cluster
						*buff32++ = FAT32_FAT_FILEEND;
					}
					else
					{
						*buff32++ = cluster_index + 1;
					}

					remain_size -= 4;
					cluster_offset++;
					cluster_index++;
				}
			}
		}
	}
	else
	{
		// Clusters
		uint32_t sectors_to_root = block_addr - FAT32_HIDDEN_SECTORS -
							FAT32_RES_SECTORS - FAT32_FAT_NUM * fat_sectors;
		uint32_t cluster_index = root_cluster +
							sectors_to_root / param->sectors_per_cluster;
		struct fakefat32_file_t *file = NULL;

		file = fakefat32_get_file_by_cluster(param, param->root, cluster_index);
		if ((file != NULL) &&
			!(file->memfile.file.attr & VSFILE_ATTR_WRITEONLY))
		{
			struct vsfile_t *rawfile = &file->memfile.file;
			uint32_t addr_offset = param->sector_size *
					(sectors_to_root - param->sectors_per_cluster *
									(file->first_cluster - root_cluster));
			uint32_t rsize;

			if ((file->memfile.f.buff != NULL) &&
				!(rawfile->attr & VSFILE_ATTR_DIRECTORY))
			{
				return vsfile_memfs_op.f_op.read(pt, evt,
						(struct vsfile_t *)file, addr_offset, page_size, buff,
						&rsize);
			}
			else if (file->cb.read != NULL)
			{
				return file->cb.read(pt, evt, file, addr_offset, buff,
										page_size);
			}
		}
	}
	return VSFERR_NONE;
}

static vsf_err_t fakefat32_mal_write(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
									uint64_t addr, uint8_t *buff, uint32_t size)
{
	struct vsfmal_t *mal = (struct vsfmal_t *)pt->user_data;
	uint32_t page_size = (uint32_t)mal->cap.block_size;
	uint32_t block_addr = (uint32_t)addr / page_size;
	struct fakefat32_param_t *param = (struct fakefat32_param_t *)mal->param;
	uint32_t fat_sectors = fakefat32_calc_fat_sectors(param);
	uint32_t sectors_to_root = block_addr - FAT32_HIDDEN_SECTORS -
								FAT32_RES_SECTORS - FAT32_FAT_NUM * fat_sectors;
	uint32_t root_cluster = FAT32_ROOT_CLUSTER;

	uint32_t cluster_index = FAT32_ROOT_CLUSTER + (block_addr -
					FAT32_HIDDEN_SECTORS - FAT32_RES_SECTORS -
					FAT32_FAT_NUM * fat_sectors) / param->sectors_per_cluster;
	struct fakefat32_file_t *file = NULL;

	// Hidden sectors, Reserved sectors, FAT can not be written
	if (block_addr < (FAT32_HIDDEN_SECTORS + FAT32_RES_SECTORS +
						FAT32_FAT_NUM * fat_sectors))
	{
		// first sector and first backup copy of boot sector
		if ((FAT32_HIDDEN_SECTORS == block_addr) ||
			((FAT32_HIDDEN_SECTORS + FAT32_BACKUP_SECTOR) == block_addr))
		{
			memcpy(fakefat32_mbr, buff, sizeof(fakefat32_mbr));
		}
		return VSFERR_NONE;
	}

	file = fakefat32_get_file_by_cluster(param, param->root, cluster_index);
	if ((file != NULL) && !(file->memfile.file.attr & VSFILE_ATTR_READONLY))
	{
		struct vsfile_t *rawfile = &file->memfile.file;
		uint32_t addr_offset = param->sector_size *
					(sectors_to_root - param->sectors_per_cluster *
									(file->first_cluster - root_cluster));
		uint32_t wsize;

		if ((file->memfile.f.buff != NULL) &&
			!(rawfile->attr & VSFILE_ATTR_DIRECTORY))
		{
			return vsfile_memfs_op.f_op.write(pt, evt, (struct vsfile_t *)file,
										addr_offset, page_size, buff, &wsize);
		}
		else if (file->cb.write != NULL)
		{
			return file->cb.write(pt, evt, file, addr_offset, buff, page_size);
		}
	}
	return VSFERR_NONE;
}

const struct vsfile_fsop_t fakefat32_fs_op =
{
	// mount / unmount
	.mount = fakefat32_fs_mount,
	.unmount = vsfile_dummy_unmount,
	// f_op
	.f_op.close = vsfile_dummy_close,
	.f_op.read = vsfile_memfs_read,
	.f_op.write = vsfile_memfs_write,
	// d_op
	.d_op.getchild = fakefat32_getchild,
};

const struct vsfmal_drv_t fakefat32_mal_drv =
{
	.block_size = fakefat32_mal_blocksize,
	.init = fakefat32_mal_init,
	.fini = fakefat32_mal_fini,
	.read = fakefat32_mal_read,
	.write = fakefat32_mal_write,
};

