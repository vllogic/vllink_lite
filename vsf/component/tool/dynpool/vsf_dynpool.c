#include "vsf.h"

#ifdef DYNPOOL_CFG_GC_EN
static void vsf_dynpool_gc_free(struct vsf_dynpool_t *dynpool,
				struct vsf_dynpool_list_t *p)
{
	sllist_remove(&dynpool->head.next, &p->list);
	vsf_bufmgr_free(p);
	dynpool->cur_pool_num--;
}

static void vsf_dynpool_gc_hit(struct vsf_dynpool_t *dynpool,
				struct vsf_dynpool_list_t *p)
{
	if (dynpool->gc.poll_ms > 0)
		p->gc_tick = vsfhal_tickclk_get_ms();
	else
		vsf_dynpool_gc_free(dynpool, p);
}

static struct vsfsm_state_t *
vsf_dynpool_evt_handler(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	struct vsf_dynpool_t *dynpool = (struct vsf_dynpool_t *)sm->user_data;
	struct vsf_dynpool_list_t *p;
	uint32_t cur_tick;

	switch (evt)
	{
	case VSFSM_EVT_INIT:
		dynpool->gc.timer.interval = 1;
		dynpool->gc.timer.trigger_cnt = -1;
		dynpool->gc.timer.evt = VSFSM_EVT_TIMER;
		dynpool->gc.timer.sm = sm;
		vsftimer_enqueue(&dynpool->gc.timer);
		break;
	case VSFSM_EVT_FINI:
		vsftimer_dequeue(&dynpool->gc.timer);
		break;
	case VSFSM_EVT_TIMER:
		cur_tick = vsfhal_tickclk_get_ms();
		p = sllist_get_container(dynpool->head.next, struct vsf_dynpool_list_t, list);
		while (p != NULL)
		{
			if (!p->allocated_num &&
				((cur_tick - p->gc_tick) > dynpool->gc.poll_ms))
			{
				vsf_dynpool_gc_free(dynpool, p);
			}
			p = sllist_get_container(p->list.next, struct vsf_dynpool_list_t, list);
		}
		break;
	}
	return NULL;
}
#endif

vsf_err_t vsf_dynpool_fini(struct vsf_dynpool_t *dynpool)
{
	struct vsf_dynpool_list_t *n, *p =
		sllist_get_container(dynpool->head.next, struct vsf_dynpool_list_t, list);

	while (p != NULL)
	{
		n = sllist_get_container(p->list.next, struct vsf_dynpool_list_t, list);
		vsf_bufmgr_free(p);
		p = n;
	}
	return VSFERR_NONE;
}

vsf_err_t vsf_dynpool_init(struct vsf_dynpool_t *dynpool)
{
	dynpool->item_size = (dynpool->item_size + 3) & ~3;
#ifdef DYNPOOL_CFG_GC_EN
	if (dynpool->gc.poll_ms > 0)
	{
		dynpool->gc.sm.user_data = dynpool;
		dynpool->gc.sm.init_state.evt_handler = vsf_dynpool_evt_handler;
		return vsfsm_init(&dynpool->gc.sm);
	}
#endif
	return VSFERR_NONE;
}

void *vsf_dynpool_alloc(struct vsf_dynpool_t *dynpool)
{
	struct vsf_dynpool_list_t *p =
		sllist_get_container(dynpool->head.next, struct vsf_dynpool_list_t, list);
	void *buff;

	while (p != NULL)
	{
		if (p->allocated_num < dynpool->pool_size)
		{
		do_allocate:
			buff = vsfpool_alloc(&p->pool);
			p->allocated_num++;
			return buff;
		}
		p = sllist_get_container(p->list.next, struct vsf_dynpool_list_t, list);
	}

	if (!dynpool->pool_num || (dynpool->pool_num < dynpool->cur_pool_num))
	{
		// memory layout:
		//	struct vsf_dynpool_list_t
		//	uint32_t mskarr[]
		//	n * buffer
		uint32_t mskarr_num = (dynpool->pool_size + 31) >> 5;
		uint32_t size = sizeof(struct vsf_dynpool_list_t) +
				(mskarr_num << 2) + dynpool->pool_size * dynpool->item_size;
		p = vsf_bufmgr_malloc(size);
		if (!p)
			return NULL;
		memset(p, 0, size);

		p->pool.flags = (uint32_t *)&p[1];
		p->pool.buffer = (void *)&p->pool.flags[mskarr_num];
		p->pool.item_size = dynpool->item_size;
		p->pool.item_num = dynpool->pool_size;
		dynpool->cur_pool_num++;

		p->list.next = dynpool->head.next;
		dynpool->head.next = &p->list;
		goto do_allocate;
	}
	return NULL;
}

bool vsf_dynpool_free(struct vsf_dynpool_t *dynpool, void *buff)
{
	struct vsf_dynpool_list_t *p =
		sllist_get_container(dynpool->head.next, struct vsf_dynpool_list_t, list);

	while (p != NULL)
	{
		if (vsfpool_free(&p->pool, buff))
		{
			p->allocated_num--;
#ifdef DYNPOOL_CFG_GC_EN
			if (!p->allocated_num)
				vsf_dynpool_gc_hit(dynpool, p);
#endif
			return true;
		}
		p = sllist_get_container(p->list.next, struct vsf_dynpool_list_t, list);
	}
	return false;
}
