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

#ifndef __VSFILE_H_INCLUDED__
#define __VSFILE_H_INCLUDED__

enum vsfile_attr_t
{
	VSFILE_ATTR_READONLY		= 1 << 0,
	VSFILE_ATTR_HIDDEN			= 1 << 1,
	VSFILE_ATTR_SYSTEM			= 1 << 2,
	VSFILE_ATTR_VOLUMID			= 1 << 3,
	VSFILE_ATTR_DIRECTORY		= 1 << 4,
	VSFILE_ATTR_ARCHIVE			= 1 << 5,
	VSFILE_ATTR_WRITEONLY		= 1 << 6,
};

struct vsfile_t;
struct vsfile_fop_t
{
	vsf_err_t (*close)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *file);
	vsf_err_t (*read)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *file, uint64_t offset,
					uint32_t size, uint8_t *buff, uint32_t *rsize);
	vsf_err_t (*write)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *file, uint64_t offset,
					uint32_t size, uint8_t *buff, uint32_t *wsize);
};
struct vsfile_dop_t
{
	// for getchild, if dir is NULL, means root, if name is NULL, use idx
	vsf_err_t (*getchild)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir, char *name, uint32_t idx,
					struct vsfile_t **file);
	vsf_err_t (*addfile)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir, char *name, enum vsfile_attr_t attr);
	vsf_err_t (*removefile)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir, char *name);
};
struct vsfile_fsop_t
{
	// dir in mount is the directory of the vfs under which fs is mounted
	vsf_err_t (*format)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
	vsf_err_t (*mount)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir);
	vsf_err_t (*unmount)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir);
	struct vsfile_fop_t f_op;
	struct vsfile_dop_t d_op;
};

struct vsfile_t
{
	char *name;
	uint64_t size;
	enum vsfile_attr_t attr;
	struct vsfile_fsop_t *op;
	struct vsfile_t *parent;
};

#define VSFILE_MOUNT(p, e, op, d)			vsfile_mount((p), (e), (op), (struct vsfile_t *)(d))
#define VSFILE_UNMOUNT(p, e, d)				vsfile_unmount((p), (e), (struct vsfile_t *)(d))
#define VSFILE_GETFILE(p, e, d, n, f)		vsfile_getfile((p), (e), (struct vsfile_t *)(d), (n), (struct vsfile_t **)(f))
#define VSFILE_FINDFIRST(p, e, d, f)		vsfile_findfirst((p), (e), (struct vsfile_t *)(d), (struct vsfile_t **)(f))
#define VSFILE_FINDNEXT(p, e, d, f)			vsfile_findnext((p), (e), (struct vsfile_t *)(d), (struct vsfile_t **)(f))
#define VSFILE_FINDEND(p, e, d)				vsfile_findend((p), (e), (struct vsfile_t *)(d))
#define VSFILE_CLOSE(p, e, f)				vsfile_close((p), (e), (struct vsfile_t *)(f))
#define VSFILE_READ(p, e, f, o, s, b, r)	vsfile_read((p), (e), (struct vsfile_t *)(f), (o), (s), (b), (r))
#define VSFILE_WRITE(p, e, f, o, s, b, w)	vsfile_write((p), (e), (struct vsfile_t *)(f), (o), (s), (b), (w))
#define VSFILE_ADDFILE(p, e, d, n, a)		vsfile_addfile((p), (e), (d), (n), (a))
#define VSFILE_REMOVEFILE(p, e, d, n)		vsfile_removefile((p), (e), (d), (n))

// dummy op
#define vsfile_dummy_mount		vsfile_dummy_file
#define vsfile_dummy_unmount	vsfile_dummy_file
#define vsfile_dummy_open		vsfile_dummy_file
#define vsfile_dummy_close		vsfile_dummy_file
#define vsfile_dummy_read		vsfile_dummy_rw
#define vsfile_dummy_write		vsfile_dummy_rw

// memfile:
// use pointer to vsfile_memfile_t as user_data of pt for mount
// ptr of file points to a buffer
// ptr of directory point to an array of children files
struct vsfile_memfile_t
{
	struct vsfile_t file;
	union
	{
		struct
		{
			uint8_t *buff;
		} f;		// msmfs file
		struct
		{
			struct vsfile_memfile_t *child;
			uint32_t child_size;
		} d;		// memfs directory
	};
};

// vfs
struct vsfile_vfsfile_t
{
	struct vsfile_t file;
	bool mounted;
	union
	{
		struct
		{
			struct vsfile_vfsfile_t *child;
		} dir;
		struct
		{
			struct vsfile_fsop_t *op;
			void *param;
		} subfs;
		void *ptr;
	};
	struct vsfile_vfsfile_t *next;
};

// init
struct vsfile_memop_t
{
	struct vsfile_vfsfile_t* (*alloc_vfs)(void);
	void (*free_vfs)(struct vsfile_vfsfile_t *vfsfile);
};

struct vsfile_local_t
{
	struct
	{
		struct vsfile_t *cur_file;
		char *cur_name;
		uint32_t cur_idx;
		struct vsfsm_crit_t crit;
		struct vsfsm_pt_t file_pt;
	} srch;

	struct vsfile_memop_t *memop;
	struct vsfile_vfsfile_t rootfs;
};

#ifndef VSFCFG_EXCLUDE_FILE
vsf_err_t vsfile_init(struct vsfile_memop_t *memop);

vsf_err_t vsfile_mount(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
						struct vsfile_fsop_t *op, struct vsfile_t *dir);
vsf_err_t vsfile_unmount(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
						struct vsfile_t *dir);

vsf_err_t vsfile_getfile(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir, char *name, struct vsfile_t **file);
vsf_err_t vsfile_findfirst(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir, struct vsfile_t **file);
vsf_err_t vsfile_findnext(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir, struct vsfile_t **file);
vsf_err_t vsfile_findend(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir);

vsf_err_t vsfile_close(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *file);
vsf_err_t vsfile_read(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *file, uint64_t offset,
					uint32_t size, uint8_t *buff, uint32_t *rsize);
vsf_err_t vsfile_write(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *file, uint64_t offset,
					uint32_t size, uint8_t *buff, uint32_t *wsize);

vsf_err_t vsfile_addfile(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir, char *name, enum vsfile_attr_t attr);
vsf_err_t vsfile_removefile(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir, char *name);

vsf_err_t vsfile_dummy_file(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir);
vsf_err_t vsfile_dummy_rw(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir, uint64_t offset,
					uint32_t size, uint8_t *buff, uint32_t *rsize);

// helper
char* vsfile_getfileext(char* name);
bool vsfile_is_div(char ch);
bool vsfile_match(char *path, char *filename);

// vfs
extern const struct vsfile_fsop_t vsfile_vfs_op;

// normal memfs
extern const struct vsfile_fsop_t vsfile_memfs_op;
// for derived memfs classes
vsf_err_t vsfile_memfs_getchild(struct vsfsm_pt_t *pt,
					vsfsm_evt_t evt, struct vsfile_t *dir, char *name,
					uint32_t idx, struct vsfile_t **file);
vsf_err_t vsfile_memfs_read(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *file, uint64_t offset,
					uint32_t size, uint8_t *buff, uint32_t *rsize);
vsf_err_t vsfile_memfs_write(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *file, uint64_t offset,
					uint32_t size, uint8_t *buff, uint32_t *wsize);
#endif

#endif		// __VSFILE_H_INCLUDED__
