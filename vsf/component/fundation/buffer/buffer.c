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

// buf_get_value and buf_set_value
uint32_t buf_get_value(uint8_t *buf, uint8_t offset, uint8_t len)
{
	uint8_t bitlen, bitpos, bytepos, mask, result_len = 0;
	uint32_t result = 0;

	while (result_len < len)
	{
		bytepos = offset >> 3;
		bitpos = offset & 7;
		bitlen = min(len - result_len, 8 - bitpos);
		mask = (1 << bitlen) - 1;

		result |= ((buf[bytepos] >> bitpos) & mask) << result_len;

		offset += bitlen;
		result_len += bitlen;
	}
	return result;
}

void buf_set_value(uint8_t *buf, uint8_t offset, uint8_t len, uint32_t value)
{
	uint8_t bitlen, bitpos, bytepos, mask, result_len = 0;

	while (result_len < len)
	{
		bytepos = offset >> 3;
		bitpos = offset & 7;
		bitlen = min(len - result_len, 8 - bitpos);
		mask = (1 << bitlen) - 1;
		
		buf[bytepos] &= ~(((~0UL >> result_len) & mask) << bitpos);
		buf[bytepos] |= ((value >> result_len) & mask) << bitpos;

		offset += bitlen;
		result_len += bitlen;
	}
}

// queue
void vsfq_init(struct vsfq_t *q)
{
	q->head = q->tail = NULL;
}

void vsfq_append(struct vsfq_t *q, struct vsfq_node_t *n)
{
	n->next = NULL;
	if (NULL == q->tail)
	{
		q->head = q->tail = n;
	}
	else
	{
		q->tail->next = n;
		q->tail = n;
	}
}

void vsfq_remove(struct vsfq_t *q, struct vsfq_node_t *n)
{
	struct vsfq_node_t *head = q->head;

	if (head == n)
	{
		q->head = head->next;
		if (NULL == q->head)
		{
			q->tail = NULL;
		}
	}
	else if (head != NULL)
	{
		while (head->next != NULL)
		{
			if (head->next == n)
			{
				head->next = head->next->next;
				if (NULL == head->next)
				{
					q->tail = head;
				}
				break;
			}
			head = head->next;
		}
	}
}

void vsfq_enqueue(struct vsfq_t *q, struct vsfq_node_t *n)
{
	struct vsfq_node_t *tmp = q->head;

	n->next = NULL;
	if (NULL == tmp)
	{
		q->head = q->tail = n;
	}
	else if (tmp->addr >= n->addr)
	{
		n->next = tmp;
		q->head = n;
	}
	else
	{
		while (tmp->next != NULL)
		{
			if (tmp->next->addr >= n->addr)
			{
				n->next = tmp->next;
				tmp->next = n;
				break;
			}
			tmp = tmp->next;
		}
		// insert last
		tmp->next = n;
	}
}

struct vsfq_node_t* vsfq_dequeue(struct vsfq_t *q)
{
	struct vsfq_node_t *head = q->head;
	if (q->head != NULL)
	{
		q->head = q->head->next;
		if (NULL == q->head)
		{
			q->tail = NULL;
		}
	}
	return head;
}

//#define vsf_fifo_get_next_index(pos, size)	(((pos) + 1) % (size))
static uint32_t vsf_fifo_get_next_index(uint32_t pos, uint32_t size)
{
	if (++pos >= size)
	{
		pos = 0;
	}
	return pos;
}

vsf_err_t vsf_fifo_init(struct vsf_fifo_t *fifo)
{
#if __VSF_DEBUG__
	if (NULL == fifo)
	{
		return VSFERR_INVALID_PARAMETER;
	}
#endif
	if (((uint32_t)fifo == 0x200012b8) && fifo->head)
		__ASM("NOP");
	
	fifo->head = fifo->tail = 0;
	return VSFERR_NONE;
}

uint32_t vsf_fifo_get_data_length(struct vsf_fifo_t *fifo)
{
#if __VSF_DEBUG__
	if (NULL == fifo)
	{
		return 0;
	}
#endif
	if (fifo->head >= fifo->tail)
	{
		return fifo->head - fifo->tail;
	}
	else
	{
		return fifo->buffer.size - (fifo->tail - fifo->head);
	}
}

uint32_t vsf_fifo_get_avail_length(struct vsf_fifo_t *fifo)
{
	uint32_t len;

#if __VSF_DEBUG__
	if (NULL == fifo)
	{
		return 0;
	}
#endif
	len = fifo->buffer.size - vsf_fifo_get_data_length(fifo);
	if (len > 0)
	{
		len--;
	}
	return len;
}

uint32_t vsf_fifo_push8(struct vsf_fifo_t *fifo, uint8_t data)
{
	if (vsf_fifo_get_avail_length(fifo) < 1)
	{
		return 0;
	}

	fifo->buffer.buffer[fifo->head] = data;
	fifo->head = vsf_fifo_get_next_index(fifo->head, fifo->buffer.size);
	return 1;
}

uint8_t vsf_fifo_pop8(struct vsf_fifo_t *fifo)
{
	uint8_t data;

	if (vsf_fifo_get_data_length(fifo) <= 0)
	{
		return 0;
	}

	data = fifo->buffer.buffer[fifo->tail];
	fifo->tail = vsf_fifo_get_next_index(fifo->tail, fifo->buffer.size);
	return data;
}

uint32_t vsf_fifo_push(struct vsf_fifo_t *fifo, uint32_t size, uint8_t *data)
{
	uint32_t tmp32;

#if __VSF_DEBUG__
	if ((NULL == fifo) || (NULL == data))
	{
		return 0;
	}
#endif
	if (size > vsf_fifo_get_avail_length(fifo))
	{
		return 0;
	}

	tmp32 = fifo->buffer.size - fifo->head;
	if (size > tmp32)
	{
		if (data)
		{
			memcpy(&fifo->buffer.buffer[fifo->head], &data[0], tmp32);
			memcpy(&fifo->buffer.buffer[0], &data[tmp32], size - tmp32);
		}
		fifo->head = size - tmp32;
	}
	else
	{
		if (data)
		{
			memcpy(&fifo->buffer.buffer[fifo->head], data, size);
		}
		fifo->head += size;
		if (fifo->head == fifo->buffer.size)
		{
			fifo->head = 0;
		}
	}
	if (fifo->head == 0)
		__ASM("NOP");
	return size;
}

uint32_t vsf_fifo_get_rbuf(struct vsf_fifo_t *fifo, uint8_t **data)
{
	uint32_t tmp32, avail_len = vsf_fifo_get_data_length(fifo);

#if __VSF_DEBUG__
	if (NULL == fifo)
	{
		return 0;
	}
#endif
	if (data)
	{
		*data = &fifo->buffer.buffer[fifo->tail];
	}
	tmp32 = fifo->buffer.size - fifo->tail;
	return min(tmp32, avail_len);
}

uint32_t vsf_fifo_get_wbuf(struct vsf_fifo_t *fifo, uint8_t **data)
{
	uint32_t tmp32, avail_len = vsf_fifo_get_avail_length(fifo);

#if __VSF_DEBUG__
	if (NULL == fifo)
	{
		return 0;
	}
#endif
	if (data)
	{
		*data = &fifo->buffer.buffer[fifo->head];
	}
	tmp32 = fifo->buffer.size - fifo->head;
	return min(tmp32, avail_len);
}

uint32_t vsf_fifo_peek(struct vsf_fifo_t *fifo, uint32_t size, uint8_t *data)
{
	uint32_t tmp32;
	uint32_t avail_len = vsf_fifo_get_data_length(fifo);

#if __VSF_DEBUG__
	if (NULL == fifo)
	{
		return 0;
	}
#endif
	if (size > avail_len)
	{
		size = avail_len;
	}

	tmp32 = fifo->buffer.size - fifo->tail;
	if (data)
	{
		if (size > tmp32)
		{
			memcpy(&data[0], &fifo->buffer.buffer[fifo->tail], tmp32);
			memcpy(&data[tmp32], &fifo->buffer.buffer[0], size - tmp32);
		}
		else
		{
			memcpy(data, &fifo->buffer.buffer[fifo->tail], size);
		}
	}
	return size;
}

uint32_t vsf_fifo_pop(struct vsf_fifo_t *fifo, uint32_t size, uint8_t *data)
{
	uint32_t tmp32;
	uint32_t ret = vsf_fifo_peek(fifo, size, data);

	if (!ret)
	{
		return 0;
	}

	tmp32 = fifo->buffer.size - fifo->tail;
	if (ret > tmp32)
	{
		fifo->tail = ret - tmp32;
	}
	else
	{
		fifo->tail += ret;
		if (fifo->tail == fifo->buffer.size)
		{
			fifo->tail = 0;
		}
	}
	return ret;
}

// multibuf
vsf_err_t vsf_multibuf_init(struct vsf_multibuf_t *mbuffer)
{
#if __VSF_DEBUG__
	if (NULL == mbuffer)
	{
		return VSFERR_INVALID_PARAMETER;
	}
#endif

	mbuffer->tail = mbuffer->head = mbuffer->length = 0;
	return VSFERR_NONE;
}

uint8_t* vsf_multibuf_get_empty(struct vsf_multibuf_t *mbuffer)
{
#if __VSF_DEBUG__
	if (NULL == mbuffer)
	{
		return NULL;
	}
#endif
	if (mbuffer->count <= mbuffer->length)
	{
		return NULL;
	}

	return mbuffer->buffer_list[mbuffer->head];
}

vsf_err_t vsf_multibuf_push(struct vsf_multibuf_t *mbuffer)
{
#if __VSF_DEBUG__
	if (NULL == mbuffer)
	{
		return VSFERR_FAIL;
	}
#endif
	if (mbuffer->count <= mbuffer->length)
	{
		return VSFERR_FAIL;
	}

	mbuffer->head = (uint16_t)vsf_fifo_get_next_index(mbuffer->head, mbuffer->count);
	mbuffer->length++;
	return VSFERR_NONE;
}

uint8_t* vsf_multibuf_get_payload(struct vsf_multibuf_t *mbuffer)
{
#if __VSF_DEBUG__
	if (NULL == mbuffer)
	{
		return NULL;
	}
#endif
	if (!mbuffer->length)
	{
		return NULL;
	}

	return mbuffer->buffer_list[mbuffer->tail];
}

vsf_err_t vsf_multibuf_pop(struct vsf_multibuf_t *mbuffer)
{
#if __VSF_DEBUG__
	if (NULL == mbuffer)
	{
		return VSFERR_FAIL;
	}
#endif
	if (!mbuffer->length)
	{
		return VSFERR_FAIL;
	}

	mbuffer->tail = (uint16_t)vsf_fifo_get_next_index(mbuffer->tail, mbuffer->count);
	mbuffer->length--;
	return VSFERR_NONE;
}

// bufmgr
#ifndef VSFCFG_BUFMGR_USE_STDLIB
#define VSF_BUFMGR_DEFAULT_ALIGN				4
#define VSF_BUFMGR_BUF_CHECK_EN					0
#define VSF_BUFMGR_POINT_CHECK_EN				0

#if	VSF_BUFMGR_BUF_CHECK_EN
#define VSF_BUFMGR_MAX_SIZE						(64 * 1024)
#define VSF_BUFMGR_CHECK0						0x78563412
#define VSF_BUFMGR_CHECK1						0xf1debc9a
#define VSF_BUFMGR_INIT_BUF(a)					\
	do{\
		(a)->temp[0] = VSF_BUFMGR_CHECK0, (a)->temp[1] = VSF_BUFMGR_CHECK1;\
	}while(0)
#define VSF_BUFMGR_CHECK_BUF(a)					\
	(((a)->temp[0] == VSF_BUFMGR_CHECK0) && ((a)->temp[1] == VSF_BUFMGR_CHECK1))
#endif		// VSF_BUFMGR_BUF_CHECK_EN
#ifdef VSFCFG_BUFMGR_LOG
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#define VSF_BUFMGR_LOG_BUF_LENGTH				40
static uint8_t bufmgr_log_buf[VSF_BUFMGR_LOG_BUF_LENGTH + 1];
#endif

struct vsf_bufmgr_mcb_t
{
	struct vsf_buffer_t buffer;
	struct sllist list;
#if VSF_BUFMGR_POINT_CHECK_EN
	void *p;
#endif
#if VSF_BUFMGR_BUF_CHECK_EN
	uint32_t temp[2];
#endif
};
struct vsf_bufmgr_t
{
	// private
	struct vsf_bufmgr_mcb_t freed_list;
	struct vsf_bufmgr_mcb_t allocated_list;

#if VSF_BUFMGR_BUF_CHECK_EN
	uint32_t err_conut;
#endif
};
#define MCB_SIZE sizeof(struct vsf_bufmgr_mcb_t)
static struct vsf_bufmgr_t bufmgr;

#if VSF_BUFMGR_BUF_CHECK_EN
static void vsf_bufmgr_error(void)
{
	if (bufmgr.err_conut < 0xffffffff)
	{
		bufmgr.err_conut++;
	}
}

static void vsf_bufmgr_check(struct vsf_bufmgr_mcb_t *mcb)
{
	if (!VSF_BUFMGR_CHECK_BUF(mcb))
	{
		vsf_bufmgr_error();
		VSF_BUFMGR_INIT_BUF(mcb);
	}
}
#endif

static void vsf_bufmgr_remove_mcb(struct vsf_bufmgr_mcb_t *list, struct vsf_bufmgr_mcb_t *mcb)
{
	struct vsf_bufmgr_mcb_t *active_mcb, *next_mcb, *prev_mcb;

	active_mcb = sllist_get_container(list->list.next, struct vsf_bufmgr_mcb_t, list);
	prev_mcb = list;
	while(active_mcb != NULL)
	{
		if(active_mcb == mcb)
		{
			next_mcb = sllist_get_container(active_mcb->list.next, struct vsf_bufmgr_mcb_t, list);
			if(next_mcb != NULL)
			{
				sllist_insert(prev_mcb->list, next_mcb->list);
			}
			else
			{
				sllist_init_node(prev_mcb->list);
			}
			sllist_init_node(active_mcb->list);
			return;
		}
		prev_mcb = active_mcb;
		active_mcb = sllist_get_container(active_mcb->list.next, struct vsf_bufmgr_mcb_t, list);
	}
}

static void vsf_bufmgr_insert_mcb(struct vsf_bufmgr_mcb_t *list, struct vsf_bufmgr_mcb_t *mcb)
{
	struct vsf_bufmgr_mcb_t *active_mcb, *prev_mcb;

#if VSF_BUFMGR_BUF_CHECK_EN
	VSF_BUFMGR_INIT_BUF(mcb);
#endif

	active_mcb = sllist_get_container(list->list.next, struct vsf_bufmgr_mcb_t, list);
	prev_mcb = list;

	while (active_mcb != NULL)
	{
		if(active_mcb->buffer.size >= mcb->buffer.size)
		{
			sllist_insert(prev_mcb->list, mcb->list);
			sllist_insert(mcb->list, active_mcb->list);
			return;
		}
		prev_mcb = active_mcb;
		active_mcb = sllist_get_container(active_mcb->list.next, struct vsf_bufmgr_mcb_t, list);
	}
	if(active_mcb == NULL)
	{
		sllist_insert(prev_mcb->list, mcb->list);
		sllist_init_node(mcb->list);
	}
}

static void vsf_bufmgr_merge_mcb(struct vsf_bufmgr_mcb_t *list, struct vsf_bufmgr_mcb_t *mcb)
{
	struct vsf_bufmgr_mcb_t *active_mcb, *prev_mcb = NULL, *next_mcb = NULL;

	active_mcb = sllist_get_container(list->list.next, struct vsf_bufmgr_mcb_t, list);

	while (active_mcb != NULL)
	{
		if(((uint32_t)mcb->buffer.buffer + mcb->buffer.size) == (uint32_t)active_mcb)
		{
			next_mcb = active_mcb;
		}
		if(((uint32_t)active_mcb->buffer.buffer + active_mcb->buffer.size) == (uint32_t)mcb)
		{
			prev_mcb = active_mcb;
		}
		active_mcb = sllist_get_container(active_mcb->list.next, struct vsf_bufmgr_mcb_t, list);
	}
	if((next_mcb == NULL) && (prev_mcb == NULL))
	{
		vsf_bufmgr_insert_mcb(list, mcb);
	}
	else if((next_mcb != NULL) && (prev_mcb == NULL))
	{
		vsf_bufmgr_remove_mcb(list, next_mcb);
		mcb->buffer.size += MCB_SIZE + next_mcb->buffer.size;
#if VSF_BUFMGR_BUF_CHECK_EN
		if (mcb->buffer.size > VSF_BUFMGR_MAX_SIZE)
		{
			vsf_bufmgr_error();
		}
#endif
		vsf_bufmgr_insert_mcb(list, mcb);
	}
	else if((next_mcb == NULL) && (prev_mcb != NULL))
	{
		vsf_bufmgr_remove_mcb(list, prev_mcb);
		prev_mcb->buffer.size += MCB_SIZE + mcb->buffer.size;
		vsf_bufmgr_insert_mcb(list, prev_mcb);
	}
	else if((next_mcb != NULL) && (prev_mcb != NULL))
	{
		vsf_bufmgr_remove_mcb(list, prev_mcb);
		vsf_bufmgr_remove_mcb(list, next_mcb);
		prev_mcb->buffer.size += MCB_SIZE * 2 + mcb->buffer.size + next_mcb->buffer.size;
		vsf_bufmgr_insert_mcb(list, prev_mcb);
	}
}

void vsf_bufmgr_init(uint8_t *buf, uint32_t size)
{
	struct vsf_bufmgr_mcb_t *mcb = (struct vsf_bufmgr_mcb_t *)buf;

	bufmgr.freed_list.buffer.buffer = NULL;
	bufmgr.freed_list.buffer.size = 0;
	sllist_init_node(bufmgr.freed_list.list);

	bufmgr.allocated_list.buffer.buffer = NULL;
	bufmgr.allocated_list.buffer.size = 0;
	sllist_init_node(bufmgr.allocated_list.list);

	mcb->buffer.buffer = (void *)(((uint32_t)buf + MCB_SIZE + 3) & 0xfffffffc);
	mcb->buffer.size = (uint32_t)buf + size - (uint32_t)mcb->buffer.buffer;
	sllist_init_node(mcb->list);
	sllist_insert(bufmgr.freed_list.list, mcb->list);

#if VSF_BUFMGR_BUF_CHECK_EN
	bufmgr.err_conut = 0;
#endif
}

#ifdef VSFCFG_BUFMGR_LOG
void* vsf_bufmgr_malloc_aligned_do(uint32_t size, uint32_t align,
		const char *format, ...)
#else
void* vsf_bufmgr_malloc_aligned(uint32_t size, uint32_t align)
#endif
{
#ifdef VSFCFG_THREAD_SAFTY
	uint8_t origlevel = vsfhal_core_set_intlevel(VSFCFG_MAX_SRT_PRIO);
#endif
	struct vsf_bufmgr_mcb_t *mcb= sllist_get_container(
				bufmgr.freed_list.list.next, struct vsf_bufmgr_mcb_t, list);
#ifdef VSFCFG_BUFMGR_LOG
	va_list ap;
	uint32_t size_out;
	va_start(ap, format);
	size_out = vsnprintf((char *)bufmgr_log_buf, VSF_BUFMGR_LOG_BUF_LENGTH, format, ap);
	va_end(ap);
#endif

	if (size == 0)
	{
#ifdef VSFCFG_THREAD_SAFTY
		vsfhal_core_set_intlevel(origlevel);
#endif
		return NULL;
	}
	if (size & 0x3)
	{
		size &= 0xfffffffc;
		size += 4;
	}
	if (align < VSF_BUFMGR_DEFAULT_ALIGN)
	{
		align = VSF_BUFMGR_DEFAULT_ALIGN;
	}
	while (mcb != NULL)
	{
		uint32_t offset = (uint32_t)mcb->buffer.buffer % align;
		offset = offset ? (align - offset) : 0;

		if (mcb->buffer.size >= size + offset)
		{
			struct vsf_bufmgr_mcb_t *mcb_align;
			if (offset >= MCB_SIZE + VSF_BUFMGR_DEFAULT_ALIGN)
			{
				mcb_align = (struct vsf_bufmgr_mcb_t *)\
									(mcb->buffer.buffer + offset - MCB_SIZE);
				mcb_align->buffer.buffer = (uint8_t *)mcb_align + MCB_SIZE;
				mcb_align->buffer.size = mcb->buffer.size - offset;

				vsf_bufmgr_remove_mcb(&bufmgr.freed_list, mcb);
				mcb->buffer.size = offset - MCB_SIZE;
				vsf_bufmgr_insert_mcb(&bufmgr.freed_list, mcb);

				offset = 0; // as return ptr offset
			}
			else
			{
				mcb_align = mcb;
				vsf_bufmgr_remove_mcb(&bufmgr.freed_list, mcb_align);
			}

			if (mcb_align->buffer.size > (size + offset + MCB_SIZE))
			{
				struct vsf_bufmgr_mcb_t *mcb_tail = (struct vsf_bufmgr_mcb_t *)\
									(mcb_align->buffer.buffer + offset + size);
				mcb_tail->buffer.buffer = (uint8_t *)mcb_tail + MCB_SIZE;
				mcb_tail->buffer.size = mcb_align->buffer.size - offset - size -
										MCB_SIZE;
				vsf_bufmgr_insert_mcb(&bufmgr.freed_list, mcb_tail);

				mcb_align->buffer.size = offset + size;
			}

			vsf_bufmgr_insert_mcb(&bufmgr.allocated_list, mcb_align);
			
#ifdef VSFCFG_BUFMGR_LOG
			if (size_out > 1)
			{
				bufmgr_log_buf[size_out] = '\0';
				vsfdbg_printf("MalcOK 0x%x:%d %s" VSFCFG_DEBUG_LINEEND,
					(uint32_t)(mcb_align->buffer.buffer + offset), mcb_align->buffer.size, bufmgr_log_buf);
			}
			else
			{
				vsfdbg_printf("MalcOK 0x%x:%d" VSFCFG_DEBUG_LINEEND,
					(uint32_t)(mcb_align->buffer.buffer + offset), mcb_align->buffer.size);
			}
#endif
			
			void *ptr = mcb_align->buffer.buffer + offset;
#if VSF_BUFMGR_POINT_CHECK_EN
			mcb_align->p = ptr;
#endif
#ifdef VSFCFG_THREAD_SAFTY
			vsfhal_core_set_intlevel(origlevel);
#endif
			return ptr;
		}
		mcb = sllist_get_container(mcb->list.next, struct vsf_bufmgr_mcb_t,
									list);
	}

#ifdef VSFCFG_BUFMGR_LOG
	if (size_out > 1)
	{
		bufmgr_log_buf[size_out] = '\0';
		vsfdbg_printf("MalcFL %s" VSFCFG_DEBUG_LINEEND, bufmgr_log_buf);
	}
	else
	{
		vsfdbg_printf("MalcFL UNKNOWN" VSFCFG_DEBUG_LINEEND);
	}
#endif
	
#if VSF_BUFMGR_BUF_CHECK_EN
	vsf_bufmgr_error();
#endif
#ifdef VSFCFG_THREAD_SAFTY
	vsfhal_core_set_intlevel(origlevel);
#endif
	return NULL;
}

#ifdef VSFCFG_BUFMGR_LOG
void vsf_bufmgr_free_do(void *ptr, const char *format, ...)
#else
void vsf_bufmgr_free(void *ptr)
#endif
{
#ifdef VSFCFG_THREAD_SAFTY
	uint8_t origlevel = vsfhal_core_set_intlevel(VSFCFG_MAX_SRT_PRIO);
#endif
	struct vsf_bufmgr_mcb_t *mcb = sllist_get_container(
			bufmgr.allocated_list.list.next,struct vsf_bufmgr_mcb_t, list);											 
#ifdef VSFCFG_BUFMGR_LOG
	va_list ap;
	uint32_t size_out;
	va_start(ap, format);
	size_out = vsnprintf((char *)bufmgr_log_buf, VSF_BUFMGR_LOG_BUF_LENGTH, format, ap);
	va_end(ap);
#endif
	
	
	while (mcb != NULL)
	{
		if (((uint32_t)mcb->buffer.buffer <= (uint32_t)ptr) &&
			((uint32_t)mcb->buffer.buffer + mcb->buffer.size > (uint32_t)ptr))
		{
#if VSF_BUFMGR_BUF_CHECK_EN
			vsf_bufmgr_check(mcb);
#endif
#if VSF_BUFMGR_POINT_CHECK_EN
			if (mcb->p != ptr)
			{
				vsf_bufmgr_error();
			}
#endif
			vsf_bufmgr_remove_mcb(&bufmgr.allocated_list, mcb);
			vsf_bufmgr_merge_mcb(&bufmgr.freed_list, mcb);
			
#ifdef VSFCFG_BUFMGR_LOG
			if (size_out > 1)
			{
				bufmgr_log_buf[size_out] = '\0';
				vsfdbg_printf("FreeOK 0x%x %s" VSFCFG_DEBUG_LINEEND,
					(uint32_t)ptr, bufmgr_log_buf);
			}
			else
			{
				vsfdbg_printf("FreeOK 0x%x" VSFCFG_DEBUG_LINEEND, (uint32_t)ptr);
			}
#endif
#ifdef VSFCFG_THREAD_SAFTY
			vsfhal_core_set_intlevel(origlevel);
#endif
			return;
		}
		mcb = sllist_get_container(mcb->list.next, struct vsf_bufmgr_mcb_t,
									list);
	}

#ifdef VSFCFG_BUFMGR_LOG
	if (size_out > 1)
	{
		bufmgr_log_buf[size_out] = '\0';
		vsfdbg_printf("FreeFL 0x%x %s" VSFCFG_DEBUG_LINEEND,
			(uint32_t)ptr, bufmgr_log_buf);
	}
	else
	{
		vsfdbg_printf("FreeFL 0x%x " VSFCFG_DEBUG_LINEEND, (uint32_t)ptr);
	}
#endif
	
#if VSF_BUFMGR_BUF_CHECK_EN
	vsf_bufmgr_error();
#endif
#ifdef VSFCFG_THREAD_SAFTY
	vsfhal_core_set_intlevel(origlevel);
#endif
}
#endif

// pool
void vsfpool_init(struct vsfpool_t *pool)
{
	pool->item_size = (pool->item_size + 3) & ~3;
	memset(pool->flags, 0, (pool->item_num + 31) >> 3);
}

void *vsfpool_alloc(struct vsfpool_t *pool)
{
	uint32_t index = mskarr_ffz(pool->flags, (pool->item_num + 31) >> 5);

	if (index >= pool->item_num)
	{
		return NULL;
	}
	mskarr_set(pool->flags, index);
	return (uint8_t *)pool->buffer + index * pool->item_size;
}

bool vsfpool_free(struct vsfpool_t *pool, void *buffer)
{
	uint32_t index = ((uint8_t *)buffer - (uint8_t *)pool->buffer) /
		pool->item_size;

	if (index < pool->item_num)
	{
		mskarr_clr(pool->flags, index);
		return true;
	}
	return false;
}
