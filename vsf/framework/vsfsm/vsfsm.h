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

#ifndef __VSFSM_H_INCLUDED__
#define __VSFSM_H_INCLUDED__

#include "vsf_type.h"
#include "vsf_cfg.h"

#ifndef VSFSM_CFG_PREMPT_EN
#define VSFSM_CFG_PREMPT_EN				1
#endif
#ifndef VSFSM_CFG_SYNC_EN
#define VSFSM_CFG_SYNC_EN				1
#endif
#ifndef VSFSM_CFG_ACTIVE_EN
#define VSFSM_CFG_ACTIVE_EN				0
#endif
#ifndef VSFSM_CFG_SM_EN
#define VSFSM_CFG_SM_EN					0
#endif
#ifndef VSFSM_CFG_SUBSM_EN
#define VSFSM_CFG_SUBSM_EN				0
#endif
#ifndef VSFSM_CFG_HSM_EN
#define VSFSM_CFG_HSM_EN				0
#endif
#ifndef VSFSM_CFG_MSM_EN
#define VSFSM_CFG_MSM_EN				0
#endif
#ifndef VSFSM_CFG_PT_EN
#define VSFSM_CFG_PT_EN					1
#endif
#ifndef VSFSM_CFG_LJMP_EN
#define VSFSM_CFG_LJMP_EN				0
#endif

enum
{
	VSFSM_EVT_INVALID = -1,
	VSFSM_EVT_NONE = 0,
	VSFSM_EVT_SYSTEM = 1,
	VSFSM_EVT_DUMMY = VSFSM_EVT_SYSTEM + 0,
	VSFSM_EVT_INIT = VSFSM_EVT_SYSTEM + 1,
	VSFSM_EVT_FINI = VSFSM_EVT_SYSTEM + 2,
	// event for vsftimer
	VSFSM_EVT_TIMER = VSFSM_EVT_SYSTEM + 3,
	VSFSM_EVT_DELAY_DONE = VSFSM_EVT_SYSTEM + 4,
	VSFSM_EVT_USER = 0x10,

	// instant message CANNOT be but in the event queue and
	// can not be sent in interrupt
	VSFSM_EVT_INSTANT_MSK = 0x2000,
	// local event can not transmit or be passed to superstate
	VSFSM_EVT_LOCAL_MSK = 0x4000,
	VSFSM_EVT_LOCAL_INSTANT_MSK = VSFSM_EVT_INSTANT_MSK | VSFSM_EVT_LOCAL_MSK,

	VSFSM_EVT_USER_INSTANT = VSFSM_EVT_INSTANT_MSK,
	VSFSM_EVT_USER_LOCAL = VSFSM_EVT_LOCAL_MSK,
	// VSFSM_EVT_ENTER and VSFSM_EVT_EXIT are local instant events
	VSFSM_EVT_ENTER = VSFSM_EVT_LOCAL_INSTANT_MSK + 0,
	VSFSM_EVT_EXIT = VSFSM_EVT_LOCAL_INSTANT_MSK + 1,
	VSFSM_EVT_USER_LOCAL_INSTANT = VSFSM_EVT_LOCAL_INSTANT_MSK + 2,
};

typedef int vsfsm_evt_t;

struct vsfsm_t;
struct vsfsm_state_t
{
	// return NULL means the event is handled, and no transition
	// return a vsfsm_state_t pointer means transition to the state
	// return -1 means the event is not handled, should redirect to superstate
	struct vsfsm_state_t * (*evt_handler)(struct vsfsm_t *sm, vsfsm_evt_t evt);

#if (VSFSM_CFG_SM_EN && VSFSM_CFG_SUBSM_EN) || VSFSM_CFG_HSM_EN
	// sub state machine list
	struct vsfsm_t *subsm;
#endif

#if VSFSM_CFG_HSM_EN
	// for top state, super is NULL; other super points to the superstate
	struct vsfsm_state_t *super;
#endif
};

struct vsfsm_evtq_t;
struct vsfsm_t
{
	// initial state
	// for protothread, evt_handler MUST point to vsfsm_pt_evt_handler
	// 		which will be initialized in vsfsm_pt_init
	struct vsfsm_state_t init_state;
	// user_data point to the user specified data for the sm
	// for protothread, user_data should point to vsfsm_pt_t structure
	// 		which will be initialized in vsfsm_pt_init
	void *user_data;

	// private
#if VSFSM_CFG_SM_EN || VSFSM_CFG_HSM_EN
	struct vsfsm_state_t *cur_state;
#endif
#if VSFSM_CFG_SYNC_EN
	// pending_next is used for vsfsm_sync_t
	struct vsfsm_t *pending_next;
#endif
#if VSFSM_CFG_ACTIVE_EN
	volatile bool active;
#endif
#if (VSFSM_CFG_SM_EN && VSFSM_CFG_SUBSM_EN) || VSFSM_CFG_HSM_EN
	// next is used to link vsfsm_t in the same level
	struct vsfsm_t *next;
#endif
#if VSFSM_CFG_PREMPT_EN
	struct vsfsm_evtq_t *evtq;
	uint32_t evt_count;
#endif
};

#if VSFSM_CFG_PREMPT_EN
struct vsfsm_evtq_element_t
{
	struct vsfsm_t *sm;
	vsfsm_evt_t evt;
};
struct vsfsm_evtq_t
{
	uint32_t size;
	struct vsfsm_evtq_element_t *queue;
	void (*activate)(struct vsfsm_evtq_t *q);
	// private
	volatile struct vsfsm_evtq_element_t *head;
	volatile struct vsfsm_evtq_element_t *tail;
	volatile uint16_t evt_count;
	volatile uint16_t tick_evt_count;
};
void vsfsm_evtq_init(struct vsfsm_evtq_t *queue);
struct vsfsm_evtq_t* vsfsm_evtq_set(struct vsfsm_evtq_t *queue);
#endif

#if VSFSM_CFG_MSM_EN
struct vsfsm_msm_t;
struct vsfsm_msm_entry_t
{
	int state;
	vsfsm_evt_t evt;
	bool (*guard)(struct vsfsm_msm_t *msm);
	int (*transact)(struct vsfsm_msm_t *msm);
};
struct vsfsm_msm_t
{
	uint32_t entry_num;
	struct vsfsm_msm_entry_t *trans_tbl;
	void *user_data;

	// protected
	int state;
	struct vsfsm_t *sm;
};
vsf_err_t vsfsm_msm_init(struct vsfsm_t *sm, struct vsfsm_msm_t *msm);
#endif

#if VSFSM_CFG_LJMP_EN
#include <setjmp.h>
struct vsfsm_ljmp_t;
typedef void (*vsfsm_ljmp_thread_t)(struct vsfsm_ljmp_t *ljmp);
struct vsfsm_ljmp_t
{
	vsfsm_ljmp_thread_t thread;
	void *user_data;

	void *stack;
	struct vsfsm_t *sm;

	jmp_buf pos;
	jmp_buf *ret;
};
vsf_err_t vsfsm_ljmp_init(struct vsfsm_t *sm, struct vsfsm_ljmp_t *ljmp);

#define vsfsm_ljmp_ret(ljmp)		longjmp(*(ljmp)->ret, 0)
#define vsfsm_ljmp_wfe(ljmp, e)		\
	do {\
		vsfsm_evt_t __evt = setjmp((ljmp)->pos);\
		if (!__evt || (__evt != (e)))\
			vsfsm_ljmp_ret(ljmp);\
	} while (0)
#endif // VSFSM_CFG_LJMP_EN

#if VSFSM_CFG_PT_EN
struct vsfsm_pt_t;
typedef vsf_err_t (*vsfsm_pt_thread_t)(struct vsfsm_pt_t *pt, vsfsm_evt_t evt);
struct vsfsm_pt_t
{
	vsfsm_pt_thread_t thread;
	void *user_data;

	// protected
	int state;
	struct vsfsm_t *sm;
};

vsf_err_t vsfsm_pt_init(struct vsfsm_t *sm, struct vsfsm_pt_t *pt);

#ifndef VSFSM_CFG_PT_GOTO
#define vsfsm_pt_begin(pt)			switch ((pt)->state) { case 0:
#define vsfsm_pt_entry(pt)			(pt)->state = __LINE__; case __LINE__:
#define vsfsm_pt_end(pt)			}
#else
#define vsfsm_pt_begin(pt)			if ((pt)->state) compiler_set_pc((pt)->state)
void vsfsm_pt_entry(struct vsfsm_pt_t *pt);
#define vsfsm_pt_end(pt)			
#endif

// wait for next event
#define vsfsm_pt_wait(pt)			\
	do {\
		evt = VSFSM_EVT_INVALID;\
		vsfsm_pt_entry(pt);\
		if (VSFSM_EVT_INVALID == evt) return VSFERR_NOT_READY;\
	} while (0)
// wait for specified event
#define vsfsm_pt_wfe(pt, e)			\
	do {\
		evt = VSFSM_EVT_INVALID;\
		vsfsm_pt_entry(pt);\
		if (evt != (e)) return VSFERR_NOT_READY;\
	} while (0)
// wait for pt, slave pt uses the same stack as the master pt
#define vsfsm_pt_wfpt(pt, ptslave)	\
	do {\
		(ptslave)->state = 0;\
		(ptslave)->sm = (pt)->sm;\
		vsfsm_pt_entry(pt);\
		{\
			vsf_err_t __err = (ptslave)->thread(ptslave, evt);\
			if (__err != VSFERR_NONE)\
			{\
				return __err;\
			}\
		}\
	} while (0)
#endif // VSFSM_CFG_PT_EN

// vsfsm_get_event_pending should be called with interrupt disabled
uint32_t vsfsm_get_event_pending(void);

#if (VSFSM_CFG_SM_EN && VSFSM_CFG_SUBSM_EN) || VSFSM_CFG_HSM_EN
extern struct vsfsm_state_t vsfsm_top;
// sub-statemachine add/remove
vsf_err_t vsfsm_add_subsm(struct vsfsm_state_t *state, struct vsfsm_t *sm);
vsf_err_t vsfsm_remove_subsm(struct vsfsm_state_t *state, struct vsfsm_t *sm);
#endif

// vsfsm_init will set the sm to be active(ready to accept events)
vsf_err_t vsfsm_init(struct vsfsm_t *sm);
// vsfsm_fini will set the sm to be inactive and remove events in the queue
vsf_err_t vsfsm_fini(struct vsfsm_t *sm);
#if VSFSM_CFG_PREMPT_EN
vsf_err_t vsfsm_poll(void);
#endif
#if VSFSM_CFG_ACTIVE_EN
// sm is avtive after init, if sm will not accept further events
// user MUST set the sm to be inactive
vsf_err_t vsfsm_set_active(struct vsfsm_t *sm, bool active);
#endif
vsf_err_t vsfsm_post_evt(struct vsfsm_t *sm, vsfsm_evt_t evt);
vsf_err_t vsfsm_post_evt_pending(struct vsfsm_t *sm, vsfsm_evt_t evt);

#if VSFSM_CFG_SYNC_EN
// vsfsm_sync_t is generic sync object
struct vsfsm_sync_t
{
	uint32_t cur_value;
	vsfsm_evt_t evt;

	// private
	uint32_t max_value;
	struct vsfsm_t *sm_pending;
};
vsf_err_t vsfsm_sync_init(struct vsfsm_sync_t *sync, uint32_t cur_value,
				uint32_t max_value, vsfsm_evt_t evt);
vsf_err_t vsfsm_sync_cancel(struct vsfsm_sync_t *sync, struct vsfsm_t *sm);
vsf_err_t vsfsm_sync_increase(struct vsfsm_sync_t *sync);
vsf_err_t vsfsm_sync_decrease(struct vsfsm_sync_t *sync, struct vsfsm_t *sm);

// SEMAPHORE
#define vsfsm_sem_t					vsfsm_sync_t
#define vsfsm_sem_init(sem, cnt, evt)\
									vsfsm_sync_init((sem), (cnt), 0xFFFFFFFF, (evt))
#define vsfsm_sem_post(sem)			vsfsm_sync_increase((sem))
#define vsfsm_sem_pend(sem, sm)		vsfsm_sync_decrease((sem), (sm))

// CRITICAL
#define vsfsm_crit_t				vsfsm_sync_t
#define vsfsm_crit_init(crit, evt)	vsfsm_sync_init((crit), 1, 1, (evt))
#define vsfsm_crit_enter(crit, sm)	vsfsm_sync_decrease((crit), (sm))
#define vsfsm_crit_leave(crit)		vsfsm_sync_increase((crit))

#endif	// VSFSM_CFG_SYNC_EN

#endif	// #ifndef __VSFSM_H_INCLUDED__
