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

#ifndef __LIST_H_INCLUDED__
#define __LIST_H_INCLUDED__

struct sllist
{
	struct sllist *next;
};

#define sllist_init_node(node)			((node).next = NULL)
#define sllist_insert(node, new)		((node).next = &(new))
#define sllist_get_container(p, t, m)	container_of(p, t, m)

int sllist_get_length(struct sllist *head);
int sllist_get_idx(struct sllist *head, struct sllist *node);
int sllist_is_in(struct sllist *head, struct sllist *node);
int sllist_remove(struct sllist **head, struct sllist *node);
void sllist_append(struct sllist *head, struct sllist *new_node);
void sllist_delete_next(struct sllist *head);

#endif // __LIST_H_INCLUDED__

