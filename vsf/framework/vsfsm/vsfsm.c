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

#if VSFSM_CFG_PREMPT_EN
static struct vsfsm_evtq_t *vsfsm_cur_evtq = NULL;
struct vsfsm_evtq_t* vsfsm_evtq_set(struct vsfsm_evtq_t *queue)
{
	struct vsfsm_evtq_t *ret = vsfsm_cur_evtq;
	vsfsm_cur_evtq = queue;
	return ret;
}

void vsfsm_evtq_init(struct vsfsm_evtq_t *queue)
{
	queue->head = queue->tail = queue->queue;
	queue->evt_count = 0;
}

// vsfsm_get_event_pending should be called with interrupt disabled
uint32_t vsfsm_get_event_pending(void)
{
	return vsfsm_cur_evtq->evt_count;
}

static vsf_err_t vsfsm_evtq_post(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	struct vsfsm_evtq_t *evtq = sm->evtq;
	istate_t gint = GET_GLOBAL_INTERRUPT_STATE();

	DISABLE_GLOBAL_INTERRUPT();

	if (evtq->evt_count >= evtq->size)
	{
		SET_GLOBAL_INTERRUPT_STATE(gint);
		return VSFERR_NOT_ENOUGH_RESOURCES;
	}
	else if (evt == VSFSM_EVT_TIMER)
	{
		if (evtq->tick_evt_count)
		{
			SET_GLOBAL_INTERRUPT_STATE(gint);
			return VSFERR_NONE;
		}
		else
			evtq->tick_evt_count++;
	}

	evtq->tail->sm = sm;
	evtq->tail->evt = evt;
	(evtq->tail == &evtq->queue[evtq->size - 1]) ?
			evtq->tail = &evtq->queue[0] : evtq->tail++;
	sm->evt_count++;
	evtq->evt_count++;

	SET_GLOBAL_INTERRUPT_STATE(gint);

	if (evtq->activate != NULL)
	{
		evtq->activate(evtq);
	}

	return VSFERR_NONE;
}
#endif		// VSFSM_CFG_PREMPT_EN

#if VSFSM_CFG_SM_EN && VSFSM_CFG_HSM_EN
static bool vsfsm_is_in(struct vsfsm_state_t *s, struct vsfsm_state_t *t)
{
	while (t != NULL)
	{
		if (s == t)
		{
			return true;
		}
		t = t->super;
	}
	return false;
}
#endif

static vsf_err_t vsfsm_dispatch_evt(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
#if VSFSM_CFG_SM_EN && VSFSM_CFG_HSM_EN
	struct vsfsm_state_t *temp_state = NULL, *lca_state;
	struct vsfsm_state_t *temp_processor_state, *temp_target_state;
	struct vsfsm_state_t *processor_state = sm->cur_state;
	struct vsfsm_state_t *target_state = processor_state->evt_handler(sm, evt);
#elif VSFSM_CFG_SM_EN
	struct vsfsm_state_t *target_state = sm->cur_state->evt_handler(sm, evt);
#else
	sm->init_state.evt_handler(sm, evt);
#endif

#if !VSFSM_CFG_SM_EN
	return VSFERR_NONE;
#else
	// local event can not transmit or be passed to superstate
	if (evt >= VSFSM_EVT_LOCAL)
	{
		return VSFERR_NONE;
	}

#if VSFSM_CFG_HSM_EN
	// superstate
	while (target_state == (struct vsfsm_state_t *)-1)
	{
		processor_state = sm->cur_state->super;
		if (processor_state != NULL)
		{
			target_state = processor_state->evt_handler(sm, evt);
		}
	}
#endif

	if ((NULL == target_state)
#if !VSFSM_CFG_HSM_EN
		|| ((struct vsfsm_state_t *)-1 == target_state)
#endif
		)
	{
		// handled, or even topstate can not handle this event
		return VSFERR_NONE;
	}

	// need to transmit
#if VSFSM_CFG_HSM_EN
	// 1. exit to processor_state
	for (temp_state = sm->cur_state; temp_state != processor_state;)
	{
		temp_state->evt_handler(sm, VSFSM_EVT_EXIT);
		temp_state = temp_state->super;
	}
	// 2. some simple transition which happens in most cases
	if ((processor_state == target_state) ||
		(processor_state->super == target_state->super))
	{
		processor_state->evt_handler(sm, VSFSM_EVT_EXIT);
		target_state->evt_handler(sm, VSFSM_EVT_ENTER);
		goto update_cur_state;
	}
	if (processor_state->super == target_state)
	{
		processor_state->evt_handler(sm, VSFSM_EVT_EXIT);
		goto update_cur_state;
	}
	if (processor_state == target_state->super)
	{
		target_state->evt_handler(sm, VSFSM_EVT_ENTER);
		goto update_cur_state;
	}
	// 3. find the LCA
	lca_state = NULL;
	temp_processor_state = processor_state;
	temp_target_state = target_state;
	do
	{
		if (temp_processor_state != NULL)
		{
			if (vsfsm_is_in(temp_processor_state, target_state))
			{
				lca_state = temp_processor_state;
				break;
			}
			temp_processor_state = temp_processor_state->super;
		}
		if (temp_target_state != NULL)
		{
			if (vsfsm_is_in(temp_target_state, processor_state))
			{
				lca_state = temp_target_state;
				break;
			}
			temp_target_state = temp_target_state->super;
		}
		if ((NULL == temp_processor_state) && (NULL == temp_target_state))
		{
			return VSFERR_BUG;
		}
	} while (NULL == lca_state);
	// 4. exit from processor_state to lca
	for (temp_state = processor_state; temp_state != lca_state;)
	{
		temp_state->evt_handler(sm, VSFSM_EVT_EXIT);
		temp_state = temp_state->super;
	}
	// 5. enter from lca to target_state
	for (temp_state = lca_state; temp_state != target_state;)
	{
		temp_state->evt_handler(sm, VSFSM_EVT_ENTER);
		temp_state = temp_state->super;
	}
	// 6. update cur_state
update_cur_state:
	sm->cur_state = target_state;
	// 7. send VSFSM_EVT_INIT to target_state
#else
	sm->cur_state->evt_handler(sm, VSFSM_EVT_EXIT);
	sm->cur_state = target_state;
	sm->cur_state->evt_handler(sm, VSFSM_EVT_ENTER);
#endif
	return vsfsm_post_evt(sm, VSFSM_EVT_INIT);
#endif
}

#if VSFSM_CFG_PREMPT_EN
static vsf_err_t vsfsm_dispatch_evt_protected(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	uint8_t origlevel = vsfhal_core_set_intlevel(VSFCFG_MAX_SRT_PRIO);
	vsf_err_t err = vsfsm_dispatch_evt(sm, evt);
	vsfhal_core_set_intlevel(origlevel);
	return err;
}
#endif

#if VSFSM_CFG_HSM_EN
static struct vsfsm_state_t *
vsfsm_top_handler(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	REFERENCE_PARAMETER(sm);
	REFERENCE_PARAMETER(evt);
	return NULL;
}
struct vsfsm_state_t vsfsm_top = {vsfsm_top_handler};
#endif

#if (VSFSM_CFG_SM_EN && VSFSM_CFG_SUBSM_EN) || VSFSM_CFG_HSM_EN
static bool vsfsm_subsm_exists(struct vsfsm_state_t *state, struct vsfsm_t *sm)
{
	struct vsfsm_t *sm_temp = state->subsm;
	while (sm_temp != NULL)
	{
		if (sm_temp == sm)
		{
			return true;
		}
		sm_temp = sm_temp->next;
	}
	return false;
}

vsf_err_t vsfsm_add_subsm(struct vsfsm_state_t *state, struct vsfsm_t *sm)
{
	if (!vsfsm_subsm_exists(state, sm))
	{
		if (NULL == state->subsm)
		{
			sm->next = NULL;
		}
		else
		{
			sm->next = state->subsm;
		}
		state->subsm = sm;
	}
	return VSFERR_NONE;
}

vsf_err_t vsfsm_remove_subsm(struct vsfsm_state_t *state, struct vsfsm_t *sm)
{
	struct vsfsm_t *sm_temp = state->subsm;
	if (sm_temp == sm)
	{
		state->subsm = sm->next;
	}
	else
	{
		while (sm_temp != NULL)
		{
			if (sm_temp->next == sm)
			{
				sm_temp->next = sm->next;
				break;
			}
			sm_temp = sm_temp->next;
		}
	}
	return VSFERR_NONE;
}
#endif

vsf_err_t vsfsm_init(struct vsfsm_t *sm)
{
#if VSFSM_CFG_PREMPT_EN
	sm->evtq = vsfsm_cur_evtq;
	sm->evt_count = 0;
#endif
#if VSFSM_CFG_SYNC_EN
	sm->pending_next = NULL;
#endif
#if VSFSM_CFG_SM_EN || VSFSM_CFG_HSM_EN
	sm->cur_state = &sm->init_state;
#endif
#if (VSFSM_CFG_SM_EN && VSFSM_CFG_SUBSM_EN) || VSFSM_CFG_HSM_EN
	sm->init_state.subsm = NULL;
#endif
	// ignore any state transition on VSFSM_EVT_ENTER
	sm->init_state.evt_handler(sm, VSFSM_EVT_ENTER);
#if VSFSM_CFG_ACTIVE_EN
	// set active so that sm can accept events
	vsfsm_set_active(sm, true);
#endif
	// process state transition on VSFSM_EVT_INIT
	return vsfsm_post_evt(sm, VSFSM_EVT_INIT);
}

// MUST make sure no event will be sent to the sm in the queue when vsfsm_fini
vsf_err_t vsfsm_fini(struct vsfsm_t *sm)
{
#if VSFSM_CFG_PREMPT_EN
	struct vsfsm_evtq_element_t *tmp;
#endif

#if VSFSM_CFG_ACTIVE_EN
	vsfsm_set_active(sm, false);
#endif

	vsftimer_clean_sm(sm);
#if VSFSM_CFG_PREMPT_EN
	if (sm->evtq != NULL)
	{
		tmp = (struct vsfsm_evtq_element_t *)sm->evtq->head;
		while (tmp != sm->evtq->tail)
		{
			if (tmp->sm == sm)
			{
				tmp->sm = NULL;
			}
			if (++tmp >= (sm->evtq->queue + sm->evtq->size))
			{
				tmp = sm->evtq->queue;
			}
		}
	}
#endif
	return VSFERR_NONE;
}

#if VSFSM_CFG_PREMPT_EN
vsf_err_t vsfsm_poll(void)
{
	struct vsfsm_evtq_element_t tmp;

	while (vsfsm_cur_evtq->evt_count)
	{
		tmp = *vsfsm_cur_evtq->head;
		(vsfsm_cur_evtq->head == &vsfsm_cur_evtq->queue[vsfsm_cur_evtq->size - 1]) ?
			vsfsm_cur_evtq->head = &vsfsm_cur_evtq->queue[0] : vsfsm_cur_evtq->head++;
		DISABLE_GLOBAL_INTERRUPT();
		if (tmp.sm != NULL)
		{
			tmp.sm->evt_count--;
		}
		vsfsm_cur_evtq->evt_count--;
		if (tmp.evt == VSFSM_EVT_TIMER)
		{
			vsfsm_cur_evtq->tick_evt_count--;
		}
		ENABLE_GLOBAL_INTERRUPT();
		// sm will be NULL after vsfsm_fini
		if (tmp.sm != NULL)
		{
			vsfsm_dispatch_evt(tmp.sm, tmp.evt);
		}
	}
	return VSFERR_NONE;
}
#endif

#if VSFSM_CFG_ACTIVE_EN
vsf_err_t vsfsm_set_active(struct vsfsm_t *sm, bool active)
{
	sm->active = active;
	return VSFERR_NONE;
}
#endif

vsf_err_t vsfsm_post_evt(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	return
#if VSFSM_CFG_PREMPT_EN
#if VSFSM_CFG_ACTIVE_EN
			!sm->active ||
#endif
			// instant event can not be sent to a valid queue
			(sm->evtq && (sm->evtq != vsfsm_cur_evtq) &&
				(evt & VSFSM_EVT_INSTANT_MSK)) ? VSFERR_FAIL :
			// empty event queue, dispatch directly with protection
			!sm->evtq ?
				vsfsm_dispatch_evt_protected(sm, evt) :
			(sm->evtq != vsfsm_cur_evtq) ||
				(((evt & VSFSM_EVT_INSTANT_MSK) == 0) && sm->evt_count) ?
				vsfsm_evtq_post(sm, evt) : vsfsm_dispatch_evt(sm, evt);
#else
#if VSFSM_CFG_ACTIVE_EN
			(!sm->active) ? VSFERR_FAIL :
#endif
			vsfsm_dispatch_evt(sm, evt);
#endif
}

// pending event will be forced to be sent to event queue
vsf_err_t vsfsm_post_evt_pending(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	return
#if VSFSM_CFG_PREMPT_EN
#if VSFSM_CFG_ACTIVE_EN
			!sm->active ||
#endif
			// can not post pending event to a invalid queue
			(evt & VSFSM_EVT_INSTANT_MSK) ?
				VSFERR_FAIL :
			!sm->evtq ?
				vsfsm_dispatch_evt_protected(sm, evt) :
				vsfsm_evtq_post(sm, evt);
#else
			vsfsm_post_evt(sm, evt);
#endif
}

#if VSFSM_CFG_MSM_EN
static struct vsfsm_state_t *
vsfsm_msm_evt_handler(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	struct vsfsm_msm_t *msm = (struct vsfsm_msm_t *)sm->user_data;
	struct vsfsm_msm_entry_t *entry;
	int target_state;

	entry = msm->trans_tbl;
	for (int i = 0; i < msm->entry_num; i++, entry++)
	{
		if ((entry->state == msm->state) && (entry->evt == evt) &&
			((entry->guard == NULL) || (entry->guard(msm))))
		{
			target_state = entry->transact(msm);
			if (target_state >= 0)
			{
				msm->state = target_state;
				break;
			}
		}
	}
	return NULL;
}

vsf_err_t vsfsm_msm_init(struct vsfsm_t *sm, struct vsfsm_msm_t *msm)
{
	sm->user_data = msm;
	sm->init_state.evt_handler = vsfsm_msm_evt_handler;
	msm->sm = sm;
	return vsfsm_init(sm);
}
#endif

#if VSFSM_CFG_LJMP_EN
static struct vsfsm_state_t *
vsfsm_ljmp_evt_handler(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	struct vsfsm_ljmp_t *ljmp = (struct vsfsm_ljmp_t *)sm->user_data;
	jmp_buf ret;

	if ((evt == VSFSM_EVT_ENTER) || (evt == VSFSM_EVT_EXIT))
	{
		return NULL;
	}

	ljmp->ret = &ret;
	if (!setjmp(ret))
	{
		if (evt == VSFSM_EVT_INIT)
		{
			// implement set_stack as a func is risky, may break the stack
			vsfhal_core_set_stack((uint32_t)ljmp->stack);
			ljmp->thread(ljmp);
		}
		else
		{
			longjmp(ljmp->pos, evt);
		}
	}
	return NULL;
}

vsf_err_t vsfsm_ljmp_init(struct vsfsm_t *sm, struct vsfsm_ljmp_t *ljmp)
{
	sm->user_data = ljmp;
	sm->init_state.evt_handler = vsfsm_ljmp_evt_handler;
	ljmp->sm = sm;
	return vsfsm_init(sm);
}
#endif

#if VSFSM_CFG_PT_EN
static struct vsfsm_state_t *
vsfsm_pt_evt_handler(struct vsfsm_t *sm, vsfsm_evt_t evt)
{
	struct vsfsm_pt_t *pt = (struct vsfsm_pt_t *)sm->user_data;

	switch (evt)
	{
	case VSFSM_EVT_ENTER:
	case VSFSM_EVT_EXIT:
		break;
	case VSFSM_EVT_INIT:
		pt->state = 0;
		// fall through
	default:
		pt->thread(pt, evt);
	}
	return NULL;
}

#ifdef VSFSM_CFG_PT_GOTO
void vsfsm_pt_entry(struct vsfsm_pt_t *pt)
{
	uint32_t lr;
	compiler_get_lr(lr);
	pt->state = lr;
}
#endif

vsf_err_t vsfsm_pt_init(struct vsfsm_t *sm, struct vsfsm_pt_t *pt)
{
	sm->user_data = pt;
	sm->init_state.evt_handler = vsfsm_pt_evt_handler;
	pt->sm = sm;
	return vsfsm_init(sm);
}
#endif

#if VSFSM_CFG_SYNC_EN
// vsfsm_sync_t
vsf_err_t vsfsm_sync_init(struct vsfsm_sync_t *sync, uint32_t cur_value,
				uint32_t max_value, vsfsm_evt_t evt)
{
	sync->cur_value = cur_value;
	sync->max_value = max_value;
	sync->sm_pending = NULL;
	sync->evt = evt;
	return VSFERR_NONE;
}

static void vsfsm_sync_append_sm(struct vsfsm_sync_t *sync, struct vsfsm_t *sm)
{
	struct vsfsm_t *sm_pending;

	sm->pending_next = NULL;
	if (NULL == sync->sm_pending)
	{
		sync->sm_pending = sm;
	}
	else
	{
		sm_pending = sync->sm_pending;
		while (sm_pending->pending_next != NULL)
		{
			sm_pending = sm_pending->pending_next;
		}
		sm_pending->pending_next = sm;
	}
}

vsf_err_t vsfsm_sync_cancel(struct vsfsm_sync_t *sync, struct vsfsm_t *sm)
{
	struct vsfsm_t *sm_pending;

#ifdef VSFCFG_THREAD_SAFTY
	uint8_t origlevel = vsfhal_core_set_intlevel(VSFCFG_MAX_SRT_PRIO);
#endif
	if (sync->sm_pending == sm)
	{
		sync->sm_pending = sm->pending_next;
	}
	else if (sync->sm_pending != NULL)
	{
		sm_pending = sync->sm_pending;
		while (sm_pending->pending_next != NULL)
		{
			if (sm_pending->pending_next == sm)
			{
				sm_pending->pending_next = sm->pending_next;
				break;
			}
			sm_pending = sm_pending->pending_next;
		}
	}
#ifdef VSFCFG_THREAD_SAFTY
	vsfhal_core_set_intlevel(origlevel);
#endif
	return VSFERR_NONE;
}

vsf_err_t vsfsm_sync_increase(struct vsfsm_sync_t *sync)
{
	struct vsfsm_t *sm;

#ifdef VSFCFG_THREAD_SAFTY
	uint8_t origlevel = vsfhal_core_set_intlevel(VSFCFG_MAX_SRT_PRIO);
#endif
	if (sync->sm_pending)
	{
		sm = sync->sm_pending;
		sync->sm_pending = sync->sm_pending->pending_next;
#ifdef VSFCFG_THREAD_SAFTY
		vsfhal_core_set_intlevel(origlevel);
#endif
		if (vsfsm_post_evt(sm, sync->evt))
		{
			// should increase the evtq buffer size
			return VSFERR_BUG;
		}
	}
	else if (sync->cur_value < sync->max_value)
	{
		sync->cur_value++;
#ifdef VSFCFG_THREAD_SAFTY
		vsfhal_core_set_intlevel(origlevel);
#endif
	}
	else
	{
#ifdef VSFCFG_THREAD_SAFTY
		vsfhal_core_set_intlevel(origlevel);
#endif
		return VSFERR_BUG;
	}
	return VSFERR_NONE;
}

vsf_err_t vsfsm_sync_decrease(struct vsfsm_sync_t *sync, struct vsfsm_t *sm)
{
#ifdef VSFCFG_THREAD_SAFTY
	uint8_t origlevel = vsfhal_core_set_intlevel(VSFCFG_MAX_SRT_PRIO);
#endif
	if (sync->cur_value > 0)
	{
		sync->cur_value--;
#ifdef VSFCFG_THREAD_SAFTY
		vsfhal_core_set_intlevel(origlevel);
#endif
		return VSFERR_NONE;
	}
	vsfsm_sync_append_sm(sync, sm);
#ifdef VSFCFG_THREAD_SAFTY
	vsfhal_core_set_intlevel(origlevel);
#endif
	return VSFERR_NOT_READY;
}
#endif	// VSFSM_CFG_SYNC_EN
