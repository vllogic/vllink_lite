
#ifndef __VSF_DYNPOOL_H_INCLUDED__
#define __VSF_DYNPOOL_H_INCLUDED__

struct vsf_dynpool_list_t
{
	struct vsfpool_t pool;
	uint32_t allocated_num;
#ifdef DYNPOOL_CFG_GC_EN
	uint32_t gc_tick;
#endif
	struct sllist list;
};

#ifdef DYNPOOL_CFG_GC_EN
// garbage_collect
struct vsf_dynpool_gc_t
{
	// public
	uint32_t poll_ms;

	// private
	struct vsfsm_t sm;
	struct vsftimer_t timer;
};
#endif

struct vsf_dynpool_t
{
	// public
	uint32_t item_size;
	uint16_t pool_size;
	uint16_t pool_num;
#ifdef DYNPOOL_CFG_GC_EN
	struct vsf_dynpool_gc_t gc;
#endif

	// private
	struct sllist head;
	uint16_t cur_pool_num;
};

vsf_err_t vsf_dynpool_init(struct vsf_dynpool_t *dynpool);
vsf_err_t vsf_dynpool_fini(struct vsf_dynpool_t *dynpool);
void *vsf_dynpool_alloc(struct vsf_dynpool_t *dynpool);
bool vsf_dynpool_free(struct vsf_dynpool_t *dynpool, void *buff);

#endif		// __VSF_DYNPOOL_H_INCLUDED__
