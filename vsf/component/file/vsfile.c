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

#define VSFILE
#define VSFILE_EVT_CRIT					VSFSM_EVT_USER

static struct vsfile_local_t vsfile;

// helper
char* vsfile_getfileext(char *fname)
{
	char *ext, *tmp;

	ext = tmp = fname;
	while (1)
	{
		tmp = strchr(ext, '.');
		if (tmp != NULL)
			ext = tmp + 1;
		else
			break;
	}
	return ext;
}

bool vsfile_is_div(char ch)
{
	return ('\\' == ch) || ('/' == ch);
}

bool vsfile_match(char *path, char *filename)
{
	if (strstr(path, filename) == path)
	{
		char ch = path[strlen(filename)];
		if (('\0' == ch) || vsfile_is_div(ch))
		{
			return true;
		}
	}
	return false;
}

vsf_err_t vsfile_init(struct vsfile_memop_t *memop)
{
	memset(&vsfile, 0, sizeof(vsfile));
	vsfile.memop = memop;
	vsfile.rootfs.file.name = "rootfs";
	vsfile.rootfs.file.attr = VSFILE_ATTR_DIRECTORY;
	vsfile.rootfs.file.op = (struct vsfile_fsop_t *)&vsfile_vfs_op;
	return vsfsm_crit_init(&vsfile.srch.crit, VSFILE_EVT_CRIT);
}

vsf_err_t vsfile_close(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *file)
{
	return file->op->f_op.close(pt, evt, file);
}

vsf_err_t vsfile_getfile(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir, char *name, struct vsfile_t **file)
{
	vsf_err_t err = VSFERR_NONE;

	if (NULL == dir)
	{
		dir = (struct vsfile_t *)&vsfile.rootfs;
	}

	vsfsm_pt_begin(pt);

	if (vsfsm_crit_enter(&vsfile.srch.crit, pt->sm))
	{
		vsfsm_pt_wfe(pt, VSFILE_EVT_CRIT);
	}

	vsfile.srch.cur_name = name;
	vsfile.srch.cur_file = dir;
	vsfile.srch.file_pt.sm = pt->sm;
	while (*vsfile.srch.cur_name != '\0')
	{
		if (vsfile_is_div(*vsfile.srch.cur_name))
		{
			if (!(vsfile.srch.cur_file->attr & VSFILE_ATTR_DIRECTORY))
			{
				// want to find something under a file not a directory
				return VSFERR_FAIL;
			}

			vsfile.srch.cur_name++;
			if ('\0' == *vsfile.srch.cur_name)
			{
				break;
			}
		}

		vsfile.srch.file_pt.state = 0;
		vsfsm_pt_entry(pt);
		err = vsfile.srch.cur_file->op->d_op.getchild(
					&vsfile.srch.file_pt, evt, vsfile.srch.cur_file,
					vsfile.srch.cur_name, 0, file);
		if (err > 0) return err; else if (err < 0)
		{
			goto end;
		}

		if (vsfile.srch.cur_file != dir)
		{
			vsfile.srch.file_pt.state = 0;
			vsfsm_pt_entry(pt);
			err = vsfile_close(&vsfile.srch.file_pt, evt, vsfile.srch.cur_file);
			if (err > 0) return err; else if (err < 0)
			{
				goto end;
			}
		}
		vsfile.srch.cur_name += strlen((*file)->name);
		vsfile.srch.cur_file = *file;
	}
	*file = vsfile.srch.cur_file;
end:
	vsfsm_crit_leave(&vsfile.srch.crit);

	vsfsm_pt_end(pt);
	return err;
}

vsf_err_t vsfile_findfirst(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir, struct vsfile_t **file)
{
	vsf_err_t err = VSFERR_NONE;

	vsfsm_pt_begin(pt);

	if (vsfsm_crit_enter(&vsfile.srch.crit, pt->sm))
	{
		vsfsm_pt_wfe(pt, VSFILE_EVT_CRIT);
	}

	vsfile.srch.cur_idx = 0;
	vsfile.srch.file_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = dir->op->d_op.getchild(&vsfile.srch.file_pt, evt, dir, NULL,
					vsfile.srch.cur_idx, file);
	if (err != 0) return err;

	vsfile.srch.cur_idx++;
	vsfsm_pt_end(pt);
	return err;
}

vsf_err_t vsfile_findnext(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir, struct vsfile_t **file)
{
	vsf_err_t err = VSFERR_NONE;

	vsfsm_pt_begin(pt);

	vsfile.srch.file_pt.state = 0;
	vsfsm_pt_entry(pt);
	err = dir->op->d_op.getchild(&vsfile.srch.file_pt, evt, dir, NULL,
					vsfile.srch.cur_idx, file);
	if (err != 0) return err;

	vsfile.srch.cur_idx++;
	vsfsm_pt_end(pt);
	return err;
}

vsf_err_t vsfile_findend(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir)
{
	return vsfsm_crit_leave(&vsfile.srch.crit);
}

vsf_err_t vsfile_mount(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
						struct vsfile_fsop_t *op, struct vsfile_t *dir)
{
	// can only mount under vfs directory
	if (dir->op != &vsfile_vfs_op)
	{
		return VSFERR_FAIL;
	}
	else
	{
		struct vsfile_vfsfile_t *vfsfile = (struct vsfile_vfsfile_t *)dir;
		vfsfile->subfs.op = op;
		return dir->op->mount(pt, evt, dir);
	}
}

vsf_err_t vsfile_unmount(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
						struct vsfile_t *dir)
{
	return dir->op->unmount(pt, evt, dir);
}

vsf_err_t vsfile_read(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *file, uint64_t offset,
					uint32_t size, uint8_t *buff, uint32_t *rsize)
{
	if (file->attr & VSFILE_ATTR_WRITEONLY)
	{
		return VSFERR_NOT_ACCESSABLE;
	}

	return file->op->f_op.read(pt, evt, file, offset, size, buff, rsize);
}

vsf_err_t vsfile_write(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *file, uint64_t offset,
					uint32_t size, uint8_t *buff, uint32_t *wsize)
{
	if (file->attr & VSFILE_ATTR_READONLY)
	{
		return VSFERR_NOT_ACCESSABLE;
	}

	return file->op->f_op.write(pt, evt, file, offset, size, buff, wsize);
}

vsf_err_t vsfile_addfile(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir, char *name, enum vsfile_attr_t attr)
{
	if (NULL == dir)
	{
		dir = (struct vsfile_t *)&vsfile.rootfs;
	}
	if (!(dir->attr & VSFILE_ATTR_DIRECTORY))
	{
		return VSFERR_INVALID_PARAMETER;
	}

	return dir->op->d_op.addfile(pt, evt, dir, name, attr);
}

vsf_err_t vsfile_removefile(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir, char *name)
{
	if (NULL == dir)
	{
		dir = (struct vsfile_t *)&vsfile.rootfs;
	}
	if (!(dir->attr & VSFILE_ATTR_DIRECTORY))
	{
		return VSFERR_INVALID_PARAMETER;
	}

	return dir->op->d_op.removefile(pt, evt, dir, name);
}

// dummy
vsf_err_t vsfile_dummy_file(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir)
{
	return VSFERR_NONE;
}
vsf_err_t vsfile_dummy_rw(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir, uint64_t offset,
					uint32_t size, uint8_t *buff, uint32_t *rsize)
{
	return VSFERR_NONE;
}

// memfs
vsf_err_t vsfile_memfs_getchild(struct vsfsm_pt_t *pt,
					vsfsm_evt_t evt, struct vsfile_t *dir, char *name,
					uint32_t idx, struct vsfile_t **file)
{
	struct vsfile_memfile_t *memfile;
	struct vsfile_t *child;

	if (NULL == dir)
	{
		dir = (struct vsfile_t *)pt->user_data;
	}
	memfile = (struct vsfile_memfile_t *)dir;
	child = (struct vsfile_t *)memfile->d.child;

	while ((child != NULL) && (child->name != NULL))
	{
		if ((name && vsfile_match(name, child->name)) ||
			(!name && !idx))
		{
			break;
		}
		idx--;
		child = (struct vsfile_t *)((uint32_t)child + memfile->d.child_size);
	}
	*file = (!child || !child->name) ? NULL : child;
	return (NULL == *file) ? VSFERR_NOT_AVAILABLE : VSFERR_NONE;
}

vsf_err_t vsfile_memfs_read(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *file, uint64_t offset,
					uint32_t size, uint8_t *buff, uint32_t *rsize)
{
	struct vsfile_memfile_t *memfile = (struct vsfile_memfile_t *)file;

	if (offset >= file->size)
	{
		*rsize = 0;
		return VSFERR_NONE;
	}

	*rsize = (uint32_t)min(size, file->size - offset);
	memcpy(buff, &memfile->f.buff[offset], *rsize);
	return VSFERR_NONE;
}

vsf_err_t vsfile_memfs_write(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *file, uint64_t offset,
					uint32_t size, uint8_t *buff, uint32_t *wsize)
{
	struct vsfile_memfile_t *memfile = (struct vsfile_memfile_t *)file;

	if (offset >= file->size)
	{
		*wsize = 0;
		return VSFERR_NONE;
	}

	*wsize = (uint32_t)min(size, file->size - offset);
	memcpy(&memfile->f.buff[offset], buff, *wsize);
	return VSFERR_NONE;
}

static void vsfile_memfs_init(struct vsfile_memfile_t *file)
{
	while (file->file.name != NULL)
	{
		file->file.op = (struct vsfile_fsop_t *)&vsfile_memfs_op;
		if (file->file.attr & VSFILE_ATTR_DIRECTORY)
		{
			file->d.child_size = sizeof(*file);
			vsfile_memfs_init(file->d.child);
		}
		file++;
	}
}

static vsf_err_t vsfile_memfs_mount(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *file)
{
	vsfile_memfs_init((struct vsfile_memfile_t *)(pt->user_data));
	return VSFERR_NONE;
}

// vfs
static vsf_err_t vsfile_vfs_mount(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir)
{
	struct vsfile_vfsfile_t *vfsfile = (struct vsfile_vfsfile_t *)dir;
	vsf_err_t err;

	vfsfile->subfs.param = pt->user_data;
	err = vfsfile->subfs.op->mount(pt, evt, NULL);
	if (!err)
	{
		vfsfile->mounted = true;
	}
	return err;
}

static vsf_err_t vsfile_vfs_unmount(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir)
{
	struct vsfile_vfsfile_t *vfsfile = (struct vsfile_vfsfile_t *)dir;
	vsf_err_t err;

	pt->user_data = vfsfile->subfs.param;
	err = vfsfile->subfs.op->unmount(pt, evt, NULL);
	if (!err)
	{
		vfsfile->mounted = false;
	}
	return err;
}

static vsf_err_t vsfile_vfs_getchild(struct vsfsm_pt_t *pt,
					vsfsm_evt_t evt, struct vsfile_t *dir, char *name,
					uint32_t idx, struct vsfile_t **file)
{
	struct vsfile_vfsfile_t *vfsfile = (struct vsfile_vfsfile_t *)dir;

	if (vfsfile->mounted)
	{
		pt->user_data = vfsfile->subfs.param;
		return vfsfile->subfs.op->d_op.getchild(pt, evt, NULL, name, idx, file);
	}
	else
	{
		struct vsfile_vfsfile_t *child = vfsfile->dir.child;

		while (child != NULL)
		{
			if ((name && vsfile_match(name, child->file.name)) ||
				(!name && !idx))
			{
				break;
			}
			idx--;
			child = child->next;
		}
		*file = (struct vsfile_t *)child;
		return (NULL == child) ? VSFERR_NOT_AVAILABLE : VSFERR_NONE;
	}
}

static vsf_err_t vsfile_vfs_addfile(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir, char *name, enum vsfile_attr_t attr)
{
	struct vsfile_vfsfile_t *vfsfile = (struct vsfile_vfsfile_t *)dir;
	struct vsfile_vfsfile_t *newfile;

	// can only add directory to vfs
	if (!(attr & VSFILE_ATTR_DIRECTORY) ||
		!vsfile_vfs_getchild(NULL, 0, dir, name, 0,
									(struct vsfile_t **)&newfile))
	{
		return VSFERR_FAIL;
	}

	newfile = vsfile.memop->alloc_vfs();
	if (NULL == newfile)
	{
		return VSFERR_NOT_ENOUGH_RESOURCES;
	}
	memset(newfile, 0, sizeof(*newfile));

	newfile->file.name = name;
	newfile->file.attr = attr;
	newfile->file.op = (struct vsfile_fsop_t *)&vsfile_vfs_op;
	newfile->next = vfsfile->dir.child;
	vfsfile->dir.child = newfile;
	return VSFERR_NONE;
}

static vsf_err_t vsfile_vfs_removefile(struct vsfsm_pt_t *pt, vsfsm_evt_t evt,
					struct vsfile_t *dir, char *name)
{
	struct vsfile_vfsfile_t *vfsfile = (struct vsfile_vfsfile_t *)dir;
	struct vsfile_vfsfile_t *child = vfsfile->dir.child, *oldfile;

	if (vsfile_vfs_getchild(NULL, 0, dir, name, 0,
									(struct vsfile_t **)&oldfile))
	{
		return VSFERR_FAIL;
	}

	if (child == oldfile)
	{
		vfsfile->dir.child = oldfile->next;
	}
	else while (child != NULL)
	{
		if (child->next == oldfile)
		{
			child->next = oldfile->next;
			break;
		}
		child = child->next;
	}

	vsfile.memop->free_vfs(oldfile);
	return VSFERR_FAIL;
}

const struct vsfile_fsop_t vsfile_memfs_op =
{
	// mount / unmount
	.mount = vsfile_memfs_mount,
	.unmount = vsfile_dummy_unmount,
	// f_op
	.f_op.close = vsfile_dummy_close,
	.f_op.read = vsfile_memfs_read,
	.f_op.write = vsfile_memfs_write,
	// d_op
	.d_op.getchild = vsfile_memfs_getchild,
};

const struct vsfile_fsop_t vsfile_vfs_op =
{
	// mount / unmount
	.mount = vsfile_vfs_mount,
	.unmount = vsfile_vfs_unmount,
	// f_op
	.f_op.close = vsfile_dummy_close,
	.f_op.read = NULL,
	.f_op.write = NULL,
	// d_op
	.d_op.getchild = vsfile_vfs_getchild,
	.d_op.addfile = vsfile_vfs_addfile,
	.d_op.removefile = vsfile_vfs_removefile,
};

